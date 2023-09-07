#include "common.h"
#include "at93c66.h"
#include "at93c66-cfg.h"


unsigned AT93CXX__addrWidth;

#if MEM_ORG
	#define AT93CXX__MAKE_ADDR(cmdArg, addrArg) (((uint16_t)((cmdArg) | 0x04) << (AT93CXX__addrWidth - 1)) | (addrArg))
	#define AT93CXX__MAKE_ADDR_FIXED(cmdArg, addrArg) (((uint16_t)((cmdArg) | 0x04) << (AT93CXX__addrWidth - 1)) | ((addrArg) << 1))
	
#else
	#define AT93CXX__MAKE_ADDR(cmdArg, addrArg) (((uint16_t)((cmdArg) | 0x04) << (AT93CXX__addrWidth - 0)) | (addrArg))
	#define AT93CXX__MAKE_ADDR_FIXED(cmdArg, addrArg) (((uint16_t)((cmdArg) | 0x04) << (AT93CXX__addrWidth - 0)) | (addrArg))
#endif


//*************************************************
//��������void AT93CXX_SPI_PORT_INIT( void )
//�����������
//�����������
//���ܣ���ʼ��AT93CXX�˿�
//*************************************************
void AT93CXX_SPI_PORT_INIT(void) {
	AT93CXX_SCS_L;
	AT93CXX_SCS_L;
	AT93CXX_SCK_L;
	AT93CXX_MOSI_L;
}



//*********************************************************
//��������void AT93CXX_SPI_Send_Word(uint16_t dat)
//������������͵�16bit����
//�����������
//���ܣ�ͨ��SPI�ӿڷ���16bit����
//*********************************************************
void AT93CXX_SPI_Send_Word(uint16_t dat) {
	uint16_t i;
	for(i = 0; i != 16; i++) {
		if(dat & 0x8000) {
			AT93CXX_MOSI_H;
		}
		else {
			AT93CXX_MOSI_L;
		}
		
		AT93CXX_SCK_L;
		AT93CXX_SCK_H;
		AT93CXX_SCK_L;
		dat <<= 1;
		
	}
	
}



//*********************************************************
//��������void AT93CXX_SPI_Send_Byte(uint8_t dat)
//������������͵�8bit����
//�����������
//���ܣ�ͨ��SPI�ӿڷ���8bit����
//*********************************************************
void AT93CXX_SPI_Send_Byte(uint8_t dat) {
	uint16_t i;
	for(i = 0; i != 8; i++) {
		if(dat & 0x80) {
			AT93CXX_MOSI_H;
		}
		else {
			AT93CXX_MOSI_L;
		}
		
		AT93CXX_SCK_L;
		AT93CXX_SCK_H;
		AT93CXX_SCK_L;
		dat <<= 1;
		
	}
	
}


//*********************************************************
//��������uint16_t AT93CXX_SPI_Rec_Word( )
//�����������
//���������16bit����
//���ܣ�ͨ��SPI�ӿڽ���16bit����
//*********************************************************
uint16_t AT93CXX_SPI_Rec_Word() {
	uint16_t dat = 0;
	uint8_t i;
	
	for(i = 0; i != 16; i++) {
		dat <<= 1;
		AT93CXX_SCK_L;
		AT93CXX_SCK_H;
		if(AT93CXX_MISO) {
			dat++;
		}
		AT93CXX_SCK_L;
		
	}
	return dat;
}



//*********************************************************
//��������uint16_t AT93CXX_SPI_Rec_Byte( )
//�����������
//���������8bit����
//���ܣ�ͨ��SPI�ӿڽ���8bit����
//*********************************************************
uint16_t AT93CXX_SPI_Rec_Byte() {
	uint8_t dat = 0;
	uint8_t i;
	
	for(i = 0; i != 8; i++) {
		dat <<= 1;
		AT93CXX_SCK_L;
		AT93CXX_SCK_H;
		if(AT93CXX_MISO) {
			dat++;
		}
		AT93CXX_SCK_L;
	}
	return dat;
}

static void AT93CXX_WaitReadyState() {
	AT93CXX_SCS_H;
	while(AT93CXX_MISO == 0); //�ȴ��������
	AT93CXX_SCS_L;
}


