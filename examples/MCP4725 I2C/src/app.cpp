#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>
#include <ftMPSSE/libMPSSE_i2c.h>
#include "common.h"

val CHANNEL_TO_OPEN = 0;
val mcp4725SlaveAddress = 0x60;

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

FT_STATUS MCP4725_setDacValue(uint8 slaveAddress, U16 dacValue) {
  uint32 bytesTransfered = 0; 
  val bytesToTransfer = sizeof(dacValue);
  uint8 buffer[bytesToTransfer] = { dacValue >> 8, U8(dacValue) };
  
  FT_STATUS status = I2C_DeviceWrite(
    ftHandle, 
    slaveAddress, 
    bytesToTransfer, 
    buffer, 
    &bytesTransfered, 
    I2C_TRANSFER_OPTIONS_START_BIT 
    | I2C_TRANSFER_OPTIONS_STOP_BIT 
    | I2C_TRANSFER_OPTIONS_BREAK_ON_NACK
  );
  
  //printf("\n%i", bytesTransfered);
  
  return status;
                             
}


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
  
  val minBrightness = 1.8 / 3.3 * (1 << 12);
  val maxBrightness = (1 << 12);
  val step = 1.02;
  double brightness = minBrightness;  
  Bool isUp = yes;
  loop {
    MCP4725_setDacValue(mcp4725SlaveAddress, U16(brightness));
    if(isUp) {
      brightness *= step;
      if(maxBrightness <= brightness) {
        isUp = no;
        brightness = maxBrightness;
      }
    }
    else {
      brightness /= step; 
      if(brightness < minBrightness) {
        isUp = yes;
        brightness = minBrightness;
      }
    }
    //printf("\n%i", (int)brightness);
    Sleep(10);
  }
    
  closeI2cChannel:
  I2C_CloseChannel(ftHandle);
  
  cleanupMpsse:
  Cleanup_libMPSSE();
  
  return 0;
}
