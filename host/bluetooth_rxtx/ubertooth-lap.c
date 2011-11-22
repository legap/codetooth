/*
 * Copyright 2010, 2011 Michael Ossmann
 *
 * This file is part of Project Ubertooth.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "ubertooth.h"
#include <bluetooth_packet.h>
#include <getopt.h>

static void usage(void)
{
	printf("ubertooth-lap - passive Bluetooth monitoring with LAP detection\n");
	printf("Usage:\n");
	printf("\t-i filename\n");
	printf("\nIf an input file is not specified, an Ubertooth device is used for live capture.\n");
}

int main(int argc, char *argv[])
{
	int opt;
	struct libusb_device_handle *devh = NULL;
	FILE* infile = NULL;

	while ((opt=getopt(argc,argv,"hi:")) != EOF) {
		switch(opt) {
		case 'h':
			usage();
			return 1;
		case 'i':
			infile = fopen(optarg, "r");
			if (infile == NULL) {
				printf("Could not open file %s\n", optarg);
				usage();
				return 1;
			}
			break;
		default:
			usage();
			return 1;
		}
	}

	if (infile == NULL) {
		devh = ubertooth_start();
		if (devh == NULL) {
			usage();
			return 1;
		}
		rx_lap(devh);
		ubertooth_stop(devh);
	} else {
		rx_lap_file(infile);
		fclose(infile);
	}

	return 0;
}
