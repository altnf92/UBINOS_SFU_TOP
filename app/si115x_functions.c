//-----------------------------------------------------------------------------
// Si115x_functions.c
//-----------------------------------------------------------------------------
// Copyright 2013 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// File Description:
//
// This file is provided in the Si1153 Programmer Toolkit and contains the
// MCU-agnostic API to communicate with the Si1153 over I2C. This API relies
// on MCU-specific routines defined in a separate .c file.
//
// DO NOT MODIFY
//
// Target:         MCU agnostic
// Command Line:   None
//
//-----------------------------------------------------------------------------
#include "si115x_functions.h"

/*

static SI1133_LuxCoeff_TypeDef lk = {
		{ 	{     0,   209 },
			{  1665,    93 },
			{  2064,    65 },
			{ -2671,   234 } 	},

		{ 	{     0,     0 },
			{  1921, 29053 },
			{ -1022, 36363 },
			{  2320, 20789 },
			{  -367, 57909 },
			{ -1774, 38240 },
			{  -608, 46775 },
			{ -1503, 51831 },
			{ -1886, 58928 } 	}
};

*/







/***************************************************************************//**
 * @brief
 *   Waits until the Si115x is sleeping before proceeding
 ******************************************************************************/
static int16_t _waitUntilSleep(HANDLE si115x_handle) {
	int16_t retval = -1;
	uint8_t count = 0;
	// This loops until the Si115x is known to be in its sleep state
	// or if an i2c error occurs
	while (count < 5) {
		retval = Si115xReadFromRegister(si115x_handle, SI115x_REG_RESPONSE0);
		if ((retval & RSP0_CHIPSTAT_MASK) == RSP0_SLEEP)
			break;
		if (retval < 0)
			return retval;
		count++;
	}
	return 0;
}

/***************************************************************************//**
 * @brief
 *   Resets the Si115x/6x, clears any interrupts and initializes the HW_KEY
 *   register.
 * @param[in] si115x_handle
 *   The programmer's toolkit handle
 * @retval  0
 *   Success
 * @retval  <0
 *   Error
 ******************************************************************************/
int16_t Si115xReset(HANDLE si115x_handle) {
	int16_t retval = 0;

	// Do not access the Si115x earlier than 25 ms from power-up.
	// Uncomment the following lines if Si115xReset() is the first
	// instruction encountered, and if your system MCU boots up too
	// quickly.
	Si115xDelay_10ms();
	Si115xDelay_10ms();
	Si115xDelay_10ms();

	// Perform the Reset Command
	retval += Si115xWriteToRegister(si115x_handle, SI115x_REG_COMMAND, 1);

	// Delay for 10 ms. This delay is needed to allow the Si115x
	// to perform internal reset sequence.
	Si115xDelay_10ms();

	return retval;
}

/***************************************************************************//**
 * @brief
 *   Helper function to send a command to the Si113x/4x
 ******************************************************************************/
static int16_t _sendCmd(HANDLE si115x_handle, uint8_t command) {
	int16_t response;
	int8_t retval;
	uint8_t count = 0;

	// Get the response register contents
	response = Si115xReadFromRegister(si115x_handle, SI115x_REG_RESPONSE0);
	if (response < 0) {
		return response;
	}

	response = response & RSP0_COUNTER_MASK;

	// Double-check the response register is consistent
	while (count < 5) {
		if ((retval = _waitUntilSleep(si115x_handle)) != 0)
			return retval;

		if (command == 0)
			break; // Skip if the command is NOP

		retval = Si115xReadFromRegister(si115x_handle, SI115x_REG_RESPONSE0);

		if ((retval & RSP0_COUNTER_MASK) == response)
			break;
		else if (retval < 0)
			return retval;
		else
			response = retval & RSP0_COUNTER_MASK;

		count++;
	} // end loop

	// Send the Command
	if ((retval = (Si115xWriteToRegister(si115x_handle, SI115x_REG_COMMAND, command)) != 0)) {
		return retval;
	}

	count = 0;
	// Expect a change in the response register
	while (count < 5) {
		if (command == 0)
			break; // Skip if the command is NOP

		retval = Si115xReadFromRegister(si115x_handle, SI115x_REG_RESPONSE0);
		if ((retval & RSP0_COUNTER_MASK) != response)
			break;
		else if (retval < 0)
			return retval;

		count++;
	} // end loop

	return 0;
}

/***************************************************************************//**
 * @brief
 *   Sends a NOP command to the Si115x/6x
 * @param[in] si115x_handle
 *   The programmer's toolkit handle
 * @retval  0
 *   Success
 * @retval  <0
 *   Error
 ******************************************************************************/
