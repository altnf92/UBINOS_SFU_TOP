/*
 * GT_i2c.h
 *
 *  Created on: 2020. 9. 2.
 *      Author: khs
 */

#ifndef APP_GT_I2C_H_
#define APP_GT_I2C_H_




void GT_i2c_init(void);
void GT_i2c_uninit(void);



unsigned int GT_i2c_read(uint8_t sAddr, uint8_t *wbuf, uint32_t wlen, uint8_t *rbuf, uint32_t rlen);
unsigned int GT_i2c_write(uint8_t sAddr, uint8_t *wbuf, uint32_t wlen);



#endif /* APP_GT_I2C_H_ */
