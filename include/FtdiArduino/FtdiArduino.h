#pragma once

#pragma comment(lib, "ftd2xx.lib")
#include <ftD2XX/ftd2xx.h>

/* gnd ctr vcc tx rx dtr */
#define PIN_TX  0  /* Orange wire on FTDI cable */
#define PIN_RX  1  /* Yellow */
#define PIN_RTS 2  /* Green */
#define PIN_CTS 3  /* Brown */
#define PIN_DTR 4
#define PIN_DSR 5
#define PIN_DCD 6
#define PIN_RI  7


namespace FtdiArduino {
  FT_HANDLE handle = NULL;
  uint8_t portMode = 0;
  uint8_t portValue = 0;
  
  
  enum Mode {
    INPUT = 0, 
    OUTPUT = 1
  };
  
  void pinMode(uint8_t pin, Mode mode) {
    if(mode == OUTPUT) {
      portMode |= (1 << pin);
    }
    else {
      portMode &= ~(1 << pin);
    }
    FT_SetBitMode(handle, portMode, 1);
  }
  
  const auto LOW = 0;
  const auto HIGH = 1;
  
  #if FTDI_ARDUINO_ENABLE_BUFFER_WRITE
    const int BUFFER_SIZE = 64 * 4;
    uint8_t buffer[BUFFER_SIZE];
    int bufferStart = 0;
    int bufferEnd = 0;
    
    void digitalWrite(uint8_t pin, uint8_t value) {
      if(value) {
        portValue |= (1 << pin);
      }
      else {
        portValue &= ~(1 << pin);
      }

      buffer[bufferEnd++] = portValue;
      if(bufferEnd == BUFFER_SIZE) {
        bufferEnd = 0;
      }
      //Sleep(500);
    }
    
    void doWrite() {
      //# ugly code
      //# real code just call { FT_Write } to the end of { buffer } and from begin if rollup
      uint8_t realBuffer[(bufferEnd + BUFFER_SIZE - bufferStart) % BUFFER_SIZE];
      int j = 0;
      for(int i = bufferStart; i != bufferEnd;) {
        realBuffer[j] = buffer[i];
        i += 1;
        if(i == BUFFER_SIZE) {
          i = 0;
        }
        else {
          
        }
        j += 1;
      }
      
      DWORD bytes;
      FT_Write(handle, realBuffer, j, &bytes);
    }
  #else
    void digitalWrite(uint8_t pin, uint8_t value) {
      DWORD bytes;
      if(value) {
        portValue |= (1 << pin);
      }
      else {
        portValue &= ~(1 << pin);
      }

      FT_Write(handle, &portValue, 1, &bytes);
      //Sleep(500);
    }
    
    void doWrite() {
    }
  #endif
  
  int ftdiInit(int speed = 19200 * 8) {
    /* Initialize, open device, set bitbang mode w/5 outputs */
    return (FT_Open(0, &handle) == FT_OK 
      //&& FT_SetBitMode(handle, (1 << LED1), 1) == FT_OK 
      /* Actually 9600 * 16 */
      && FT_SetBaudRate(handle, speed / 8) == FT_OK
    );  
  }
  
  void delay(uint16_t ms) {
    Sleep(ms);
  }
  
  void delayMicroseconds(uint16_t us) {
    LARGE_INTEGER startTime; QueryPerformanceCounter(&startTime);
    
    LARGE_INTEGER cpuFreq; QueryPerformanceFrequency(&cpuFreq);
    
    LARGE_INTEGER endTime = { .QuadPart = startTime.QuadPart + us * cpuFreq.QuadPart / uint32_t(1e6) };
    
    LARGE_INTEGER currentTime;
    do {
      QueryPerformanceCounter(&currentTime);
    } while(currentTime.QuadPart < endTime.QuadPart);
  }
}