int16_t Si115xNop(HANDLE si115x_handle) {
	return _sendCmd(si115x_handle, 0x00);
}

/***************************************************************************//**
 * @brief
 *   Sends a FORCE command to the Si115x/6x
 * @param[in] si115x_handle
 *   The programmer's toolkit handle
 * @retval  0
 *   Success
 * @retval  <0
 *   Error
 ******************************************************************************/
int16_t Si115xForce(HANDLE si115x_handle) {
	return _sendCmd(si115x_handle, 0x11);
}

/***************************************************************************//**
 * @brief
 *   Sends a PSALSAUTO command to the Si113x/4x
 * @param[in] si115x_handle
 *   The programmer's toolkit handle
 * @retval  0
 *   Success
 * @retval  <0
 *   Error
 ******************************************************************************/
int16_t Si115xStart(HANDLE si115x_handle) {
	return _sendCmd(si115x_handle, 0x13);
}

/***************************************************************************//**
 * @brief
 *   Reads a Parameter from the Si115x/6x
 * @param[in] si115x_handle
 *   The programmer's toolkit handle
 * @param[in] address
 *   The address of the parameter.
 * @retval <0
 *   Error
 * @retval 0-255
 *   Parameter contents
 ******************************************************************************/
int16_t Si115xParamRead(HANDLE si115x_handle, uint8_t address) {
	// returns Parameter[address]
	int16_t retval;
	uint8_t cmd = 0x40 + (address & 0x3F);

	retval = _sendCmd(si115x_handle, cmd);
	if (retval != 0) {
		return retval;
	}
	retval = Si115xReadFromRegister(si115x_handle, SI115x_REG_RESPONSE1);
	return retval;
}

/***************************************************************************//**
 * @brief
 *   Writes a byte to an Si115x/6x Parameter
 * @param[in] si115x_handle
 *   The programmer's toolkit handle
 * @param[in] address
 *   The parameter address
 * @param[in] value
 *   The byte value to be written to the Si113x/4x parameter
 * @retval 0
 *   Success
 * @retval <0
 *   Error
 * @note This function ensures that the Si115x/6x is idle and ready to
 * receive a command before writing the parameter. Furthermore,
 * command completion is checked. If setting parameter is not done
 * properly, no measurements will occur. This is the most common
 * error. It is highly recommended that host code make use of this
 * function.
 ******************************************************************************/
int16_t Si115xParamSet(HANDLE si115x_handle, uint8_t address, uint8_t value) {
	int16_t retval;
	uint8_t buffer[2];
	int16_t response_stored;
	int16_t response;

	retval = _waitUntilSleep(si115x_handle);
	if (retval != 0) {
		return retval;
	}

	response_stored = RSP0_COUNTER_MASK & Si115xReadFromRegister(si115x_handle, SI115x_REG_RESPONSE0);

	buffer[0] = value;
	buffer[1] = 0x80 + (address & 0x3F);

	retval = Si115xBlockWrite(si115x_handle, SI115x_REG_HOSTIN0, 2, (uint8_t*) buffer);
	if (retval != 0)
		return retval;

	// Wait for command to finish
	response = Si115xReadFromRegister(si115x_handle, SI115x_REG_RESPONSE0);
	while ((response & RSP0_COUNTER_MASK) == response_stored) {
		response = Si115xReadFromRegister(si115x_handle, SI115x_REG_RESPONSE0);
	}

	if (retval < 0)
		return retval;
	else
		return 0;
}

/***************************************************************************//**
 * @brief
 *   Pause measurement helper function
 ******************************************************************************/
static int16_t _Pause(HANDLE si115x_handle) {
	return _sendCmd(si115x_handle, 0x12);
}

/***************************************************************************//**
 * @brief
 *   Pauses autonomous measurements
 * @param[in] si115x_handle
 *  The programmer's toolkit handle
 * @retval  0
 *   Success
 * @retval  <0
 *   Error
 ******************************************************************************/
