/*
 * HAT_main.h
 *
 *  Created on: 2020. 06. 11.
 *      Author: YJPark
 */

#ifndef APP_SFU_MAIN_H_
#define APP_SFU_MAIN_H_

#include <ubinos.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>


enum{
	SFU_EVT_BUTTON1 = 0,

	SFU_UART_RCV,					// Evt will be issued in the UART handler.
	SFU_TIMER,						// Evt will be issued in the timer handler (debug).

	SFU_EVT_BLE_OPT_MAP,			// Evt will be issued when server send OPT MSG to set combination of some actuator and sensor.
	SFU_EVT_BLE_OPT_ACTUATOR,		// Evt will be issued when server send OPT MSG to set the function of specific actuator.
	SFU_EVT_BLE_OPT_PERIOD,			// Evt will be issued when server send OPT MSG to set the RTC & act period.

	SFU_EVT_BLE_ACT_REQ,			// Evt will be issued when server send REQ MSG to activate specific actuator immediately.
	SFU_EVT_RES,					// unused
	SFU_EVT_BLE_SEN_REQ,			// Evt will be issued when server send REQ MSG to get sensor data from bottom board.

	SFU_EVT_LOCAL_CLK_SET,			// Evt will be issued when server send OPT MSG to set the local clock at bottom board.
	SFU_EVT_LOCAL_CLK_REQ,			// Evt will be issued when server send REQ MSG to get the local clock val.
	SFU_EVT_TIME_SCHEDULE,			// Evt will be issued when server send OPT MSG to set activation schedule.
};

enum{
	SFU_DEFAULT_ST
};

typedef struct
{
	uint8_t event;
	uint8_t status;
	uint8_t* msg;
}SFUEvt_msgt;

void SFU_main_task_init(void);
int SFU_event_send(uint8_t evt, uint8_t state, uint8_t* msg);

#endif /* APP_SFU_MAIN_H_ */
