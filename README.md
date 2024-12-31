# TeclastF6ProTabletMode
Script to switch to tablet mode in linux based on KIONIX Accelerometers in base and display which does not work due identity matrix used as fallback despite correct matrix being defined in 60-sensor.hwdb.
Moreover, for KIONIX accelerometers in base and display, `SW_TABLET_MODE` is explicitly disabled as can be seen in [Linux Kernel dual_accel_detect.h](https://github.com/torvalds/linux/blob/7503345ac5f5e82fd9a36d6e6b447c016376403a/drivers/platform/x86/dual_accel_detect.h#L9)

Use tablet_mode.c for performance, tablet_mode_pitch_comp.c for accuracy. Further performance improvements in vector maths to be done.

# Installation
## Compilation
```
gcc -o tablet_mode tablet_mode.c
```

## Copy Service
```bash
sudo cp tablet_mode /usr/local/bin/tablet_mode
```

## Create Service
```bash
sudo nano /etc/systemd/system/tablet-mode.service
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

### Enable Service
```bash
sudo systemctl enable tablet-mode.service
```
### Start Service
```bash
sudo systemctl start tablet-mode.service
```
### Stop Service
(only if needed when you won't to suspend the service or replace/update it)
```bash
sudo systemctl stop tablet-mode.service
```
___
# TODO
## Installation
- [ ] Automate steps below using bash script
- [ ] Clean up code, remove commented out stuff
- [ ] Update README.md
- [ ] **Create makefile for compile, deploy, enable service etc**

## Functionality
### Linear algebra
- [ ] Custom OSDWindow Toast when enabling/disabing
- [ ] Provide different approaches
  - [x] Using determinant between y- & z-axis reading the full vectors
  - [x] Using angle between y- & z-axis with atan2 reading the full vectors
  - [x] Using reduced input by just reading needed values (x can be neglected)


## Service: udriver events
- [ ] **Return screen orientation to normal when leaving tablet mode (will keep last orientation currently) (State machine: on transition from enable to disable: return to normal orientation. Don't trigger this if already disabled (from disabled to disabled)**
- [ ] Fix mouse or keyboard  sometimes  not being reactivated. (sporadic)
- [ ] Increase efficiency (write/read/cpu cycles)?
- [ ] Provide different approaches for execution events
  - [x] Polling using while-loop with nanosleep
  - [ ] Event-based using inotify for event-driven architecture instead of while loop with nanosleep
- [ ] Catch error if return -1
- [ ] log errors

## Performance
## Robustness
- [ ] Increase robustness in diagonal or 90° sideways situations or fully folded. Here the accuracy is very low
### Hysteresis
- [ ] Adjust hysteresis threshold based on X-acceleration? (if X-acceleration is high, hystresis value will be small, thus appropriate threshold might be better)
- [ ] Time hysteresis (if enable conditions are met: test again $4$ times)
- [ ] Low Pass Filter (if needed) on accel values
- [ ] PID?
- [ ] Improve robustness if laptop is rotated 90° to the left or right (determinant will be close to 0 and thus can fluke to above 0). hysteresis is added as workaround, but will prevent tablet mode when rotatet 90° sideways. In this case, maybe take vectors into account in more detail?
  - For example: if determinant close to 0, then check if y and z acceleration of base and display are similar?? (but that is the same for 0 and 360 degrees)? not sure...
  - Hysteresis might solve this already?
  - Prevent wrapping from -180 to 180
  If Y and Z acceleration are close to 0, then it is indistinguishable from a physical viewpoint due to noisy signals
- [ ] Some sort of fallback / safe state to trigger disable tablet mode if something weird is detected? To be able to get control of keyboard again
- [ ] Break from while loop if error occurs to stop service (then close all devices)
- [ ] Improve behaviour when laptop wakes up from sleep. If state changes while in sleep, it is not registered
- [ ] Add both hysteresis and roll angle threshold as `IFDEF` makros to be able to quickly comment out this check

## Modularity
- [ ] // TODO: Retrieve from device config in 60-sensor.hwdb or udev rules instead of hardcoding
- [ ] Use [libudev](https://www.freedesktop.org/software/systemd/man/latest/libudev.html) with [Overview](https://www.freedesktop.org/software/systemd/man/latest/) or [udev_device_get_sysattr_value](https://www.freedesktop.org/software/systemd/man/latest/udev_device_get_sysattr_value.html#)
- [ ] `get_mount_matrix_udev.c` work in progress, but it does not show the same mount matrix as `udevadm info -n  /dev/iio:device0` (`ACCEL_MOUNT_MATRIX`). Showing `in_accel_mount_matrix` (identity), as in `sys/bus/iio/devices/iio\:device*/`
