#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>
#include <ftMPSSE/libMPSSE_i2c.h>
#include "common.h"

#define PROGMEM
#define TRANSACTION_START
#define TRANSACTION_END

// #define Debug_print(fmt, ...) fprintf(stderr, fmt "\n", __VA_ARGS__);
#define Debug_print(fmt, ...) 

val CHANNEL_TO_OPEN = 0;
val ina219SlaveAddress = 0x3C;

FT_HANDLE ftHandle = null;

#define SSD1306_I2C_MAX_BUFFER 128

#define SSD1306_MEMORYMODE 0x20          ///< See datasheet
#define SSD1306_COLUMNADDR 0x21          ///< See datasheet
#define SSD1306_PAGEADDR 0x22            ///< See datasheet
#define SSD1306_SETCONTRAST 0x81         ///< See datasheet
#define SSD1306_CHARGEPUMP 0x8D          ///< See datasheet
#define SSD1306_SEGREMAP 0xA0            ///< See datasheet
#define SSD1306_DISPLAYALLON_RESUME 0xA4 ///< See datasheet
#define SSD1306_DISPLAYALLON 0xA5        ///< Not currently used
#define SSD1306_NORMALDISPLAY 0xA6       ///< See datasheet
#define SSD1306_INVERTDISPLAY 0xA7       ///< See datasheet
#define SSD1306_SETMULTIPLEX 0xA8        ///< See datasheet
#define SSD1306_DISPLAYOFF 0xAE          ///< See datasheet
#define SSD1306_DISPLAYON 0xAF           ///< See datasheet
#define SSD1306_COMSCANINC 0xC0          ///< Not currently used
#define SSD1306_COMSCANDEC 0xC8          ///< See datasheet
#define SSD1306_SETDISPLAYOFFSET 0xD3    ///< See datasheet
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5  ///< See datasheet
#define SSD1306_SETPRECHARGE 0xD9        ///< See datasheet
#define SSD1306_SETCOMPINS 0xDA          ///< See datasheet
#define SSD1306_SETVCOMDETECT 0xDB       ///< See datasheet

#define SSD1306_SETLOWCOLUMN 0x00  ///< Not currently used
#define SSD1306_SETHIGHCOLUMN 0x10 ///< Not currently used
#define SSD1306_SETSTARTLINE 0x40  ///< See datasheet

#define SSD1306_EXTERNALVCC 0x01  ///< External display voltage source
#define SSD1306_SWITCHCAPVCC 0x02 ///< Gen. display voltage from 3.3V

#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26              ///< Init rt scroll
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27               ///< Init left scroll
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29 ///< Init diag scroll
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A  ///< Init diag scroll
#define SSD1306_DEACTIVATE_SCROLL 0x2E                    ///< Stop scroll
#define SSD1306_ACTIVATE_SCROLL 0x2F                      ///< Start scroll
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3             ///< Set scroll range

enum { WIDTH = 128, HEIGHT = 32 };
enum { vccstate = !SSD1306_EXTERNALVCC };

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

FT_STATUS ssd1306_commandList(const uint8_t* data, uint8_t dataSize, uint8_t mode = 0) {
  const int dataCopySize = dataSize + 1;
  uint8_t* dataCopy = (uint8_t*)malloc(dataCopySize * sizeof(uint8_t));
  dataCopy[0] = mode;
  memcpy(&dataCopy[1], data, dataSize * sizeof(uint8_t)); 
  
  uint32 bytesTransfered = 0;
  
  // Write address pointer
  FT_STATUS status = I2C_DeviceWrite(ftHandle, ina219SlaveAddress, dataCopySize, dataCopy, &bytesTransfered, I2C_TRANSFER_OPTIONS_START_BIT | I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES | I2C_TRANSFER_OPTIONS_STOP_BIT);
                             
  Debug_print("status = %i, bytesToTransfer = %i + 1, bytesTransfered = %i", (int)status, (int)dataSize, (int)bytesTransfered);
  
  free(dataCopy);
  return status;
}

void ssd1306_command1(uint8_t c) {
	uint8_t buffer[1] = { c };
  ssd1306_commandList(buffer, 1);
}
void ssd1306_command(uint8_t c) {
  ssd1306_command1(c);
}


