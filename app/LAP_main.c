/*
 * LAP_main.c
 *
 *  Created on: 2020. 06. 11.
 *      Author: YJPark
 */


#include <ubinos.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <malloc.h>

#include "SFU_main.h"

#include "ble_stack.h"
#include "LAP_api.h"
#include "LAP_main.h"
#include "ble_gap.h"
#include "ble_process.h"
#include "ble_profile.h"

#include "sw_config.h"
#include "hw_config.h"



//----------------------------------------------------------------------------
#define DEVICE_ID				0x99




#define PAC_TYPE_INDEX			0
#define PAC_SERVICE_INDEX		1
#define PAC_SEQ_INDEX			2
#define PAC_LEN_INDEX			3
#define PAC_CMD_INDEX			4
#define PAC_ID_INDEX			5

// CMD OPT1
#define PAC_PROFILE_INDEX		6
#define PAC_MAPPING_INDEX		7
#define PAC_THRES1_INDEX		8
#define PAC_THRES2_INDEX		9
#define PAC_THRES3_INDEX		10
#define PAC_THRES4_INDEX		11
#define PAC_TRIG_INDEX			12

// CMD RES
#define PAC_SENSOR_DATA1		6
#define PAC_SENSOR_DATA2		7
#define PAC_SENSOR_DATA3		8
#define PAC_SENSOR_DATA4		9
#define PAC_SENSOR_DATA5		10
#define PAC_SENSOR_DATA6		11
#define PAC_SENSOR_DATA7		12
#define PAC_SENSOR_DATA8		13
#define PAC_ACTED				14

// CMD UVI
#define PAC_LUX_DATA1			6
#define PAC_LUX_DATA2			7
#define PAC_LUX_DATA3			8
#define PAC_LUX_DATA4			9
#define PAC_UVI_DATA1			10
#define PAC_UVI_DATA2			11
#define PAC_UVI_DATA3			12
#define PAC_UVI_DATA4			13

// CMD_LOCAL_CLK
#define PAC_LOCAL_HOUR			6
#define PAC_LOCAL_MIN			7

// CMD_CLK_REQ_RPLY
#define PAC_CLK_RPLY_HOUR		6
#define PAC_CLK_RPLY_MIN		7

// CMD SCHEDULE
#define PAC_START_HOUR			6
#define PAC_START_MIN			7
#define PAC_END_HOUR			8
#define PAC_END_MIN				9
#define PAC_ACTUATOR			10
#define PAC_SCH_NUM				11


#define PAC_TYPE 				0x88
#define SERVICE_ID				0x17
#define SEQ_NUM					0x11


#define LEN_OPT1				0x09
#define LEN_OPT2				0x05
#define LEN_OPT3				0x06
#define LEN_ACT_REQ				0x05
#define LEN_RES					0x0B
#define LEN_ACK					0x01
#define LEN_SEN_REQ				0x02
#define LEN_UVI					0x0A
#define LEN_LOCAL_CLK			0x04
#define LEN_LOCAL_CLK_REQ		0x02
#define LEN_CLK_REQ_RPLY		0x04
#define LEN_ACT_SCH				0x08


#define CMD_OPT1				0x01
#define CMD_OPT2				0x02
#define CMD_OPT3				0x03
#define CMD_ACT_REQ				0x04
#define CMD_RES					0x05
#define CMD_SEN_REQ				0x06
#define CMD_UVI					0x07
#define CMD_LOCAL_CLK			0x08
#define CMD_LOCAL_CLK_REQ		0x09
#define CMD_CLK_REQ_RPLY		0x0A
#define CMD_ACT_SCH				0x0B


#define CMD_OPT1_ACK				0x01
#define CMD_OPT2_ACK				0x02
#define CMD_OPT3_ACK				0x03
#define CMD_ACT_REQ_ACK				0x04
#define CMD_RES_ACK					0x05





#define MAX_REPLY_COUNT				5


//----------------------------------------------------------------------------


uint8_t cmd_len[] = { 0,
		LEN_OPT1, LEN_OPT2, LEN_OPT3,
		LEN_ACT_REQ, LEN_RES, LEN_SEN_REQ, 0,
		LEN_LOCAL_CLK, LEN_LOCAL_CLK_REQ, 0,
		LEN_ACT_SCH,
};
uint8_t evt[] = {0,
		SFU_EVT_BLE_OPT_MAP,
		SFU_EVT_BLE_OPT_ACTUATOR,
		SFU_EVT_BLE_OPT_PERIOD,
		SFU_EVT_BLE_ACT_REQ,
		SFU_EVT_RES,
		SFU_EVT_BLE_SEN_REQ,
		0,
		SFU_EVT_LOCAL_CLK_SET,
		SFU_EVT_LOCAL_CLK_REQ,
		0,
		SFU_EVT_TIME_SCHEDULE,
};


uint16_t test_target_conn_handle = BLE_CONN_HANDLE_INVALID;
paar_uuidhandle test_target_uuid_handle;
uint8_t test_send_count = 0;

static msgq_pt LAP_msgq;

APP_TIMER_DEF(scan_fail_timeout_timer);

uint8_t test_paar_id[4] = {TEST_TARTGET_PAAR_ID0, TEST_TARTGET_PAAR_ID1,
									TEST_TARTGET_PAAR_ID2, TEST_TARTGET_PAAR_ID3};

static void send_test_msg_central(uint16_t conn_handle, uint16_t handle)
{
	uint8_t* temp_packet;

	temp_packet = (uint8_t*)malloc(PAAR_MAXIMUM_PACKET_SIZE);

	if(temp_packet != NULL)
	{
		memset(temp_packet, 0, PAAR_MAXIMUM_PACKET_SIZE);
		temp_packet[0] = test_send_count++;
		if(test_send_count >= 4)
			test_send_count = 1;

		printf("BLE send msg : test_msg %d\r\n", temp_packet[0]);

		LAP_send_ble_msg_central(conn_handle, handle, temp_packet, PAAR_MAXIMUM_PACKET_SIZE);
	}
}

//---------------------------------------------------------------------
void BLE_send_short(uint8_t sig)
{
	uint8_t* temp_packet;

	temp_packet = (uint8_t*)malloc(PAAR_MAXIMUM_PACKET_SIZE);

	if(temp_packet != NULL)
	{
		memset(temp_packet, 0, PAAR_MAXIMUM_PACKET_SIZE);
		temp_packet[PAC_TYPE_INDEX] 	= PAC_TYPE;
		temp_packet[PAC_SERVICE_INDEX] 	= SERVICE_ID;
		temp_packet[PAC_SEQ_INDEX] 		= SEQ_NUM;
		temp_packet[PAC_LEN_INDEX] 		= LEN_ACK;

		temp_packet[PAC_CMD_INDEX] 		= sig;
		printf("Short msg : %X\r\n", temp_packet[PAC_CMD_INDEX]);

		task_sleep(TEST_SEND_MSG_DELAY);

		LAP_send_ble_msg_peripheral(temp_packet, PAAR_MAXIMUM_PACKET_SIZE);
	}

}



void BLE_send_light(int32_t lux, int32_t uvi){
	uint8_t* temp_packet;

	temp_packet = (uint8_t*)malloc(PAAR_MAXIMUM_PACKET_SIZE);

	if(temp_packet != NULL)
	{
		memset(temp_packet, 0, PAAR_MAXIMUM_PACKET_SIZE);
		temp_packet[PAC_TYPE_INDEX] 	= PAC_TYPE;
		temp_packet[PAC_SERVICE_INDEX] 	= SERVICE_ID;
		temp_packet[PAC_SEQ_INDEX] 		= SEQ_NUM;
		temp_packet[PAC_LEN_INDEX] 		= LEN_UVI;
		temp_packet[PAC_CMD_INDEX] 		= CMD_UVI;
		temp_packet[PAC_ID_INDEX]		= DEVICE_ID;


		temp_packet[PAC_LUX_DATA1]	= (lux >> 24) & 0xff;
		temp_packet[PAC_LUX_DATA2]	= (lux >> 16) & 0xff;
		temp_packet[PAC_LUX_DATA3]	= (lux >> 8	) & 0xff;
		temp_packet[PAC_LUX_DATA4]	= (lux		) & 0xff;

		temp_packet[PAC_UVI_DATA1]	= (uvi >> 24) & 0xff;
		temp_packet[PAC_UVI_DATA2]	= (uvi >> 16) & 0xff;
		temp_packet[PAC_UVI_DATA3]	= (uvi >> 8	) & 0xff;
		temp_packet[PAC_UVI_DATA4]	= (uvi		) & 0xff;


		task_sleep(TEST_SEND_MSG_DELAY);

		LAP_send_ble_msg_peripheral(temp_packet, PAAR_MAXIMUM_PACKET_SIZE);
	}
}


void BLE_send_res(uint16_t sensor_data1, uint16_t sensor_data2, uint16_t sensor_data3, uint16_t sensor_data4, uint8_t acted){
	uint8_t* temp_packet;

	temp_packet = (uint8_t*)malloc(PAAR_MAXIMUM_PACKET_SIZE);

	if(temp_packet != NULL)
	{
		memset(temp_packet, 0, PAAR_MAXIMUM_PACKET_SIZE);
		temp_packet[PAC_TYPE_INDEX] 	= PAC_TYPE;
		temp_packet[PAC_SERVICE_INDEX] 	= SERVICE_ID;
		temp_packet[PAC_SEQ_INDEX] 		= SEQ_NUM;
		temp_packet[PAC_LEN_INDEX] 		= LEN_RES;
		temp_packet[PAC_CMD_INDEX] 		= CMD_RES;
		temp_packet[PAC_ID_INDEX]		= DEVICE_ID;

		temp_packet[PAC_SENSOR_DATA1]	= (sensor_data1 >> 8) & 0xff;
		temp_packet[PAC_SENSOR_DATA2]	= sensor_data1 & 0xff;
		temp_packet[PAC_SENSOR_DATA3]	= (sensor_data2 >> 8) & 0xff;
		temp_packet[PAC_SENSOR_DATA4]	= sensor_data2 & 0xff;
		temp_packet[PAC_SENSOR_DATA5]	= (sensor_data3 >> 8) & 0xff;
		temp_packet[PAC_SENSOR_DATA6]	= sensor_data3 & 0xff;
		temp_packet[PAC_SENSOR_DATA7]	= (sensor_data4 >> 8) & 0xff;
		temp_packet[PAC_SENSOR_DATA8]	= sensor_data4 & 0xff;

		temp_packet[PAC_ACTED]			= acted;


		task_sleep(TEST_SEND_MSG_DELAY);

		LAP_send_ble_msg_peripheral(temp_packet, PAAR_MAXIMUM_PACKET_SIZE);
	}


}

void BLE_send_time(uint8_t hour, uint8_t min){
	uint8_t* temp_packet;

	temp_packet = (uint8_t*)malloc(PAAR_MAXIMUM_PACKET_SIZE);

	if(temp_packet != NULL)
	{
		memset(temp_packet, 0, PAAR_MAXIMUM_PACKET_SIZE);
		temp_packet[PAC_TYPE_INDEX] 	= PAC_TYPE;
		temp_packet[PAC_SERVICE_INDEX] 	= SERVICE_ID;
		temp_packet[PAC_SEQ_INDEX] 		= SEQ_NUM;
		temp_packet[PAC_LEN_INDEX] 		= LEN_CLK_REQ_RPLY;
		temp_packet[PAC_CMD_INDEX] 		= CMD_CLK_REQ_RPLY;
		temp_packet[PAC_ID_INDEX]		= DEVICE_ID;


		temp_packet[PAC_CLK_RPLY_HOUR] 		= hour;
		temp_packet[PAC_CLK_RPLY_MIN] 		= min;


		task_sleep(TEST_SEND_MSG_DELAY);

		LAP_send_ble_msg_peripheral(temp_packet, PAAR_MAXIMUM_PACKET_SIZE);
	}

}

