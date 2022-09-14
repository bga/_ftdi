#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>
#include <ftMPSSE/libMPSSE_i2c.h>
#include "common.h"

val CHANNEL_TO_OPEN = 0;
val i2cAddress = 0x51;

#define DUMMY_NAME BGA__UNIQUE_NAME
struct PCF8563_RegesterMap {
	struct {
		uint8_t DUMMY_NAME: 3;
		
		//# 0 - Power-On Reset (POR) override facility is disabled; set to logic 0 for normal operation, 1 - Power-On Reset (POR) override may be enabled
		uint8_t TESTC_bit: 1;
		uint8_t DUMMY_NAME: 1;
		
		//# 0 - RTC source clock runs, 1 - all RTC divider chain flip-flops are asynchronously set to logic 0; the RTC clock is stopped (CLKOUT at 32.768 kHz is still available)
		uint8_t STOP_bit: 1;
		uint8_t DUMMY_NAME: 1;
		
		//# 0 - normal mode (must be set to logic 0 during normal operations), 1 - EXT_CLK test mode
		uint8_t TEST1_bit: 1;
	} Control_status_1;
	struct {
		uint8_t Control_status_2_TIE: 1;
		uint8_t Control_status_2_AIE: 1;
		uint8_t Control_status_2_TF: 1;
		uint8_t Control_status_2_AF: 1;
		uint8_t Control_status_2_TI_TP: 1;
		uint8_t DUMMY_NAME: 3;
	} Control_status_2;
	struct {
		uint8_t secondsCount: 7; //# 0 - 59
		uint8_t VL_bit: 1;
	} VL_seconds;
	struct {
		uint8_t munutesCount: 7; //# 0 - 59
		uint8_t DUMMY_NAME: 1;
	} minutesCountReg;
	struct {
		uint8_t hoursCount: 6; //# 0 - 24
		uint8_t DUMMY_NAME: 2;
	} hoursCountReg;
	struct {
		uint8_t daysCount: 6; //# 1 - 31
		uint8_t DUMMY_NAME: 2;
	} daysCountReg;
	struct {
		uint8_t weekdaysCount: 3; //# 1 - 6
		uint8_t DUMMY_NAME: 5;
	} weekdaysCountReg;
	struct {
		uint8_t monthCount: 4; //# 1 - 12
		uint8_t DUMMY_NAME: 3;
		uint8_t century_bit: 1;
	} monthCountReg;
	uint8_t yearsCount; //# 0 - 99
};

FT_HANDLE ftHandle = null;

#define APP_CHECK_STATUS(expArg) \
do { \
	val exp = (expArg); \
	if(exp != FT_OK) { \
		printf("%s:%d:%s(): status(0x%x) 	!= FT_OK\n", __FILE__, __LINE__, __FUNCTION__, (Int)(exp)); \
		exit(1); \
	} \
	else { \
	} \
} while(0)


FT_STATUS PCF8563_readTime(uint8 slaveAddress, uint8 registerAddress, uint8* data) {
	uint32 bytesToTransfer = 0;
	uint32 bytesTransfered;
	uint8 buffer[2];
	
	bytesToTransfer = bytesTransfered = 0;
	
	// Write address pointer
	buffer[bytesToTransfer++] = registerAddress;
	FT_STATUS status = I2C_DeviceWrite(
		ftHandle, 
		slaveAddress, 
		bytesToTransfer, 
		buffer, 
		&bytesTransfered, 
		I2C_TRANSFER_OPTIONS_START_BIT | I2C_TRANSFER_OPTIONS_STOP_BIT
	);
																		 
	//  printf("I2C_DeviceWrite status: %d, %d\n", status, FT_DEVICE_NOT_FOUND);
	//  APP_CHECK_STATUS(status);
	
	bytesToTransfer = 7;
	bytesTransfered = 0;
	FT_STATUS status2 = I2C_DeviceRead(
		ftHandle, 
		slaveAddress, 
		bytesToTransfer, 
		data,
		&bytesTransfered, I2C_TRANSFER_OPTIONS_START_BIT | /* I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES | */ I2C_TRANSFER_OPTIONS_STOP_BIT
	); // |I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BITS |I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE
																		
	//  printf("I2C_DeviceWrite status: %d, %d\n", status2, FT_INVALID_PARAMETER);
	
	//  APP_CHECK_STATUS(status);
	
	return status | status2;
}