int16_t Si115xPause(HANDLE si115x_handle) {
	uint8_t countA, countB;
	int8_t retval;

	//  After a RESET, if the Si115x receives a command (including NOP) before
	//  the Si115x has gone to sleep, the chip hangs. This first while loop
	//  avoids this.  The reading of the REG_RESPONS0 does not disturb
	//  the internal MCU.

	retval = 0; // initialize data so that we guarantee to enter the loop
	while ((RSP0_CHIPSTAT_MASK & retval) != RSP0_SLEEP) {
		retval = Si115xReadFromRegister(si115x_handle, SI115x_REG_RESPONSE0);
	}

	countA = 0;
	while (countA < 5) {
		countB = 0;
		// Keep sending nops until the response is zero
		while (countB < 5) {
			retval = Si115xReadFromRegister(si115x_handle, SI115x_REG_RESPONSE0);
			if ((retval & RSP0_COUNTER_MASK) == 0)
				break;
			else {
				// Send the NOP Command to clear any error...we cannot use
				// Si115xNop() because it first checks if REG_RESPONSE < 0 and
				// if so it does not perform the cmd. Since we have a saturation
				// REG_RESPONSE will be < 0
				Si115xWriteToRegister(si115x_handle, SI115x_REG_COMMAND, 0x00);
			}
			countB++;
		} // end inner loop

		// Pause the device
		_Pause(si115x_handle);

		countB = 0;
		// Wait for response
		while (countB < 5) {
			retval = Si115xReadFromRegister(si115x_handle, SI115x_REG_RESPONSE0);
			if ((retval & RSP0_COUNTER_MASK) != 0)
				break;
			countB++;
		}

		// When the PsAlsPause() response is good, we expect it to be a '1'.
		retval = Si115xReadFromRegister(si115x_handle, SI115x_REG_RESPONSE0);
		if ((retval & RSP0_COUNTER_MASK) == 1)
			break;  // otherwise, start over.
		countA++;
	} // end outer loop
	return 0;
}

/***************************************************************************//**
 * @brief
 *   Writes a byte to an Si115x/6x Parameter
 * @param[in] si115x_handle
 *   The programmer's toolkit handle
 * @param[in] param_addr
 *   The parameter address
 * @param[in] param_value
 *   The byte value to be written to the Si115x/6x parameter
 * @retval 0
 *   Success
 * @retval <0
 *   Error
 * @note This function ensures that the Si115x/6x is idle and ready to
 * receive a command before writing the parameter. Furthermore,
 * command completion is checked. If setting parameter is not done
 * properly, no measurements will occur. This is the most common
 * error. It is highly recommended that host code make use of this
 * function.
 ******************************************************************************/
void SetParam(HANDLE si115x_handle, uint8_t param_addr, uint8_t param_value) {
	uint8_t temp;

	temp = Si115xReadFromRegister(si115x_handle, SI115x_REG_RESPONSE0);
	while (!(temp & 0x20)) {
		// wait for device to sleep
		temp = Si115xReadFromRegister(si115x_handle, SI115x_REG_RESPONSE0);
	}
	if (temp & 0x10) {
		// if error code is present, NOP to clear
		Si115xWriteToRegister(si115x_handle, SI115x_REG_COMMAND, CMD_NOP);
		while (temp & 0xDF) {
			// wait for device to sleep and clear
			temp = Si115xReadFromRegister(si115x_handle, SI115x_REG_RESPONSE0);
		} // end loop
	} // end if

	Si115xWriteToRegister(si115x_handle,
	SI115x_REG_HOSTIN0, param_value);
	Si115xWriteToRegister(si115x_handle,
	SI115x_REG_COMMAND,
	CMD_PARAM_SET | param_addr);
	while ((temp & 0x1f) == (Si115xReadFromRegister(si115x_handle, SI115x_REG_RESPONSE0) & 0x1f)) {
		// Do Nothing
		;
	}
}

/***************************************************************************//**
 * @brief
 *   Reads a Parameter from the Si115x/6x
 * @param[in] si115x_handle
 *   The programmer's toolkit handle
 * @param[in] param_addr
 *   The address of the parameter.
 * @retval <0
 *   Error
 * @retval 0-255
 *   Parameter contents
 ******************************************************************************/
uint8_t QueryParam(HANDLE si115x_handle, uint8_t param_addr) {
	uint8_t temp;

	temp = Si115xReadFromRegister(si115x_handle, SI115x_REG_RESPONSE0);
	while (!(temp & 0x20)) {
		// wait for device to sleep
		temp = Si115xReadFromRegister(si115x_handle, SI115x_REG_RESPONSE0);
	}
	if (temp & 0x10) {
		// if error code is present, NOP to clear
		Si115xWriteToRegister(si115x_handle, SI115x_REG_COMMAND, CMD_NOP);
		while (temp & 0xDF) {
			// wait for device to sleep and clear
			temp = Si115xReadFromRegister(si115x_handle, SI115x_REG_RESPONSE0);
		}
	}
	Si115xWriteToRegister(si115x_handle,
	SI115x_REG_COMMAND, (CMD_PARAM_QUERY | param_addr));
	while ((temp & 0x1f) == (Si115xReadFromRegister(si115x_handle, SI115x_REG_RESPONSE0) & 0x1f)) {
		;
	}
	return Si115xReadFromRegister(si115x_handle, SI115x_REG_HOSTOUT0);
}

