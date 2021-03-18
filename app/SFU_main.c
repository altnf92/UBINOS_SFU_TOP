/*
 * SFU_main.c
 *
 *  Created on: 2020. 06. 11.
 *      Author: YJPark
 */

#include <ubinos.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "SFU_main.h"
#include "ble_stack.h"
#include "Button.h"
#include "LAP_main.h"

#include "app_uart.h"
#include "app_error.h"
#include "nrf_uart.h"
#include "nrf_delay.h"

#include "nrf_gpiote.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"

#include "nrf_drv_twi.h"
#include "GT_i2c.h"
#include "si115x_functions.h"


#include "nrf.h"









/* UART config macros
 * ** UART_TX_BUFF_SIZE is also used for uart_buffer[UART_TX_BUFF_SIZE]
 * */
#define FLOW_CNT	APP_UART_FLOW_CONTROL_DISABLED
#define PARITY		false
#define BOUDRATE	NRF_UART_BAUDRATE_9600
#define UART_TX_BUFF_SIZE		8
#define UART_RX_BUFF_SIZE		10



/* Macros for sending external sensor value
 * Current ext sensors : LUX, UVI(SI1133) ~ updated : 2021.02.16
 * */
#define CMD_EXT						0x07
#define LEN_EXT						7
#define EXT_SEN_NUM_LUX				4
#define EXT_SEN_NUM_UVI				5






static msgq_pt SFU_msgq;
ret_code_t err_code;


#define APP_TICK_EVENT_INTERVAL     APP_TIMER_TICKS(2000)
APP_TIMER_DEF(m_tick_timer);







/* Structures & macros for SI1133 module code */
// nrf_drv_twi_t twi_instance = NRF_DRV_TWI_INSTANCE(0);
bool device_found = false;
#define UV_SENSOR_TWI_ADR			0x55		// or 0x52
#define TWI_INSTANCE_ID 			0
#define MAX_PENDING_TRANSACTIONS 	20
HANDLE si1133_handle = 0;
SI115X_SAMPLES UV_samples;
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
static SI1133_Coeff_TypeDef uk[2] = {
		{ 1281, 30902 },
		{ -638, 46301 }
};






void module_sensor_data_uart(uint8_t label, int32_t data);
static void si1133_init(void);
static uint8_t getUVSensorData(void);

void ext_sensors_send(uint8_t function_choice);
#define EXT_FUNC_BLE	1
#define EXT_FUNC_UART	2




/* UART CMD macros
 * ACK : ACK for commands sent by top board
 * HISTORY_CMD : Sensor data & act history
 * NACK : Err sent by bottom board
 * EXT_SEN_REQ : Request for sensor value from bottom board.
 * REQ_CLK : Local clock of bottom board
 * */
#define ACK				0xaa
#define HISTORY_CMD		0xbb
#define NACK			0xdd
#define EXT_SEN_REQ		0xcc
#define REQ_CLK			0xee


/* UART variables
 * uart_buffer : Buffer
 * cnt : CMD string length
 * index : Location of the current character in CMD string
 * */
uint8_t uart_buffer[UART_RX_BUFF_SIZE];
uint8_t uart_cnt = 0;
uint8_t uart_index = 0;



/* Timer evt handler for Debugging (unused) */
void app_tick_handler(void* p_context){
	SFU_event_send(SFU_TIMER, 0, NULL);
}


/* @ UART EVT handler function
 * @ updated : 2021.02.17
 * @ return void
 * @ param p_event : Data structure containing events from the UART module */
