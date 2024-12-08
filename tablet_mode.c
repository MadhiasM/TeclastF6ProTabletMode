#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#include <linux/uinput.h>
#include <stdlib.h>


//#define ACCEL_SCALE 0.019163f  // Scale accelerometer values 0.019163 from "/sys/bus/iio/devices/iio\:device0/in_accel_scale"
#define SLEEP_TIME 1         // Time in seconds between checks
//#define TABLET_MODE_HYSTERESIS 5.0f  // Hysteresis in degrees from 180° position in which tablet mode is not changed
#define TABLET_MODE_HYSTERESIS 5.0f  // Hysteresis of the determinant that triggers enabling or disabling the tablet mode
//#define TABLET_MODE_COS_THRESHOLD -0.866f  // Threshold of cos below which tablet mode will be enabled. This is to unsere that in fully opened position, angle will not wrape from -180° to +180° due to noise and disable tablet mode
//#define PI  3.14159265358979323846

#

typedef struct {
    float matrix[3][3];
} MountMatrix;

// Reads accelerometer values from Sysfs
int read_accel_value(const char *path, const char *path_scale, float *value) {
    FILE *file_raw = fopen(path, "r");
    if (!file_raw) {
        perror("Failed to open accelerometer file");
        return -1;
    }
    int raw;
    if (fscanf(file_raw, "%d", &raw) != 1) {
        perror("Failed to read accelerometer value");
        fclose(file_raw);
        return -1;
    }

    FILE *file_scale = fopen(path_scale, "r");
    if (!file_scale) {
        perror("Failed to open accelerometer file");
        return -1;
    }
    float scale;
    if (fscanf(file_scale, "%f", &scale) != 1) {
        perror("Failed to read accelerometer value");
        fclose(file_scale);
        return -1;
    }

    fclose(file_raw);
    *value = (float)raw * scale;  // Scale based on device specs
    fclose(file_scale);
    return 0;
}

// Applies the mount matrix to raw accelerometer values
void apply_mount_matrix(const MountMatrix *matrix, const float raw[3], float corrected[3]) {
    for (int i = 0; i < 3; i++) {
        corrected[i] = matrix->matrix[i][0] * raw[0] +
                       matrix->matrix[i][1] * raw[1] +
                       matrix->matrix[i][2] * raw[2];
    }
}

/*
// Calculates the dot product of two vectors
float dot_product(const float vec1[3], const float vec2[3]) {
    return vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2];
}

// Calculates the magnitude of a vector
float magnitude(const float vec[3]) {
    return sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

float *cross_product(const float vec1[3], const float vec2[3]) {
    static float cross_product[3];

    cross_product[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1];
    cross_product[1] = vec1[2] * vec2[0] - vec1[0] * vec2[2];
    cross_product[2] = vec1[0] * vec2[1] - vec1[1] * vec2[0];

    return cross_product;
}

bool same_sign(float a, float b) {
    return a * b >= 0.0f;
}

// Calculates the cosine of the angle between two vectors
float cosine_of_angle(const float vec1[3], const float vec2[3]) {
    float dot = dot_product(vec1, vec2);

    float mag1 = magnitude(vec1);
    float mag2 = magnitude(vec2);
    printf("Dot Product: %f\n", dot); // TODO: REMOVE

    if (mag1 == 0 || mag2 == 0) {
        return 0.0f;  // Avoid division by zero
    }

    return dot / (mag1 * mag2);
}
*/

/*
// TODO: REMOVE IF NOT NEEDED
// Calculates the sine of the angle between two vectors
float sine_of_angle(const float vec1[3], const float vec2[3]) {
    float *normal_vec = cross_product(vec1, vec2);

    float mag_norm = magnitude(normal_vec);


    float mag1 = magnitude(vec1); // TODO: remove redundancy with cosine
    float mag2 = magnitude(vec2); // TODO: remove redundancy with cosine

    if (mag1 == 0 || mag2 == 0) {
        return 0.0f;  // Avoid division by zero
    }

    return mag_norm / (mag1 * mag2);
}
*/

