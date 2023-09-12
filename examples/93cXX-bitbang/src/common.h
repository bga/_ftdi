#pragma once

#include <stdio.h>

#define ARRAY_SIZE(arr) ((size_t)(sizeof( (arr) ) / sizeof( (arr)[0] )))

#define _BV(xArg) (((uintmax_t)1) << (xArg))
#define SET_BIT(vArg, bitNoArg) vArg |= _BV(bitNoArg) 
#define CLEAR_BIT(vArg, bitNoArg) vArg &= ~_BV(bitNoArg) 
#define HAS_BIT(vArg, bitNoArg) ((vArg) & _BV(bitNoArg))
#define BIT_SHIFT_LEFT(vArg, bitNoArg) ((bitNoArg) < 0 ? ((vArg) >> -(bitNoArg)) : ((vArg) << (bitNoArg)))


#define FT232__BITBANG__PIN__TX  0
#define FT232__BITBANG__PIN__RX  1
#define FT232__BITBANG__PIN__RTS 2
#define FT232__BITBANG__PIN__CTS 3
#define FT232__BITBANG__PIN__DTR 4
#define FT232__BITBANG__PIN__DSR 5
#define FT232__BITBANG__PIN__DCD 6
#define FT232__BITBANG__PIN__RI 7

#define debug(...) do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while(0) 
