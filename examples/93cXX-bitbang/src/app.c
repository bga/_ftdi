/* This program is distributed under the GPL, version 2 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libftdi1/ftdi.h>

#include "common.h"
#include "config.h"
#include "at93c66-cfg.h"
#include "at93c66.h"

const char help[] = (""
	"%s chipId [read|write|verify|fill|erase|write-disable] readOffset [readSize|'max']" 
	"\n\tread to stdout"
	"\n\twrites from stdin"
	"\n\tverify from stdin and return non 0 if failed"
	"\n\tfill read 1 byte from stdin and fill region. Vcc should be 5v!"
	"\n\terase erase region. if 0 max then fast erase all cmd is used. Vcc should be 5v!"
	"\n\twrite-disable temporary prevent future writing or erasing"
	"\n\t\tany modification operation unlock it automatically"
	"\n"
	"\n\taccepted env vars FT232__DEV_VID=num FT232__DEV_PID=num FT232__BAUDRATE=num"
);

#define APP__TRACE(...) do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while(0) 
#define EXIT__ERROR(exitCodeArg, ...) do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n");	exit((exitCodeArg)); } while(0) 

int APP__GETENV_OR_DEFAULT__INT__impl(char const* name, int defaultV) {
	char const* vStr = getenv(name);
	if(vStr == NULL) return defaultV;
	int v;
	if(1 != sscanf(vStr,"%i", &v)) EXIT__ERROR(EXIT_FAILURE, "Unable to parse env var %s=%s", name, vStr);
	return v;
}
#define APP__GETENV_OR_DEFAULT__INT(nameArg) APP__GETENV_OR_DEFAULT__INT__impl(#nameArg, nameArg)


typedef struct Chip {
	char const* name;
	unsigned addrBitWidth;
	unsigned size;
} Chip;

struct Chip chipDb[] = {
	{ "93c46", 7, 128 },
	{ "93c56", 8, 256 },
	{ "93c66", 9, 512 },
	{ "93c76", 10, 1024 },
	{ "93c86", 11, 2048 },
};

#define FT232__CHECK_ERROR(fArg) EXIT__ERROR((fArg), "Generic ftdi error %i %s", (fArg), ftdi_get_error_string(ftdi))

struct ftdi_context* ftdi;
uint8_t ft232_byte = 0;
uint8_t ft232_byte_old = 0;
uint8_t ft232_read() {
	uint8_t ret;
	int f = ftdi_read_pins(ftdi, &ret);
	debug("r %u", (unsigned)ret);
	if(f < 0) FT232__CHECK_ERROR(f);
	return ret;
}
void ft232_write() {
	if(ft232_byte_old == ft232_byte) return;
	uint8_t b = ft232_byte;
	debug("w %u", (unsigned)b);
	int f = ftdi_write_data(ftdi, &b, 1);
	ft232_byte_old = ft232_byte;
	if(f != 1) FT232__CHECK_ERROR(f);
}

int ft232_init(uint8_t bitmask) {
	if((ftdi = ftdi_new()) == NULL) {
		fprintf(stderr, "ftdi_new failed\n");
		return -1;
	};

	return (ftdi_usb_open(ftdi, APP__GETENV_OR_DEFAULT__INT(FT232__DEV_VID), APP__GETENV_OR_DEFAULT__INT(FT232__DEV_PID)) 
		|| ftdi_set_baudrate(ftdi, APP__GETENV_OR_DEFAULT__INT(FT232__BAUDRATE)) 
		|| ftdi_set_bitmode(ftdi, 0, BITMODE_RESET)
		|| ftdi_set_bitmode(ftdi, bitmask, BITMODE_BITBANG)
	);
}
void	ft232_deinit() {
	if(ftdi == NULL) return;
	ftdi_disable_bitbang(ftdi);
	
	ftdi_usb_close(ftdi);
	ftdi_free(ftdi);
	ftdi = NULL;
}


int main(int argc, char* argv[]) {
	int retval = EXIT_SUCCESS;
	char const* const programName = argv[0];
	
	int maxArg = 1;
	char const* chipIdStr = argv[maxArg++];
	char const* verbStr = argv[maxArg++];
	char const* readOffsetStr = argv[maxArg++];
	char const* readSizeStr = argv[maxArg++];
	
	
	if(argc < maxArg) {
		EXIT__ERROR(EXIT_SUCCESS, help, programName);
	};
	
		
	size_t chipIndex = 0;
	for(; chipIndex < ARRAY_SIZE(chipDb); chipIndex += 1) {
		if(strcmp(chipIdStr, chipDb[chipIndex].name) == 0) {
			break;
		};
	}

	if(chipIndex == ARRAY_SIZE(chipDb)) {
		EXIT__ERROR(EXIT_FAILURE, "Unknown chip %s", chipIdStr);
	};
	
	struct Chip const* const chipPtr = &chipDb[chipIndex];
	
	AT93CXX__addrWidth = chipPtr->addrBitWidth;
	debug("chipPtr->addrBitWidth = %u", (unsigned)chipPtr->addrBitWidth);
	
	unsigned readOffset;
	if(sscanf(readOffsetStr, "%u", &readOffset) != 1) {
		EXIT__ERROR(EXIT_FAILURE, "Unable to parse readOff t %s,", readOffsetStr);
	};

	unsigned readSize;
	if(strcmp(readSizeStr, "max") == 0) {
		readSize = chipPtr->size;
	}
	else if(sscanf(readSizeStr, "%u", &readSize) != 1) {
		EXIT__ERROR(EXIT_FAILURE, "Unable to parse readSize %s,", readSizeStr);
	};
	
	const uint8_t portBitmask = _BV(AT93CXX_MOSI_BIT) | _BV(AT93CXX_SCK_BIT) | _BV(AT93CXX_SCS_BIT);
	for(int f = ft232_init(portBitmask);;) {
		if(f < 0) {
			FT232__CHECK_ERROR(f);
		};
		break;
	}
	
	unsigned readEnd = readOffset + readSize;
	if(chipPtr->size < readEnd) {
		readEnd = chipPtr->size; 
	};
	
	if(0);
	else if(strcmp(verbStr, "read") == 0) {
		AT93CXX_SPI_PORT_INIT();
		for(size_t i = readOffset; i < readEnd; i += sizeof(AT93CXX__DataStore)) {
			APP__TRACE("reading address %u", (unsigned)i);
			AT93CXX__Data byte = AT93CXX_Read_Data(AT93CXX__ByteAddrToAddr(i));
			fwrite(&byte, 1, sizeof(AT93CXX__DataStore), stdout);
		}
	}
	else if(strcmp(verbStr, "write") == 0) {
		AT93CXX_SPI_PORT_INIT();
		AT93CXX_EN_Write();
		AT93CXX__Data byte;
		for(size_t i = readOffset; i < readEnd; i += sizeof(AT93CXX__DataStore)) {
			fread(&byte, 1, sizeof(AT93CXX__DataStore), stdin);
			APP__TRACE("writing address %u", (unsigned)i);
			AT93CXX_Write_Data(AT93CXX__ByteAddrToAddr(i), byte);
		}
	}
	else if(strcmp(verbStr, "erase") == 0) {
		AT93CXX_SPI_PORT_INIT();
		AT93CXX_EN_Write();
		if(readOffset == 0 && readSize == chipPtr->size) {
			APP__TRACE("Warning: Erase all requires VCC=5v+-10%%");
			AT93CXX_Erase_All();
		}
		else {
			AT93CXX__Data byte;
			for(size_t i = readOffset; i < readEnd; i += sizeof(AT93CXX__DataStore)) {
				APP__TRACE("erasing address %u", (unsigned)i);
				AT93CXX_Erase_Dat(AT93CXX__ByteAddrToAddr(i));
			}
		}
	}
	else if(strcmp(verbStr, "fill") == 0) {
		APP__TRACE("Warning: Fillig all requires VCC=5v+-10%%");
		AT93CXX_SPI_PORT_INIT();
		AT93CXX_EN_Write();
		AT93CXX__Data byte;
		fread(&byte, 1, sizeof(AT93CXX__DataStore), stdin);
		APP__TRACE("filling all memory with %u", (unsigned)byte);
		AT93CXX_Write_All(byte);
	}
	else if(strcmp(verbStr, "verify") == 0) {
		AT93CXX_SPI_PORT_INIT();
		AT93CXX__Data byte;
		for(size_t i = readOffset; i < readEnd; i += sizeof(AT93CXX__DataStore)) {
			fread(&byte, 1, sizeof(AT93CXX__DataStore), stdin);
			APP__TRACE("reading address %u", (unsigned)i);
			AT93CXX__Data byte2 = AT93CXX_Read_Data(AT93CXX__ByteAddrToAddr(i));
			if(byte != byte2) {
				APP__TRACE("verify failed at address %u, got %u, expected %u", (unsigned)i, (unsigned)byte2, (unsigned)byte);
				retval = EXIT_FAILURE;
			}
		}
	}
	else if(strcmp(verbStr, "write-disable") == 0) {
		AT93CXX_SPI_PORT_INIT();
		AT93CXX_Erase_Write_Disable();
	}
	else {
		EXIT__ERROR(EXIT_FAILURE, "Unknown verb %s", verbStr);
	}
	
	ft232_deinit();
	
	return retval;
}
