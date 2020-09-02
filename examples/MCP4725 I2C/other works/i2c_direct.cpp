//# [https://habr.com/ru/post/207214/]

#include "stdafx.h"

/*
using namespace std;


int _tmain(int argc, _TCHAR* argv[])
{
	cout << "Hello world!\n";
	return 0;
}
*/


/*!
 * \file sample-dynamic.c
 *
 * \author FTDI
 * \date 20110512
 *
 * Copyright © 2011 Future Technology Devices International Limited
 * Company Confidential
 *
 * Project: libMPSSE
 * Module: I2C Sample Application - Interfacing 24LC024H I2C EEPROM
 *
 * Rivision History:
 * 0.1 - 20110513 - initial version
 * 0.2 - 20110801 - Changed LatencyTimer to 255
 * 				  Attempt to open channel only if available
 *				  Added & modified macros
 *                		  Change APIs I2C_GetChannelInfo & OpenChannel to start indexing from 0
 * 0.3 - 20111212 - Added comments
 */

/******************************************************************************/
/* 							 Include files										   */
/******************************************************************************/
/* Standard C libraries */
#include<stdio.h>
#include<stdlib.h>

/* OS specific libraries */
#ifdef _WIN32
#include<windows.h>
#endif

#ifdef __linux
#include<dlfcn.h>
#endif

/* Include D2XX header*/
#include "ftd2xx.h"

/* Include libMPSSE header */
#include "libMPSSE_i2c.h"

/******************************************************************************/
/*								Macro and type defines							   */
/******************************************************************************/
/* Helper macros */
#ifdef _WIN32
	#define GET_FUN_POINTER	GetProcAddress
	#define CHECK_ERROR(exp) {if(exp==NULL){printf("%s:%d:%s(): NULL expression\
		encountered \n",__FILE__, __LINE__, __FUNCTION__);exit(1);}else{;}};
#endif

#ifdef __linux
	#define GET_FUN_POINTER	dlsym
	#define CHECK_ERROR(exp) {if(dlerror() != NULL){printf("line %d: ERROR \
		dlsym\n",__LINE__);}}
#endif
#define APP_CHECK_STATUS(exp) {if(exp!=FT_OK){printf("%s:%d:%s(): status(0x%x) \
	!= FT_OK\n",__FILE__, __LINE__, __FUNCTION__,exp);exit(1);}else{;}};
#define CHECK_NULL(exp){if(exp==NULL){printf("%s:%d:%s():  NULL expression \
	encountered \n",__FILE__, __LINE__, __FUNCTION__);exit(1);}else{;}};

/* Application specific macro definations */
#define I2C_DEVICE_ADDRESS_EEPROM		0x57
#define I2C_DEVICE_BUFFER_SIZE		256
#define I2C_WRITE_COMPLETION_RETRY		10
#define START_ADDRESS_EEPROM 	0x00 /*read/write start address inside the EEPROM*/
#define END_ADDRESS_EEPROM		0x10
#define RETRY_COUNT_EEPROM		10	/* number of retries if read/write fails */
#define CHANNEL_TO_OPEN			0	/*0 for first available channel, 1 for next... */
#define DATA_OFFSET				2


/* Declaration of function pointers */
typedef FT_STATUS (*pfunc_I2C_GetNumChannels)(uint32 *numChannels);
pfunc_I2C_GetNumChannels p_I2C_GetNumChannels;
typedef FT_STATUS (*pfunc_I2C_GetChannelInfo)(uint32 index, \
	FT_DEVICE_LIST_INFO_NODE *chanInfo);
pfunc_I2C_GetChannelInfo p_I2C_GetChannelInfo;
typedef FT_STATUS (*pfunc_I2C_OpenChannel)(uint32 index, FT_HANDLE *handle);
pfunc_I2C_OpenChannel p_I2C_OpenChannel;
typedef FT_STATUS (*pfunc_I2C_CloseChannel)(FT_HANDLE handle);
pfunc_I2C_CloseChannel p_I2C_CloseChannel;
typedef FT_STATUS (*pfunc_I2C_InitChannel)(FT_HANDLE handle, ChannelConfig \
	*config);