/***************************************************************************//**
 * @brief
 *   Helper function to send a command to the Si113x/4x
 ******************************************************************************/
uint8_t SendCmd(HANDLE si115x_handle, uint8_t cmd) {
	uint8_t temp;

	temp = Si115xReadFromRegister(si115x_handle, SI115x_REG_RESPONSE0);
	while (!(temp & 0x20)) {
		// wait for device to sleep
		temp = Si115xReadFromRegister(si115x_handle, SI115x_REG_RESPONSE0);
	}
	if (temp & 0x10) {
		// if error code is present, NOP to clear
		Si115xWriteToRegister(si115x_handle, SI115x_REG_COMMAND, CMD_NOP);
		while (temp & 0xDF) {
			// wait for device to sleep and clear
			temp = Si115xReadFromRegister(si115x_handle, SI115x_REG_RESPONSE0);
		}
	}
	Si115xWriteToRegister(si115x_handle, SI115x_REG_COMMAND, cmd);
	while ((temp & 0x1f) == (Si115xReadFromRegister(si115x_handle, SI115x_REG_RESPONSE0) & 0x1f))
		;
	{
		;
	}
	return Si115xReadFromRegister(si115x_handle, SI115x_REG_RESPONSE0);
}








int32_t SI1133_calcPolyInner( int32_t input, int8_t fraction, uint16_t mag, int8_t shift ){
	int32_t value;
	if( shift < 0 ) 	{value = ( ( input << fraction ) / mag ) >> -shift;}
	else 				{value = ( ( input << fraction ) / mag ) << shift;}

	return value;
}


int32_t SI1133_calcEvalPoly( int32_t x, int32_t y,
		uint8_t input_fraction, uint8_t output_fraction,
		uint8_t num_coeff, SI1133_Coeff_TypeDef *kp ){

	uint8_t info, x_order, y_order, counter;
	int8_t sign, shift;
	uint16_t mag;
	int32_t output = 0, x1, x2, y1, y2;

	for( counter = 0; counter < num_coeff; counter++ ) {

		info = kp->info;
		x_order = get_x_order( info );
		y_order = get_y_order( info );

		shift = ( (uint16_t) kp->info & 0xff00 ) >> 8;
		shift ^= 0x00ff;
		shift += 1;
		shift = -shift;

		mag = kp->mag;

		if( get_sign( info ) ) 	{sign = -1;}
		else 					{sign = 1;}

		if( ( x_order == 0 ) && ( y_order == 0 ) ) {
			output += sign * mag << output_fraction;
		}
		else {
			if( x_order > 0 ) {
				x1 = SI1133_calcPolyInner( x, input_fraction, mag, shift );

				if( x_order > 1 ) 	{x2 = SI1133_calcPolyInner( x, input_fraction, mag, shift );}
				else 				{x2 = 1;}
			}
			else{
				x1 = 1;
				x2 = 1;
			}

			if( y_order > 0 ) {
				y1 = SI1133_calcPolyInner( y, input_fraction, mag, shift );

				if( y_order > 1 ) 	{y2 = SI1133_calcPolyInner( y, input_fraction, mag, shift );}
				else 				{y2 = 1;}
			}
			else{
				y1 = 1;
				y2 = 1;
			}

			output += sign * x1 * x2 * y1 * y2;
		}

		kp++;
	}

	if( output < 0 ) {output = -output;}

	return output;
}


int32_t SI1133_getLux( int32_t vis_high, int32_t vis_low, int32_t ir, SI1133_LuxCoeff_TypeDef *lk ){
	int32_t lux;

	if( ( vis_high > ADC_THRESHOLD ) || ( ir > ADC_THRESHOLD ) ) {
		lux = SI1133_calcEvalPoly(
				vis_high,
				ir,
				INPUT_FRACTION_HIGH,
				LUX_OUTPUT_FRACTION,
				NUMCOEFF_HIGH,
				&( lk->coeff_high[0] ) );
	}
	else{
		lux = SI1133_calcEvalPoly(
				vis_low,
				ir,
				INPUT_FRACTION_LOW,
				LUX_OUTPUT_FRACTION,
				NUMCOEFF_LOW,
				&( lk->coeff_low[0] ) );
	}

	return lux;
}

int32_t SI1133_getUv( int32_t uv, SI1133_Coeff_TypeDef *uk ){
	int32_t uvi;
	uvi = SI1133_calcEvalPoly( 0, uv, UV_INPUT_FRACTION, UV_OUTPUT_FRACTION, UV_NUMCOEFF, uk );

	return uvi;
}