//---------------------------------------------------------------------


static void send_cccd_handle_enable(uint16_t conn_handle, uint16_t cccd_handle)
{
	uint8_t* temp_packet;

	temp_packet = (uint8_t*)malloc(2);

	memset(temp_packet, 0, 2);

	temp_packet[0] = NRF_NOTI_INDI_ENABLE;		// ble notification msg �뜲�씠�꽣
	temp_packet[1] = 0x00;

	printf("BLE send msg : CCCD enable\r\n");

	LAP_send_ble_msg_central(conn_handle, cccd_handle, temp_packet, 2);

}

static void processing_LAP_Central_Conn_timeout(LAPEvt_msgt LAP_evt_msg)
{
	printf("Connection timeout\r\n");
	printf("BLE SCAN start\r\n");
	uint8_t* temp_test_paar_id;

	temp_test_paar_id = (uint8_t*)malloc(TEST_TARTGET_PAAR_ID_SIZE);

	temp_test_paar_id[0] = TEST_TARTGET_PAAR_ID0;
	temp_test_paar_id[1] = TEST_TARTGET_PAAR_ID1;
	temp_test_paar_id[2] = TEST_TARTGET_PAAR_ID2;
	temp_test_paar_id[3] = TEST_TARTGET_PAAR_ID3;

	task_sleep(TEST_SCAN_START_DELAY);

	printf("BLE SCAN start\r\n");
	LAP_start_ble_scan(temp_test_paar_id);
}

static void processing_LAP_Central_Scan_timeout(LAPEvt_msgt LAP_evt_msg)
{
	printf("BLE SCAN timeout\r\n");

	uint8_t* temp_test_paar_id;

	temp_test_paar_id = (uint8_t*)malloc(TEST_TARTGET_PAAR_ID_SIZE);

	temp_test_paar_id[0] = TEST_TARTGET_PAAR_ID0;
	temp_test_paar_id[1] = TEST_TARTGET_PAAR_ID1;
	temp_test_paar_id[2] = TEST_TARTGET_PAAR_ID2;
	temp_test_paar_id[3] = TEST_TARTGET_PAAR_ID3;

	task_sleep(TEST_SCAN_START_DELAY);

	printf("BLE SCAN start\r\n");
	LAP_start_ble_scan(temp_test_paar_id);
}

static void processing_LAP_Central_Scan_result(LAPEvt_msgt LAP_evt_msg)
{
	ble_gap_evt_adv_report_t* target_adv_pkt;

	target_adv_pkt = PAAR_get_ble_scan_target_result();

	//save cccd/rx/tx handle
	LAP_save_uuid_handle((ble_gap_evt_adv_report_t*) target_adv_pkt, &test_target_uuid_handle);

	task_sleep(TEST_CONNECTION_DELAY);

	printf("BLE Connection : test Target\r\n");
	//start ble connection to target PAAR ID device
	LAP_start_ble_connect((ble_gap_evt_adv_report_t*) target_adv_pkt);
}

static void processing_LAP_Central_Connected(LAPEvt_msgt LAP_evt_msg)
{
	printf("BLE Central connect\r\n");

	uuidhandle temp_uuid_handle;

	int r;

	//save test_connection_handle
	test_target_conn_handle = LAP_evt_msg.conn_handle;

	task_sleep(TEST_SEND_MSG_DELAY);

	//send cccd enable
	send_cccd_handle_enable(test_target_conn_handle, test_target_uuid_handle.cccd_handle);

	task_sleep(TEST_SEND_MSG_DELAY);

	//send test msg
	send_test_msg_central(test_target_conn_handle, test_target_uuid_handle.rx_handle);
}

