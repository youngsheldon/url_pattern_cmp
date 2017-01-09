#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <iostream>
using namespace std;

#include "cryptodes.h"
#include "commonlib.h"


CDES1::CDES1()
{
	memset(basicKey, 0x00, LEN_OF_KEY1);
	
	DES_set_key_unchecked((const_DES_cblock*)basicKey, &ks);
}

CDES1::~CDES1()
{
	
}

bool CDES1::init(const char *key_)
{
	int keyLen;
	
	keyLen = strlen(key_);
	if(keyLen > LEN_OF_KEY1)
	{
		return false;
	}
	memcpy(basicKey, key_, keyLen);     
	memset(basicKey + keyLen, 0x00, LEN_OF_KEY1 - keyLen);
	
	DES_set_key_unchecked((const_DES_cblock*)basicKey, &ks);
	
	return true;
	
}

/*
 * get key from database
 * 
 */
bool CDES1::getKey()
{
/*	SybConnection *conn= NULL;
	SybCommand *cmd= NULL;
	SybResultSet *rs= NULL;
	
	conn = SybManager::GetConnection(this->Serv.c_str(), this->App.c_str(),
			this->User.c_str(), this->Pass.c_str());
	if (conn == NULL) 
	{
		return table;
	}
	conn->ChangeDB(this->db.c_str());
	cmd = conn->CreateCommand();
	
	if (cmd == NULL) 
	{
		conn->Close();
		return false;
	}
	
	rs = cmd->ExecuteQuery("");
	if (rs == NULL) 
	{
		cmd->Close();
		conn->Close();
		return false;
	}
	
	
	rs->Close();
	cmd->Close();
	conn->Close();*/
	return true;
}

/*
 * 1des加密 
 * cipherOut 需要完成初始化后再传进来
 * retrun:   密文长度
 */
int CDES1::doEncrypt(const UCHAR* plain, UCHAR* cipherOut, int plainLen)
{
	int encryptCount;           //加密的分组数
	int paddingLen;             //明文不足8个字节的长度
	int plainLength;            //补齐后的明文长度
	UCHAR ch;                   //明文不足8个字节的填充内容
	UCHAR *plainSrc;            //补齐后的明文
	UCHAR *cipherDst;           //加密后的密文
	UCHAR *cipherHex;           //密文的十六进制串
	UCHAR plainBlock[8];        //8个字节明文块
	UCHAR cipherBlock[8];       //8个字节密文块
	
	paddingLen = 8 - plainLen % 8;
	plainLength = plainLen + paddingLen;
	ch = paddingLen;
	encryptCount = plainLength / 8;
	
	cipherHex = (UCHAR *)malloc(plainLength * 2 * sizeof(UCHAR));
	plainSrc = (UCHAR *)malloc(plainLength * sizeof(UCHAR));
	cipherDst = (UCHAR *)malloc(plainLength * sizeof(UCHAR));
	
	if (plainSrc==NULL || cipherDst==NULL)
	{
		return 0;
	}
	
	
	memset(cipherDst, 0, plainLength);	
	memset(plainSrc, 0, plainLength);
	memcpy(plainSrc, plain, plainLen);            //拷贝明文串
	memset(plainSrc+plainLen, ch, paddingLen);    //填充指定的内容
	
	for (int i=0; i<encryptCount; i++)
	{
		memset(plainBlock, 0, 8);
		memset(cipherBlock, 0, 8);
		memcpy(plainBlock, plainSrc + 8 * i, 8);   //截取8个字节长度的明文
		
		DES_ecb_encrypt((const_DES_cblock*)plainBlock, (DES_cblock*)cipherBlock, &ks, DES_ENCRYPT); //加密处理
		
		memcpy(cipherDst + 8 * i, cipherBlock, 8);
	}
	
	for (int i=0; i<plainLength; i++)
	{
		cipherHex[i*2]   = zs_encode(((unsigned char) *(cipherDst+i)>>4)&0x0F);
		cipherHex[i*2+1] = zs_encode( (unsigned char) *(cipherDst+i)    &0x0F);
		
//		printf("%02x ", cipherHex[i]);
	}
	
	memcpy(cipherOut, cipherHex, plainLength*2); //拷贝密文
	
	if (plainSrc != NULL)
	{
		free(plainSrc);
		plainSrc = NULL;
	}
	
	if (cipherDst != NULL)
	{
		free(cipherDst);
		cipherDst = NULL;
	}
	
	if (cipherHex != NULL)
	{
		free(cipherHex);
		cipherDst = NULL;
	}
	
	plainLength = plainLength * 2;
	
	return plainLength;
}

/*
 * 1des密文解密
 * plainOut 需要完成初始化的工作 memset(plainOut, 0, len)
 * return : 明文长度
 */