//****************************************************************
//��������uint16_t AT93CXX_Read_Data(uint16_t addr)
//�����������ַ
//���������16bit����
//���ܣ���ȡAT93CXXָ����ַ������
//****************************************************************
uint16_t AT93CXX_Read_Data(uint16_t addr) {
	uint16_t address;
	
	AT93CXX_SCS_H;
	

	address = AT93CXX__MAKE_ADDR(AT93CXX_READ, addr);
	
	AT93CXX_SPI_Send_Word(address);
	
#if MEM_ORG
	//16λ���ݴ洢
	address = AT93CXX_SPI_Rec_Word();
#else
	//8λ���ݴ洢
	address = AT93CXX_SPI_Rec_Byte();
#endif
	
	
	AT93CXX_SCS_L;
	
	return address;
}





//****************************************************************
//��������void  AT93CXX_EN_Write( void )
//�����������
//�����������
//���ܣ�ʹ��д�����Ͳ�������
//****************************************************************
void  AT93CXX_EN_Write(void) {
	uint16_t address;
	
	AT93CXX_SCS_H;
	
	address = AT93CXX__MAKE_ADDR_FIXED(AT93CXX_EWEN, 0xc0);
	AT93CXX_SPI_Send_Word(address);
	
	AT93CXX_SCS_L;
	
}




//****************************************************************
//��������void  AT93CXX_Erase_Write_Disable( void )
//�����������
//�����������
//���ܣ���ֹд�����Ͳ�������
//****************************************************************
void  AT93CXX_Erase_Write_Disable(void) {
	uint16_t address;
	
	AT93CXX_SCS_H;
	
	address = AT93CXX__MAKE_ADDR_FIXED(AT93CXX_EWDS, 0);

	AT93CXX_SPI_Send_Word(address);
	
	AT93CXX_SCS_L;
}




//**************************************************************************
//��������void  AT93CXX_Write_Data( uint16_t addr,uint16_t dat )
//�����������ַ������
//�����������
//���ܣ���ָ����ַд������
//****************************************************************************
void  AT93CXX_Write_Data(uint16_t addr, uint16_t dat) {
	uint16_t address;
	
	AT93CXX_SCS_H;
	
	address = AT93CXX__MAKE_ADDR(AT93CXX_WRITE, addr);
	
	AT93CXX_SPI_Send_Word(address);
	
#if MEM_ORG
	//16λ���ݴ洢
	AT93CXX_SPI_Send_Word(dat);
#else
	//8λ���ݴ洢
	AT93CXX_SPI_Send_Byte((uint8_t)dat);
#endif
	AT93CXX_SCS_L;
	
	AT93CXX_WaitReadyState();
}




//**************************************************************************
//��������void AT93CXX_Write_All( uint16_t dat)
//�������������
//�����������
//���ܣ������е�ַд��̶�����
//****************************************************************************
void AT93CXX_Write_All(uint16_t dat) {
	uint16_t address;
	AT93CXX_SCS_H;
	
	address = AT93CXX__MAKE_ADDR_FIXED(AT93CXX_WRAL, 0x40);
	
	AT93CXX_SPI_Send_Word(address);
	
#if MEM_ORG
	//16λ���ݴ洢
	AT93CXX_SPI_Send_Word(dat);
	
#else
	//8λ���ݴ洢
	AT93CXX_SPI_Send_Byte((uint8_t)dat);
#endif
	
	AT93CXX_SCS_L;

	AT93CXX_WaitReadyState();
}




//**************************************************************************
//��������void AT93CXX_Erase_Dat( uint16_t addr)
//�����������ַ
//�����������
//���ܣ�����ָ����ַ����
//****************************************************************************
void AT93CXX_Erase_Dat(uint16_t addr) {
	uint16_t address;
	AT93CXX_SCS_H;
	
	address = AT93CXX__MAKE_ADDR(AT93CXX_ERASE, addr);
	
	AT93CXX_SPI_Send_Word(address);
	
	AT93CXX_SCS_L;
	
	AT93CXX_WaitReadyState();
	
}


//**************************************************************************
//��������void AT93CXX_Erase_All( )
//�����������
//�����������
//���ܣ��������е�ַ����
//****************************************************************************
void AT93CXX_Erase_All() {
	uint16_t address;
	AT93CXX_SCS_H;
	
	address = AT93CXX__MAKE_ADDR_FIXED(AT93CXX_ERAL, 0x80);

	AT93CXX_SPI_Send_Word(address);
	
	AT93CXX_SCS_L;
	
	AT93CXX_WaitReadyState();
	
}









