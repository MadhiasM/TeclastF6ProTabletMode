# TeclastF6ProTabletMode
Script to switch to tablet mode in linux based on KIONIX Accelerometers in base and display which does not work due identity matrix used as fallback despite correct matrix being defined in 60-sensor.hwdb.
Moreover, for KIONIX accelerometers in base and display, `SW_TABLET_MODE` is explicitly disabled as can be seen in [Linux Kernel dual_accel_detect.h](https://github.com/torvalds/linux/blob/7503345ac5f5e82fd9a36d6e6b447c016376403a/drivers/platform/x86/dual_accel_detect.h#L9)

# TODO
## Driver
- [ ] **Transform into user space driver to be loaded at startup (if performance is acceptable)**

## Functionality
### Linear algebra
- [x] Angle between two vectors needs to be between-180 and +180°, standard method is always between 0 and 180°. This way one cannot destinguish between degrees above and below 180, like 90 and 270.
- [x] [Maths](https://math.stackexchange.com/questions/1904152/how-to-find-an-angle-in-range-180-180-between-2-vectors)
- [ ] ~~Calculate angle only between y and z, since x is aligned. Moreover angle is polluted by x if is is big in magnitude (rotated sideway 90°)~~ (DOES NOT WORK)
- [ ] Understand why angle calculation is off if rotated diagonally (Due to aligned X-axis between base and display?)
- [ ] Test if same issue is in windows

## udriver events
- [x] Tablet mode activation SW_TABLET_MODE not working, but it was working in previous version. Copy over state
- [ ] **Return screen orientation to normal when leaving tablet mode (will keep last orientation currently) (State machine: on transition from enable to disable: return to normal orientation. Don't trigger this if already disabled (from disabled to disabled)**
- [ ] Fix that sometimes mouse or keyboard is not reactivated. (sporadic)
- [ ] Increase efficiency (write/read/cpu cycles)?
- [ ] **Event/Interrupt based instead of polling (Sleep, then check)?**

## Performance
### X-Axis
- [x] Move vector declaration out of loop
- [x] Move is_tablet_mode declaration out of loop
- [x] Compine read accel values into one loop
- [ ] **Potentially skip mount matrix by directly adjusting formula for calculating  (performance better, but it would be hardcoded, harder to understand)**
- [ ] **X-axis can be disregarded since it is same between base and display, only read Y and Z data. Only read in Y and Z data**
- Updated functions:
```c
// Calculates the dot product of two vectors
float dot_product(const float vec1[3], const float vec2[3]) {
    return vec1[1] * vec2[1] + vec1[2] * vec2[2];
}

// Calculates the magnitude of a vector
float magnitude(const float vec[3]) {
    return sqrt(vec[1] * vec[1] + vec[2] * vec[2]);
}
```
### Cosine
- [ ] Use Cosine instead of angle for hysteresis , since it is faster to compute. For now angle is fine since it is easier to understand and debug
### Mount Matrix
- [ ] **Remove apply_mount_matrix from while loop if possible. Hardcoding this should be easy by swapping the indices when reading values according to mount matrix.**
## Robustness
- [x] Angle Activation Hysteresis (enable at $-\alpha°$, disable at $+\alpha°$) (Done with determinant instead)
- [x] Enable at values close to 180
- [ ] Increase robustness in diagonal or 90° sideways situations or fully folded. Here the accuracy is very low
- [x] Keep state of previous activation status
- [ ] Time hysteresis (if enable conditions are met: test again $4$ times)
- [ ] Sleep (if enable conditions are not met: sleep for $n$ secs)
- [ ] Low Pass Filter (if needed) on accel values
- [ ] PID?
- [ ] Improve robustness if laptop is rotated 90° to the left or right (determinant will be close to 0 and thus can fluke to above 0). hysteresis is added as workaround, but will prevent tablet mode when rotatet 90° sideways. In this case, maybe take vectors into account in more detail?
  - For example: if determinant close to 0, then check if y and z acceleration of base and display are similar?? (but that is the same for 0 and 360 degrees)? not sure...
  - Hysteresis might solve this already?
  - Prevent wrapping from -180 to 180
  If Y and Z acceleration are close to 0, then it is indistinguishable from a physical viewpoint due to noisy signals
- [ ] Some sort of fallback / safe state to trigger disable tablet mode if something weird is detected? To be able to get control of keyboard again

## Modularity
- [ ] // TODO: Retrieve from device config in 60-sensor.hwdb or udev rules instead of hardcoding
- [ ] Use [libudev](https://www.freedesktop.org/software/systemd/man/latest/libudev.html) with [Overview](https://www.freedesktop.org/software/systemd/man/latest/) or [udev_device_get_sysattr_value](https://www.freedesktop.org/software/systemd/man/latest/udev_device_get_sysattr_value.html#)
- [ ] `get_mount_matrix_udev.c` work in progress, but it does not show the same mount matrix as `udevadm info -n  /dev/iio:device0` (`ACCEL_MOUNT_MATRIX`). Showing `in_accel_mount_matrix` (identity), as in `sys/bus/iio/devices/iio\:device*/`
