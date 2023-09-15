#pragma once

#include <stdint.h>


#define AT93CXX_READ    0x02		   //������

#define AT93CXX_EWEN    0			   //ʹ��д����

#define AT93CXX_ERASE   0x03		   //����ָ��λ��

#define AT93CXX_WRITE   0x01		   //д����

#define AT93CXX_ERAL    0			   //��Ƭ��������

#define AT93CXX_WRAL    0			   //��Ƭд��̶�����

#define AT93CXX_EWDS	0			   //��ֹ����д����������



#ifndef MEM_ORG
	#define MEM_ORG         0              //�洢��������λ��0Ϊ8λ��1Ϊ16λ  ��Ӳ������ORG��Ӧ
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

extern void AT93CXX_SPI_PORT_INIT(void);        //AT93CXX�˿ڳ�ʼ��

extern AT93CXX__Data AT93CXX_Read_Data(AT93CXX__Addr addr);	   //��AT93CXXָ����ַ������

extern void  AT93CXX_EN_Write(void);							     //AT93CXXдʹ��

extern void  AT93CXX_Erase_Write_Disable(void);				     //AT93CXX��ֹд����������

extern void  AT93CXX_Write_Data(AT93CXX__Addr addr, AT93CXX__Data dat);	//��AT93CXXָ����ַд������

extern void AT93CXX_Write_All(AT93CXX__Data dat);					//�����е�ַд��̶�����

extern void AT93CXX_Erase_Dat(AT93CXX__Addr addr);				//�����̶���ַ������

extern void AT93CXX_Erase_All(void);								//��Ƭ����



