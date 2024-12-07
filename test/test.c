#include <linux/uinput.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main() {
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("Failed to open uinput");
        return 1;
    }

    // Enable SW_TABLET_MODE events
    ioctl(fd, UI_SET_EVBIT, EV_SW);
    ioctl(fd, UI_SET_SWBIT, SW_TABLET_MODE);

    // Create virtual device
    struct uinput_setup usetup;
    memset(&usetup, 0, sizeof(usetup));
    snprintf(usetup.name, UINPUT_MAX_NAME_SIZE, "tablet-mode-simulator");
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1;
    usetup.id.product = 0x1;
    usetup.id.version = 1;

    ioctl(fd, UI_DEV_SETUP, &usetup);
    ioctl(fd, UI_DEV_CREATE);

    sleep(1);

    // Simulate SW_TABLET_MODE = 1
    struct input_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_SW;
    ev.code = SW_TABLET_MODE;
    ev.value = 1; // Tablet mode
    write(fd, &ev, sizeof(ev));

    // Synchronize
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(fd, &ev, sizeof(ev));

    printf("Tablet mode simulated. Press Enter to exit.\n");
    getchar();

    // Destroy virtual device
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
    return 0;
}
