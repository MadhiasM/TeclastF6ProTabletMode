#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#include <linux/input.h>
#include <linux/uinput.h>


#define SLEEP_TIME 1         // Time in seconds between checks
#define TABLET_MODE_HYSTERESIS 0.3f  // Hysteresis of the determinant that triggers enabling or disabling the tablet mode
#define TABLET_MODE_ROLL_THRESHOLD 8 // Threshold of the acceleration in x-direction (hinge axis), which will prevent tablet mode change if exceeded. This is done since it is no longer physically possible to determine the hinge angle in this situation since the two acceleration vector of base and display will be nearly identical.
#define PATH "/sys/bus/iio/devices/"
#define BASE_DEVICE "iio:device0/"
#define DISPLAY_DEVICE "iio:device1/"
#define ACCEL_X "in_accel_x_raw"
#define ACCEL_Y "in_accel_y_raw"
#define ACCEL_Z "in_accel_z_raw"
#define ACCEL_SCALE "in_accel_scale"

#define PI 3.141592654

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

// update tablet mode with hysteresis
// variante 1
void update_mode(float val, float thresh, int dev, int *mod) {
     if (*mod == 1 && val < -thresh) {
         *mod = 0;
         printf("Tablet mode deactivated.\n"); // TODO: REMOVE
         emit_event(dev, *mod);

     } else if (*mod == 0 && val > thresh) {
         *mod = 1;
         printf("Tablet mode activated.\n"); // TODO: REMOVE
         emit_event(dev, *mod);

     }
 }

int main() {
    struct timespec ts = { .tv_sec = SLEEP_TIME, .tv_nsec = 0 }; // 1 Sekunde


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
        PATH BASE_DEVICE ACCEL_X,
        PATH BASE_DEVICE ACCEL_Y,
        PATH BASE_DEVICE ACCEL_Z};
    const char *display_accel_paths[3] = {
        PATH DISPLAY_DEVICE ACCEL_X,
        PATH DISPLAY_DEVICE ACCEL_Y,
        PATH DISPLAY_DEVICE ACCEL_Z};


    const char *base_accel_scale_path = PATH BASE_DEVICE ACCEL_SCALE;
    const char *display_accel_scale_path = PATH DISPLAY_DEVICE ACCEL_SCALE;

    int is_tablet_mode = 0;
    float raw_base[3], raw_display[3];
    float corrected_base[3], corrected_display[3];


    // TODO:
    // READ MOUNT MATRIX HERE
    // THEN BASED ON READINGS: READ 2 DIMENSIONS NEEDED (in our case: Y and Z of both (after correction))
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
        This is effectively the x-component of the normal vector (cross product) between base and display accelerometer vector
        The x-Component is the rotational axis of the hinge, which means that base and display x componentent will be pretty simular.
        More importantly, the sign of the x component of the normal vector compared to the sign of either x component will show if we are above or below 180° hinge angle
        */
        //float determinant = corrected_base[1] * corrected_display[2] - corrected_base[2] * corrected_display[1];

        //float pitch_angle_base = atan2(-corrected_base[1], -corrected_base[2]);  // TODO: Vorzeichen?
        //float pitch_angle_display = atan2(-corrected_display[1], -corrected_display[2]);  // TODO: Vorzeichen?
        float hinge_angle = atan2(-corrected_base[1], -corrected_base[2]) - atan2(-corrected_display[1], -corrected_display[2]);

        //printf("Base:    Pitch: %.2f\n", pitch_angle_base * 360 / (2 * PI));
        //printf("Display: Pitch: %.2f\n", pitch_angle_display * 360 / (2 * PI));
        printf("Hinge:   Pitch: %.2f\n", hinge_angle * 360 / (2 * PI));


        // TODO: ADD AS IFDEF, so the check for roll threshold and hinge angle hysteresis can be quickly commented out
        // Check if in tablet mode based on determinant sign with hysteresis. emit update on mode change
        if (corrected_base[0] < TABLET_MODE_ROLL_THRESHOLD) {
            update_mode(hinge_angle, TABLET_MODE_HYSTERESIS, uinput_fd, &is_tablet_mode);
        }

        // Sleep for a while before re-checking
        nanosleep(&ts, NULL);
        // TODO: Catch error if return -1
    }

    ioctl(uinput_fd, UI_DEV_DESTROY);
    close(uinput_fd);

    return 0;
}
