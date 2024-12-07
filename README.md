# TeclastF6ProTabletMode
Script to switch to tablet mode in linux based on KIONIX Accelerometers in base and display which does not work due to faulty mount matrix


# TODO
Angle between two vectors needs to be between 0 and 360, standard method is always below 180. This way one cannot destinguish between degrees above and below 180, like 90 and 270.

Tablet mode actiovation not working, but it was working in previous version
Ideas:
  - Change mount matrix so and 270° open laptop will be registered at 0°. This way we can switch to tablet mode between 0 and 90 degrees.

Hysteresis
