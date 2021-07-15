/*
 * 24FC64F.h
 *
 *  Created on: Jan 28, 2021
 *      Author: electro
 */

#ifndef INC_24FC64F_H_
#define INC_24FC64F_H_

#include "main.h"

#define EEPROM_ADDRESS 0b10100000

#define I2C_REQUEST_WRITE 0
#define I2C_REQUEST_READ 1

void ee24fc64_multi_write(uint8_t *arr, uint8_t amount, uint16_t address);
void ee24fc64_byte_write(uint8_t data, uint16_t address);

void ee24fc64_multi_read_read(uint8_t *arr, uint8_t amount, uint16_t start_address);
int ee24fc64_byte_read(uint16_t start_address);

#endif /* INC_24FC64F_H_ */
