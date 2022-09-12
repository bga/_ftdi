#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>
#include <ftMPSSE/libMPSSE_i2c.h>
#include "common.h"

val CHANNEL_TO_OPEN = 0;
val i2cAddress = 0x36;

#define DUMMY_NAME BGA__UNIQUE_NAME
#pragma pack(push, 1) 
struct AS5600_RegesterMap {
	union {
		uint8_t DUMMY_NAME;
		struct {
			unsigned zmco:2;
		};
	};
	
	union {
		uint8_t DUMMY_NAME;
		struct {
			unsigned zpos_hi:4;
		};
	};
	uint8_t zpos_lo;
	FU16 get_zpos() const {
		return this->zpos_lo | (this->zpos_hi << 8);
	}
	
	union {
		uint8_t reg3;
		struct {
			unsigned mpos_hi:4;
		};
	};
	uint8_t mpos_lo;
	FU16 get_mpos() const {
		return this->mpos_lo | (this->mpos_hi << 8);
	}
	
	union {
		uint8_t reg5;
		struct {
			unsigned mang_hi:4;
		};
	};
	uint8_t mang_lo;
	FU16 get_mang() const {
		return this->mang_lo | (this->mang_hi << 8);
	}
	
	union {
		uint8_t DUMMY_NAME;
		struct {
			unsigned sf: 2;
			unsigned fth: 3;
			unsigned wd: 1;
		};
	};
	
	union {
		uint8_t DUMMY_NAME;
		struct {
			unsigned pm: 2;
			unsigned hyst: 2;
			unsigned outs: 2;
			unsigned pwmf: 2;
		};
	};
	
	uint8_t DUMMY_NAME;
	uint8_t DUMMY_NAME;
	
	union {
		uint8_t DUMMY_NAME;
		struct {
			unsigned DUMMY_NAME: 3;
			unsigned mh: 1;
			unsigned ml: 1;
			unsigned md: 1;
		};
	};

	union {
		uint8_t DUMMY_NAME;
		struct {
			unsigned rawAngle_hi:4;
		};
	};
	uint8_t rawAngle_lo;
	FU16 get_rawAngle() const {
		return this->rawAngle_lo | (this->rawAngle_hi << 8);
	}

	union {
		uint8_t DUMMY_NAME;
		struct {
			unsigned angle_hi:4;
		};
	};
	uint8_t angle_lo;
	FU16 get_angle() const {
		return this->angle_lo | (this->angle_hi << 8);
	}
};
#pragma pack(pop)

static_assert_eq(offsetof(AS5600_RegesterMap, angle_lo), 0x0F);


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


FT_STATUS AS5600_read(uint8 slaveAddress, AS5600_RegesterMap* dataObj) {
	uint8* data = (uint8 *)dataObj;
	uint32 bytesToTransfer = 0;
	uint32 bytesTransfered;
	uint8 buffer[2];
	
	bytesToTransfer = bytesTransfered = 0;
	
	#if 1
	// Write address pointer
	buffer[bytesToTransfer++] = 0;
	FT_STATUS status = I2C_DeviceWrite(
		ftHandle, 
		slaveAddress, 
		bytesToTransfer, 
		buffer, 
		&bytesTransfered, 
		I2C_TRANSFER_OPTIONS_START_BIT | I2C_TRANSFER_OPTIONS_STOP_BIT
	);
																		 
	// printf("I2C_DeviceWrite status: %d %d, %d\n", int(bytesTransfered), status, FT_DEVICE_NOT_FOUND);
	APP_CHECK_STATUS(status);
	#endif
	
	bytesToTransfer = sizeof(*dataObj);
	bytesTransfered = 0;
	FT_STATUS status2 = I2C_DeviceRead(
		ftHandle, 
		slaveAddress, 
		bytesToTransfer, 
		data,
		&bytesTransfered, I2C_TRANSFER_OPTIONS_START_BIT | /* I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES | */ I2C_TRANSFER_OPTIONS_STOP_BIT
	); // |I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BITS |I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE
																		
	// printf("I2C_DeviceWrite status: %d %d, %d\n", int(bytesTransfered), status2, FT_INVALID_PARAMETER);
	
	APP_CHECK_STATUS(status2);
	
	return status | status2;
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
			AS5600_RegesterMap data;

			memset((char *)(&data), 0x00, sizeof(data));
			AS5600_read(i2cAddress, &data);
			#if 0
			forInc(int, i, 0, sizeof(data)) {
				printf("%02X ", ((char const*)&data)[i]);
			}
			#endif
			printf("mpos = %4d zpos = %4d angle = %4d rawAngle = %4d mang = %4d", int(data.get_mpos()), int(data.get_zpos()), int(data.get_angle()), int(data.get_rawAngle()), int(data.get_mang()));
			printf("\r");
			fflush(stdout);
			
			Sleep(900);
			
		}
		
		I2C_CloseChannel(ftHandle);
	}
	return 0;
}
