//-----------------------------------------------------------------------------
// Si115x_functions_lowlevel.c
//-----------------------------------------------------------------------------
// Copyright 2015 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// File Description:
//
// MCU-specific routines to needed by Si115x_functions.c
//
// Target:         Silicon Labs EFM8
// Command Line:   None
//
//-----------------------------------------------------------------------------

#include <string.h>
#include "ubinos.h"
#include "nrf_error.h"
#include "GT_i2c.h"
#include "si115x_functions.h"

int16_t Si115xWriteToRegister(HANDLE si115x_handle, uint8_t address, uint8_t value) {
	uint8_t r;
	uint8_t wbuf[2];

	wbuf[0] = address;
	wbuf[1] = value;

	r = GT_i2c_write(SI1133_ADDR, wbuf, 2);

	if (r == NRF_SUCCESS) {
		return 0;
	} else {
		return -1;
	}
}

/*
 S16 check_error(void)
 {
 if( SLAVENACK )       return -1;
 else if( SMBUS_FAIL ) return -2;
 else return 0;
 }

 S16 check_error_return_byte(void)
 {
 if( SLAVENACK )       return -1;
 else if( SMBUS_FAIL ) return -2;
 else return ( *SMB_DATA_IN & 0xff );
 }
 */

int16_t Si115xReadFromRegister(HANDLE si115x_handle, uint8_t address) {
	uint8_t r;
	uint8_t value;

	r = GT_i2c_read(SI1133_ADDR, &address, 1, &value, 1);

	// TODO add error checking. See check_error_return_byte().
	return value;
}

int16_t Si115xBlockWrite(HANDLE si115x_handle, uint8_t address, uint8_t length, uint8_t *values) {
	uint8_t r;
	uint8_t counter;
	uint8_t wbuf[10];

	wbuf[0] = address;
	for (counter = 0; counter < length; counter++) {
		wbuf[counter + 1] = values[counter];
	}

	r = GT_i2c_write(SI1133_ADDR, wbuf, length + 1);

	if (r == NRF_SUCCESS) {
		return 0;
	} else {
		return -1;
	}
}

int16_t Si115xBlockRead(HANDLE si115x_handle, uint8_t address, uint8_t length, uint8_t *values) {
	uint8_t r;
	uint8_t counter;
	uint8_t rbuf[13];

	r = GT_i2c_read(SI1133_ADDR, &address, 1, rbuf, length);

	// TODO add error checking. See check_error_return_byte().
	for (counter = 0; counter < length; counter++) {
		values[counter] = rbuf[counter];
	}

	return 0;
}

void Si115xDelay_10ms(void) {
	task_sleepms(10);
}