static void processing_LAP_Central_Disconnected(LAPEvt_msgt LAP_evt_msg)
{
	printf("BLE central disconnect\r\n");

	uint8_t* temp_test_paar_id;

	temp_test_paar_id = (uint8_t*)malloc(TEST_TARTGET_PAAR_ID_SIZE);

	temp_test_paar_id[0] = TEST_TARTGET_PAAR_ID0;
	temp_test_paar_id[1] = TEST_TARTGET_PAAR_ID1;
	temp_test_paar_id[2] = TEST_TARTGET_PAAR_ID2;
	temp_test_paar_id[3] = TEST_TARTGET_PAAR_ID3;

	task_sleep(TEST_SCAN_START_DELAY);

	printf("BLE SCAN start\r\n");
	LAP_start_ble_scan(temp_test_paar_id);
}

static void processing_LAP_Central_Data_Received(LAPEvt_msgt LAP_evt_msg)
{
	printf("BLE msg rv : %d\r\n", LAP_evt_msg.msg[0]);

	task_sleep(TEST_SEND_MSG_DELAY);
}

static void processing_LAP_Peripheral_Connected(LAPEvt_msgt LAP_evt_msg)
{
	printf("BLE Peripheral connect\r\n");
	test_target_conn_handle = LAP_evt_msg.conn_handle;
}

static void processing_LAP_Peripheral_Disconnected(LAPEvt_msgt LAP_evt_msg)
{

	task_sleep(TEST_ADV_START_DELAY);
	printf("BLE Peripheral disconnect\r\n");

	printf("BLE ADV start\r\n");
	LAP_start_ble_adv_LIDx();
}





static void processing_LAP_Peripheral_Data_Received(LAPEvt_msgt LAP_evt_msg)
{

	if(LAP_evt_msg.msg[0] != PAC_TYPE || LAP_evt_msg.msg[PAC_ID_INDEX] != DEVICE_ID) return;

	uint8_t* msg_for_main;


	msg_for_main = (uint8_t*)malloc( cmd_len[LAP_evt_msg.msg[PAC_CMD_INDEX]] );
	msg_for_main[0] = cmd_len[ LAP_evt_msg.msg[PAC_CMD_INDEX] ];
	msg_for_main[1] = LAP_evt_msg.msg[PAC_CMD_INDEX];




	memcpy(msg_for_main + 2, LAP_evt_msg.msg + 6, msg_for_main[0] - 2);


	SFU_event_send(evt[msg_for_main[1]], msg_for_main[0], msg_for_main);




/*
	switch(LAP_evt_msg.msg[PAC_CMD_INDEX]){

	case CMD_OPT1 :
		SFU_event_send(SFU_EVT_BLE_OPT1, msg_for_main[0], msg_for_main);
		break;
	case CMD_OPT2 :

	default : break;
	}
*/

	task_sleep(TEST_SEND_MSG_DELAY);
}







static void processing_LAP_Central_event(LAPEvt_msgt LAP_evt_msg)
{
	switch(LAP_evt_msg.status)
	{
	case LAP_CENTRAL_ST_SCAN_TIMEOUT :
		processing_LAP_Central_Scan_timeout(LAP_evt_msg);
		break;
	case LAP_CENTRAL_ST_CONN_TIMEOUT :
		processing_LAP_Central_Conn_timeout(LAP_evt_msg);
		break;
	case LAP_CENTRAL_ST_SCAN_RESULT :
		processing_LAP_Central_Scan_result(LAP_evt_msg);
		break;
	case LAP_CENTRAL_ST_CONNECTED :
		processing_LAP_Central_Connected(LAP_evt_msg);
		break;
	case LAP_CENTRAL_ST_DISCONNECTED :
		processing_LAP_Central_Disconnected(LAP_evt_msg);
		break;
	case LAP_CENTRAL_ST_DATA_RECEIVED :
		processing_LAP_Central_Data_Received(LAP_evt_msg);
		break;
	}
}

static void processing_LAP_Peripheral_event(LAPEvt_msgt LAP_evt_msg)
{
	switch(LAP_evt_msg.status)
	{
	case LAP_PERIPHERAL_ST_CONNECTED :
		processing_LAP_Peripheral_Connected(LAP_evt_msg);
		break;
	case LAP_PERIPHERAL_ST_DISCONNECTED :
		processing_LAP_Peripheral_Disconnected(LAP_evt_msg);
		break;
	case LAP_PERIPHERAL_ST_DATA_RECEIVED :
		processing_LAP_Peripheral_Data_Received(LAP_evt_msg);
		break;
	}
}

