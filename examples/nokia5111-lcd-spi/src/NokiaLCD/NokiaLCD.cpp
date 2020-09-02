/*--------------------------------------------------------------------------------------------------

  Name         :  NokiaLCD.c

  Description  :  This is a driver for the Nokia 84x48 graphic LCD.

  Author       :  2003-03-08 - Sylvain Bissonnette

  History      :  2003-02-08 - First release (v0.1) derived from Sylvain Bissonnette code base.
                  2003-03-09 - v0.2, Louis Frigon: 2x fonts support.
                  2003-03-20 - v0.3: Serialization optimized,

  Info       :  Max RAM    - 512 uint8_t
           :  Max Stack  - 10  uint8_t

--------------------------------------------------------------------------------------------------*/
// #include <macros.h>
// #include <iom8v.h>
//#include <stdio.h>

#include "NokiaLCD.h"
#include "font5x7.h"

#include "ftdiMpSseShim.h"
using namespace FtdiArduino;


#define LCD_FIRMWARE_VERSION       0.3

/*--------------------------------------------------------------------------------------------------
                                Private function prototypes
--------------------------------------------------------------------------------------------------*/
//  Function prototypes are mandatory otherwise the compiler generates unreliable code.

static void LcdSend(uint8_t data, LcdCmdData cd);
static void Delay(void);

/*--------------------------------------------------------------------------------------------------
                                      Global Variables
--------------------------------------------------------------------------------------------------*/
static uint8_t  LcdCache [ LCD_CACHE_SIZE ];

static int   LcdCacheIdx;
static int   LoWaterMark;
static int   HiWaterMark;
static unsigned char  UpdateLcd;


/*--------------------------------------------------------------------------------------------------

  Name         :  LcdPower ( uint8_t stat )

  Description  :  Performs MCU SPI & LCD controller initialization.

  Argument(s)  :  stat -> true or false

  Return value :  None.

  Notes        :  Power ON or OFF LCD

--------------------------------------------------------------------------------------------------*/
void LcdPower(uint8_t stat) {
  if(stat) {
    LcdInit();
  }
  else {
    // PORTB = 0xc0;   // All LCD pin at 0
    // SPCR = 0x00;    // Disable SPI
  }
}

/*--------------------------------------------------------------------------------------------------

  Name         :  LcdInit

  Description  :  Performs MCU SPI & LCD controller initialization.

  Argument(s)  :  None.

  Return value :  None.

--------------------------------------------------------------------------------------------------*/
void LcdInit(void) {
  static uint8_t FirstInit = TRUE;

  //  Pull-up on reset pin.
  // PORTB |= LCD_RST_PIN;

  //  Set output bits on port B.
  pinMode(LCD_RST_PIN, OUTPUT);
  pinMode(LCD_DC_PIN, OUTPUT);
  pinMode(LCD_CE_PIN, OUTPUT);
  // DDRB |= LCD_RST_PIN | LCD_DC_PIN | LCD_CE_PIN | SPI_MOSI_PIN | SPI_CLK_PIN | LCD_POWER;

  // PORTB |= LCD_POWER;

  // Delay();

  //  Toggle display reset pin.
  digitalWrite(LCD_RST_PIN, 0);
  _delay_ms(1);
  digitalWrite(LCD_RST_PIN, 1);

  //  Enable SPI port: No interrupt, MSBit first, Master mode, CPOL->0, CPHA->0, Clk/4
  // SPCR = 0x50;

  //  Disable LCD controller
  digitalWrite(LCD_CE_PIN, 1);

  LcdSend(0x21, LCD_CMD);    // LCD Extended Commands.
  LcdSend(0xC8, LCD_CMD);    // Set LCD Vop (Contrast).
  LcdSend(0x06, LCD_CMD);    // Set Temp coefficent.
  LcdSend(0x13, LCD_CMD);    // LCD bias mode 1:48.
  LcdSend(0x20, LCD_CMD);    // LCD Standard Commands, Horizontal addressing mode.
  LcdSend(0x0C, LCD_CMD);    // LCD in normal mode.

  if(FirstInit == TRUE) {
    LoWaterMark = LCD_CACHE_SIZE;
    HiWaterMark = 0;
    LcdClear();
    FirstInit = FALSE;
  }
  else {
    LoWaterMark = 0;
    HiWaterMark = LCD_CACHE_SIZE;
  }
  LcdUpdate();
}

