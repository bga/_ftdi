#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>

namespace Ftdi {
  #include <ftMPSSE/libMPSSE_spi.h>
}

// #include "glcd/stlcd.h"
namespace NokiaLCD {
  #include "NokiaLCD/NokiaLCD.h"
}
#include "common.h"

#define Debug_logUrl "r:\\nokia5111-lcd-test.log"
#include <!cpp/debug.h>

namespace App {
using namespace Ftdi;

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
    .ClockRate = 100000, /* Hz */
    .LatencyTimer = 255,
    .configOptions = SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS3,
  };

  uint32 channels = 0; APP_CHECK_STATUS(SPI_GetNumChannels(&channels));

  printf("Number of available SPI channels = %d\n", (Int)channels);

  if(channels == 0) {
    fprintf(stderr, "\nCan not find FTDI device");
    return 1;
    //goto cleanupMpsse;
  }

  FT_DEVICE_LIST_INFO_NODE devList; APP_CHECK_STATUS(SPI_GetChannelInfo(CHANNEL_TO_OPEN, &devList));

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
  FT_STATUS status = SPI_OpenChannel(CHANNEL_TO_OPEN, &ftHandle);
  if(status != FT_OK) {
    fprintf(stderr, "\nCan not open SPI channel");
    return 1;
    //goto closeSpiChannel;
  }

  printf("\nhandle=0x%x status=%d\n", (UInt)ftHandle, (Int)status);
  APP_CHECK_STATUS(SPI_InitChannel(ftHandle, &channelConf));


  #if 0
  if(1) {
    using namespace SSD1306;
    st7565_init();
    st7565_command(CMD_DISPLAY_ON);
    st7565_command(CMD_SET_ALLPTS_NORMAL);
    st7565_set_brightness(0x18);
    clear_screen();
    write_buffer(SSD1306::buffer);
  }
  #else
  if(1) {
    using namespace NokiaLCD;
    LcdInit();
  }
  #endif
  int counter = 0;
  loop {
    using namespace NokiaLCD;

    counter += 1;
    // LcdClear();
    LcdGotoXY(24, 24);
    char t[5];
    sprintf(t, "%04d", counter);
    printf("%s\n", t);
    fflush(stdout);
    LcdStrConst(FONT_1X, t);
    Sleep(1000);
  }

  closeSpiChannel:
  SPI_CloseChannel(ftHandle);

  cleanupMpsse:
  Cleanup_libMPSSE();

  return 0;
}

} // namespace App

int main() {
  return App::main();
}
