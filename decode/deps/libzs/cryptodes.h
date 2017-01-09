#include <openssl/des.h> 

/************************************************************************  
 * **  *
 * *    
 * ecb加密方式；  
 *     
 * 密钥:   不足位的右补0x00；  
 * 内容：   加密内容8位补齐，补齐方式为：少1位补一个0x01,少2位补两个0x02,...  
 * **     本身已8位对齐的，后面补八个0x08。  
 * ************************************************************************/

#ifndef CRYPTO_DES_H_
#define CRYPTO_DES_H_

#define LEN_OF_KEY1              8		//1des密钥字节数
#define LEN_OF_KEY3              24     //3des密钥字节数
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
	char basicKey[8];				 //8位基础密钥
	DES_key_schedule ks;             //构造补齐后的密钥 
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
	char basic3Key[24];				 //24位基础密钥
	DES_key_schedule ks1,ks2,ks3;     //构造补齐后的密钥 
};

#endif /*CRYPTO_DES_H_*/