/*--------------------------------------------------------------------------------------------------

  Name         :  LcdContrast

  Description  :  Set display contrast.

  Argument(s)  :  contrast -> Contrast value from 0x00 to 0x7F.

  Return value :  None.

  Notes        :  No change visible at ambient temperature.

--------------------------------------------------------------------------------------------------*/
void LcdContrast(uint8_t contrast) {
  //  LCD Extended Commands.
  LcdSend(0x21, LCD_CMD);

  // Set LCD Vop (Contrast).
  LcdSend(0x80 | contrast, LCD_CMD);

  //  LCD Standard Commands, horizontal addressing mode.
  LcdSend(0x20, LCD_CMD);
}

/*--------------------------------------------------------------------------------------------------

  Name         :  LcdClear

  Description  :  Clears the display. LcdUpdate must be called next.

  Argument(s)  :  None.

  Return value :  None.

--------------------------------------------------------------------------------------------------*/
void LcdClear(void) {
  int i;

  for(i = 0; i < LCD_CACHE_SIZE; i++) {
    LcdCache[i] = 0x00;
  }

  //  Reset watermark pointers.
  LoWaterMark = 0;
  HiWaterMark = LCD_CACHE_SIZE - 1;

  UpdateLcd = TRUE;
}

/*--------------------------------------------------------------------------------------------------

  Name         :  LcdGotoXY

  Description  :  Sets cursor location to xy location corresponding to basic font size.

  Argument(s)  :  x, y -> Coordinate for new cursor position. Range: 1,1 .. 14,6

  Return value :  None.

--------------------------------------------------------------------------------------------------*/
void LcdGotoXY(uint8_t x, uint8_t y) {
  LcdCacheIdx = (x - 1) * 6 + (y - 1) * 84;
}

/*--------------------------------------------------------------------------------------------------

  Name         :  LcdChr

  Description  :  Displays a character at current cursor location and increment cursor location.

  Argument(s)  :  size -> Font size. See enum.
                  ch   -> Character to write.

  Return value :  None.

--------------------------------------------------------------------------------------------------*/
void LcdChr(LcdFontSize size, uint8_t ch) {
  uint8_t i, c;
  uint8_t b1, b2;
  int  tmpIdx;

  if(LcdCacheIdx < LoWaterMark) {
    //  Update low marker.
    LoWaterMark = LcdCacheIdx;
  }

  if(ch < 0x20) {
    ch = 148;
  }

  if(ch > 151) { // Convert ISO8859-1 to ascii
    if(ch == 0xc0) {
      ch = 133;
    }
    else if(ch == 0xc2) {
      ch = 131;
    }
    else if(ch == 0xc7) {
      ch = 128;
    }
    else if(ch == 0xc9) {
      ch = 144;
    }
    else if(ch == 0xca) {
      ch = 136;
    }
    else if(ch == 0xce) {
      ch = 140;
    }
    else if(ch == 0xe0) {
      ch = 133;
    }
    else if(ch == 0xe2) {
      ch = 131;
    }
    else if(ch == 0xe7) {
      ch = 135;
    }
    else if(ch == 0xe8) {
      ch = 138;
    }
    else if(ch == 0xe9) {
      ch = 130;
    }
    else if(ch == 0xea) {
      ch = 136;
    }
    else if(ch == 0xeb) {
      ch = 137;
    }
    else if(ch == 0xee) {
      ch = 140;
    }
    else if(ch == 0xef) {
      ch = 139;
    }
    else if(ch == 0xf4) {
      ch = 147;
    }
    else if(ch == 0xf9) {
      ch = 151;
    }
    else if(ch == 0xfb) {
      ch = 150;
    }
    else {
      ch = 148;
    }
  }

  if(size == FONT_1X) {
    for(i = 0; i < 5; i++) {
      LcdCache[LcdCacheIdx++] = FontLookup[ch - 32][i] << 1;
    }
  }
  else if(size == FONT_2X) {
    tmpIdx = LcdCacheIdx - 84;

    if(tmpIdx < LoWaterMark) {
      LoWaterMark = tmpIdx;
    }

    if(tmpIdx < 0) {
      return;
    }

    for(i = 0; i < 5; i++) {
      c = FontLookup[ch - 32][i] << 1;
      b1 = (c & 0x01) * 3;
      b1 |= (c & 0x02) * 6;
      b1 |= (c & 0x04) * 12;
      b1 |= (c & 0x08) * 24;

      c >>= 4;
      b2 = (c & 0x01) * 3;
      b2 |= (c & 0x02) * 6;
      b2 |= (c & 0x04) * 12;
      b2 |= (c & 0x08) * 24;

      LcdCache[tmpIdx++] = b1;
      LcdCache[tmpIdx++] = b1;
      LcdCache[tmpIdx + 82] = b2;
      LcdCache[tmpIdx + 83] = b2;
    }

    //  Update x cursor position.
    LcdCacheIdx += 11;
  }

  if(LcdCacheIdx > HiWaterMark) {
    //  Update high marker.
    HiWaterMark = LcdCacheIdx;
  }

  //  Horizontal gap between characters.
  LcdCache[LcdCacheIdx++] = 0x00;
  UpdateLcd = TRUE;
}