void uart_handle(app_uart_evt_t * p_event){


	if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    	APP_ERROR_HANDLER(p_event->data.error_communication);
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    	APP_ERROR_HANDLER(p_event->data.error_code);

    else if (p_event->evt_type == APP_UART_DATA_READY){
    	/* The UART format between the two platforms does not match, so it sends 1 byte each.
    	 * tmp : byte from bottom board(ATSAMD21)
    	 * uart_cnt : static counter indicating how many bytes should be sent.
    	 * uart_buffer : accumulated bytes. it is CMD string.
    	 *
    	 * ACK : EVT to receive ACK for MSG sent
    	 * NACK : EVT to receive NACK for MSG sent ~ required act failed
    	 * HISTORY_CMD : EVT that occur when sensor values and act history come in
    	 * EXT_SEN_REQ : EVT that occur when bottom board require external sensor values
    	 * REQ_CLK : EVT that occur when bottom board send its local clock time*/
    	uint8_t tmp;
    	app_uart_get(&tmp);

    	// At first byte of string, CMD distinguished
    	if(uart_cnt == 0 && tmp == ACK){
    		BLE_send_short(0xff);

    		uart_buffer[uart_index++] = ACK;
    		uart_cnt = 2;
    	}
    	else if(uart_cnt == 0 && tmp == NACK){
    		uart_buffer[uart_index++] = NACK;
    		uart_cnt = 2;
    	}
    	else if(uart_cnt == 0 && tmp == HISTORY_CMD){
    		uart_buffer[uart_index++] = HISTORY_CMD;
    		uart_cnt = 10;
    	}
    	else if(uart_cnt == 0 && tmp == EXT_SEN_REQ){
			uart_index = 0;
			uart_cnt = 0;
			SFU_event_send(SFU_UART_RCV, EXT_SEN_REQ, NULL);
    	}
    	else if(uart_cnt == 0 && tmp == REQ_CLK){
    		uart_buffer[uart_index++] = REQ_CLK;
    		uart_cnt = 3;
    	}


    	/* From the second byte, accumulate in the buffer a specified number(uart_cnt).
    	 * After filling the buffer, the event is issued to the main function. */
    	else if(uart_cnt != 0){
    		uart_buffer[uart_index++] = tmp;
    		if(uart_index == uart_cnt){
    			SFU_event_send(SFU_UART_RCV, uart_buffer[0], NULL);
    			uart_index = 0;
    			uart_cnt = 0;
    		}
    	}
    }
}


/* @ Main task
 * @ updated : 2021.02.17
 * @ return void
 * @ param arg : arguments */