void processing_LAP_LIDx_event(LAPEvt_msgt LAP_evt_msg)
{

}

void processing_LAP_PNIP_event(LAPEvt_msgt LAP_evt_msg)
{

}

void processing_LAP_AMD_event(LAPEvt_msgt LAP_evt_msg)
{

}

void LAP_Protocol_start_operation()
{

#if(HAT_SW_MODE_SETUP == HAT_SW_MODE_TEST_CENTRAL)
	uint8_t* temp_test_paar_id;

	temp_test_paar_id = (uint8_t*)malloc(TEST_TARTGET_PAAR_ID_SIZE);

	temp_test_paar_id[0] = TEST_TARTGET_PAAR_ID0;
	temp_test_paar_id[1] = TEST_TARTGET_PAAR_ID1;
	temp_test_paar_id[2] = TEST_TARTGET_PAAR_ID2;
	temp_test_paar_id[3] = TEST_TARTGET_PAAR_ID3;

	printf("BLE SCAN start\r\n");
	LAP_start_ble_scan(temp_test_paar_id);
#elif(HAT_SW_MODE_SETUP == HAT_SW_MODE_TEST_PERIPHERAL)
	printf("BLE ADV start\r\n");
	task_sleep(TEST_ADV_START_DELAY);
	LAP_start_ble_adv_LIDx();
#endif
}

void scan_fail_timer_handler()
{
	/*
	LAP_start_ble_scan(NULL);

	app_timer_start(scan_fail_timeout_timer, APP_TIMER_TICKS(5000), NULL);
	*/
}


void LAP_main_task(void* arg){
	int r;
	LAPEvt_msgt LAP_evt_msg;

	ble_stack_init_wait();

	LAP_Protocol_start_operation();

	for (;;) {
		r = msgq_receive(LAP_msgq, (unsigned char*) &LAP_evt_msg);
		if (0 != r) {
			logme("fail at msgq_receive\r\n");
		} else {
			switch( LAP_evt_msg.event ){
			case LAP_CENTRAL_EVT :
				processing_LAP_Central_event(LAP_evt_msg);
				break;
			case LAP_PERIPHERAL_EVT :
				processing_LAP_Peripheral_event(LAP_evt_msg);
				break;


			case LAP_LIDx_EVT :
				processing_LAP_LIDx_event(LAP_evt_msg);
				break;
			case LAP_PNIP_EVT :
				processing_LAP_PNIP_event(LAP_evt_msg);
				break;
			case LAP_AMD_EVT :
				processing_LAP_AMD_event(LAP_evt_msg);
				break;
			}

			if( LAP_evt_msg.msg != NULL ){
				free(LAP_evt_msg.msg);
			}
		}
	}
}

void LAP_main_task_init(void){
	int r;

	r = msgq_create(&LAP_msgq, sizeof(LAPEvt_msgt), 20);
	if (0 != r) {
		printf("fail at msgq create\r\n");
	}

	r = task_create(NULL, LAP_main_task, NULL, task_gethighestpriority()-2, 512, NULL);
	if (r != 0) {
		printf("== LAP_main_task failed \n\r");
	} else {
		printf("== LAP_main_task created \n\r");
	}
}


int LAP_event_send(uint8_t evt, uint8_t state, uint16_t conn_handle, uint16_t handle,
															uint32_t msg_len, uint8_t* msg)
{
	LAPEvt_msgt lap_msg;

	lap_msg.event = evt;
	lap_msg.status = state;
	lap_msg.handle = handle;
	lap_msg.conn_handle = conn_handle;
	lap_msg.msg_len = msg_len;
	lap_msg.msg = msg;

	return msgq_send(LAP_msgq, (unsigned char*) &lap_msg);
}
