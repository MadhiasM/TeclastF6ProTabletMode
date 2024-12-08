#include <stdio.h>
#include <stdlib.h>
#include <wayland-client.h>
#include <wlroots.h>

// Diese Funktion initialisiert die Wayland-Verbindung
struct wl_display *display;
struct wl_registry *registry;

static void registry_handler(void *data, struct wl_registry *registry, uint32_t id, const char *interface, uint32_t version) {
    if (strcmp(interface, "wl_output") == 0) {
        printf("Monitore gefunden!\n");
        // Hier würdest du mit dem Monitor und der Rotation weiterarbeiten
    }
}

int main() {
    display = wl_display_connect(NULL); // Verbinde mit dem Wayland-Server
    if (display == NULL) {
        fprintf(stderr, "Fehler beim Verbinden zum Wayland-Server.\n");
        return -1;
    }

    registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &(struct wl_registry_listener){
        .global = registry_handler,
    }, NULL);

    // Event-Schleife starten
    wl_display_dispatch(display);
    wl_display_roundtrip(display);

    // Hier könntest du die Transformation des Monitors setzen (über `wlroots` API)
    // Das würde ein tiefgreifendes Beispiel benötigen, das sich direkt mit dem Compositor verbindet

    wl_display_disconnect(display); // Verbindung trennen
    return 0;
}