void SFU_main_task(void* arg){
	int r;
	SFUEvt_msgt SFU_evt_msg;
	uint8_t ble_buf[UART_RX_BUFF_SIZE];
	uint8_t tmp;


	/* GPIO init
	 * Wait for UBINOS BLE stack init */
	nrf_drv_gpiote_init();
	ble_stack_init_wait();

	task_sleepms(100);


// initializing =========================================================================================================================
// uart ---------------------------------------------------------------------------------------------------------------------------------
	const app_uart_comm_params_t uart_configuration = {
		RX_PIN,	TX_PIN,	RTS_PIN, CTS_PIN,
		FLOW_CNT, PARITY, BOUDRATE
	};
	APP_UART_FIFO_INIT(&uart_configuration, 8, 8,
						uart_handle, APP_IRQ_PRIORITY_LOW, err_code);
	// UART_RX_BUFF_SIZE
	// UART_TX_BUFF_SIZE
// uart ---------------------------------------------------------------------------------------------------------------------------------

// app timer ----------------------------------------------------------------------------------------------------------------------------
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_create(&m_tick_timer, APP_TIMER_MODE_REPEATED, app_tick_handler);
    APP_ERROR_CHECK(err_code);
	err_code = app_timer_start(m_tick_timer, APP_TICK_EVENT_INTERVAL, NULL);
//	err_code = app_timer_stop(m_tick_timer, APP_TICK_EVENT_INTERVAL, NULL);
// app timer ----------------------------------------------------------------------------------------------------------------------------

// TWI ----------------------------------------------------------------------------------------------------------------------------------
	GT_i2c_init();
	si1133_init();
// TWI ----------------------------------------------------------------------------------------------------------------------------------
// initializing =========================================================================================================================



	uint16_t sensor_data[4] = {0, 0, 0, 0};
	// buffer for sensor data came from bottom board (MAX num 4)


	for (;;) {
		r = msgq_receive(SFU_msgq, (unsigned char*) &SFU_evt_msg);
		if (0 != r) {
			logme("fail at msgq_receive\r\n");
		}
		else {
			switch( SFU_evt_msg.event ){

			/* Evt flag issued from UART handler (connected with bottom board)
	    	 * ACK (unused) : EVT to receive ACK for MSG sent
	    	 * NACK (unused) : EVT to receive NACK for MSG sent ~ required act failed
	    	 * HISTORY_CMD : EVT that occur when sensor values and act history come in
	    	 * EXT_SEN_REQ : EVT that occur when bottom board require external sensor values
	    	 * REQ_CLK : EVT that occur when bottom board send its local clock time
	    	 * */
			case SFU_UART_RCV :
				if(SFU_evt_msg.status == HISTORY_CMD){	// MSG Length
					// get sensor values(16 bit)
					sensor_data[0] = (uart_buffer[2] << 8) + uart_buffer[3];
					sensor_data[1] = (uart_buffer[4] << 8) + uart_buffer[5];
					sensor_data[2] = (uart_buffer[6] << 8) + uart_buffer[7];
					sensor_data[3] = (uart_buffer[8] << 8) + uart_buffer[9];
					BLE_send_res(sensor_data[0], sensor_data[1], sensor_data[2], sensor_data[3], uart_buffer[1]);

					ext_sensors_send(EXT_FUNC_BLE);
					// EXT_FUNC_BLE ~ this function will send BLE MSG including external sensor values to server
					memset(uart_buffer, 0, sizeof(uart_buffer));
				}
				else if(SFU_evt_msg.status == EXT_SEN_REQ){
					ext_sensors_send(EXT_FUNC_UART);
					// EXT_FUNC_BLE ~ this function will send UART MSG including external sensor values to bottom board
				}
				else if(SFU_evt_msg.status == REQ_CLK){
					BLE_send_time(uart_buffer[1], uart_buffer[2]);
					// send local time to server
					memset(uart_buffer, 0, sizeof(uart_buffer));
				}

				break;



			/* EVT issued by BLE receive
			 * It usually delivers messages directly from the server to the bottom board.
			 * */
			case SFU_EVT_BLE_OPT_MAP :
			case SFU_EVT_BLE_OPT_ACTUATOR :
			case SFU_EVT_BLE_OPT_PERIOD :
			case SFU_EVT_BLE_ACT_REQ :
			case SFU_EVT_BLE_SEN_REQ :
			case SFU_EVT_LOCAL_CLK_SET:
			case SFU_EVT_LOCAL_CLK_REQ:
			case SFU_EVT_TIME_SCHEDULE:

				tmp = SFU_evt_msg.status;
				memcpy(ble_buf, SFU_evt_msg.msg, tmp);


				for(uint8_t i = 0; i < tmp; i++){
//					BLE_send_short(ble_buf[i]);
					app_uart_put(ble_buf[i]);
				}

				memset(ble_buf, 0, UART_RX_BUFF_SIZE);

				break;


			// for debugging
			case SFU_TIMER :
//				si1133_sample();
//				app_uart_put('B');
				break;
			default :
				break;

			}


			if( SFU_evt_msg.msg != NULL ){
				free(SFU_evt_msg.msg);
			}

		}
	}
}


void SFU_main_task_init(void){
	int r;

	r = msgq_create(&SFU_msgq, sizeof(SFUEvt_msgt), 20);
	if (0 != r) {
		printf("fail at msgq create\r\n");
	}

	r = task_create(NULL, SFU_main_task, NULL, task_gethighestpriority()-2, 512, NULL);
	if (r != 0) {
		printf("== HAT_main_task failed \n\r");
	} else {
		printf("== HAT_main_task created \n\r");
	}
}

int SFU_event_send(uint8_t evt, uint8_t state, uint8_t* msg)
{
	SFUEvt_msgt hat_msg;

	hat_msg.event = evt;
	hat_msg.status = state;
	hat_msg.msg = msg;

	return msgq_send(SFU_msgq, (unsigned char*) &hat_msg);
}