// update tablet mode with hysteresis
// Alternative: use *stat and include as input variable

// variante 2
void update_mode(float val, float thresh, int *mod) {
     if (*mod == 1 && val < -thresh) {
         *mod = 0;
         printf("Tablet mode deactivated.\n"); // TODO: REMOVE
     } else if (*mod == 0 && val > thresh) {
         *mod = 1;
         printf("Tablet mode activated.\n"); // TODO: REMOVE
     }
 }


 /*
// variante 2
int update_mode(float val, float thresh) {
     static int stat = 0;
     if (stat == 1 && val < -thresh) {
         stat = 0;
     } else if (stat == 0 && val > thresh) {
         stat = 1;
     }
     return stat;
 }
 */

// Setup uinput device
int setup_uinput_device() {
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("Failed to open /dev/uinput");
        exit(EXIT_FAILURE);
    }

    // Enable switch events
    ioctl(fd, UI_SET_EVBIT, EV_SW);
    ioctl(fd, UI_SET_SWBIT, SW_TABLET_MODE);

    // Configure uinput device
    struct uinput_setup usetup = {
        .id.bustype = BUS_VIRTUAL,
        .id.vendor = 0x1234,
        .id.product = 0x5678,
        .name = "Custom Tablet Mode Switch"
    };
    ioctl(fd, UI_DEV_SETUP, &usetup);
    ioctl(fd, UI_DEV_CREATE);

    return fd;
}

// Emit SW_TABLET_MODE event
void emit_event(int fd, int value) {
    struct input_event ev = {
        .type = EV_SW,
        .code = SW_TABLET_MODE,
        .value = value
    };
    write(fd, &ev, sizeof(ev));

    // Synchronize event
    struct input_event syn = {
        .type = EV_SYN,
        .code = SYN_REPORT,
        .value = 0
    };
    write(fd, &syn, sizeof(syn));
}

