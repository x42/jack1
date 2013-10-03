/*
    Copyright (C) 2007 Paul Davis
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <jack/jack.h>
#include <jack/metadata.h>
#include <jack/uuid.h>

void
port_callback (jack_port_id_t port, int yn, void* arg)
{
	printf ("Port %d %s\n", port, (yn ? "registered" : "unregistered"));
}

void
connect_callback (jack_port_id_t a, jack_port_id_t b, int yn, void* arg)
{
	printf ("Ports %d and %d %s\n", a, b, (yn ? "connected" : "disconnected"));
}

void
client_callback (const char* client, int yn, void* arg)
{
	printf ("Client %s %s\n", client, (yn ? "registered" : "unregistered"));
}

int
graph_callback (void* arg)
{
	printf ("Graph reordered\n");
	return 0;
}

void
propchange (jack_uuid_t subject, const char* key, jack_property_change_t change)
{
        char buf[JACK_UUID_STRING_SIZE];
        const char* action = "";

        switch (change) {
        case PropertyCreated:
                action = "created";
                break;

        case PropertyChanged:
                action = "changed";
                break;

        case PropertyDeleted:
                action = "deleted";
                break;
        }

        if (jack_uuid_empty (subject)) {
                printf ("All properties changed!\n");
        } else {
                jack_uuid_unparse (subject, buf);
                
                if (key) {
                        printf ("key [%s] for %s %s\n", key, buf, action);
                } else {
                        printf ("all keys for %s %s\n", buf, action);
                }
        }
}

int
main (int argc, char *argv[])
{
	jack_client_t *client;
	jack_options_t options = JackNullOption;
	jack_status_t status;

	if ((client = jack_client_open ("event-monitor", options, &status, NULL)) == 0) {
		fprintf (stderr, "jack_client_open() failed, "
			 "status = 0x%2.0x\n", status);
		if (status & JackServerFailed) {
			fprintf (stderr, "Unable to connect to JACK server\n");
		}
		return 1;
	}
	
	if (jack_set_port_registration_callback (client, port_callback, NULL)) {
		fprintf (stderr, "cannot set port registration callback\n");
		return 1;
	}
	if (jack_set_port_connect_callback (client, connect_callback, NULL)) {
		fprintf (stderr, "cannot set port connect callback\n");
		return 1;
	}
	if (jack_set_client_registration_callback (client, client_callback, NULL)) {
		fprintf (stderr, "cannot set client registration callback\n");
		return 1;
	}
	if (jack_set_graph_order_callback (client, graph_callback, NULL)) {
		fprintf (stderr, "cannot set graph order registration callback\n");
		return 1;
	}
        if (jack_set_property_change_callback (client, (JackPropertyChangeCallback) propchange, 0)) {
                fprintf (stderr, "cannot set property change callback\n");
                return 1;
        }
	if (jack_activate (client)) {
		fprintf (stderr, "cannot activate client");
		return 1;
	}

	sleep (-1);
	exit (0);
}
