#pragma once

#include <stdint.h>


#define AT93CXX_READ    0x02		   //读操作

#define AT93CXX_EWEN    0			   //使能写操作

#define AT93CXX_ERASE   0x03		   //擦除指定位置

#define AT93CXX_WRITE   0x01		   //写操作

#define AT93CXX_ERAL    0			   //整片擦除操作

#define AT93CXX_WRAL    0			   //整片写入固定数据

#define AT93CXX_EWDS	0			   //禁止所有写，擦除操作



#ifndef MEM_ORG
	#define MEM_ORG         0              //存储器的数据位宽，0为8位，1为16位  与硬件引脚ORG对应
#endif

extern unsigned AT93CXX__addrWidth;

typedef uint_fast16_t AT93CXX__Addr;
typedef uint_fast16_t AT93CXX__AddrStore;

#if MEM_ORG
	typedef uint_fast16_t AT93CXX__Data;
	typedef uint16_t AT93CXX__DataStore;
#else
	typedef uint_fast8_t AT93CXX__Data;
	typedef uint8_t AT93CXX__DataStore;
#endif

#define AT93CXX__ByteAddrToAddr(addrArg) ((addrArg) / sizeof(AT93CXX__DataStore))

extern void AT93CXX_SPI_PORT_INIT(void);        //AT93CXX端口初始化

extern AT93CXX__Data AT93CXX_Read_Data(AT93CXX__Addr addr);	   //读AT93CXX指定地址的数据

extern void  AT93CXX_EN_Write(void);							     //AT93CXX写使能

extern void  AT93CXX_Erase_Write_Disable(void);				     //AT93CXX禁止写，擦除操作

extern void  AT93CXX_Write_Data(AT93CXX__Addr addr, AT93CXX__Data dat);	//往AT93CXX指定地址写入数据

extern void AT93CXX_Write_All(AT93CXX__Data dat);					//往所有地址写入固定数据

extern void AT93CXX_Erase_Dat(AT93CXX__Addr addr);				//擦除固定地址的数据

extern void AT93CXX_Erase_All(void);								//整片擦除



