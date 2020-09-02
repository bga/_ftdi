#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <windows.h>

#include <FtdiArduino/FtdiArduino.h>

using namespace FtdiArduino;

#include "common.h"

BOOL WINAPI consoleHandler(DWORD signal) {
  if (signal == CTRL_C_EVENT) {
    printf("Ctrl-C handled\n"); // do cleanup
  }
  else {
    
  }
  
  exit(0);
    
  return TRUE;
}

const int SHIFT_REG_DATA_PIN = PIN_TX;
const int SHIFT_REG_CLK_PIN = PIN_RX;
const int SHIFT_REG_LATCH_PIN = PIN_CTS;

enum ShiftOutOrder {
  MSBFIRST = 0, 
  LSBFIRST = 1 
}; 

void shiftOut(uint8_t dataPin, uint8_t clockPin, ShiftOutOrder order, uint8_t data) {
  //# clear everything out just in case to prepare shift register for bit shifting
  //digitalWrite(dataPin, 0);
  //digitalWrite(clockPin, 0);
  
  for (uint8_t i = 8; i--;)  {
    digitalWrite(clockPin, 0);
    const uint8_t bit = data & 1;
    data >>= 1;
    digitalWrite(dataPin, bit);
    /*
    if(bit) {
      puts("write HIGH");
    }
    else {
      puts("write LOW");
    }
    */
    //# register shifts bits on upstroke of clock pin  
    digitalWrite(clockPin, 1);
    //# zero the data pin after shift to prevent bleed through
    digitalWrite(dataPin, 0);
  }

  //# stop shifting
  digitalWrite(clockPin, 0);
}

int main(int argc, char * argv[]) {
  if(!SetConsoleCtrlHandler(consoleHandler, TRUE)) {
    printf("\nERROR: Could not set control handler"); 
    return 1;
  }
  
  if(!ftdiInit()) {
    puts("Can't open device");
    exit(1);
  }
  
  pinMode(SHIFT_REG_LATCH_PIN, OUTPUT);
  pinMode(SHIFT_REG_DATA_PIN, OUTPUT);
  pinMode(SHIFT_REG_CLK_PIN, OUTPUT);
  
  digitalWrite(SHIFT_REG_LATCH_PIN, LOW);
  digitalWrite(SHIFT_REG_DATA_PIN, LOW);
  digitalWrite(SHIFT_REG_CLK_PIN, LOW);
 
  #define mod(aArg, bArg) ((aArg) % (bArg))
  
  if(strcmp(argv[1], "bits") == 0) {
    uint16_t x = strtol(argv[2], null, 2);
    
    loop {
      digitalWrite(SHIFT_REG_LATCH_PIN, HIGH);
      shiftOut(SHIFT_REG_DATA_PIN, SHIFT_REG_CLK_PIN, MSBFIRST, x);  
      digitalWrite(SHIFT_REG_LATCH_PIN, LOW);
      
      doWrite();
      Sleep(1000);
    }
  }
  else if(strcmp(argv[1], "bitRotate") == 0) {
    uint16_t x = (2 <= argc) ? strtol(argv[2], null, 2) : 0;

    loop {
      digitalWrite(SHIFT_REG_LATCH_PIN, HIGH);
      shiftOut(SHIFT_REG_DATA_PIN, SHIFT_REG_CLK_PIN, MSBFIRST, x);  
      digitalWrite(SHIFT_REG_LATCH_PIN, LOW);
      
      doWrite();
      x = bitRotate(x, 1);
      Sleep(1000);
    }
  }
  else {
    fprintf(stderr, "Unknown params");
  }
}