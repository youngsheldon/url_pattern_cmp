#include <openssl/des.h> 

/************************************************************************  
 * **  *
 * *    
 * ecb���ܷ�ʽ��  
 *     
 * ��Կ:   ����λ���Ҳ�0x00��  
 * ���ݣ�   ��������8λ���룬���뷽ʽΪ����1λ��һ��0x01,��2λ������0x02,...  
 * **     ������8λ����ģ����油�˸�0x08��  
 * ************************************************************************/

#ifndef CRYPTO_DES_H_
#define CRYPTO_DES_H_

#define LEN_OF_KEY1              8		//1des��Կ�ֽ���
#define LEN_OF_KEY3              24     //3des��Կ�ֽ���
typedef unsigned char UCHAR;

class CDES1
{
public:
	CDES1();
	~CDES1();
	bool init(const char *key_);
	bool getKey();
	int doEncrypt(const UCHAR * plain, UCHAR * cipherOut, int plainLen);
	int doDecrypt(const UCHAR * cipher, UCHAR * plainOut, int cipherLen);
	
private:
	char basicKey[8];				 //8λ������Կ
	DES_key_schedule ks;             //���첹������Կ 
};

class CDESede : public CDES1
{
public:
	CDESede();
	~CDESede();
	bool init(const char *key_);
	int doEncrypt(const UCHAR * plain, UCHAR * cipherOut, int plainLen);
	int doDecrypt(const UCHAR * cipher, UCHAR * plainOut, int cipherLen);
	
private:
	char basic3Key[24];				 //24λ������Կ
	DES_key_schedule ks1,ks2,ks3;     //���첹������Կ 
};

#endif /*CRYPTO_DES_H_*/
