#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <windows.h>

#include "_7SegmentsFont.h"
#include "FtdiArduino/FtdiArduino.h"
#include "common.h"

BOOL WINAPI consoleHandler(DWORD signal) {
  if (signal == CTRL_C_EVENT) {
    printf("Ctrl-C handled\n"); // do cleanup
  }
    
  exit(0);
  return TRUE;
}

using namespace FtdiArduino;


const int SHIFT_REG_DATA_PIN = PIN_TX;
const int SHIFT_REG_CLK_PIN = PIN_RX;
const int SHIFT_REG_LATCH_PIN = PIN_CTS;

void SDA2131_shiftOut(uint8_t dataPin, uint8_t clockPin, uint16_t data) {
  //# clear everything out just in case to prepare shift register for bit shifting
  //digitalWrite(dataPin, HIGH);
  //digitalWrite(clockPin, LOW);
  //digitalWrite(dataPin, LOW);
  //digitalWrite(clockPin, HIGH);
  
  for (uint8_t i = 16; i--;)  {
    const uint8_t bit = data & 1;
    data >>= 1;
    digitalWrite(dataPin, bit);
    
    digitalWrite(clockPin, HIGH);
    /*
    if(bit) {
      puts("write HIGH");
    }
    else {
      puts("write LOW");
    }
    */
    //# register shifts bits on upstroke of clock pin  
    digitalWrite(clockPin, LOW);
    //# zero the data pin after shift to prevent bleed through
    //digitalWrite(dataPin, 0);
  }

  //# stop shifting
  //digitalWrite(clockPin, LOW);
  //digitalWrite(dataPin, LOW);
}


uint32_t rol(uint32_t x, int bitsCount) {
  return (x << bitsCount) | (x >> (32 - bitsCount));
}
uint16_t rol(uint16_t x, int bitsCount) {
  return (x << bitsCount) | (x >> (16 - bitsCount));
}

int main(int argc, char * argv[]) {
  using namespace FtdiArduino;
  
  if(!SetConsoleCtrlHandler(consoleHandler, TRUE)) {
    printf("\nERROR: Could not set control handler"); 
    return 1;
  }
  
  if(!ftdiInit()) {
    puts("Can't open device");
    exit(1);
  }
  
  pinMode(SHIFT_REG_LATCH_PIN, OUTPUT);
  digitalWrite(SHIFT_REG_LATCH_PIN, LOW);
  pinMode(SHIFT_REG_DATA_PIN, OUTPUT);
  digitalWrite(SHIFT_REG_DATA_PIN, LOW);
  pinMode(SHIFT_REG_CLK_PIN, OUTPUT);
  digitalWrite(SHIFT_REG_CLK_PIN, LOW);
 
  #define mod(aArg, bArg) ((aArg) % (bArg))
  
  const uint16_t pattern = 0x7FFF;
  if(strcmp(argv[1], "bits") == 0) {
    uint16_t x = strtol(argv[2], null, 2);
    
    loop {
      digitalWrite(SHIFT_REG_LATCH_PIN, HIGH);
      SDA2131_shiftOut(SHIFT_REG_DATA_PIN, SHIFT_REG_CLK_PIN, x | (x << 7));  
      digitalWrite(SHIFT_REG_LATCH_PIN, LOW);
      
      doWrite();
      Sleep(1000);
    }
  }
  else if(strcmp(argv[1], "digit") == 0) {
    uint16_t n = atoi(argv[2]);
    uint16_t x = _7SegmentsFont::digits[mod(n, 10)] | (_7SegmentsFont::digits[mod(n / 10, 10)] << 7);

    loop {
      digitalWrite(SHIFT_REG_LATCH_PIN, HIGH);
      SDA2131_shiftOut(SHIFT_REG_DATA_PIN, SHIFT_REG_CLK_PIN, x);  
      digitalWrite(SHIFT_REG_LATCH_PIN, LOW);
      
      doWrite();
      Sleep(1000);
    }
  }
  else if(strcmp(argv[1], "counter") == 0) {
    uint16_t n = (2 <= argc) ? atoi(argv[2]) : 0;

    loop {
      digitalWrite(SHIFT_REG_LATCH_PIN, HIGH);
      SDA2131_shiftOut(SHIFT_REG_DATA_PIN, SHIFT_REG_CLK_PIN, _7SegmentsFont::digits[mod(n, 10)] | (_7SegmentsFont::digits[mod(n / 10, 10)] << 7));  
      digitalWrite(SHIFT_REG_LATCH_PIN, LOW);
      
      doWrite();
      Sleep(1000);
      n += 1;
    }
  }
  else if(strcmp(argv[1], "letters") == 0) {
    uint16_t n = 1;
    const U8 letters[] = {
      _7SegmentsFont::L, 
      _7SegmentsFont::H, 
      _7SegmentsFont::C, 
      _7SegmentsFont::S, 
      _7SegmentsFont::P, 
    };

    loop {
      digitalWrite(SHIFT_REG_LATCH_PIN, HIGH);
      uint8_t x = letters[n % arraySize(letters)];
      SDA2131_shiftOut(SHIFT_REG_DATA_PIN, SHIFT_REG_CLK_PIN, x | (x << 7));  
      digitalWrite(SHIFT_REG_LATCH_PIN, LOW);
      
      doWrite();
      Sleep(1000);
      n += 1;
    }
  }
  else {
    fprintf(stderr, "Unknown params");
  }
}