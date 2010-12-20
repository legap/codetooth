/*
 * Copyright 2010 Michael Ossmann
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

#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t

#define VENDORID    0xffff
#define PRODUCTID   0x0004
#define DATA_IN     (0x82 | LIBUSB_ENDPOINT_IN)
#define DATA_OUT    (0x05 | LIBUSB_ENDPOINT_OUT)
#define CTRL_IN     (LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN)
#define CTRL_OUT    (LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT)
#define TIMEOUT     2000
#define BUFFER_SIZE 102400

enum ubertooth_usb_commands {
    UBERTOOTH_PING        = 0,
    UBERTOOTH_RX_SYMBOLS  = 1,
    UBERTOOTH_TX_SYMBOLS  = 2,
    UBERTOOTH_GET_USRLED  = 3,
    UBERTOOTH_SET_USRLED  = 4,
    UBERTOOTH_GET_RXLED   = 5,
    UBERTOOTH_SET_RXLED   = 6,
    UBERTOOTH_GET_TXLED   = 7,
    UBERTOOTH_SET_TXLED   = 8,
    UBERTOOTH_GET_1V8     = 9,
    UBERTOOTH_SET_1V8     = 10,
    UBERTOOTH_GET_CHANNEL = 11,
    UBERTOOTH_SET_CHANNEL = 12,
    UBERTOOTH_RESET       = 13
};

static struct libusb_device_handle *devh = NULL;

typedef struct {
	u8 reserved[8];
} ctrl_msg_t;

/*
 * USB packet for Bluetooth RX (64 total bytes)
 */
typedef struct {
	u8     pkt_type;
	u8     status;
	u8     channel;
	u8     clkn_high;
	u32    clk100ns;
	u8     reserved[6];
	u8     data[50];
} usb_pkt_rx;

static int find_ubertooth_device(void)
{
	devh = libusb_open_device_with_vid_pid(NULL, VENDORID, PRODUCTID);
	return devh ? 0 : -1;
}

static int send_cmd_rx_syms(u16 num)
{
	int r;

	r = libusb_control_transfer(devh, CTRL_OUT, UBERTOOTH_RX_SYMBOLS, num, 0,
			NULL, 0, 1000);
	if (r < 0) {
		fprintf(stderr, "command error %d\n", r);
		return r;
	}
}

static int stream_rx(int xfer_size, u16 num_blocks)
{
	u8 buffer[BUFFER_SIZE];
	int r;
	int i, j, k;
	int xfer_blocks;
	int num_xfers;
	int transferred;
	u32 time; /* in 100 nanosecond units */

	/*
	 * A block is 64 bytes transferred over USB (includes 50 bytes of rx symbol
	 * payload).  A transfer consists of one or more blocks.  Consecutive
	 * blocks should be approximately 400 microseconds apart (timestamps about
	 * 4000 apart in units of 100 nanoseconds).
	 */

	if (xfer_size > BUFFER_SIZE)
		xfer_size = BUFFER_SIZE;
	xfer_blocks = xfer_size / 64;
	xfer_size = xfer_blocks * 64;
	num_xfers = num_blocks / xfer_blocks;
	num_blocks = num_xfers * xfer_blocks;

	fprintf(stderr, "rx %d blocks of 64 bytes in %d byte transfers\n",
			num_blocks, xfer_size);

	send_cmd_rx_syms(xfer_blocks);

	while (num_xfers--) {
		r = libusb_bulk_transfer(devh, DATA_IN, buffer, xfer_size,
				&transferred, TIMEOUT);
		if (r < 0) {
			printf("[Read] returned: %d , failed to read\n", r);
			return -1;
		}
		if (transferred != xfer_size) {
			fprintf(stderr, "bad data read size (%d)\n", transferred);
			return -1;
		}
		fprintf(stderr, "transferred %d bytes\n", transferred);

		/* process each received block */
		for (i = 0; i < xfer_blocks; i++) {
			time = buffer[4 + 64 * i]
					| (buffer[5 + 64 * i] << 8)
					| (buffer[6 + 64 * i] << 16)
					| (buffer[7 + 64 * i] << 24);
			fprintf(stderr, "rx block timestamp %u * 100 nanoseconds\n", time);
			for (j = 64 * i + 14; j < 64 * i + 64; j++) {
				//printf("%02x", buffer[j]);
				/* output one byte for each received symbol (0 or 1) */
				for (k = 0; k < 8; k++) {
					printf("%c", (buffer[j] & 0x80) >> 7 );
					buffer[j] <<= 1;
				}
			}
		}
		fflush(stderr);
	}
}

int main(void)
{
	int r = 1;

	r = libusb_init(NULL);
	if (r < 0) {
		fprintf(stderr, "libusb_init failed (got 1.0?)\n");
		exit(1);
	}

	r = find_ubertooth_device();
	if (r < 0) {
		fprintf(stderr, "could not find Ubertooth device\n");
		goto out;
	}

	r = libusb_claim_interface(devh, 0);
	if (r < 0) {
		fprintf(stderr, "usb_claim_interface error %d\n", r);
		goto out;
	}

	while (1)
		stream_rx(512, 0xFFFF);

	libusb_release_interface(devh, 0);
out:
	libusb_close(devh);
	libusb_exit(NULL);
	return r >= 0 ? r : -r;
}