FT_STATUS I2C_write(uint8 slaveAddress, uint8 registerAddress, uint8 data) {
	uint32 bytesToTransfer = 0;
	uint32 bytesTransfered;
	uint8 buffer[3];
	
	bytesToTransfer = bytesTransfered = 0;
	
	// Write address pointer
	buffer[bytesToTransfer++] = registerAddress;
	buffer[bytesToTransfer++] = data;
	return I2C_DeviceWrite(
						 ftHandle,
						 slaveAddress,
						 bytesToTransfer,
						 buffer, &bytesTransfered, I2C_TRANSFER_OPTIONS_START_BIT | I2C_TRANSFER_OPTIONS_STOP_BIT
				 );
}

BOOL WINAPI consoleHandler(DWORD signal) {
	if(signal == CTRL_C_EVENT) {
		printf("Ctrl-C handled\n"); // do cleanup
	}
	else {
	
	}
	
	exit(0);
	
	return TRUE;
}

uint8_t bcdToInt(uint8_t x) {
	return (x & 0x0F) + (x >> 4) * 10;
}

static_assert(offsetof(PCF8563_RegesterMap, VL_seconds) == 0x02);
static_assert(offsetof(PCF8563_RegesterMap, minutesCountReg) == 0x03);
static_assert(offsetof(PCF8563_RegesterMap, hoursCountReg) == 0x04);

int main() {
	if(!SetConsoleCtrlHandler(consoleHandler, TRUE)) {
		printf("\nERROR: Could not set control handler");
		return 1;
	}
	
	
	
	ChannelConfig channelConf = {
		.ClockRate = I2C_CLOCK_STANDARD_MODE, /*i.e. 100000 KHz*/
		.LatencyTimer = 10,
		.Options = I2C_DISABLE_3PHASE_CLOCKING /* | I2C_ENABLE_DRIVE_ONLY_ZERO */
	};
	
	
	uint32 channels = 0;
	APP_CHECK_STATUS(I2C_GetNumChannels(&channels));
	
	printf("Number of available I2C channels = %d\n", (Int)channels);
	
	if(channels > 0) {
		for(uint32 i = 0; i < channels; i++) {
			FT_DEVICE_LIST_INFO_NODE devList;
			APP_CHECK_STATUS(I2C_GetChannelInfo(i, &devList));
			
			printf("Information on channel number %d:\n", (Int)i);
			/*print the dev info*/
			printf("		Flags=0x%x\n", (UInt)devList.Flags);
			printf("		Type=0x%x\n", (UInt)devList.Type);
			printf("		ID=0x%x\n", (UInt)devList.ID);
			printf("		LocId=0x%x\n", (UInt)devList.LocId);
			printf("		SerialNumber=%s\n", devList.SerialNumber);
			printf("		Description=%s\n", devList.Description);
			printf("		ftHandle=0x%x\n", (UInt)devList.ftHandle); /* 0 if not open*/
		}
		
		/* Open the first available channel */
		FT_STATUS status = I2C_OpenChannel(CHANNEL_TO_OPEN, &ftHandle);
		APP_CHECK_STATUS(status);
		printf("\nhandle=0x%x status=%d\n", (UInt)ftHandle, (Int)status);
		APP_CHECK_STATUS(I2C_InitChannel(ftHandle, &channelConf));
		
		for(;;) {
			enum { dataSize = 16 };
			uint8_t data[dataSize];

			memset(data, 0, dataSize);
			PCF8563_readTime(i2cAddress, 2, &data[2]);
			forInc(int, i, 0, dataSize) {
				printf("%02X ", data[i]);
			}
			printf("\r");
			fflush(stdout);
			
			Sleep(900);
			
		}
		
		I2C_CloseChannel(ftHandle);
	}
	return 0;
}
