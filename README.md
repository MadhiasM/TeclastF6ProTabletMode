# TeclastF6ProTabletMode
Script to switch to tablet mode in linux based on KIONIX Accelerometers in base and display which does not work due identity matrix used as fallback despite correct matrix being defined in 60-sensor.hwdb.
Moreover, for KIONIX accelerometers in base and display, `SW_TABLET_MODE` is explicitly disabled as can be seen in [Linux Kernel dual_accel_detect.h](https://github.com/torvalds/linux/blob/7503345ac5f5e82fd9a36d6e6b447c016376403a/drivers/platform/x86/dual_accel_detect.h#L9)

# TODO
## Driver
- [ ] Transform into user space driver to be loaded at startup (if performance is acceptable)

## Functionality
### Linear algebra
- [x] Angle between two vectors needs to be between-180 and +180°, standard method is always between 0 and 180°. This way one cannot destinguish between degrees above and below 180, like 90 and 270.
- [x] [Maths](https://math.stackexchange.com/questions/1904152/how-to-find-an-angle-in-range-180-180-between-2-vectors)

## udriver events
- [x] Tablet mode activation SW_TABLET_MODE not working, but it was working in previous version. Copy over state
- [x] Return screen orientation to normal when leaving tablet mode (will keep last orientation currently)


## Performance
### X-Axis
- [ ] X-axis can be disregarded since it is same between base and display, only read Y and Z data. Only read in Y and Z data
- [ ] Potentially skip mount matrix by directly adjusting formula for calculating  (performance better, but it would be hardcoded, harder to understand)

## Robustness
- [ ] Angle Activation Hysteresis (enable at $-\alpha°$, disable at $+\alpha°$)
- [ ] Time hysteresis (if enable conditions are met: test again $4$ times)
- [ ] Sleep (if enable conditions are not met: sleep for $n$ secs)
- [ ] Low Pass Filter (if needed) on accel values
- [ ] PID?
- [ ] Improve robustness if laptop is rotated 90° to the left or right (determinant will be close to 0 and thus can fluke to above 0). hysteresis is added as workaround, but will prevent tablet mode when rotatet 90° sideways. In this case, maybe take vectors into account in more detail?
  - For example: if determinant close to 0, then check if y and z acceleration of base and display are similar?? (but that is the same for 0 and 360 degrees)? not sure...
  - Hysteresis might solve this already?
  If Y and Z acceleration are close to 0, then it is indistinguishable from a physical viewpoint due to noisy signals


## Modularity
- [ ] // TODO: Retrieve from device config in 60-sensor.hwdb or udev rules instead of hardcoding
- [ ] Use [libudev](https://www.freedesktop.org/software/systemd/man/latest/libudev.html) with [Overview](https://www.freedesktop.org/software/systemd/man/latest/) or [udev_device_get_sysattr_value](https://www.freedesktop.org/software/systemd/man/latest/udev_device_get_sysattr_value.html#)
- [ ] `get_mount_matrix_udev.c` work in progress, but it does not show the same mount matrix as `udevadm info -n  /dev/iio:device0` (`ACCEL_MOUNT_MATRIX`). Showing `in_accel_mount_matrix` (identity), as in `sys/bus/iio/devices/iio\:device*/`
