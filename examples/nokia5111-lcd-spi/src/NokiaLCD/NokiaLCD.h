/*--------------------------------------------------------------------------------------------------

  Name         :  NokiaLCD.h

  Description  :  Header file for Nokia 84x48 graphic LCD driver.

  Author       :  2003-03-08 - Louis Frigon.

  History      :  2003-03-08 - First release.

--------------------------------------------------------------------------------------------------*/
#ifndef _NOKIALCD_H_
#define _NOKIALCD_H_

#include <stdint.h>

/*--------------------------------------------------------------------------------------------------
                                  General purpose constants
--------------------------------------------------------------------------------------------------*/
// #define NULL                       0
// #define FALSE                      0
// #define TRUE                       1

#define LCD_X_RES                  84
#define LCD_Y_RES                  84

#if 0
  //  Mega8 port B pinout for LCD.
  #define LCD_DC_PIN                 0x02  //  PB1
  #define LCD_CE_PIN                 0x04  //  PB2
  #define SPI_MOSI_PIN               0x08  //  PB3
  #define LCD_RST_PIN                0x10  //  PB4
  #define SPI_CLK_PIN                0x20  //  PB5
  #define LCD_POWER          0x01  //  PB0
#else
  #define LCD_DC_PIN                 2
  #define LCD_CE_PIN                 0x04  //  PB2
  // #define SPI_MOSI_PIN               0x08  //  PB3
  #define LCD_RST_PIN                0
  // #define SPI_CLK_PIN                0x20  //  PB5
  // #define LCD_POWER          0x01  //  PB0
#endif

#define LCD_CACHE_SIZE             ((LCD_X_RES * LCD_Y_RES) / 8)

/*--------------------------------------------------------------------------------------------------
                                       Type definitions
--------------------------------------------------------------------------------------------------*/
// typedef char                       bool;
// typedef unsigned char              uint8_t;
// typedef unsigned int               word;

typedef enum {
  LCD_CMD  = 0,
  LCD_DATA = 1

} LcdCmdData;

typedef enum {
  PIXEL_OFF =  0,
  PIXEL_ON  =  1,
  PIXEL_XOR =  2

} LcdPixelMode;

typedef enum {
  FONT_1X = 1,
  FONT_2X = 2

} LcdFontSize;

/*--------------------------------------------------------------------------------------------------
                                 Public function prototypes
--------------------------------------------------------------------------------------------------*/
extern "C" {
void LcdInit(void);
void LcdPower(uint8_t stat);
void LcdContrast(uint8_t contrast);
void LcdClear(void);
void LcdUpdate(void);
void LcdGotoXY(uint8_t x, uint8_t y);
void LcdChr(LcdFontSize size, uint8_t ch);
void LcdStr(LcdFontSize size, uint8_t* dataPtr);
void LcdStrConst(LcdFontSize size, const char* dataPtr);
void LcdPixel(uint8_t x, uint8_t y, LcdPixelMode mode);
void LcdLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, LcdPixelMode mode);
}
#endif  //  _NOKIALCD_H_
/*--------------------------------------------------------------------------------------------------
                                         End of file.
--------------------------------------------------------------------------------------------------*/

