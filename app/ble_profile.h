/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/*
 * ble_profile.h
 *
 *  Created on: 2020. 06. 11.
 *      Author: YJPark
 */

#ifndef BLE_PROFILE_H__
#define BLE_PROFILE_H__

#include "ble.h"
#include "ble_srv_common.h"
#include "ble_gatt.h"

#include <stdint.h>
#include <stdbool.h>

//todo GATT MTU Size 확인
#define BLE_PAAR_MAX_DATA_LEN (BLE_GATT_ATT_MTU_DEFAULT - 3) /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */

/* Forward declaration of the ble_paar_service_t type. */
typedef struct ble_paar_s ble_paar_t;

/**@brief Nordic UART Service event handler type. */
typedef void (*ble_paar_data_handler_t) (ble_paar_t * p_nus, uint8_t * p_data, uint16_t length);

/**@brief Nordic UART Service initialization structure.
 *
 * @details This structure contains the initialization information for the service. The application
 * must fill this structure and pass it to the service using the @ref ble_nus_init
 *          function.
 */
typedef struct
{
	ble_paar_data_handler_t data_handler; /**< Event handler to be called for handling received data. */
} ble_paar_init_t;

/**@brief Nordic UART Service structure.
 *
 * @details This structure contains status information related to the service.
 */
struct ble_paar_s
{
    uint8_t                  uuid_type;               /**< UUID type for Nordic UART Service Base UUID. */
    uint16_t                 service_handle;          /**< Handle of Nordic UART Service (as provided by the SoftDevice). */
    ble_gatts_char_handles_t rx_handles;              /**< Handles related to the RX characteristic (as provided by the SoftDevice). */
    ble_gatts_char_handles_t tx_handles;              /**< Handles related to the TX characteristic (as provided by the SoftDevice). */
    uint16_t                 conn_handle;             /**< Handle of the current connection (as provided by the SoftDevice). BLE_CONN_HANDLE_INVALID if not in a connection. */
    bool                     is_notification_enabled; /**< Variable to indicate if the peer has enabled notification of the RX characteristic.*/
    ble_paar_data_handler_t   data_handler;            /**< Event handler to be called for handling received data. */
};

/**@brief Function for initializing the Nordic UART Service.
 *
 * @param[out] p_nus      Nordic UART Service structure. This structure must be supplied
 *                        by the application. It is initialized by this function and will
 *                        later be used to identify this particular service instance.
 * @param[in] p_nus_init  Information needed to initialize the service.
 *
 * @retval NRF_SUCCESS If the service was successfully initialized. Otherwise, an error code is returned.
 * @retval NRF_ERROR_NULL If either of the pointers p_nus or p_nus_init is NULL.
 */
uint32_t ble_paar_service_init(ble_paar_t * p_nus, const ble_paar_init_t * p_nus_init);

/**@brief Function for handling the Nordic UART Service's BLE events.
 *
 * @details The Nordic UART Service expects the application to call this function each time an
 * event is received from the SoftDevice. This function processes the event if it
 * is relevant and calls the Nordic UART Service event handler of the
 * application if necessary.
 *
 * @param[in] p_nus       Nordic UART Service structure.
 * @param[in] p_ble_evt   Event received from the SoftDevice.
 */
void ble_nus_on_ble_evt(ble_paar_t * p_nus, ble_evt_t * p_ble_evt);

/**@brief Function for sending a string to the peer.
 *
 * @details This function sends the input string as an RX characteristic notification to the
 *          peer.
 *
 * @param[in] p_nus       Pointer to the Nordic UART Service structure.
 * @param[in] p_string    String to be sent.
 * @param[in] length      Length of the string.
 *
 * @retval NRF_SUCCESS If the string was sent successfully. Otherwise, an error code is returned.
 */
uint32_t ble_paar_ble_send_msg(ble_paar_t * p_paar_s, uint8_t * msg, uint16_t length);

#define PAAR_MAXIMUM_PACKET_SIZE	19

#endif // BLE_NUS_H__

/** @} */
