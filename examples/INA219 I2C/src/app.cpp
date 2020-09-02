#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>
#include <ftMPSSE/libMPSSE_i2c.h>
#include "common.h"

val CHANNEL_TO_OPEN = 0;
val ina219SlaveAddress = 0x40;

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

FT_STATUS read_ina219(uint8 slaveAddress, uint8 registerAddress, uint16* data) {
  uint32 bytesToTransfer = 0;
  uint32 bytesTransfered;
  uint8 buffer[2];
  
  bytesToTransfer = bytesTransfered = 0;
  
  // Write address pointer
  buffer[bytesToTransfer++] = registerAddress; /* Register addressed inside INA219 */
  FT_STATUS status = I2C_DeviceWrite(ftHandle, slaveAddress, bytesToTransfer, buffer, 
                             &bytesTransfered, I2C_TRANSFER_OPTIONS_START_BIT);
                             
  //  printf("I2C_DeviceWrite status: %d, %d\n", status, FT_DEVICE_NOT_FOUND);
  //  APP_CHECK_STATUS(status);
  
  bytesToTransfer = 2;
  bytesTransfered = 0;
  FT_STATUS status2 = I2C_DeviceRead(ftHandle, slaveAddress, bytesToTransfer, buffer, 
                             &bytesTransfered, I2C_TRANSFER_OPTIONS_START_BIT | I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES | I2C_TRANSFER_OPTIONS_STOP_BIT
                            ); // |I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BITS |I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE
  *data = (buffer[0] << 8) | buffer[1];
  
  //  printf("I2C_DeviceWrite status: %d, %d\n", status2, FT_INVALID_PARAMETER);
  
  //  APP_CHECK_STATUS(status);
  
  return status | status2;
}


int main() {
  
  ChannelConfig channelConf = {
    .ClockRate = I2C_CLOCK_STANDARD_MODE, /*i.e. 100000 KHz*/
    .LatencyTimer = 10, 
    .Options = I2C_DISABLE_3PHASE_CLOCKING /* | I2C_ENABLE_DRIVE_ONLY_ZERO */
  };
  
  
  uint32 channels = 0; APP_CHECK_STATUS(I2C_GetNumChannels(&channels));
  
  printf("Number of available I2C channels = %d\n", (Int)channels);
  
  if(channels > 0) {
    for(Int i = 0; i < channels; i++) {
      FT_DEVICE_LIST_INFO_NODE devList; APP_CHECK_STATUS(I2C_GetChannelInfo(i, &devList));
      
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
    
    //    write_byte(0x80, 3, 13);
    
    for(Int i = 0; i < 1; i++) {
      uint16 data;
      FT_STATUS status = read_ina219(ina219SlaveAddress, 1, &data);
      if(FT_OK != status) {
        printf("failed reading\n");
        return -1;
      }
      //      else
      printf("reading address %d data read=0x%04x\n", 1, data);
      int v_sh = data;
      
      for(val status = read_ina219(ina219SlaveAddress, 2, &data); FT_OK != status;) {
        printf("failed reading\n");
        return -1;
        break;
      }
      //      else
      printf("reading address %d data read=0x%04x\n", 2, data);
      int v_bus_code = data;
      
      int i_sh = v_sh; // mA   ( (v_sh/100)/100 mA
      int v_bus = (v_bus_code >> 3) * 4; // mV
      int power = i_sh * v_bus / 1000; // mW
      printf("Current: %d mA, Voltage: %d mV, Power: %d mW\r", i_sh, v_bus, power);
      
      Sleep(100);
      
    }
    
    I2C_CloseChannel(ftHandle);
  }
  return 0;
}
