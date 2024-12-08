#include <stdio.h>
#include <stdlib.h>
#include <dbus/dbus.h>

void rotate_screen(const char *rotation) {
    DBusConnection *conn;
    DBusError err;
    DBusMessage *msg;
    DBusMessage *reply;

    // D-Bus-Verbindung aufbauen
    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Connection failed: %s\n", err.message);
        dbus_error_free(&err);
        return;
    }

    // Nachricht erstellen, um die Bildschirmrotation zu ändern
    msg = dbus_message_new_method_call(
        "org.gnome.Shell",                  // GNOME Shell als D-Bus-Service
        "/org/gnome/Shell/Screen",          // Objektpfad
        "org.gnome.Shell.Screen",           // D-Bus-Interface
        "SetRotation"                       // Methode
    );

    if (msg == NULL) {
        fprintf(stderr, "Message creation failed\n");
        dbus_connection_unref(conn);
        return;
    }

    // Die Methode erwartet einen Parameter (Rotation)
    dbus_message_append_args(msg, DBUS_TYPE_STRING, &rotation, DBUS_TYPE_INVALID);

    // Nachricht senden und auf Antwort warten
    reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Error during D-Bus call: %s\n", err.message);
        dbus_error_free(&err);
    } else {
        printf("Screen rotation changed successfully.\n");
    }

    // Ressourcen freigeben
    dbus_message_unref(msg);
    dbus_message_unref(reply);
    dbus_connection_unref(conn);
}

int main() {
    // Beispiel für die Bildschirmrotation (z. B. "left", "right", "inverted", "normal")
    rotate_screen("left");  // Drehung um 90 Grad gegen den Uhrzeigersinn

    return 0;
}