int CDES1::doDecrypt(const UCHAR * cipher, UCHAR * plainOut, int cipherLen)
{
	int decryptCount;           //解密的分组数
	int plainLen;               //明文长度
	UCHAR *plainDst;            //明文
	UCHAR *cipherSrc;           //密文
	UCHAR *cipherDst;           //转化后的密文
	UCHAR hightmp;             
	UCHAR lowtmp;                
	UCHAR plainBlock[8];        //8个字节明文块
	UCHAR cipherBlock[8];       //8个字节密文块
	
	decryptCount = cipherLen / 8;
	plainLen = cipherLen /2;
	
    plainDst = (UCHAR *)malloc(cipherLen * sizeof(UCHAR));
	cipherSrc = (UCHAR *)malloc(cipherLen * sizeof(UCHAR));
	cipherDst = (UCHAR *)malloc(cipherLen * sizeof(UCHAR));
	
	if (plainDst==NULL || cipherSrc==NULL)
	{
		return 0;
	}
	
	memset(plainDst, 0, cipherLen);	
	memset(cipherSrc, 0, cipherLen);
	memcpy(cipherSrc, cipher, cipherLen);            //拷贝密文串
	
	//将十六进制字符串还原
	for (int i=0,j=0; i<cipherLen; i=i+2,j++)
	{
		hightmp = zs_decode(*(cipherSrc+i));
		lowtmp = zs_decode(*(cipherSrc+i+1));
		
		cipherDst[j] = (hightmp<<4) | lowtmp;
		
//		printf("%02x ", cipherDst[j]);
	}
	
	for (int i=0; i<decryptCount; i++)
	{
		memset(plainBlock, 0, 8);
		memset(cipherBlock, 0, 8);
		memcpy(cipherBlock, cipherDst + 8 * i, 8);   //截取8个字节长度的密文
		
		DES_ecb_encrypt((const_DES_cblock*)cipherBlock, (DES_cblock*)plainBlock, &ks, DES_DECRYPT); //解密处理
		
		memcpy(plainDst + 8 * i, plainBlock, 8);
	}
	
	memcpy(plainOut, plainDst, plainLen); //拷贝明文
	
	if (plainDst != NULL)
	{
		free(plainDst);
		plainDst = NULL;
	}
	
	if (cipherSrc != NULL)
	{
		free(cipherSrc);
		cipherSrc = NULL;
	}
	
	if (cipherDst != NULL)
	{
		free(cipherDst);
		cipherDst = NULL;
	}
	
	return plainLen;
}

CDESede::CDESede()
{
	UCHAR keyBlock[8];
	
	memset(keyBlock, 0, sizeof(keyBlock));
	memset(basic3Key, 0x00, LEN_OF_KEY3); //初始密钥0x00
	
	memcpy(keyBlock, basic3Key + 0, 8);         
	DES_set_key_unchecked((const_DES_cblock*)keyBlock, &ks1);         
	memcpy(keyBlock, basic3Key + 8, 8);         
	DES_set_key_unchecked((const_DES_cblock*)keyBlock, &ks2);         
	memcpy(keyBlock, basic3Key + 16, 8);         
	DES_set_key_unchecked((const_DES_cblock*)keyBlock, &ks3);
}

CDESede::~CDESede()
{
	
}

bool CDESede::init(const char *key_)
{
	int keyLen;
	UCHAR keyBlock[8];
	
	keyLen = strlen(key_);
	if (keyLen > LEN_OF_KEY3)
	{
		return false;
	}
	memcpy(basic3Key, key_, keyLen);     
	memset(basic3Key + keyLen, 0x00, LEN_OF_KEY3 - keyLen); //补齐密钥	
	
	memset(keyBlock, 0, sizeof(keyBlock));         
	memcpy(keyBlock, basic3Key + 0, 8);         
	DES_set_key_unchecked((const_DES_cblock*)keyBlock, &ks1);         
	memcpy(keyBlock, basic3Key + 8, 8);         
	DES_set_key_unchecked((const_DES_cblock*)keyBlock, &ks2);         
	memcpy(keyBlock, basic3Key + 16, 8);         
	DES_set_key_unchecked((const_DES_cblock*)keyBlock, &ks3);
	
	return true;
}

/*
 * 3des加密
 * cipherOut 需要完成初始化后再传进来
 * retrun:   密文长度
 */
