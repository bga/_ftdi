#pragma once

#include <windows.h>
namespace Ftdi {
	#include <ftMPSSE/libMPSSE_spi.h>
}
#include <!cpp/bitManipulations.h>
// #include <!cpp/debug.h>
#include <!cpp/common.h>

namespace App {
	extern Ftdi::FT_HANDLE ftHandle;
}

namespace FtdiArduino {

#define BRRL_192 192
void uart_init(U8 bitRate) {

}

void putstring_nl(const char * t) {
	// Debug_print("%s", t);
}

void _delay_ms(int ms) {
	Sleep(ms);
}

// U8 DDRB, PORTB, PINB;
// U8 DDRD, PORTD, PIND;

uint8_t portMode = 0;
uint8_t portValue = 0;


enum Mode {
	INPUT = 0,
	OUTPUT = 1
};

void pinMode(uint8_t pin, Mode mode) {
	setBitValue(portMode, pin, !!mode);
}

enum {
	LOW = 0,
	HIGH = 1
};

void digitalWrite(uint8_t pin, uint8_t value) {
	setBitValue(portValue, pin, value);

	Ftdi::FT_WriteGPIO(App::ftHandle, portMode, !!portValue);
	//Sleep(500);
}

void SPI_write(U8 c) {
	using namespace Ftdi;
	uint32 bytesTransfered = 0;
	SPI_Write(App::ftHandle, &c, 1, &bytesTransfered,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES | SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE | SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE
	);
}
} // namespace FtdiArduino
