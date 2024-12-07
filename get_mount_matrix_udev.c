#include <stdio.h>
#include <libudev.h>

int main() {
    struct udev *udev;
    struct udev_device *dev;
    const char *device_path = "/dev/iio:device0";

    const char *mount_matrix;

    // Udev-Objekt initialisieren
    udev = udev_new();
    if (!udev) {
        fprintf(stderr, "Fehler beim Erstellen des udev-Objekts\n");
        return 1;
    }

    dev = udev_device_new_from_syspath(udev, "/sys/devices/pci0000:00/0000:00:15.0/i2c_designware.0/i2c-1/i2c-KIOX020A:00/iio:device0");
    if (!dev) {
        fprintf(stderr, "Fehler beim Finden des Geräts\n");
        udev_unref(udev);
        return 1;
    }


    // Enumerate-Objekt erstellen
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    enumerate = udev_enumerate_new(udev);
    if (!enumerate) {
        fprintf(stderr, "Fehler beim Erstellen des Enumerate-Objekts\n");
        udev_unref(udev);
        return 1;
    }

    // Alle Geräte im System durchlaufen
    udev_enumerate_scan_devices(enumerate);

    // Liste der gefundenen Geräte abrufen
    devices = udev_enumerate_get_list_entry(enumerate);
    if (!devices) {
        printf("Keine Geräte gefunden\n");
    } else {
        // Alle Geräte durchlaufen und ausgeben
        udev_list_entry_foreach(dev_list_entry, devices) {
            const char *path = udev_list_entry_get_name(dev_list_entry);
            dev = udev_device_new_from_syspath(udev, path);
            if (dev) {
                // Geräteinformationen anzeigen
                printf("Gerät: %s\n", path);
                printf("  - Name: %s\n", udev_device_get_sysname(dev));
                printf("  - Pfad: %s\n", udev_device_get_devpath(dev));
                printf("  - Subsystem: %s\n", udev_device_get_subsystem(dev));
                printf("  - Treiber: %s\n", udev_device_get_driver(dev));
                printf("\n");
                udev_device_unref(dev);
            }
        }
    }

    /*
    struct udev_list_entry *entry;
    entry = udev_device_get_sysattr_list_entry(dev);
    if (!entry) {
        fprintf(stderr, "Keine Systemattribute für das Gerät gefunden\n");
        udev_device_unref(dev);
        udev_unref(udev);
        return 1;
    }
    printf("Alle Systemattribute für Gerät %s:\n", device_path);
    while (entry) {
        const char *attr_name = udev_list_entry_get_name(entry);
        const char *attr_value = udev_device_get_sysattr_value(dev, attr_name);

        // Attributname und Wert ausgeben
        if (attr_value) {
            printf("%s = %s\n", attr_name, attr_value);
        } else {
            printf("%s = (kein Wert gefunden)\n", attr_name);
        }

        // Nächsten Listeneintrag abrufen
        entry = udev_list_entry_get_next(entry);
    }
    */


    // Mount-Matrix (ACCEL_MOUNT_MATRIX) aus den Geräteeigenschaften abrufen
    mount_matrix = udev_device_get_sysattr_value(dev, "ACCEL_MOUNT_MATRIX");
    if (mount_matrix) {
        printf("Mount-Matrix des Geräts %s: %s\n", device_path, mount_matrix);
    } else {
        printf("Mount-Matrix nicht gefunden\n");
    }

    // Ressourcen freigeben
    udev_device_unref(dev);
    udev_unref(udev);

    return 0;
}