/*--------------------------------------------------------------------------------------------------

  Name         :  LcdStr

  Description  :  Displays a character at current cursor location and increment cursor location
                  according to font size.

  Argument(s)  :  size    -> Font size. See enum.
                  dataPtr -> Pointer to null terminated ASCII string to display.

  Return value :  None.

--------------------------------------------------------------------------------------------------*/
void LcdStr(LcdFontSize size, uint8_t* dataPtr) {
  while(*dataPtr) {
    LcdChr(size, *dataPtr++);
  }
}

/*--------------------------------------------------------------------------------------------------

  Name         :  LcdStrConst

  Description  :  Displays a character at current cursor location and increment cursor location
                  according to font size.

  Argument(s)  :  size    -> Font size. See enum.
                  dataPtr -> Pointer to null terminated ASCII string to display.

  Return value :  None.

--------------------------------------------------------------------------------------------------*/
void LcdStrConst(LcdFontSize size, const char* dataPtr) {
  while(*dataPtr) {
    LcdChr(size, *dataPtr++);
  }
}

/*--------------------------------------------------------------------------------------------------

  Name         :  LcdPixel

  Description  :  Displays a pixel at given absolute (x, y) location.

  Argument(s)  :  x, y -> Absolute pixel coordinates
                  mode -> Off, On or Xor. See enum.

  Return value :  None.

--------------------------------------------------------------------------------------------------*/
void LcdPixel(uint8_t x, uint8_t y, LcdPixelMode mode) {
  uint16_t  index;
  uint8_t  offset;
  uint8_t  data;

  if(x > LCD_X_RES) {
    return;
  }
  if(y > LCD_Y_RES) {
    return;
  }

  index = ((y / 8) * 84) + x;
  offset  = y - ((y / 8) * 8);

  data = LcdCache[index];

  if(mode == PIXEL_OFF) {
    data &= (~(0x01 << offset));
  }
  else if(mode == PIXEL_ON) {
    data |= (0x01 << offset);
  }
  else if(mode  == PIXEL_XOR) {
    data ^= (0x01 << offset);
  }

  LcdCache[index] = data;

  if(index < LoWaterMark) {
    //  Update low marker.
    LoWaterMark = index;
  }

  if(index > HiWaterMark) {
    //  Update high marker.
    HiWaterMark = index;
  }
  UpdateLcd = TRUE;
}

