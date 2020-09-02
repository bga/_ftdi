#include <stdint.h>

namespace SSD1306 {

enum {
	CMD_DISPLAY_OFF = 0xAE,
	CMD_DISPLAY_ON = 0xAF,

	CMD_SET_DISP_START_LINE = 0x40,
	CMD_SET_PAGE = 0xB0,

	CMD_SET_COLUMN_UPPER = 0x10,
	CMD_SET_COLUMN_LOWER = 0x00,

	CMD_SET_ADC_NORMAL = 0xA0,
	CMD_SET_ADC_REVERSE = 0xA1,

	CMD_SET_DISP_NORMAL = 0xA6,
	CMD_SET_DISP_REVERSE = 0xA7,

	CMD_SET_ALLPTS_NORMAL = 0xA4,
	CMD_SET_ALLPTS_ON = 0xA5,
	CMD_SET_BIAS_9 = 0xA2,
	CMD_SET_BIAS_7 = 0xA3,

	CMD_RMW = 0xE0,
	CMD_RMW_CLEAR = 0xEE,
	CMD_INTERNAL_RESET = 0xE2,
	CMD_SET_COM_NORMAL = 0xC0,
	CMD_SET_COM_REVERSE = 0xC8,
	CMD_SET_POWER_CONTROL = 0x28,
	CMD_SET_RESISTOR_RATIO = 0x20,
	CMD_SET_VOLUME_FIRST = 0x81,
	CMD_SET_VOLUME_SECOND = 0,
	CMD_SET_STATIC_OFF = 0xAC,
	CMD_SET_STATIC_ON = 0xAD,
	CMD_SET_STATIC_REG = 0x0,
	CMD_SET_BOOSTER_FIRST = 0xF8,
	CMD_SET_BOOSTER_234 = 0,
	CMD_SET_BOOSTER_5 = 1,
	CMD_SET_BOOSTER_6 = 3,
	CMD_NOP = 0xE3,
	CMD_TEST = 0xF0,
};

//# internal command
// void spiwrite(uint8_t c);

extern uint8_t buffer[128*64/8];

void st7565_init(void);
void st7565_command(uint8_t c);
void st7565_data(uint8_t c);
void st7565_set_brightness(uint8_t val);


void clear_screen(void);
void clear_buffer(uint8_t *buffer);
void write_buffer(uint8_t *buffer);

// void testdrawbitmap(uint8_t *buff, const uint8_t *bitmap, uint8_t w, uint8_t h);

} // namespace SSD1306