int CDESede::doEncrypt(const UCHAR* plain, UCHAR* cipherOut, int plainLen)
{
	int encryptCount;           //加密的分组数
	int paddingLen;             //明文不足8个字节的长度
	int plainLength;            //补齐后的明文长度
	UCHAR ch;                   //明文不足8个字节的填充内容
	UCHAR *plainSrc;            //补齐后的明文
	UCHAR *cipherDst;           //加密后的密文
	UCHAR *cipherHex;           //密文的十六进制串
	UCHAR plainBlock[8];         //8个字节明文块
	UCHAR cipherBlock[8];        //8个字节密文块
	
	paddingLen = 8 - plainLen % 8;
	plainLength = plainLen + paddingLen;
	ch = paddingLen;
	encryptCount = plainLength / 8;
	
	plainSrc = (UCHAR *)malloc(plainLength * sizeof(UCHAR));
	cipherDst = (UCHAR *)malloc(plainLength * sizeof(UCHAR));
	cipherHex = (UCHAR *)malloc(plainLength*2 * sizeof(UCHAR));
	
	if (plainSrc==NULL || cipherDst==NULL)
	{
		return 0;
	}
	
	memset(cipherHex, 0, plainLength*2);
	memset(cipherDst, 0, plainLength);
	memset(plainSrc, 0, plainLength);
	memcpy(plainSrc, plain, plainLen);            //拷贝明文串
	memset(plainSrc+plainLen, ch, paddingLen);    //填充指定的内容
	
	for (int i=0; i<encryptCount; i++)
	{
		memset(plainBlock, 0, 8);
		memset(cipherBlock, 0, 8);
		memcpy(plainBlock, plainSrc + 8 * i, 8);   //截取8个字节长度的明文
		
		DES_ecb3_encrypt((const_DES_cblock*)plainBlock, (DES_cblock*)cipherBlock, &ks1, &ks2, &ks3, DES_ENCRYPT); //加密处理
		
		memcpy(cipherDst + 8 * i, cipherBlock , 8);
	}
	
	for (int i=0; i<plainLength; i++)
	{
		cipherHex[i*2]   = zs_encode(((unsigned char) *(cipherDst+i)>>4)&0x0F);
		cipherHex[i*2+1] = zs_encode( (unsigned char) *(cipherDst+i)    &0x0F);
		
//		printf("%02x ", cipherHex[i]);
	}
	
	memcpy(cipherOut, cipherHex, plainLength*2); //拷贝密文
	
	if (plainSrc != NULL)
	{
		free(plainSrc);
		plainSrc = NULL;
	}
	
	if (cipherDst != NULL)
	{
		free(cipherDst);
		cipherDst = NULL;
	}
	
	if (cipherHex != NULL)
	{
		free(cipherHex);
		cipherDst = NULL;
	}
	
	return plainLength;
}

/*
 * 1des密文解密
 * plainOut 需要完成初始化的工作 memset(plainOut, 0, len)
 * return : 明文长度
 */
int CDESede::doDecrypt(const UCHAR * cipher, UCHAR * plainOut, int cipherLen)
{
	int decryptCount;           //解密的分组数
	int plainLen;               //明文长度
	UCHAR *plainDst;            //明文
	UCHAR *cipherSrc;           //密文
	UCHAR *cipherDst;           //转化后的密文
	UCHAR hightmp;             
	UCHAR lowtmp;                
	UCHAR plainBlock[8];        //8个字节明文块
	UCHAR cipherBlock[8];       //8个字节密文块
	
	decryptCount = cipherLen / 8;
	plainLen = cipherLen / 2;
	
    plainDst = (UCHAR *)malloc(cipherLen * sizeof(UCHAR));
	cipherSrc = (UCHAR *)malloc(cipherLen * sizeof(UCHAR));
	cipherDst = (UCHAR *)malloc(cipherLen * sizeof(UCHAR));
	
	if (plainDst==NULL || cipherSrc==NULL)
	{
		return 0;
	}
	
	memset(plainDst, 0, cipherLen);	
	memset(cipherSrc, 0, cipherLen);
	memcpy(cipherSrc, cipher, cipherLen);            //拷贝密文串
	
	//将十六进制字符串还原
	for (int i=0,j=0; i<cipherLen; i=i+2,j++)
	{
		hightmp = zs_decode(*(cipherSrc+i));
		lowtmp = zs_decode(*(cipherSrc+i+1));
		
		cipherDst[j] = (hightmp<<4) | lowtmp;
		
//		printf("%02x ", cipherDst[j]);
	}
	
	for (int i=0; i<decryptCount; i++)
	{
		memset(plainBlock, 0, 8);
		memset(cipherBlock, 0, 8);
		memcpy(cipherBlock, cipherDst + 8 * i, 8);   //截取8个字节长度的密文
		
		DES_ecb3_encrypt((const_DES_cblock*)plainBlock, (DES_cblock*)cipherBlock, &ks1, &ks2, &ks3, DES_DECRYPT); //解密处理
		
		memcpy(plainDst + 8 * i, plainBlock, 8);
	}
	
	memcpy(plainOut, plainDst, plainLen); //拷贝明文
	
	if (plainDst != NULL)
	{
		free(plainDst);
		plainDst = NULL;
	}
	
	if (cipherSrc != NULL)
	{
		free(cipherSrc);
		cipherSrc = NULL;
	}
	
	if (cipherDst != NULL)
	{
		free(cipherDst);
		cipherDst = NULL;
	}
	
	return plainLen;
}