// Triggers SW_TABLET_MODE via input_event
int set_tablet_mode(int mode) {
    const char *input_device = "/dev/input/event0";  // Adjust to your device
    int fd = open(input_device, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open input device");
        return -1;
    }

    struct input_event event = {0};
    event.type = EV_SW;
    event.code = SW_TABLET_MODE;
    event.value = mode;

    if (write(fd, &event, sizeof(event)) < 0) {
        perror("Failed to write input event");
        close(fd);
        return -1;
    }

    // Sync event
    memset(&event, 0, sizeof(event));
    event.type = EV_SYN;
    event.code = SYN_REPORT;
    event.value = 0;

    if (write(fd, &event, sizeof(event)) < 0) {
        perror("Failed to write sync event");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

int main() {
    // TODO: Retrieve from device config in 60-sensor.hwdb or udev rules instead of hardcoding
    // Mount matrices for base and display
    MountMatrix base_matrix = {{{0, 1, 0}, {1, 0, 0}, {0, 0, 1}}};
    MountMatrix display_matrix = {{{-1, 0, 0}, {0, -1, 0}, {0, 0, -1}}};

    int uinput_fd = setup_uinput_device();
    if (!uinput_fd) {
        printf("Could not create Uinput device.\n");
    }

    //printf("Uinput device created.\n");

    // Paths to accelerometer data in Sysfs
    const char *base_accel_paths[3] = {
        "/sys/bus/iio/devices/iio:device0/in_accel_x_raw",
        "/sys/bus/iio/devices/iio:device0/in_accel_y_raw",
        "/sys/bus/iio/devices/iio:device0/in_accel_z_raw"};
    const char *display_accel_paths[3] = {
        "/sys/bus/iio/devices/iio:device1/in_accel_x_raw",
        "/sys/bus/iio/devices/iio:device1/in_accel_y_raw",
        "/sys/bus/iio/devices/iio:device1/in_accel_z_raw"};

    const char *base_accel_scale_path = "/sys/bus/iio/devices/iio:device0/in_accel_scale";
    const char *display_accel_scale_path = "/sys/bus/iio/devices/iio:device1/in_accel_scale";

    int is_tablet_mode = 0;
    float raw_base[3], raw_display[3];
    float corrected_base[3], corrected_display[3];

    while (1) {

        // Read raw accelerometer values for display and base
        for (int i = 0; i < 3; i++) {
            if (read_accel_value(base_accel_paths[i], base_accel_scale_path, &raw_base[i]) < 0 ||
                read_accel_value(display_accel_paths[i], display_accel_scale_path, &raw_display[i]) < 0) {
                return 1;
            }
        }

        // TODO: Make more efficient by removing most of this from the loop. Maybe this can be done before
        // Apply mount matrices
        apply_mount_matrix(&base_matrix, raw_base, corrected_base);
        apply_mount_matrix(&display_matrix, raw_display, corrected_display);

        /*
        float *normal_vec = cross_product(corrected_base, corrected_display); // TODO: Refactor
        printf("Normal Vector: X=%.2f, Y=%.2f, Z=%.2f\n",
            normal_vec[0], normal_vec[1], normal_vec[1]);
        */

        /*
        Efficient way
        This is effectively the x-component of the normal vector (cross product) between base and display accelerometer vector
        The x-Component is the rotational axis of the hinge, which means that base and display x componentent will be pretty simular.
        More importantly, the sign of the x component of the normal vector compared to the sign of either x component will show if we are above or below 180° hinge angle
        */
        float determinant = corrected_base[1] * corrected_display[2] - corrected_base[2] * corrected_display[1];
        //printf("Determinant: %f\n", determinant);

        // TODO: REMOVE
        //float angle = atan2(corrected_display[2] * corrected_base[1] - corrected_display[1] * corrected_base[2], corrected_display[1] * corrected_base[2] - corrected_display[2] * corrected_base[1])*360/(2 * 3.1415926);
        //float angle = atan2(corrected_display[2], corrected_display[1]) - atan2(corrected_base[2], corrected_base[1]) * 360 / (2 * 3.1415926);
        //printf("ANGLE: %.2f\n", angle);


        /*
        // TODO: REMOVE
        // Calculate cosine of the angle between base and display accelerometer vectors
        float cos_angle = cosine_of_angle(corrected_base, corrected_display);

        int is_det_pos = determinant > 0;
        float angle = (is_det_pos? -1 : 1) * acos(cos_angle)*360/(2 * PI); // TODO: REMOVE
        printf("Cosine of angle: %.2f\n", cos_angle);
        printf("Angle: %.2f°\n", angle);
        */

        // Check if in tablet mode based on determinant sign
        //int is_tablet_mode = (determinant > 0);
        update_mode(determinant, TABLET_MODE_HYSTERESIS, &is_tablet_mode);
        // Trigger SW_TABLET_MODE
        if (set_tablet_mode(is_tablet_mode) < 0) {
            return 1;
        }

        emit_event(uinput_fd, is_tablet_mode);

        /*
        // Print accelerometer values
        printf("Base Accelerometer: X=%.2f, Y=%.2f, Z=%.2f\n",
               corrected_base[0], corrected_base[1], corrected_base[2]);
        printf("Display Accelerometer: X=%.2f, Y=%.2f, Z=%.2f\n",
               corrected_display[0], corrected_display[1], corrected_display[2]);
        */

        // Print tablet mode status
        // TODO: REMOVE
        //printf("Tablet mode: %s\n", is_tablet_mode ? "Enabled" : "Disabled");

        // TODO: Find alternative
        // Sleep for a while before re-checking
        sleep(SLEEP_TIME);
    }

    ioctl(uinput_fd, UI_DEV_DESTROY);
    close(uinput_fd);

    return 0;
}
