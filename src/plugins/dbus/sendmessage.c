#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dbus/dbus.h>

int elektraDbusSendMessage (DBusBusType type)
{
	DBusConnection *connection;
	DBusError error;
	DBusMessage *message;
	const char *dest = 0;
	const char *name = "org.libelektra";
	const char *path = "/org/libelektra/configuration";

	dbus_error_init (&error);
	connection = dbus_bus_get (type, &error);
	if (connection == NULL)
	{
		fprintf (stderr, "Failed to open connection to %s message bus: %s\n",
			(type == DBUS_BUS_SYSTEM) ? "system" : "session",
			error.message);
		dbus_error_free (&error);
		return -1;
	}

	message = dbus_message_new_signal (path, name, "changed");

	if (message == NULL)
	{
		fprintf (stderr, "Couldn't allocate D-Bus message\n");
		return -1;
	}

	if (dest && !dbus_message_set_destination (message, dest))
	{
		fprintf (stderr, "Not enough memory\n");
		return -1;
	}

	dbus_connection_send (connection, message, NULL);
	dbus_connection_flush (connection);

	dbus_message_unref (message);

	dbus_connection_unref (connection);

	return 1;
}
