/*
 * GT_i2c.c
 *
 *  Created on: 2019. 7. 19.
 *      Author: KEUMSS
 */

#include <stdio.h>
#include <stdbool.h>

// ubinos library include
#include "bsp.h"
#include "ubinos.h"

#include "nrf_drv_twi.h"
#include "app_util_platform.h"

#include "hw_config.h"

#include "GT_i2c.h"

static nrf_drv_twi_t twi_master = NRF_DRV_TWI_INSTANCE(0);

static mutex_pt GT_i2c_mutex;

//
void GT_i2c_init(void) {

	ret_code_t err_code;

	nrf_drv_twi_config_t twi_master_config = NRF_DRV_TWI_DEFAULT_CONFIG;

	twi_master_config.scl = PIN_SCL;
	twi_master_config.sda = PIN_SDA;
	twi_master_config.frequency = NRF_TWI_FREQ_400K;

	err_code = nrf_drv_twi_init(&twi_master, &twi_master_config, NULL, NULL);
	APP_ERROR_CHECK(err_code);

	nrf_drv_twi_enable(&twi_master);

	int r;
	r = mutex_create(&GT_i2c_mutex);
	if (0 != r) {
		logme("fail at mutex_create");
	}

}
void GT_i2c_uninit(void) {
	nrf_drv_twi_uninit(&twi_master);
}

//void i2cm_uninit(void);
unsigned int GT_i2c_read(uint8_t sAddr, uint8_t *wbuf, uint32_t wlen, uint8_t *rbuf, uint32_t rlen) {

	ret_code_t err_code;

	mutex_lock(GT_i2c_mutex);
	/* if do not initialize i2c master, initialize i2c master */
	err_code = nrf_drv_twi_tx(&twi_master, sAddr, wbuf, wlen, true);		// wbuf에 주소
	if(err_code != NRF_SUCCESS) {
		mutex_unlock(GT_i2c_mutex);
		return err_code;
	}
	err_code = nrf_drv_twi_rx(&twi_master, sAddr, rbuf, rlen);
	if(err_code != NRF_SUCCESS) {
		mutex_unlock(GT_i2c_mutex);
		return err_code;
	}

	mutex_unlock(GT_i2c_mutex);

	return err_code;
}

unsigned int GT_i2c_write(uint8_t sAddr, uint8_t *wbuf, uint32_t wlen) {

	ret_code_t err_code;

	mutex_lock(GT_i2c_mutex);

	err_code = nrf_drv_twi_tx(&twi_master, sAddr, wbuf, wlen, false);			// 인스턴스, 슬레이브 id, wbuf - 주소 + data
	if(err_code != NRF_SUCCESS) {
		mutex_unlock(GT_i2c_mutex);
		return err_code;
	}

	mutex_unlock(GT_i2c_mutex);

	return err_code;
}


