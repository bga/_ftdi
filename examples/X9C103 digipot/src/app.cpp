#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <windows.h>

#include <string.h>
//# TODO smart #if(def) 
#define stricmp strcasecmp

#include "FtdiArduino/FtdiArduino.h"
#include "common.h"

BOOL WINAPI consoleHandler(DWORD signal) {

    if (signal == CTRL_C_EVENT)
        printf("Ctrl-C handled\n"); // do cleanup
    
    exit(0);
    
    return TRUE;
}

using namespace FtdiArduino;


const int upNDownPin = PIN_TX;
const int nIncPin = PIN_RX;
const int nCsPin = PIN_CTS;

#define delayUs(delayArg) delayMicroseconds(delayArg)
#define delayMs(delayArg) delay(delayArg)

void setWiperPos(int newWiperPos);

void wiperPosInit() {
  pinMode(nIncPin, OUTPUT);
  pinMode(upNDownPin, OUTPUT);
  pinMode(nCsPin, OUTPUT);
  
  digitalWrite(nCsPin, HIGH);
  delayUs(1);
  
  setWiperPos(0);
}

int lastWiperPos = 100;
void setWiperPos(int newWiperPos) {
  int stepsCount = newWiperPos - lastWiperPos;
  
  if(stepsCount == 0) {
    
  }
  else {
    const int dirBit = ((newWiperPos < lastWiperPos) ? LOW : HIGH);
    int stepsCountAbs = abs(stepsCount);
    
    digitalWrite(upNDownPin, dirBit);
    digitalWrite(nIncPin, HIGH);
    digitalWrite(nCsPin, LOW);
    while(--stepsCountAbs) {
      delayUs(1);
      digitalWrite(nIncPin, LOW);
      delayUs(1);
      digitalWrite(nIncPin, HIGH);
    }
    
    digitalWrite(nIncPin, LOW);
    delayUs(1);
    
    //# no store
    digitalWrite(nCsPin, HIGH);
    lastWiperPos = newWiperPos;
  }
}

void storeWiperPos() {
  digitalWrite(nIncPin, HIGH);
  digitalWrite(nCsPin, LOW);
  delayUs(1);
  digitalWrite(nCsPin, HIGH);
  delayMs(20);
}

void cmdStoreWiperPos(int wiperPos) {
  setWiperPos(wiperPos);
  storeWiperPos();
  fprintf(stdout, "setWiperPos(%d) OK\n", wiperPos);
}

void cmdCandleDemo() {
  int x = 0;
  const int xMax = 3.3 / 5 * 100;
  for(;;) {
    setWiperPos(x);

    x += 1;
    (x == xMax) ? (x = 0) : 0;
    if(1) Sleep(1);
    
    //getchar();
  }
}

int main(int argc, char *argv[]) {
  using namespace FtdiArduino;
  
  if(!SetConsoleCtrlHandler(consoleHandler, TRUE)) {
    printf("\nERROR: Could not set control handler"); 
    return 1;
  }
  
  if(!ftdiInit()) {
    puts("Can't open device");
    exit(1);
  }
  
  wiperPosInit();
  if(0) {  }
  else if(argc == 1) {
    cmdCandleDemo();
  }
  else if(argc == 2 && stricmp(argv[1], "candleDemo") == 0) {
    cmdCandleDemo();
  }
  else if(argc == 3 && stricmp(argv[1], "storeWiperPos") == 0) {
    cmdStoreWiperPos(atoi(argv[2]));
  }
  else {
    fprintf(stderr, "Unknown option\n");
  }
  
  return 0;
}