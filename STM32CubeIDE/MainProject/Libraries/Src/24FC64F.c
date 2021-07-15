#include "24FC64F.h"

void ee24fc64_multi_write(uint8_t *arr, uint8_t amount, uint16_t address){
	if(amount > 32){return;}

	LL_I2C_HandleTransfer(I2C1, EEPROM_ADDRESS, LL_I2C_ADDRSLAVE_7BIT, amount+2, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

	for(int i=1;i>=0;i--){
		while(!(LL_I2C_IsActiveFlag_TXIS(I2C1) || LL_I2C_IsActiveFlag_NACK(I2C1)));
		if(LL_I2C_IsActiveFlag_NACK(I2C1)){
			LL_I2C_ClearFlag_NACK(I2C1);
			return;
		}
		LL_I2C_TransmitData8(I2C1, (uint8_t)(address >> (8*i)));
	}

	for(int i=0;i<amount;i++){
		while(!(LL_I2C_IsActiveFlag_TXIS(I2C1) || LL_I2C_IsActiveFlag_NACK(I2C1)));
		if(LL_I2C_IsActiveFlag_NACK(I2C1)){
			LL_I2C_ClearFlag_NACK(I2C1);
			return;
		}
		LL_I2C_TransmitData8(I2C1, arr[i]);
	}

//	while(LL_I2C_IsActiveFlag_STOP(I2C1));


	LL_mDelay(20);
}

void ee24fc64_byte_write(uint8_t data, uint16_t address){
	LL_I2C_HandleTransfer(I2C1, EEPROM_ADDRESS, LL_I2C_ADDRSLAVE_7BIT, 3, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

	for(int i=1;i>=0;i--){
		while(!(LL_I2C_IsActiveFlag_TXIS(I2C1) || LL_I2C_IsActiveFlag_NACK(I2C1)));
		if(LL_I2C_IsActiveFlag_NACK(I2C1)){
			LL_I2C_ClearFlag_NACK(I2C1);
			return;
		}
		LL_I2C_TransmitData8(I2C1, (uint8_t)(address >> (8*i)));
	}

	while(!(LL_I2C_IsActiveFlag_TXIS(I2C1) || LL_I2C_IsActiveFlag_NACK(I2C1)));
	if(LL_I2C_IsActiveFlag_NACK(I2C1)){
		LL_I2C_ClearFlag_NACK(I2C1);
		return;
	}
	LL_I2C_TransmitData8(I2C1, data);

//	while(LL_I2C_IsActiveFlag_STOP(I2C1));


	LL_mDelay(20);
}

void ee24fc64_multi_read_read(uint8_t *arr, uint8_t amount, uint16_t start_address){
	LL_I2C_HandleTransfer(I2C1, EEPROM_ADDRESS, LL_I2C_ADDRSLAVE_7BIT, 2, LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_WRITE);
	for(int i=1;i>=0;i--){
		while(!(LL_I2C_IsActiveFlag_TXIS(I2C1) || LL_I2C_IsActiveFlag_NACK(I2C1)));
		if(LL_I2C_IsActiveFlag_NACK(I2C1)){
			LL_I2C_ClearFlag_NACK(I2C1);
			return;
		}
		LL_I2C_TransmitData8(I2C1, (uint8_t)(start_address >> (8*i)));
	}
	while(!LL_I2C_IsActiveFlag_TC(I2C1));

	LL_I2C_HandleTransfer(I2C1, EEPROM_ADDRESS, LL_I2C_ADDRSLAVE_7BIT, amount, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_RESTART_7BIT_READ);
	for(int i=0;i<amount;i++){
		while(!LL_I2C_IsActiveFlag_RXNE(I2C1));
		arr[i] = LL_I2C_ReceiveData8(I2C1);
	}

//	while(LL_I2C_IsActiveFlag_STOP(I2C1));
}

int ee24fc64_byte_read(uint16_t start_address){
	uint8_t ret;
	LL_I2C_HandleTransfer(I2C1, EEPROM_ADDRESS, LL_I2C_ADDRSLAVE_7BIT, 2, LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_WRITE);
	for(int i=1;i>=0;i--){
		while(!(LL_I2C_IsActiveFlag_TXIS(I2C1) || LL_I2C_IsActiveFlag_NACK(I2C1)));
		if(LL_I2C_IsActiveFlag_NACK(I2C1)){
			LL_I2C_ClearFlag_NACK(I2C1);
			return 0;
		}
		LL_I2C_TransmitData8(I2C1, (uint8_t)(start_address >> (8*i)));
	}
	while(!LL_I2C_IsActiveFlag_TC(I2C1));

	LL_I2C_HandleTransfer(I2C1, EEPROM_ADDRESS, LL_I2C_ADDRSLAVE_7BIT, 1, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_RESTART_7BIT_READ);
	while(!LL_I2C_IsActiveFlag_RXNE(I2C1));
	ret = LL_I2C_ReceiveData8(I2C1);

//	while(LL_I2C_IsActiveFlag_STOP(I2C1));
	return ret;
}
