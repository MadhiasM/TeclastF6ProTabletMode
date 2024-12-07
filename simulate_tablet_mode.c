#include <stdio.h>
//#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <math.h>
#include <string.h>

//#define ACCEL_SCALE 0.019163f  // Scale accelerometer values 0.019163 from "/sys/bus/iio/devices/iio\:device0/in_accel_scale"
#define SLEEP_TIME 1         // Time in seconds between checks
#define TABLET_MODE_THRESHOLD -0.5f  // Cosine of 120° (approx. 180° rotation)
#define PI  3.14159265358979323846

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

// Calculates the dot product of two vectors
float dot_product(const float vec1[3], const float vec2[3]) {
    return vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2];
}

// Calculates the magnitude of a vector
float magnitude(const float vec[3]) {
    return sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

const float *cross_product(const float vec1[3], const float vec2[3]) {
    static float cross_product[3];

    cross_product[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1];
    cross_product[1] = vec1[2] * vec2[0] - vec1[0] * vec2[2];
    cross_product[2] = vec1[0] * vec2[1] - vec1[1] * vec2[0];

    return cross_product;
}



// Calculates the cosine of the angle between two vectors
float cosine_of_angle(const float vec1[3], const float vec2[3]) {
    float dot = dot_product(vec1, vec2);

    printf("Dot Product: %f\n", dot); // TODO: Remove
    const float *normal_vec = cross_product(vec1, vec2); // TODO: Refactor
    printf("Normal Vector: ");
    printf("X: %f, ", normal_vec[0]);
    printf("Y: %f, ", normal_vec[1]);
    printf("Z: %f, ", normal_vec[2]);

    float mag1 = magnitude(vec1);
    float mag2 = magnitude(vec2);
    if (mag1 == 0 || mag2 == 0) {
        return 0.0f;  // Avoid division by zero
    }

    float ang = acos(dot / (mag1 * mag2))*360/(2 * PI); // TODO: Remove
    printf("Angle: %f\n", ang); // TODO: REMOVE

    return dot / (mag1 * mag2);
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
    // Mount matrices for base and display
    MountMatrix base_matrix = {{{0, 1, 0}, {1, 0, 0}, {0, 0, 1}}};
    MountMatrix display_matrix = {{{-1, 0, 0}, {0, -1, 0}, {0, 0, -1}}};

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

    while (1) {
        float raw_base[3], raw_display[3];
        float corrected_base[3], corrected_display[3];



        // Read raw accelerometer values for base
        for (int i = 0; i < 3; i++) {
            if (read_accel_value(base_accel_paths[i], base_accel_scale_path, &raw_base[i]) < 0) {
                return 1;
            }
        }

        // Read raw accelerometer values for display
        for (int i = 0; i < 3; i++) {
            if (read_accel_value(display_accel_paths[i], display_accel_scale_path, &raw_display[i]) < 0) {
                return 1;
            }
        }

        // Apply mount matrices
        apply_mount_matrix(&base_matrix, raw_base, corrected_base);
        apply_mount_matrix(&display_matrix, raw_display, corrected_display);

        // Calculate cosine of the angle between base and display accelerometer vectors
        float cos_angle = cosine_of_angle(corrected_base, corrected_display);

        // Check if in tablet mode based on cosine threshold
        int tablet_mode = (cos_angle < TABLET_MODE_THRESHOLD);

        // Trigger SW_TABLET_MODE
        if (set_tablet_mode(tablet_mode) < 0) {
            return 1;
        }

        // Print accelerometer values and tablet mode status
        printf("Base Accelerometer: X=%.2f, Y=%.2f, Z=%.2f\n",
               corrected_base[0], corrected_base[1], corrected_base[2]);
        printf("Display Accelerometer: X=%.2f, Y=%.2f, Z=%.2f\n",
               corrected_display[0], corrected_display[1], corrected_display[2]);
        printf("Cosine of angle: %.2f\n", cos_angle);
        printf("Tablet mode: %s\n", tablet_mode ? "Enabled" : "Disabled");

        // Sleep for a while before re-checking
        sleep(SLEEP_TIME);
    }

    return 0;
}
