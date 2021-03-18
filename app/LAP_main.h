/*
 * LAP_main.h
 *
 *  Created on: 2020. 06. 11.
 *      Author: YJPark
 */

#ifndef APPLICATION_GASTAG_EXE_GASTAG_SRC_LAP_MAIN_H_
#define APPLICATION_GASTAG_EXE_GASTAG_SRC_LAP_MAIN_H_

typedef struct
{
	uint8_t event;
	uint8_t status;
	uint16_t conn_handle;
	uint16_t handle;
	uint32_t msg_len;
	uint8_t* msg;
}LAPEvt_msgt;

enum
{
	LAP_CENTRAL_EVT = 0,
	LAP_PERIPHERAL_EVT,
	LAP_LIDx_EVT,
	LAP_PNIP_EVT,
	LAP_AMD_EVT,
};

enum{
	LAP_CENTRAL_ST_SCAN_RESULT = 0,
	LAP_CENTRAL_ST_SCAN_TIMEOUT,
	LAP_CENTRAL_ST_CONN_TIMEOUT,
	LAP_CENTRAL_ST_SCAN_ADV_REPORT,
	LAP_CENTRAL_ST_CONNECTED,
	LAP_CENTRAL_ST_DISCONNECTED,
	LAP_CENTRAL_ST_DATA_RECEIVED,
};

enum
{
	LAP_PERIPHERAL_ST_CONNECTED = 0,
	LAP_PERIPHERAL_ST_DISCONNECTED,
	LAP_PERIPHERAL_ST_DATA_RECEIVED,
};

enum{
	LAP_EVT_LIDX = 0,
	LAP_EVT_PNIP,
	LAP_EVT_AMD,

	EC_EVT_CENTRAL,
	EC_EVT_PERIPHERAL,
};

enum{
	EC_STATUS_ADV_REPORT,
	EC_STATUS_CONNECTED,
	EC_STATUS_DISCONNECTED,

	EC_STATUS_DATA_RECEIVE,
	EC_STATUS_ENTRANCE_INFO,

};

#define LAP_EVENT_HANDLE_NULL				0
#define LAP_EVENT_MSG_LEN_NULL				0

void LAP_main_task_init(void);

int LAP_event_send(uint8_t evt, uint8_t state, uint16_t conn_handle, uint16_t handle,
									uint32_t msg_len, uint8_t* msg);


/* @ Send 1 byte to the server.
 * @ updated : 2021.02.16
 * @ return void
 * @ param ack_sig : Byte to be sent to the server */
void BLE_send_short(uint8_t ack_sig);


/* @ Send sensor values and act history received from the bottom board to the server.
 * @ updated : 2021.02.16
 * @ return void
 * @ param sensor_data1 : Sensor data obtained from sensor port 0 on the bottom board
 * @ param sensor_data2 : Sensor data obtained from sensor port 1 on the bottom board
 * @ param sensor_data3 : Sensor data obtained from sensor port 2 on the bottom board
 * @ param sensor_data4 : Sensor data obtained from sensor port 3 on the bottom board
 * @ param acted : Actuator act history. Each bit represents each actuator number. If actuator 0 was acted before these values obtained, 0000 0001. */
void BLE_send_res(uint16_t sensor_data1, uint16_t sensor_data2, uint16_t sensor_data3, uint16_t sensor_data4, uint8_t acted);


/* @ Send sensor values obtained from SI1133.
 * @ updated : 2021.02.16
 * @ return void
 * @ param lux : Brightness obtained from SI1133 [lux]
 * @ param uvi : UV index obtained from SI1133 [uW/m^2] */
void BLE_send_light(int32_t lux, int32_t uvi);


/* @ Send local clock (bottom board RTC time)
 * @ updated : 2021.02.16
 * @ return void
 * @ param hour : local hour
 * @ param min : local minute */
void BLE_send_time(uint8_t hour, uint8_t min);

#endif /* APPLICATION_GASTAG_EXE_GASTAG_SRC_LAP_MAIN_H_ */