Bool SSD1306_init() {
	// Init sequence
	static const uint8_t PROGMEM init1[] = {SSD1306_DISPLAYOFF,         // 0xAE
	                                        SSD1306_SETDISPLAYCLOCKDIV, // 0xD5
	                                        0x80, // the suggested ratio 0x80
	                                        SSD1306_SETMULTIPLEX
	                                       }; // 0xA8
	ssd1306_commandList(init1, sizeof(init1));
	ssd1306_command1(HEIGHT - 1);
	
	static const uint8_t PROGMEM init2[] = {SSD1306_SETDISPLAYOFFSET, // 0xD3
	                                        0x0,                      // no offset
	                                        SSD1306_SETSTARTLINE | 0x0, // line #0
	                                        SSD1306_CHARGEPUMP
	                                       };        // 0x8D
	ssd1306_commandList(init2, sizeof(init2));
	
	ssd1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x10 : 0x14);
	
	static const uint8_t PROGMEM init3[] = {SSD1306_MEMORYMODE, // 0x20
	                                        0x00, // 0x0 act like ks0108
	                                        SSD1306_SEGREMAP | 0x1,
	                                        SSD1306_COMSCANDEC
	                                       };
	ssd1306_commandList(init3, sizeof(init3));
	
	uint8_t comPins = 0x02;
	uint8_t contrast = 0x8F;
	
	if((WIDTH == 128) && (HEIGHT == 32)) {
		comPins = 0x02;
		contrast = 0x8F;
	}
	else if((WIDTH == 128) && (HEIGHT == 64)) {
		comPins = 0x12;
		contrast = (vccstate == SSD1306_EXTERNALVCC) ? 0x9F : 0xCF;
	}
	else if((WIDTH == 96) && (HEIGHT == 16)) {
		comPins = 0x2; // ada x12
		contrast = (vccstate == SSD1306_EXTERNALVCC) ? 0x10 : 0xAF;
	}
	else {
		// Other screen varieties -- TBD
	}
	
	ssd1306_command1(SSD1306_SETCOMPINS);
	ssd1306_command1(comPins);
	ssd1306_command1(SSD1306_SETCONTRAST);
	ssd1306_command1(contrast);
	
	ssd1306_command1(SSD1306_SETPRECHARGE); // 0xd9
	ssd1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x22 : 0xF1);
	static const uint8_t PROGMEM init5[] = {
		SSD1306_SETVCOMDETECT, // 0xDB
		0x40,
		SSD1306_DISPLAYALLON_RESUME, // 0xA4
		SSD1306_NORMALDISPLAY,       // 0xA6
		SSD1306_DEACTIVATE_SCROLL,
		SSD1306_DISPLAYON
	}; // Main screen turn on
	ssd1306_commandList(init5, sizeof(init5));
	
	return true; // Success
  
}

void  SSD1306_displayPattern(uint8_t pattern) {
	TRANSACTION_START
	static const uint8_t PROGMEM dlist1[] = {
		SSD1306_PAGEADDR,
		0,                      // Page start address
		0xFF,                   // Page end (not really, but works here)
		SSD1306_COLUMNADDR, 0
	}; // Column start address
	ssd1306_commandList(dlist1, sizeof(dlist1));
	ssd1306_command1(WIDTH - 1); // Column end address
	
#if defined(ESP8266)
	// ESP8266 needs a periodic yield() call to avoid watchdog reset.
	// With the limited size of SSD1306 displays, and the fast bitrate
	// being used (1 MHz or more), I think one yield() immediately before
	// a screen write and one immediately after should cover it.  But if
	// not, if this becomes a problem, yields() might be added in the
	// 32-byte transfer condition below.
	yield();
#endif
	uint8_t buffer[SSD1306_I2C_MAX_BUFFER];
  memset(buffer, pattern, arraySize(buffer));
  
  forInc(int, i, 0, WIDTH * HEIGHT / 8 / arraySize(buffer)) {
    ssd1306_commandList(buffer, arraySize(buffer), 0x40);
  }
  
	TRANSACTION_END
#if defined(ESP8266)
	yield();
#endif
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
    .ClockRate = I2C_CLOCK_FAST_MODE, /*i.e. 100000 KHz*/
    .LatencyTimer = 20, 
    .Options = (0
      // | I2C_DISABLE_3PHASE_CLOCKING 
      // | I2C_ENABLE_DRIVE_ONLY_ZERO
    )
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
    
    SSD1306_init();
    
    uint8_t pattern = 1;
    for(;;) {
      pattern <<= 1;
      if(pattern == 0) pattern = 1;
      SSD1306_displayPattern(pattern);
      
      Sleep(900);
    }
    
    I2C_CloseChannel(ftHandle);
  }
  return 0;
}
