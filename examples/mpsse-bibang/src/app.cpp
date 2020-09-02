#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>
#include <ftMPSSE/libMPSSE_i2c.h>
#include "common.h"

val CHANNEL_TO_OPEN = 0;

FT_HANDLE ftHandle = null;

BOOL WINAPI consoleHandler(DWORD signal) {
  if (signal == CTRL_C_EVENT) {
    printf("Ctrl-C handled\n"); // do cleanup
  }

  exit(0);
  return TRUE;
}


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

int main() {
  if(!SetConsoleCtrlHandler(consoleHandler, TRUE)) {
    printf("\nERROR: Could not set control handler");
    return 1;
  }

  Init_libMPSSE();

  ChannelConfig channelConf = {
    .ClockRate = I2C_CLOCK_STANDARD_MODE, /*i.e. 100000 KHz*/
    .LatencyTimer = 10,
    .Options = I2C_DISABLE_3PHASE_CLOCKING /* | I2C_ENABLE_DRIVE_ONLY_ZERO */
    //.Options = 0 /* | I2C_ENABLE_DRIVE_ONLY_ZERO */
  };

  uint32 channels = 0; APP_CHECK_STATUS(I2C_GetNumChannels(&channels));

  printf("Number of available I2C channels = %d\n", (Int)channels);

  if(channels == 0) {
    fprintf(stderr, "\nCan not find FTDI device");
    return 1;
    //goto cleanupMpsse;
  }

  FT_DEVICE_LIST_INFO_NODE devList; APP_CHECK_STATUS(I2C_GetChannelInfo(CHANNEL_TO_OPEN, &devList));

  printf("Information on channel number %d:\n", (Int)CHANNEL_TO_OPEN);
  /*print the dev info*/
  printf("		Flags=0x%x\n", (UInt)devList.Flags);
  printf("		Type=0x%x\n", (UInt)devList.Type);
  printf("		ID=0x%x\n", (UInt)devList.ID);
  printf("		LocId=0x%x\n", (UInt)devList.LocId);
  printf("		SerialNumber=%s\n", devList.SerialNumber);
  printf("		Description=%s\n", devList.Description);
  printf("		ftHandle=0x%x\n", (UInt)devList.ftHandle); /* 0 if not open*/

  /* Open the first available channel */
  FT_STATUS status = I2C_OpenChannel(CHANNEL_TO_OPEN, &ftHandle);
  if(status != FT_OK) {
    fprintf(stderr, "\nCan not open I2C channel");
    return 1;
    //goto closeI2cChannel;
  }

  printf("\nhandle=0x%x status=%d\n", (UInt)ftHandle, (Int)status);
  APP_CHECK_STATUS(I2C_InitChannel(ftHandle, &channelConf));

  loop {
    FT_WriteGPIO(ftHandle, 0xFF, 0x1);
    Sleep(500);
    FT_WriteGPIO(ftHandle, 0xFF, 0x0);
    Sleep(500);
  }

  closeI2cChannel:
  I2C_CloseChannel(ftHandle);

  cleanupMpsse:
  Cleanup_libMPSSE();

  return 0;
}
