# TeclastF6ProTabletMode
Script to switch to tablet mode in linux based on KIONIX Accelerometers in base and display which does not work due identity matrix used as fallback despite correct matrix being defined in 60-sensor.hwdb.
Moreover, for KIONIX accelerometers in base and display, `SW_TABLET_MODE` is explicitly disabled as can be seen in [Linux Kernel dual_accel_detect.h](https://github.com/torvalds/linux/blob/7503345ac5f5e82fd9a36d6e6b447c016376403a/drivers/platform/x86/dual_accel_detect.h#L9)

# TODO
## Installation
- [ ] Automate steps below using bash script
- [ ] Clean up code, remove commented out stuff
- [ ] Update README.md
- [ ] **Create makefile for compile, deploy, enable service etc**
## Driver
- [x] ~~Transform into user space driver to be loaded at startup (once performance is acceptable)~~

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
- [ ] Fix mouse or keyboard  sometimes  not being reactivated. (sporadic)
- [x] ~~Emit event only on change (integrated into update mode)~~
- [ ] Increase efficiency (write/read/cpu cycles)?
- [x] ~~Event/Interrupt based instead of polling (Sleep, then check)?~~ Using Nanosleep
  - `usleep` (deprecated), `nanosleep` (POSIX), `thrd_sleep` (C11), `usleep_range_idle` (no userspace), `hrtimer` (precision, callback, more)

## Performance
### Reduce loop load
- [x] ~~Move vector declaration out of loop~~
- [x] ~~Move is_tablet_mode declaration out of loop~~
- [x] ~~Compine read accel values into one loop~~
### Simplification
- [ ] **Potentially skip mount matrix by directly adjusting formula for calculating  (performance better, but it would be hardcoded, less modular)**
- [ ] **X-axis can be disregarded since it is same between base and display, only read Y and Z data**

### Cosine
- [x] ~~Use Cosine instead of angle for hysteresis , since it is faster to compute. For now angle is fine since it is easier to understand and debug~~ Using determinant of Y-Z instead
### Mount Matrix
- [ ] **Remove apply_mount_matrix from while loop if possible. Hardcoding this should be easy by swapping the indices when reading values according to mount matrix.**
## Robustness
- [x] ~~Angle Activation Hysteresis (enable at $-\alpha°$, disable at $+\alpha°$) (Done with determinant instead)~~
- [x] ~~Enable at values close to 180~~
- [ ] Increase robustness in diagonal or 90° sideways situations or fully folded. Here the accuracy is very low
- [x] ~~Keep state of previous activation status~~
- [ ] Time hysteresis (if enable conditions are met: test again $4$ times)
- [x] ~~Sleep (if enable conditions are not met: sleep for $n$ secs)~~
- [ ] TODO: Catch error if return -1
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

# Installation
## Location
```bash
sudo cp tablet_mode /usr/local/bin/tablet_mode
```

## Create service
```bash
sudo nano /etc/systemd/system/tabler-mode.service
```

Paste:
```ini
[Unit]
Description=Custom Tablet Mode Switch
After=multi-user.target

[Service]
ExecStart=/usr/local/bin/tablet_mode
Restart=always
User=root

[Install]
WantedBy=multi-user.target
```

### Enable
```bash
sudo systemctl enable tablet-mode.service
```
### Start
```bash
sudo systemctl start tablet-mode.service
```
### Stop
```bash
sudo systemctl stop tablet-mode.service
```
(only if service shall not be used)