/* @ initialize SI1133 sensor module. module handler variable ~ [si1133_handle]
 * @ updated : 2020.02.17
 * @ return void
 * @ param void */
static void si1133_init() {
	if (Si115xReadFromRegister(si1133_handle, SI115x_REG_PART_ID) != 0x33) {
		return;
	}
	nrf_gpio_cfg_input(PIN_TWI_INT, NRF_GPIO_PIN_PULLUP);

	Si113xInitUvAls(si1133_handle);
//	Si115xForce(si1133_handle);
}


/* @ get raw data from SI1133. raw data will be stored in [UV_samples] and it has 4 channels.
 * @ updated : 2020.02.17
 * @ return 0 : normal function
 * @ return 1 : not ready for read action
 * @ param void */
static uint8_t getUVSensorData() {
	// Check interrupt pin
	if (nrf_gpio_pin_read(PIN_TWI_INT) == 0) {
		// Sensor data ready
		// Process measurement
		Si115xHandler(si1133_handle, &UV_samples);

		// Start next measurement
//		Si115xForce(si1133_handle);

		Si115xNop(si1133_handle);
		return 0;
	}
	else
		return 1;
}


/* @ send external sensor data through UART or BLE. List : LUX(SI1133), UVI(SI1133).
 * @ updated : 2020.02.17
 * @ return void
 * @ param function_choice :
 * 		if EXT_FUNC_BLE : send ext val through BLE to server
 * 		if EXT_FUNC_UART : send ext val through UART to bottom board */
int32_t lux, uvi;
void ext_sensors_send(uint8_t function_choice) {

	Si115xForce(si1133_handle);
	task_sleepms(200);

	uint8_t tmp;


	if (!getUVSensorData()) {
		// none diffuser

		// SI1133_getLux (https://siliconlabs.github.io/Gecko_SDK_Doc/efm32pg12/html/si1133_8c_source.html)
		lux = SI1133_getLux( UV_samples.ch1, UV_samples.ch3, UV_samples.ch2, &lk );
		lux = lux / 9;
		lux = lux / ( 1 << LUX_OUTPUT_FRACTION );

		// SI1133_getUV (https://siliconlabs.github.io/Gecko_SDK_Doc/efm32pg12/html/si1133_8c_source.html)
		uvi = SI1133_getUv( UV_samples.ch0, uk );
		uvi = uvi * 1000 / 9; 		// mW -> uW
		uvi = uvi / ( 1 << UV_OUTPUT_FRACTION );
	}
	else{
		lux = -1;
		uvi = -1;
	}

	Si115xNop(si1133_handle);

	if(function_choice == EXT_FUNC_BLE){ 	// BLE to host
		BLE_send_light(lux, uvi);
	}

	else if(function_choice == EXT_FUNC_UART){	// UART to bottom board
		tmp = EXT_SEN_NUM_LUX;
		module_sensor_data_uart(tmp, lux);
		tmp = EXT_SEN_NUM_UVI;
		module_sensor_data_uart(tmp, uvi);
		// ext sensors data get & uart send

		// LEN-CMD-LABEL
		// 3-7-ff
		tmp = 0xff;
		module_sensor_data_uart(tmp, 0);
	}
}


/* @ Send label(indicate external sensor number) and sensor data to bottom board through UART.
 * @ updated : 2020.02.17
 * @ return void
 * @ param label : indicate ext sensor number
 * 		if EXT_SEN_NUM_LUX : Lux data (SI1133)
 * 		if EXT_SEN_NUM_UVI : UVI data (SI1133)
 * @ param data : sensor value (maximum 32 bits) */
void module_sensor_data_uart(uint8_t label, int32_t data) {
	// LEN-CMD-LABEL-DATA1-DATA2-DATA3-DATA4

	app_uart_put(LEN_EXT);
	app_uart_put(CMD_EXT);
	app_uart_put(label);

	for(int8_t i = 3; i >= 0; i--){
		app_uart_put( (data >> (8*i)) & 0xff );
	}
	task_sleepms(10);
}























