#pragma once


#include <stdint.h>
#include "common.h"
#include "config.h"

extern uint8_t ft232_byte;
uint8_t ft232_read();
void ft232_write();


#define AT93CXX_SCS_H     SET_BIT(ft232_byte, AT93CXX_SCS_BIT); ft232_write()
#define AT93CXX_SCS_L     CLEAR_BIT(ft232_byte, AT93CXX_SCS_BIT); ft232_write()

#define AT93CXX_SCK_H     SET_BIT(ft232_byte, AT93CXX_SCK_BIT); ft232_write() 
#define AT93CXX_SCK_L     CLEAR_BIT(ft232_byte, AT93CXX_SCK_BIT); ft232_write() 

#define AT93CXX_MOSI_H     SET_BIT(ft232_byte, AT93CXX_MOSI_BIT); ft232_write()
#define AT93CXX_MOSI_L     CLEAR_BIT(ft232_byte, AT93CXX_MOSI_BIT); ft232_write()

#define AT93CXX_MISO       (ft232_read() & _BV(AT93CXX_MISO_BIT))
