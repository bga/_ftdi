

#define AT93CXX_READ    0x02		   //������

#define AT93CXX_EWEN    0			   //ʹ��д����

#define AT93CXX_ERASE   0x03		   //����ָ��λ��

#define AT93CXX_WRITE   0x01		   //д����

#define AT93CXX_ERAL    0			   //��Ƭ��������

#define AT93CXX_WRAL    0			   //��Ƭд��̶�����

#define AT93CXX_EWDS	0			   //��ֹ����д����������



#define MEM_ORG         0              //�洢��������λ��0Ϊ8λ��1Ϊ16λ  ��Ӳ������ORG��Ӧ


extern unsigned AT93CXX__addrWidth;


extern void AT93CXX_SPI_PORT_INIT(void);        //AT93CXX�˿ڳ�ʼ��

extern unsigned short AT93CXX_Read_Data(unsigned short addr);	   //��AT93CXXָ����ַ������

extern void  AT93CXX_EN_Write(void);							     //AT93CXXдʹ��

extern void  AT93CXX_Erase_Write_Disable(void);				     //AT93CXX��ֹд����������

extern void  AT93CXX_Write_Data(unsigned short addr, unsigned short dat);	//��AT93CXXָ����ַд������

extern void AT93CXX_Write_All(unsigned short dat);					//�����е�ַд��̶�����

extern void AT93CXX_Erase_Dat(unsigned short addr);				//�����̶���ַ������

extern void AT93CXX_Erase_All(void);								//��Ƭ����



