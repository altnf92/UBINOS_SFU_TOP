//-----------------------------------------------------------------------------
// Si115x_functions.c
//-----------------------------------------------------------------------------
// Copyright 2013 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// File Description:
//
// This file contains auto generated code Si1153 Programmer Toolkit which
// initializes the Si115x.
//
// Target:         MCU agnostic
// Command Line:   None
//
//-----------------------------------------------------------------------------
#include "si115x_functions.h"

// ch0: prox, large IR photodiode, 24us integration time, high signal range, LED1 = 390mA, LED2 = LED3 = none, accumulate 1, no right shift
// ch1: als, med white photodiode, d2_g1 integration time, high signal range, LED1 = LED2 = LED3 = none, accumulate 64, no right shift
// ch2: als, med white photodiode, d2_g1 integration time, high signal range, LED1 = LED2 = LED3 = none, accumulate 64, right shift 2
// ch3: als, med white photodiode, d2_g7 integration time, high signal range, LED1 = LED2 = LED3 = none, accumulate 64, no right shift
int16_t Si115xInitProxAls(HANDLE si115x_handle, bool proxOnly) {
	int16_t retval;

	retval = Si115xReset(si115x_handle);
	Si115xDelay_10ms();

	if (proxOnly) // prox only, no als
	{
		retval += Si115xParamSet(si115x_handle, PARAM_LED1_A, 0x3f);
		retval += Si115xParamSet(si115x_handle, PARAM_CH_LIST, 0x01);
		retval += Si115xParamSet(si115x_handle, PARAM_ADCCONFIG0, 0x62);
		retval += Si115xParamSet(si115x_handle, PARAM_ADCSENS0, 0x80);
		retval += Si115xParamSet(si115x_handle, PARAM_ADCPOST0, 0x40);
		retval += Si115xParamSet(si115x_handle, PARAM_MEASCONFIG0, 0x21);
		retval += Si115xWriteToRegister(si115x_handle, SI115x_REG_IRQ_ENABLE, 0x01);
	} else // prox + als
	{
		retval += Si115xParamSet(si115x_handle, PARAM_LED1_A, 0x3f); // LED1
		retval += Si115xParamSet(si115x_handle, PARAM_CH_LIST, 0x0f);
		retval += Si115xParamSet(si115x_handle, PARAM_ADCCONFIG0, 0x62);
		retval += Si115xParamSet(si115x_handle, PARAM_ADCSENS0, 0x80);
		retval += Si115xParamSet(si115x_handle, PARAM_ADCPOST0, 0x40);
		retval += Si115xParamSet(si115x_handle, PARAM_MEASCONFIG0, 0x21); //LED1
		retval += Si115xParamSet(si115x_handle, PARAM_ADCCONFIG1, 0x4d);
		retval += Si115xParamSet(si115x_handle, PARAM_ADCSENS1, 0xe1);
		retval += Si115xParamSet(si115x_handle, PARAM_ADCPOST1, 0x40);
		retval += Si115xParamSet(si115x_handle, PARAM_ADCCONFIG2, 0x41);
		retval += Si115xParamSet(si115x_handle, PARAM_ADCSENS2, 0xe1);
		retval += Si115xParamSet(si115x_handle, PARAM_ADCPOST2, 0x50);
		retval += Si115xParamSet(si115x_handle, PARAM_ADCCONFIG3, 0x4d);
		retval += Si115xParamSet(si115x_handle, PARAM_ADCSENS3, 0x87);
		retval += Si115xParamSet(si115x_handle, PARAM_ADCPOST3, 0x40);
		retval += Si115xWriteToRegister(si115x_handle, SI115x_REG_IRQ_ENABLE, 0x0f);
	}

	return retval;
}

// ch0: prox, large IR photodiode, 97us integration time, low signal range, LED2 = 321mA, LED1 = LED3 = none, accumulate 1, no right shift
int16_t Si115xInitLongRangeProx(HANDLE si115x_handle) {
	int16_t retval;

	retval = Si115xReset(si115x_handle);
	Si115xDelay_10ms();
	retval += Si115xParamSet(si115x_handle, PARAM_LED2_A, 0x3f);
	retval += Si115xParamSet(si115x_handle, PARAM_CH_LIST, 0x01);
	retval += Si115xParamSet(si115x_handle, PARAM_ADCCONFIG0, 0x62);
	retval += Si115xParamSet(si115x_handle, PARAM_ADCSENS0, 0x02);
	retval += Si115xParamSet(si115x_handle, PARAM_ADCPOST0, 0x40);
	retval += Si115xParamSet(si115x_handle, PARAM_MEASCONFIG0, 0x32);
	retval += Si115xWriteToRegister(si115x_handle, SI115x_REG_IRQ_ENABLE, 0x01);

	return retval;
}

// ch0: prox, large IR photodiode, 24us integration time, low signal range, LED1 = 321mA, LED2 = LED3 = none, accumulate 1, no right shift
int16_t Si115xInitSunlightImmuneProx(HANDLE si115x_handle) {
	int16_t retval;

	retval = Si115xReset(si115x_handle);
	Si115xDelay_10ms();
	retval += Si115xParamSet(si115x_handle, PARAM_LED1_A, 0x3f);
	retval += Si115xParamSet(si115x_handle, PARAM_CH_LIST, 0x01);
	retval += Si115xParamSet(si115x_handle, PARAM_ADCCONFIG0, 0x62);
	retval += Si115xParamSet(si115x_handle, PARAM_ADCPOST0, 0x40);
	retval += Si115xParamSet(si115x_handle, PARAM_MEASCONFIG0, 0x21);
	retval += Si115xWriteToRegister(si115x_handle, SI115x_REG_IRQ_ENABLE, 0x01);
	return retval;
}

//---------------------------------------------------------------------------
/* from uv/als demo in Silabs Optical Sensors Programmer Toolkit
 *
 * Si113x uses same API as Si115x
 */
int16_t Si113xInitUvAls(HANDLE si115x_handle) {
	int16_t retval;

	retval = Si115xReset(si115x_handle);
	Si115xDelay_10ms();
	retval += Si115xParamSet(si115x_handle, PARAM_CH_LIST, 0x0f);
	retval += Si115xParamSet(si115x_handle, PARAM_ADCCONFIG0, 0x78);
	retval += Si115xParamSet(si115x_handle, PARAM_ADCSENS0, 0x09);
	retval += Si115xParamSet(si115x_handle, PARAM_ADCPOST0, 0x40);
	retval += Si115xParamSet(si115x_handle, PARAM_ADCCONFIG1, 0x4d);
	retval += Si115xParamSet(si115x_handle, PARAM_ADCSENS1, 0x61);
	retval += Si115xParamSet(si115x_handle, PARAM_ADCPOST1, 0x40);
	retval += Si115xParamSet(si115x_handle, PARAM_ADCCONFIG2, 0x41);
	retval += Si115xParamSet(si115x_handle, PARAM_ADCSENS2, 0x61);
	retval += Si115xParamSet(si115x_handle, PARAM_ADCPOST2, 0x50);
	retval += Si115xParamSet(si115x_handle, PARAM_ADCCONFIG3, 0x4d);
	retval += Si115xParamSet(si115x_handle, PARAM_ADCSENS3, 0x07);
	retval += Si115xParamSet(si115x_handle, PARAM_ADCPOST3, 0x40);
	retval += Si115xWriteToRegister(si115x_handle, SI115x_REG_IRQ_ENABLE, 0x0f);
	return retval;
}

//
// To start forced measurements
//     Si115xForce( si115x_handle)
//

void Si115xHandler(HANDLE si115x_handle, SI115X_SAMPLES *samples) {
	uint8_t buffer[13];
	Si115xBlockRead(si115x_handle, SI115x_REG_IRQ_STATUS, 13, buffer);
	samples->irq_status = buffer[0];
	samples->ch0 = buffer[1] << 16;
	samples->ch0 |= buffer[2] << 8;
	samples->ch0 |= buffer[3];
	if (samples->ch0 & 0x800000)
		samples->ch0 |= 0xFF000000;
	samples->ch1 = buffer[4] << 16;
	samples->ch1 |= buffer[5] << 8;
	samples->ch1 |= buffer[6];
	if (samples->ch1 & 0x800000)
		samples->ch1 |= 0xFF000000;
	samples->ch2 = buffer[7] << 16;
	samples->ch2 |= buffer[8] << 8;
	samples->ch2 |= buffer[9];
	if (samples->ch2 & 0x800000)
		samples->ch2 |= 0xFF000000;
	samples->ch3 = buffer[10] << 16;
	samples->ch3 |= buffer[11] << 8;
	samples->ch3 |= buffer[12];
	if (samples->ch3 & 0x800000)
		samples->ch3 |= 0xFF000000;
}