/*--------------------------------------------------------------------------------------------------

  Name         :  LcdLine

  Description  :  Draws a line between two points on the display.

  Argument(s)  :  x1, y1 -> Absolute pixel coordinates for line origin.
                  x2, y2 -> Absolute pixel coordinates for line end.
                  mode   -> Off, On or Xor. See enum.

  Return value :  None.

--------------------------------------------------------------------------------------------------*/
void LcdLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, LcdPixelMode mode) {
  int dx, dy, stepx, stepy, fraction;

  dy = y2 - y1;
  dx = x2 - x1;

  if(dy < 0) {
    dy    = -dy;
    stepy = -1;
  }
  else {
    stepy = 1;
  }

  if(dx < 0) {
    dx    = -dx;
    stepx = -1;
  }
  else {
    stepx = 1;
  }

  dx <<= 1;
  dy <<= 1;

  LcdPixel(x1, y1, mode);

  if(dx > dy) {
    fraction = dy - (dx >> 1);
    while(x1 != x2) {
      if(fraction >= 0) {
        y1 += stepy;
        fraction -= dx;
      }
      x1 += stepx;
      fraction += dy;
      LcdPixel(x1, y1, mode);
    }
  }
  else {
    fraction = dx - (dy >> 1);
    while(y1 != y2) {
      if(fraction >= 0) {
        x1 += stepx;
        fraction -= dy;
      }
      y1 += stepy;
      fraction += dx;
      LcdPixel(x1, y1, mode);
    }
  }

  UpdateLcd = TRUE;
}

/*--------------------------------------------------------------------------------------------------

  Name         :  LcdUpdate

  Description  :  Copies the LCD cache into the device RAM.

  Argument(s)  :  None.

  Return value :  None.

--------------------------------------------------------------------------------------------------*/
void LcdUpdate(void) {
  int i;

  if(LoWaterMark < 0) {
    LoWaterMark = 0;
  }
  else if(LoWaterMark >= LCD_CACHE_SIZE) {
    LoWaterMark = LCD_CACHE_SIZE - 1;
  }

  if(HiWaterMark < 0) {
    HiWaterMark = 0;
  }
  else if(HiWaterMark >= LCD_CACHE_SIZE) {
    HiWaterMark = LCD_CACHE_SIZE - 1;
  }

  //  Set base address according to LoWaterMark.
  LcdSend(0x80 | (LoWaterMark % LCD_X_RES), LCD_CMD);
  LcdSend(0x40 | (LoWaterMark / LCD_X_RES), LCD_CMD);

  //  Serialize the video buffer.
  for(i = LoWaterMark; i <= HiWaterMark; i++) {
    LcdSend(LcdCache[i], LCD_DATA);
  }

  //  Reset watermark pointers.
  LoWaterMark = LCD_CACHE_SIZE - 1;
  HiWaterMark = 0;

  UpdateLcd = FALSE;
}

/*--------------------------------------------------------------------------------------------------

  Name         :  LcdSend

  Description  :  Sends data to display controller.

  Argument(s)  :  data -> Data to be sent
                  cd   -> Command or data (see/use enum)

  Return value :  None.

--------------------------------------------------------------------------------------------------*/
static void LcdSend(uint8_t data, LcdCmdData cd) {
#if 0
  //  Enable display controller (active low).
  PORTB &= ~LCD_CE_PIN;

  if(cd == LCD_DATA) {
    PORTB |= LCD_DC_PIN;
  }
  else {
    PORTB &= ~LCD_DC_PIN;
  }

  //  Send data to display controller.
  SPDR = data;

  //  Wait until Tx register empty.
  while((SPSR & 0x80) != 0x80);

  //  Disable display controller.
  PORTB |= LCD_CE_PIN;

  // Only for 38Khz share pin for remote controle
  PORTB &= ~LCD_DC_PIN;
#else
  digitalWrite(LCD_DC_PIN, cd == LCD_DATA);
  SPI_write(data);
#endif
}

/*--------------------------------------------------------------------------------------------------

  Name         :  Delay

  Description  :  Uncalibrated delay for LCD init routine.

  Argument(s)  :  None.

  Return value :  None.

--------------------------------------------------------------------------------------------------*/
static void Delay(void) {
  int i;

  for(i = -32000; i < 32000; i++);
}

/*--------------------------------------------------------------------------------------------------
                                         End of file.
--------------------------------------------------------------------------------------------------*/

