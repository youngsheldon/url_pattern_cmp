
#ifndef _COMMONLIB_H_
#define _COMMONLIB_H_

#include <stdio.h>
#include <unistd.h>
#include <iconv.h>
#include <stdlib.h>
#include <string.h>

//******************************************************
//This part is for System Wide Definition

#define MODE_NODEBUG     0
#define MODE_DEBUG_LOG   1
#define MODE_DEBUG_PRINT 2
#define INVALID_SOCKET 	 -1

#define SYSTEM_CONFIG_PATH "conf/"
#define SYSTEM_LOG_PATH    "log/"
#define SYSTEM_SHELL_PATH  "proc/"

#define INIFILENAME	SYSTEM_CONFIG_PATH "sysconfig.ini"


#ifdef	WIN32
#define TEMPPATH		"c:"
#else
#define TEMPPATH		"/tmp"
#endif
#define TEMPFILE		"TEMPFILE.TXT"

//End part for System Wide Definition
//******************************************************

//******************************************************
//This part is for MD5
short getMD5Digest(const char* input,short inlen,char* output,short retbit);
//End part for MD5
//******************************************************

//******************************************************
//This part is for Codepage Convert and Other Decode
int General_Convert(const char* from,const char* to,char* srcbuf,int& srclen,char* outbuf,int& outlen);
int Codepage_Convert(iconv_t* converter,char* srcbuf,int& srclen,char*outbuf,int& outlen);
//End part ICONV
//**********************************************************************

//**********************************************************************
//This part is for CommonUse

#ifndef MAX
#define MAX(X,Y)	((X) > (Y)) ? (X) : (Y)
#endif
#ifndef MIN
#define MIN(X,Y)	((X) < (Y)) ? (X) : (Y)
#endif

char zs_encode(char ch);
char zs_decode(char ch);
int MachineCode2Unicode(unsigned char* out,int& outlen,const char* in,int inlen);
int bit7convert(unsigned char *buffer, int length,char * output);

short GetLocalIP(char* buf);
unsigned long GetLocalIP_L();

//2012-08-20，新增通过网卡名获取IP
short GetLocalIP(char* buf, const char *NIC);
unsigned long GetLocalIP_L(const char *NIC);

short Check_Endian(void);
int Output_to_Screen(int outputtype,char* info,int infolen);
int Output_to_Logfile(int outputtype,char* filename,char* info,int infolen);

int GetTimedFileName(char* buf,char* path,char* pre);

extern int ___gDebugMode;
int log_init(int mode,const char* logfile);
void log_request(const char* fmt,...);
void log_cleanup();

//End part CommonUse
//**********************************************************************

//**********************************************************************
//This part is for Socket
int nConnect( char * ip, unsigned short port);
int nListen(unsigned short port,int backlog);
int nAccept(int listenid,unsigned long& ulip,char* sip,int& port);
ssize_t readn(int fd, void *vptr, size_t n);
ssize_t writen(int fd, const void *vptr, size_t n);
//End part Socket
//**********************************************************************

//**********************************************************************
//This part is for IniFileHandle

//this is for ini file like
//##COL1    COL2    COL3 ...... COLn
//  value1  value2  value3  ... valuen
//  value1' value2' value3' ... valuen'
class FPRT
{
public:
	enum Return_Type
	{
		RT_EOLINE,
		RT_EOFILE,
		RT_SUCCESS,
		RT_COMMENT,
		RT_DEFAULT,
		RT_PARSE_ERROR
	};
};
typedef FPRT::Return_Type FP_RETURN_TYPE;

template <class ENTRY> class File_Parser
{
public:
	virtual ~File_Parser (void);
	int open (const char filename[]);
	int close (void);
	virtual FP_RETURN_TYPE read_entry (ENTRY &entry,int &line_number) = 0;
protected:
	FP_RETURN_TYPE getword (char buf[]);
	FP_RETURN_TYPE getint (int &value);
	FP_RETURN_TYPE readword (char buf[]);
	int delimiter (char ch);
	int comments (char ch);
	int skipline (void);
	FILE *infile_;
};

//this is for ini file like 
//[GROUP1]
//		NAME1=VALUE1
//		NAME2=VALUE2
//[GROUP2]
//		NAME3=VALUE3
#define MacCFGS_LineLength	1024
#define MacCFGS_CharDivide	'#'
#define MacCFGS_CharEscape	'\\'
#define MacCFGS_CharAssign	'='
#define MacCFGS_LeftsQuote	'['
#define MacCFGS_RightQuote	']'
char *	sCfgsReadString(const char * sFileName, char * sGroupName,char * sIdentName, char * sRet, int nSize );
char *	sCfgsReadNumber(const char * sFilename, char * sGroupName,char * sIdentName, int  * pnRet );
char *	sCfgsReadLongNu(const char * sFilename, char * sGroupName,char * sIdentName, long * plRet );
char *	sCfgsReadGrpOff(const char * sFileName, int nGrpOff,char * sGrpNam, int nSize );
char *	sCfgsReadItmLin(const char * sFileName, char * sSect, int nItmLin,char * sItmLin, int nSize );
int 	nvCfgsReadItmDat(const char * sFileName, char * sSect, char * sIdnt,char * sFmt, ... );
int 	nvCfgsReadItmOff(const char * sFileName, char * sSect, int nItmOff,char * sFmt, ... );
void	vCfgsLTrim(char* sOprt );
void	vCfgsRTrim(char* sOprt );
void	vCfgsATrim(char* sOprt );
void	vCfgsEscCh(char* sOprt,bool bDivide);

//End part IniFileHandle
//**********************************************************************

#endif
