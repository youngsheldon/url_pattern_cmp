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
 * 1des���� 
 * cipherOut ��Ҫ��ɳ�ʼ�����ٴ�����
 * retrun:   ���ĳ���
 */
int CDES1::doEncrypt(const UCHAR* plain, UCHAR* cipherOut, int plainLen)
{
	int encryptCount;           //���ܵķ�����
	int paddingLen;             //���Ĳ���8���ֽڵĳ���
	int plainLength;            //���������ĳ���
	UCHAR ch;                   //���Ĳ���8���ֽڵ��������
	UCHAR *plainSrc;            //����������
	UCHAR *cipherDst;           //���ܺ������
	UCHAR *cipherHex;           //���ĵ�ʮ�����ƴ�
	UCHAR plainBlock[8];        //8���ֽ����Ŀ�
	UCHAR cipherBlock[8];       //8���ֽ����Ŀ�
	
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
	memcpy(plainSrc, plain, plainLen);            //�������Ĵ�
	memset(plainSrc+plainLen, ch, paddingLen);    //���ָ��������
	
	for (int i=0; i<encryptCount; i++)
	{
		memset(plainBlock, 0, 8);
		memset(cipherBlock, 0, 8);
		memcpy(plainBlock, plainSrc + 8 * i, 8);   //��ȡ8���ֽڳ��ȵ�����
		
		DES_ecb_encrypt((const_DES_cblock*)plainBlock, (DES_cblock*)cipherBlock, &ks, DES_ENCRYPT); //���ܴ���
		
		memcpy(cipherDst + 8 * i, cipherBlock, 8);
	}
	
	for (int i=0; i<plainLength; i++)
	{
		cipherHex[i*2]   = zs_encode(((unsigned char) *(cipherDst+i)>>4)&0x0F);
		cipherHex[i*2+1] = zs_encode( (unsigned char) *(cipherDst+i)    &0x0F);
		
//		printf("%02x ", cipherHex[i]);
	}
	
	memcpy(cipherOut, cipherHex, plainLength*2); //��������
	
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
 * 1des���Ľ���
 * plainOut ��Ҫ��ɳ�ʼ���Ĺ��� memset(plainOut, 0, len)
 * return : ���ĳ���
 */
int CDES1::doDecrypt(const UCHAR * cipher, UCHAR * plainOut, int cipherLen)
{
	int decryptCount;           //���ܵķ�����
	int plainLen;               //���ĳ���
	UCHAR *plainDst;            //����
	UCHAR *cipherSrc;           //����
	UCHAR *cipherDst;           //ת���������
	UCHAR hightmp;             
	UCHAR lowtmp;                
	UCHAR plainBlock[8];        //8���ֽ����Ŀ�
	UCHAR cipherBlock[8];       //8���ֽ����Ŀ�
	
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
	memcpy(cipherSrc, cipher, cipherLen);            //�������Ĵ�
	
	//��ʮ�������ַ�����ԭ
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
		memcpy(cipherBlock, cipherDst + 8 * i, 8);   //��ȡ8���ֽڳ��ȵ�����
		
		DES_ecb_encrypt((const_DES_cblock*)cipherBlock, (DES_cblock*)plainBlock, &ks, DES_DECRYPT); //���ܴ���
		
		memcpy(plainDst + 8 * i, plainBlock, 8);
	}
	
	memcpy(plainOut, plainDst, plainLen); //��������
	
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
	memset(basic3Key, 0x00, LEN_OF_KEY3); //��ʼ��Կ0x00
	
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
	memset(basic3Key + keyLen, 0x00, LEN_OF_KEY3 - keyLen); //������Կ	
	
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
 * 3des����
 * cipherOut ��Ҫ��ɳ�ʼ�����ٴ�����
 * retrun:   ���ĳ���
 */
int CDESede::doEncrypt(const UCHAR* plain, UCHAR* cipherOut, int plainLen)
{
	int encryptCount;           //���ܵķ�����
	int paddingLen;             //���Ĳ���8���ֽڵĳ���
	int plainLength;            //���������ĳ���
	UCHAR ch;                   //���Ĳ���8���ֽڵ��������
	UCHAR *plainSrc;            //����������
	UCHAR *cipherDst;           //���ܺ������
	UCHAR *cipherHex;           //���ĵ�ʮ�����ƴ�
	UCHAR plainBlock[8];         //8���ֽ����Ŀ�
	UCHAR cipherBlock[8];        //8���ֽ����Ŀ�
	
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
	memcpy(plainSrc, plain, plainLen);            //�������Ĵ�
	memset(plainSrc+plainLen, ch, paddingLen);    //���ָ��������
	
	for (int i=0; i<encryptCount; i++)
	{
		memset(plainBlock, 0, 8);
		memset(cipherBlock, 0, 8);
		memcpy(plainBlock, plainSrc + 8 * i, 8);   //��ȡ8���ֽڳ��ȵ�����
		
		DES_ecb3_encrypt((const_DES_cblock*)plainBlock, (DES_cblock*)cipherBlock, &ks1, &ks2, &ks3, DES_ENCRYPT); //���ܴ���
		
		memcpy(cipherDst + 8 * i, cipherBlock , 8);
	}
	
	for (int i=0; i<plainLength; i++)
	{
		cipherHex[i*2]   = zs_encode(((unsigned char) *(cipherDst+i)>>4)&0x0F);
		cipherHex[i*2+1] = zs_encode( (unsigned char) *(cipherDst+i)    &0x0F);
		
//		printf("%02x ", cipherHex[i]);
	}
	
	memcpy(cipherOut, cipherHex, plainLength*2); //��������
	
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
 * 1des���Ľ���
 * plainOut ��Ҫ��ɳ�ʼ���Ĺ��� memset(plainOut, 0, len)
 * return : ���ĳ���
 */
int CDESede::doDecrypt(const UCHAR * cipher, UCHAR * plainOut, int cipherLen)
{
	int decryptCount;           //���ܵķ�����
	int plainLen;               //���ĳ���
	UCHAR *plainDst;            //����
	UCHAR *cipherSrc;           //����
	UCHAR *cipherDst;           //ת���������
	UCHAR hightmp;             
	UCHAR lowtmp;                
	UCHAR plainBlock[8];        //8���ֽ����Ŀ�
	UCHAR cipherBlock[8];       //8���ֽ����Ŀ�
	
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
	memcpy(cipherSrc, cipher, cipherLen);            //�������Ĵ�
	
	//��ʮ�������ַ�����ԭ
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
		memcpy(cipherBlock, cipherDst + 8 * i, 8);   //��ȡ8���ֽڳ��ȵ�����
		
		DES_ecb3_encrypt((const_DES_cblock*)plainBlock, (DES_cblock*)cipherBlock, &ks1, &ks2, &ks3, DES_DECRYPT); //���ܴ���
		
		memcpy(plainDst + 8 * i, plainBlock, 8);
	}
	
	memcpy(plainOut, plainDst, plainLen); //��������
	
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