pfunc_I2C_InitChannel p_I2C_InitChannel;
typedef FT_STATUS (*pfunc_I2C_DeviceRead)(FT_HANDLE handle,uint32 \
	deviceAddress,uint32 sizeToTransfer, uint8 *buffer, uint32 \
	*sizeTransfered, uint32 options);
pfunc_I2C_DeviceRead p_I2C_DeviceRead;
typedef FT_STATUS (*pfunc_I2C_DeviceWrite)(FT_HANDLE handle, uint32 \
	deviceAddress,uint32 sizeToTransfer, uint8 *buffer, uint32 \
	*sizeTransfered, uint32 options);
pfunc_I2C_DeviceWrite p_I2C_DeviceWrite;

/******************************************************************************/
/*								Global variables							  	    */
/******************************************************************************/
uint32 channels;
FT_HANDLE ftHandle;
ChannelConfig channelConf;
FT_STATUS status;
uint8 buffer[I2C_DEVICE_BUFFER_SIZE];

/******************************************************************************/
/*						Public function definitions						  		   */
/******************************************************************************/

FT_STATUS read_ina219(uint8 slaveAddress, uint8 registerAddress, uint16 *data)
{

	FT_STATUS status=0, status2=0;
	uint32 bytesToTransfer = 0;
	uint32 bytesTransfered;
	
	bytesToTransfer=0;
	bytesTransfered=0;

	// Write address pointer
	buffer[bytesToTransfer++]=registerAddress; /* Register addressed inside INA219 */
	status = p_I2C_DeviceWrite(ftHandle, slaveAddress, bytesToTransfer, buffer,\
		&bytesTransfered, I2C_TRANSFER_OPTIONS_START_BIT);

//	printf("I2C_DeviceWrite status: %d, %d\n", status, FT_DEVICE_NOT_FOUND);
//	APP_CHECK_STATUS(status);

	bytesToTransfer=2;
	bytesTransfered=0;
	status2 = p_I2C_DeviceRead(ftHandle, slaveAddress, bytesToTransfer, buffer,\
		&bytesTransfered, I2C_TRANSFER_OPTIONS_START_BIT|I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES|I2C_TRANSFER_OPTIONS_STOP_BIT
		); // |I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BITS |I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE
	*data = buffer[0] <<8 | buffer[1];

//	printf("I2C_DeviceWrite status: %d, %d\n", status2, FT_INVALID_PARAMETER);

//	APP_CHECK_STATUS(status);

	return status|status2;



}


/*!
 * \brief Main function / Entry point of the sample application
 *
 * This function is the entry point to the sample application. It opens the channel, writes to the
 * EEPROM and reads back.
 *
 * \param[in] none
 * \return Returns 0 for success
 * \sa
 * \note 
 * \warning
 */
