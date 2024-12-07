# TeclastF6ProTabletMode
Script to switch to tablet mode in linux based on KIONIX Accelerometers in base and display which does not work due to faulty mount matrix


# TODO
## Angle between -180 and +180°
Angle between two vectors needs to be between 0° and 360° (or ideally +-180°), standard method is always below 180°. This way one cannot destinguish between degrees above and below 180, like 90 and 270.

## Performance
### X-Axis
X-axis can be disregarded since it is same between base and display
Only read Y and X data




## Robustness
Angle Hysteresis
Time hysteresis
  if tabled mode disabled:
    sleep
  else:
    check again before activating (time hysteresis)

Low Pass Filter (if needed) on accel values

## Tablet mode activation
Tablet mode activation not working, but it was working in previous version. Copy over state

## Modularity
// TODO: Retrieve from device config in 60-sensor.hwdb or udev rules instead of hardcoding
https://www.freedesktop.org/software/systemd/man/latest/libudev.html
get_mount_matrix_udev.c work in progress, but it does not show the same mount matrix as udevadm info -n  /dev/iio:device0
