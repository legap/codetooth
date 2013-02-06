/*
 * Copyright 2010 - 2013 Michael Ossmann, Dominic Spill, Will Code, Mike Ryan
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

#ifndef __UBERTOOTH_H__
#define __UBERTOOTH_H__

#include "ubertooth_control.h"

/* gnuplot output types
 * see https://github.com/dkogan/feedgnuplot for plotter */
#define GNUPLOT_NORMAL		1
#define GNUPLOT_3D		2

enum board_ids {
	BOARD_ID_UBERTOOTH_ZERO = 0,
	BOARD_ID_UBERTOOTH_ONE  = 1,
	BOARD_ID_TC13BADGE      = 2
};

typedef void (*rx_callback)(void* args, usb_pkt_rx *rx, int bank);

struct libusb_device_handle* ubertooth_start(int ubertooth_device);
void ubertooth_stop(struct libusb_device_handle *devh);
int specan(struct libusb_device_handle* devh, int xfer_size, u16 num_blocks,
	u16 low_freq, u16 high_freq);
int do_specan(struct libusb_device_handle* devh, int xfer_size, u16 num_blocks,
	u16 low_freq, u16 high_freq, char gnuplot);
int cmd_ping(struct libusb_device_handle* devh);
int stream_rx_usb(struct libusb_device_handle* devh, int xfer_size,
	uint16_t num_blocks, rx_callback cb, void* cb_args);
int stream_rx_file(FILE* fp, uint16_t num_blocks, rx_callback cb, void* cb_args);
void rx_live(struct libusb_device_handle* devh, btbb_piconet* pn);
void rx_file(FILE* fp, btbb_piconet* pn);
void rx_follow(struct libusb_device_handle* devh, btbb_piconet* pn, uint32_t clock, uint32_t delay);
void rx_follow_offset(struct libusb_device_handle* devh, btbb_piconet* pn);
void rx_dump(struct libusb_device_handle* devh, int full);
void rx_btle(struct libusb_device_handle* devh);
void rx_btle_file(FILE* fp);
void cb_btle(void* args, usb_pkt_rx *rx, int bank);

typedef struct pnet_list_item {
	btbb_piconet* pnet;
	struct pnet_list_item* next;
} pnet_list_item;

pnet_list_item* ubertooth_scan(struct libusb_device_handle* devh, int timeout);
#endif /* __UBERTOOTH_H__ */