int main()
{
/*
#ifdef _WIN32
	#ifdef _MSC_VER
		HMODULE h_libMPSSE;
	#else
		HANDLE h_libMPSSE;
	#endif
#endif

#ifdef __linux
	void *h_libMPSSE;
#endif

*/

  HMODULE h_libMPSSE;


	FT_STATUS status;
	FT_DEVICE_LIST_INFO_NODE devList;
	uint8 address;
	uint16 data;
	int i,j;

	int v_sh, v_bus, i_sh, v_bus_code, power;


	channelConf.ClockRate = I2C_CLOCK_STANDARD_MODE;/*i.e. 100000 KHz*/
	channelConf.LatencyTimer= 10; //255;
	channelConf.Options = I2C_DISABLE_3PHASE_CLOCKING;
	//channelConf.Options = I2C_ENABLE_DRIVE_ONLY_ZERO;

	/* load library */
#ifdef _WIN32
	#ifdef _MSC_VER
		h_libMPSSE = LoadLibrary(L"libMPSSE.dll");
	#else
		h_libMPSSE = LoadLibrary("libMPSSE.dll");
	#endif
	if(NULL == h_libMPSSE)
	{
		printf("Failed loading libMPSSE.dll. Please check if the file exists in\
 the working directory\n");
	}
#endif
	
#ifdef __linux
	h_libMPSSE = dlopen("libMPSSE.so",RTLD_LAZY);
	if(!h_libMPSSE)
	{
		printf("Failed loading libMPSSE.so\n");
	}
#endif

	/* init function pointers */
	p_I2C_GetNumChannels = (pfunc_I2C_GetNumChannels)GET_FUN_POINTER(h_libMPSSE\
, "I2C_GetNumChannels");
	CHECK_ERROR (p_I2C_GetNumChannels);
	p_I2C_GetChannelInfo = (pfunc_I2C_GetChannelInfo)GET_FUN_POINTER(h_libMPSSE\
, "I2C_GetChannelInfo");
	CHECK_ERROR(p_I2C_GetChannelInfo);
	p_I2C_OpenChannel = (pfunc_I2C_OpenChannel)GET_FUN_POINTER(h_libMPSSE\
, "I2C_OpenChannel");
	CHECK_ERROR(p_I2C_OpenChannel);
	p_I2C_CloseChannel = (pfunc_I2C_CloseChannel)GET_FUN_POINTER(h_libMPSSE\
,"I2C_CloseChannel");
	CHECK_ERROR(p_I2C_CloseChannel);
	p_I2C_InitChannel = (pfunc_I2C_InitChannel)GET_FUN_POINTER(h_libMPSSE\
, "I2C_InitChannel");
	CHECK_ERROR(p_I2C_InitChannel);
	p_I2C_DeviceRead = (pfunc_I2C_DeviceRead)GET_FUN_POINTER(h_libMPSSE\
, "I2C_DeviceRead");
	CHECK_ERROR(p_I2C_DeviceRead);
	p_I2C_DeviceWrite = (pfunc_I2C_DeviceWrite)GET_FUN_POINTER(h_libMPSSE\
, "I2C_DeviceWrite");
	CHECK_ERROR(p_I2C_DeviceWrite);

	status = p_I2C_GetNumChannels(&channels);
	APP_CHECK_STATUS(status);
	printf("Number of available I2C channels = %d\n",channels);
	if(channels>0)
	{
		for(i=0;i<channels;i++)
		{
			status = p_I2C_GetChannelInfo(i,&devList);
			APP_CHECK_STATUS(status);
			printf("Information on channel number %d:\n",i);
			/*print the dev info*/
			printf("		Flags=0x%x\n",devList.Flags); 
			printf("		Type=0x%x\n",devList.Type); 
			printf("		ID=0x%x\n",devList.ID); 
			printf("		LocId=0x%x\n",devList.LocId); 
			printf("		SerialNumber=%s\n",devList.SerialNumber); 
			printf("		Description=%s\n",devList.Description); 
			printf("		ftHandle=0x%x\n",devList.ftHandle);/* 0 if not open*/
		}

		/* Open the first available channel */
		status = p_I2C_OpenChannel(CHANNEL_TO_OPEN,&ftHandle);
		APP_CHECK_STATUS(status);
		printf("\nhandle=0x%x status=%d\n",ftHandle,status);
		status = p_I2C_InitChannel(ftHandle,&channelConf);
		APP_CHECK_STATUS(status);
			
//		write_byte(0x80, 3, 13);

		for(i=0; i<1; i++)
		{
			status = read_ina219(0x40, 1, &data);
			if (FT_OK !=status)
			{
				printf("failed reading\n");
				return -1;
			}
//			else
				printf("reading address %d data read=0x%04x\n", 1, data);
			v_sh = data;

			status = read_ina219(0x40, 2, &data);
			if (FT_OK !=status)
			{
				printf("failed reading\n");
				return -1;
			}
//			else
				printf("reading address %d data read=0x%04x\n", 2, data);
			v_bus_code = data;

			i_sh = v_sh; // mA   ( (v_sh/100)/100 mA
			v_bus = (v_bus_code>>3) * 4;  // mV
			power = i_sh * v_bus /1000; // mW
			printf("Current: %d mA, Voltage: %d mV, Power: %d mW\r", i_sh, v_bus, power);

			Sleep(100);

		}

		status = p_I2C_CloseChannel(ftHandle);
	}
	return 0;
}
