#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>   
#include <fstream>
#include <openssl/md5.h>
#include "commonlib.h"

//******************************************************
//This part is for MD5

short getMD5Digest(const char* input,short inlen,char* output,short retbit)
{
	if(retbit==16)
	{
		MD5((const unsigned char*)input,inlen,(unsigned char*)output);
		return 16;
	}
	else if (retbit==32)
	{
		int p=0;
		unsigned char buf[17];
		MD5((const unsigned char*)input,inlen,buf);
		for (int i = 0; i < 16; i++)
		{
			sprintf(output+p,"%02X", buf[i]);
			p+=2;
		}
		*(output+32)=0;
		return 32;
	}
	else
	{
		*output=0;
		return 0;
	}
}

//End part for MD5
//******************************************************


//******************************************************
//This part is for ICONV

int General_Convert(const char* from,const char* to,char* srcbuf,int& srclen,char* outbuf,int& outlen)
{
	iconv_t cd;
	
	if(from==NULL||to==NULL||strlen(from)==0||strlen(to)==0)
		return 0;
	if(srcbuf==NULL||outbuf==NULL||srclen<1)
		return 0;
	cd = iconv_open(to,from);
	if(cd == (iconv_t) (-1))
		return 0;
	Codepage_Convert(&cd,srcbuf,srclen,outbuf,outlen);
	iconv_close(cd);
	return 1;
}

int Codepage_Convert(iconv_t* converter,char* srcbuf,int& srclen,char*outbuf,int& outlen)
{
	int ret;
	char*  ip   = srcbuf;
	char*  op   = outbuf;
	size_t ilen = srclen;
	size_t olen = outlen;
	
	ret = (int)iconv(*converter,(char**)&ip,&ilen,&op,&olen);
	while(ret==-1 && ilen>0)
	{
		ip++;
		ilen--;
		if(ilen>0)
			ret = iconv(*converter,(char**)&ip,&ilen,&op,&olen);
	}
	srclen = ilen;
	outlen = olen;
	return ret;
}

//End part for ICONV
//******************************************************

//**********************************************************************
//This part is for CommonUse

short numchar_decode(char ch)
{
	short n;
	switch(ch)
	{
	case '0':
		n = 0;
		break;
	case '1':
		n = 1;
		break;
	case '2':
		n = 2;
		break;
	case '3':
		n = 3;
		break;
	case '4':
		n = 4;
		break;
	case '5':
		n = 5;
		break;
	case '6':
		n = 6;
		break;
	case '7':
		n = 7;
		break;
	case '8':
		n = 8;
		break;
	case '9':
		n = 9;
		break;
	default:
		n = -1;
	}
	return n;
}

unsigned char* pack(unsigned char* out,int&outlen,unsigned char ch)
{
	unsigned short value = ch;
//	value = htons(value);
	memcpy(out,&value,2);
	outlen +=2;
	return out+2;
}

int MachineCode2Unicode(unsigned char* out,int& outlen,const char* in,int inlen)
{
	int		inpos = 0;
	short		i,s,t;
	unsigned char*	pos;
	unsigned short	value;
	unsigned char	a,b,c;
	unsigned char	numbuf[10];

	i = 0xFEFF;
	memcpy(out,&i,2);
	outlen = 2;
	pos = out+2;
	while(inpos<inlen && outlen<1024)
	{
		a = *(in+inpos);
		if (a!='&')
		{
			pos = pack(pos,outlen,a);
			inpos ++;
			continue;
		}
		b = *(in+inpos+1);
		if (b!='#')
		{
			pos = pack(pos,outlen,a);
			inpos ++;
			continue;
		}
		value = 0;
		t = 0;	//this is first met '&#',so we need to buffer the following numbers
		while(1)
		{
			c = *(in+inpos+2+t);
			s = numchar_decode((char)c);
			if (s==-1)
			{
				if (t==0||c!=';')
				{
					pos = pack(pos,outlen,a);
					pos = pack(pos,outlen,b);
					for(i=0;i<t-1;i++)
						pos = pack(pos,outlen,numbuf[i]);
					inpos = inpos+1+t;
					break;
				}
				else
				{
//					value = htons(value);
					memcpy(pos,&value,2);
					outlen +=2;
					pos +=2;
					inpos = inpos+3+t;
					break;
				}
			}
			else
			{
				if (t<8)
				{
					value = value*10 + s;
					numbuf[t] = c;
				}
				else
				{
					pos = pack(pos,outlen,a);
					pos = pack(pos,outlen,b);
					for(i=0;i<t;i++)
						pos = pack(pos,outlen,numbuf[i]);
					inpos = inpos+3+t;
					pos = pack(pos,outlen,c);
					break;
				}
			}
			t++;
		}
	}
	return 0;
}

const char MASK_M[7] = {0x01,0x03,0x07,0x0F,0x1F,0x3F,0x7F};
int bit7convert(unsigned char *buffer, int length,char * output)
{
	unsigned char *tempbuf;
	int i, j, k;

	tempbuf = (unsigned char*)output;
	j = 1;
	k = 6;
	tempbuf[0] = buffer[0] & 0x7f;
	for(i = 1; i< length; i++)
	{
		if(k != 0)
		{
			tempbuf[j] = (buffer[i-1]>>(k+1))|((buffer[i]& MASK_M[k-1])<<(7-k));
			k --;
		}
		else
		{
			tempbuf[j] = buffer[i-1]>>1;
			j ++;
			tempbuf[j] = buffer[i]& MASK_M[6];
			k = 6;
		}
		j ++;
	}
	tempbuf[j] = 0;
	return j;
}

//获取本机IP地址
short GetLocalIP(char* buf)
{
	int sock;
	struct sockaddr_in sin;
	struct ifreq ifr;
	
	sock = socket(AF_INET,SOCK_DGRAM,0);
	if (sock == -1)
	{
		perror("socket");
		return -1;
	}
	strncpy(ifr.ifr_name,"eth0",IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = 0;
	if(ioctl(sock,SIOCGIFADDR,&ifr) < 0)
	{
		perror("ioctl");
		return -1;
	}
	close(sock);
	memcpy(&sin,&ifr.ifr_addr,sizeof(sin));
	strcpy(buf,inet_ntoa(sin.sin_addr));
	return strlen(buf);
}

//获取本机IP地址
unsigned long GetLocalIP_L()
{
	int sock;   
	struct sockaddr_in sin;   
	struct ifreq ifr;

	sock = socket(AF_INET,SOCK_DGRAM,0);   
	if (sock == -1)
	{
		perror("socket");   
		return 0;   
	}
	strncpy(ifr.ifr_name,"eth0",IFNAMSIZ);  
	ifr.ifr_name[IFNAMSIZ-1] = 0;   
	if (ioctl(sock,SIOCGIFADDR,&ifr) < 0)   
	{
		perror("ioctl");   
		return 0;   
	}
	close(sock);
	memcpy(&sin,&ifr.ifr_addr,sizeof(sin));   
	return sin.sin_addr.s_addr;   
}

//2012-08-20,通过网卡名获取本机IP地址
short GetLocalIP(char* buf, const char *NIC)
{
	int sock;
	struct sockaddr_in sin;
	struct ifreq ifr;

	sock = socket(AF_INET,SOCK_DGRAM,0);
	if (sock == -1)
	{
		perror("socket");
		return -1;
	}
	strncpy(ifr.ifr_name,NIC,IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = 0;
	if(ioctl(sock,SIOCGIFADDR,&ifr) < 0)
	{
		perror("ioctl");
		return -1;
	}
	close(sock);
	memcpy(&sin,&ifr.ifr_addr,sizeof(sin));
	strcpy(buf,inet_ntoa(sin.sin_addr));
	return strlen(buf);
}

//2012-08-20，通过网卡名获取本机IP地址
unsigned long GetLocalIP_L(const char *NIC)
{
	int sock;
	struct sockaddr_in sin;
	struct ifreq ifr;

	sock = socket(AF_INET,SOCK_DGRAM,0);
	if (sock == -1)
	{
		perror("socket");
		return 0;
	}
	strncpy(ifr.ifr_name,NIC,IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = 0;
	if (ioctl(sock,SIOCGIFADDR,&ifr) < 0)
	{
		perror("ioctl");
		return 0;
	}
	close(sock);
	memcpy(&sin,&ifr.ifr_addr,sizeof(sin));
	return sin.sin_addr.s_addr;
}

//获得整型数的存储方式(高位在前或是低位在前)
//返回值：	1 -> 高位在前
//			2 -> 低位在前
//			0 -> 出错
short Check_Endian(void)
{
	union
	{
		short  s;
		char   c[sizeof(short)];
	}un;

	un.s = 0x0102;
	if (sizeof(short) == 2)
	{
		if (un.c[0] == 1 && un.c[1] == 2)
			return 1;
		else if (un.c[0] == 2 && un.c[1] == 1)
			return 2;
		else
			return 0;
	}
	else
		return 0;
}

//十六进制数转成ASCII码
char zs_encode(char ch)
{
	char out = 0;

	switch(ch)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		out = ch + 48;
		break;
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
		out = ch + 55;
		break;
	}
	return out;
}

//ASCII码转成十六进制数
char zs_decode(char ch)
{
	char out = 0;
	switch(ch)
	{
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		out = (0x0F & ch);
		break;
	case 'a':
	case 'A':
		out = 0x0A;
		break;
	case 'b':
	case 'B':
		out = 0x0B;
		break;
	case 'c':
	case 'C':
		out = 0x0C;
		break;
	case 'd':
	case 'D':
		out = 0x0D;
		break;
	case 'e':
	case 'E':
		out = 0x0E;
		break;
	case 'f':
	case 'F':
		out = 0x0F;
		break;
	}
	return out;
}

//调试信息输出函数(字符或十六进制)，输出到屏幕
int Output_to_Screen(int outputtype,char* info,int infolen)
{
	time_t now = 0;
	char *buf,*part;
	unsigned char *temp;
	int linenum,n,rest;
	if (outputtype!=1&&outputtype!=2)
	{
		printf("**ERROR** Output type must be (1:as text) or (2:as binary)\n");
		return -1;
	}
	if (infolen<1||infolen>40960)
	{
		printf("**ERROR** Length must between 1 to 4096\n");
		return -1;
	}
	
	now = time(NULL);
	switch(outputtype)
	{
	case 1:	//output as text
		printf("\n************************************************************\n");
		printf("*****  %s",asctime(localtime(&now)));
		buf = new char[infolen + 16];
		strcpy(buf,"* Output = ");
		memcpy(buf+11,info,(size_t)infolen);
		printf(buf);
		delete buf;
		printf("\n************************************************************\n");
		break;
	case 2:	//output as hex
		printf("\n************************************************************\n");
		printf("*****  %s",asctime(localtime(&now)));
		printf ("*       01 02 03 04 05 06 07 08    09 10 11 12 13 14 15 16\n");
		printf ("* ----  -----------------------    -----------------------\n");
		buf = new char[64];
		part = new char[4];
		linenum = 0;
		rest = infolen;
		while(rest)
		{
			linenum ++;
			n = rest>16 ? 16 : rest;
			temp = (unsigned char*)(info+infolen-rest);
			if (n==16)
				sprintf(buf,"* %04X  %02X %02X %02X %02X %02X %02X %02X %02X    %02X %02X %02X %02X %02X %02X %02X %02X\n",linenum,*temp,*(temp+1),*(temp+2),*(temp+3),*(temp+4),*(temp+5),*(temp+6),*(temp+7),*(temp+8),*(temp+9),*(temp+10),*(temp+11),*(temp+12),*(temp+13),*(temp+14),*(temp+15));
			else
			{
				sprintf(buf,"* %04X ",linenum);
				for (int i=0;i<n;i++)
				{
					sprintf(part," %02X",*(temp+i));
					strcat(buf,part);
					if (i==7)
						strcat(buf,"   ");
				}
				strcat(buf," **\n");
			}
			printf(buf);
			rest = rest - n;
		}
		delete part;
		delete buf;
		printf("************************************************************\n");
		break;
	}
	return 0;
}

//调试信息输出函数(字符或十六进制)，输出到文件
int Output_to_Logfile(int outputtype,char* filename,char* info,int infolen)
{
	time_t now = 0;
	FILE * fp;
	char part[32];
	char buf[256];
	unsigned char *temp;
	int linenum,n,rest;
	if (outputtype!=1&&outputtype!=2&&outputtype!=3)
	{
		printf("**ERROR** Output type must be (1:as text) or (2:as ascii hex) or (3:as binary)\n");
		return -1;
	}
	if (infolen<1||infolen>1024*1024)
	{
		printf("**ERROR** Length must between 1 to 1024*1024\n");
		return -1;
	}
	fp = fopen(filename,"w+");
	if (fp==NULL)
	{
		printf("**ERROR** Error openning file\n");
		return -1;
	}
	now = time(NULL);
	switch(outputtype)
	{
	case 1:	//output as text
		fprintf(fp,"\n************************************************************\n");
		fprintf(fp,"*** len=%d *** time=%s *****",infolen,asctime(localtime(&now)));
		fprintf(fp,"* Output = %s\n",info);
		fprintf(fp,"\n************************************************************\n");
		break;
	case 2:	//output as hex
		fprintf(fp,"\n************************************************************\n");
		fprintf(fp,"*** len=%d *** time=%s",infolen,asctime(localtime(&now)));
		fprintf(fp,"*       01 02 03 04 05 06 07 08    09 10 11 12 13 14 15 16\n");
		fprintf(fp,"* ----  -----------------------    -----------------------\n");
		linenum = 0;
		rest = infolen;
		while(rest)
		{
			linenum ++;
			n = rest>16 ? 16 : rest;
			temp = (unsigned char*)(info+infolen-rest);
			if (n==16)
				fprintf(fp,"* %04X  %02X %02X %02X %02X %02X %02X %02X %02X    %02X %02X %02X %02X %02X %02X %02X %02X\n",linenum,*temp,*(temp+1),*(temp+2),*(temp+3),*(temp+4),*(temp+5),*(temp+6),*(temp+7),*(temp+8),*(temp+9),*(temp+10),*(temp+11),*(temp+12),*(temp+13),*(temp+14),*(temp+15));
			else
			{
				fprintf(fp,"* %04X ",linenum);
				memset(buf,0,sizeof(buf));
				for (int i=0;i<n;i++)
				{
					sprintf(part," %02X",*(temp+i));
					strcat(buf,part);
					if (i==7)
						strcat(buf,"   ");
				}
				strcat(buf," **\n");
				fprintf(fp,"%s",buf);
			}
			rest = rest - n;
		}
		fprintf(fp,"************************************************************\n");
		break;
	case 3:
		fwrite(info,1,infolen,fp);
		break;
	}
	fclose(fp);
	return 0;
}

//获取以时间方式来命名的文件名（包含文件存放路径）
//参数：	buf -> 存放文件名
//		path -> 文件存放路径
//		pre -> 文件名前缀
//返回值：文件名长度（包含文件存放路径）
unsigned int __static_id_ = 0;
int GetTimedFileName(char* buf,char* path,char* pre)
{
	time_t tt = time(NULL);
	tm*     t = localtime(&tt);

	__static_id_ ++;
	if(strlen(path)>0)
		sprintf(buf,"%s/%s_%02d%02d%02d%02d%02d%02d_%06d",path,pre,t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec,__static_id_%1000000);
	else
		sprintf(buf,"%s_%02d%02d%02d%02d%02d%02d_%06d",pre,t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec,__static_id_%1000000);
	return (int)strlen(buf);
}

//日志文件初始化
//参数：	mode -> 日志输出模式(调试模式、运行模式)
//		logfile -> 日志文件名
//返回值：-1 -> 文件打开失败
//		-2 -> 模式参数错误
//		0 -> 初始化成功
int	___gDebugMode = MODE_NODEBUG;
std::ofstream 	___gLogFilePtr;

int log_init(int mode,const char* logfile)
{
	switch(mode)
	{
	case MODE_DEBUG_LOG:
		if(___gLogFilePtr.is_open())
			___gLogFilePtr.close();
		___gLogFilePtr.open(logfile, std::ios::out|std::ios::app);
		if (!___gLogFilePtr.is_open())
		{
			printf("Can not open log file < %s >,Exit now...\n",logfile);
			return -1;
		}
	case MODE_DEBUG_PRINT:
	case MODE_NODEBUG:
		___gDebugMode = mode;
		break;
	default:
		return -2;
	}
	return 0;
}

void log_request(const char* fmt,...)
{
	va_list vl;
	switch(___gDebugMode)
	{
	case MODE_DEBUG_LOG:
		struct tm* t;
		time_t     tt;
		char       buf[32];
		char       out[1024];
		if(!___gLogFilePtr.is_open())
		{
			log_init(MODE_DEBUG_LOG,TEMPFILE);
			return;
		}
		tt = time(NULL);
		t = localtime(&tt);
		strftime(buf, 32, "[%Y-%m-%d %H:%M:%S] ",t);
//		sprintf(buf,"%04d-%02d-%02d %02d:%02d:%02d",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
		va_start(vl, fmt);
		vsnprintf(out,sizeof(out),fmt,vl);
		va_end(vl);
		___gLogFilePtr << buf << out << std::endl;
		break;
	case MODE_DEBUG_PRINT:
		va_start(vl, fmt);
		vprintf(fmt,vl);
		va_end(vl);
		break;
	}
}

//关闭调试日志信息输出
void log_cleanup()
{
	if(___gDebugMode==MODE_DEBUG_LOG)
	{
		if(___gLogFilePtr.is_open())
			___gLogFilePtr.close();
	}
	___gDebugMode = MODE_NODEBUG;
}

//End part CommonUse
//**********************************************************************

//**********************************************************************
//This part is for Socket

//客户端连接服务器
//参数：	ip -> 服务器IP地址
//		port -> 服务器端口
//返回值:-1 -> 连接失败 ； 其它值 -> 连接请求句柄值
int  nConnect( char * ip, unsigned short port)
{
	int sockfd,flag,nSock,nError,nRet;
	struct sockaddr_in sa_in;
	struct linger      ling;	//linger结构{ int l_onoff; // 0=off, nonzero=on int l_linger; //linger time in seconds };
								//指定函数CLOSE对面相连接的协议如何操作——当由数据残留在套接口发送缓冲区时的处理
	socklen_t          nLen;
	struct timeval     tv;
	struct sockaddr    sa;
	fd_set             rset,wset;

	nError = 0;
	//if( (sockfd=socket(AF_INET,SOCK_STREAM,0))<0)		//创建套接口
	//	return -1;										//创建失败，退出
	sockfd=socket(AF_INET,SOCK_STREAM,0);				//创建套接口
	if(sockfd==INVALID_SOCKET)							
		return -1;										//创建失败，退出
	memset( &sa_in, 0,sizeof(sa_in));
	sa_in.sin_family = AF_INET;
	sa_in.sin_port = htons(port);
	if(inet_pton(AF_INET, ip ,&sa_in.sin_addr)<0)		//转换IP地址(IPV4)
	{
		close(sockfd);
		return -1;
	}
	flag = fcntl( sockfd, F_GETFL, 0 );					//获取套接口属性
	fcntl( sockfd, F_SETFL, flag | O_NONBLOCK | O_NDELAY );	//设置套接口属性(不延时)
	nLen = sizeof(sa_in);
	nSock = connect( sockfd,(struct sockaddr*)&sa_in, nLen );	//初始化套接口上的连接
	if( !nSock )								//初始化失败
	{
		fcntl(sockfd,F_SETFL,flag);
		ling.l_onoff = 1;
		ling.l_linger = 0;
		nLen = sizeof(ling);
		if( setsockopt( sockfd, SOL_SOCKET, SO_LINGER, (const void *) &ling, nLen ) )	//设置套接口相关的属性选项
		{
			close( sockfd );
			return -1;
		}
		nLen = sizeof(nError);
		if( (nRet=getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&nError,&nLen))<0)	//获取套接口相关的属性选项
		{ 
			close(sockfd);
			return -1;
		}
		if(nError!=0)
		{
			close(sockfd);
			return -1;
		}
		nLen = sizeof(sa);
		if( getpeername(sockfd,&sa,&nLen)!=0)		//获取套接口上的对等通讯方的名字
		{
			close(sockfd);
			return -1;
		}
		return sockfd;
	}
	FD_ZERO( &rset );
	FD_SET( sockfd, &rset );
	wset = rset;
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	if( select( sockfd+1,&rset, &wset,NULL,&tv) == 0) 	//获取套接口读写状态
	{
		close(sockfd);
		return -1;
	}
	if( FD_ISSET(sockfd,&rset) || FD_ISSET(sockfd,&wset) )
	{
		nLen = sizeof(nError);
		if( (nRet=getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&nError,&nLen))<0)
		{ 
			close(sockfd);
			return -1;
		}
		if(nError!=0)
		{
			close(sockfd);
			return -1;
		}
	}
	else
	{ 
		close(sockfd);
		return -1;
	}
	fcntl(sockfd,F_SETFL,flag);

	ling.l_onoff = 1;
	ling.l_linger = 0;
	nLen = sizeof(ling);
	if( setsockopt( sockfd, SOL_SOCKET, SO_LINGER, (const void *) &ling, nLen) )
	{
		close( sockfd );
		return -1;
	}
	return sockfd;
}

//监听客户端连接请求
//参数：	port -> 监听端口
//		backlog -> 等待连接队列的最大长度
//返回值:0 -> 失败 ；其它值 -> 连接请求句柄值
int nListen(unsigned short port,int backlog)
{
	int i,connfd;
	struct sockaddr_in servaddr;
	struct linger stuLinger;

	connfd = socket(AF_INET, SOCK_STREAM, 0);			//创建套接口
	if (connfd==INVALID_SOCKET)
		return 0;										//创建失败，退出
	i = 1;
	if(setsockopt(connfd,SOL_SOCKET,SO_REUSEADDR,&i,sizeof(int)))		//设置套接口相关的属性选项
	{
		close(connfd);
		return 0;
	}
	bzero(&servaddr, sizeof(servaddr));			//置servaddr值为NULL
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(port);
	if (bind(connfd,(struct sockaddr *) &servaddr, sizeof(servaddr))==-1)
	{
		close(connfd);
		return 0;
	}
	stuLinger.l_onoff = 1;
	stuLinger.l_linger = 0;
	setsockopt(connfd,SOL_SOCKET,SO_LINGER,(const char *) &stuLinger,sizeof( stuLinger ) ) ; 
	if (listen(connfd,backlog)==-1)				//监听套接口上连接请求
	{
		close(connfd);
		return 0;
	}
	i = fcntl(connfd,F_GETFL,0);
	fcntl(connfd,F_SETFL,i|O_NONBLOCK);
	return connfd;
}

//响应连接请求，并且新建一个套接口
//参数：	listenid -> 套接口句柄
//		ulip -> 对端ip地址(返回值)
//		sip -> 本地ip地址
//		port -> 对端端口号
//返回值:0和-1 -> 出错； 其它值 -> 响应的套接口句柄 
int nAccept(int listenid,unsigned long& ulip,char* sip,int& port)
{
	int                nSocketId;
	struct sockaddr_in stuSA_in;
	struct sockaddr    stuSA;			
	socklen_t          nIpLen;

	if( (nSocketId=accept(listenid,&stuSA, &nIpLen))<0)		//响应连接请求，并且新建一个套接口。原来的套接口则返回监听状态
	{
		if (errno==EAGAIN)
			return 0;
		return -1;
	}
	getpeername(nSocketId,&stuSA,&nIpLen);					//获取套接口上的对等通讯方的名字
	memcpy(&stuSA_in,&stuSA,sizeof(struct sockaddr_in));
	ulip = stuSA_in.sin_addr.s_addr;
	inet_ntop(AF_INET,&(stuSA_in.sin_addr),sip,INET_ADDRSTRLEN);		//把IP地址转换成字符串形式
	port = ntohs( stuSA_in.sin_port ); 
	return nSocketId;
}

//读取文件数据
//参数：	fd -> 文件句柄
//		vptr -> 存放读取数据缓冲区
//		n -> 读取字节(单位)数
//返回值：读取成功的字节(单位)数 ；-1表示读取错误
ssize_t readn(int fd, void *vptr, size_t n)
{
	ssize_t nleft, nread;
	char *ptr;

	ptr = (char*)vptr;
	nleft = n;
	while(nleft > 0)
	{
		if( (nread = read(fd, ptr, nleft)) < 0 )
		{
			if(errno == EINTR)// || errno == EAGAIN)
				nread = 0;
			else
				return -1;
		}
		else
			if(nread == 0)
				break;
		nleft -= nread;
		ptr += nread;
	}
	return (n - nleft);
}

//写数据到文件中
//参数：	fd -> 文件句柄
//		vptr -> 待写数据缓冲区
//		n -> 待写数据字节(单位)数
//返回值：写成功的字节(单位)数 ；-1表示写文件错误
ssize_t writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = (char*)vptr;
	nleft = n;
	while (nleft > 0)
	{
		if ( (nwritten = write(fd, ptr, nleft)) <= 0)
		{
			if (errno == EINTR)// || errno == EAGAIN)
				nwritten = 0;
			else
				return -1;
		}
		nleft -= nwritten;
		ptr   += nwritten;
	}
	return n;
}

//End part Socket
//**********************************************************************

//**********************************************************************
//This part is for IniFileHandle

template <class ENTRY> File_Parser<ENTRY>::~File_Parser (void)
{
}
template <class ENTRY> int File_Parser<ENTRY>::open (const char filename[])
{
	this->infile_ = fopen (filename, "rt");
	if (this->infile_ == 0)
		return -1;
	else
		return 0;
}
template <class ENTRY> int File_Parser<ENTRY>::close (void)
{
	return fclose (this->infile_);
}

template <class ENTRY> FP_RETURN_TYPE File_Parser<ENTRY>::getword (char buf[])
{
	return this->readword (buf);
}

template <class ENTRY> FP_RETURN_TYPE File_Parser<ENTRY>::getint (int &value)
{
	char buf[BUFSIZ];
	FP_RETURN_TYPE read_result = this->readword (buf);

	if (read_result == FPRT::RT_SUCCESS)
	{
		if (buf[0] == '*')
			return FPRT::RT_DEFAULT;
		else
		{
			char *ptr = 0;
			value = strtol (buf, &ptr, 10);
			if (value == 0 && ptr == buf)
				return FPRT::RT_PARSE_ERROR;
			else
				return FPRT::RT_SUCCESS;
		}
	}
	else
		return read_result;
}

template <class ENTRY> FP_RETURN_TYPE File_Parser<ENTRY>::readword (char buf[])
{
	int wordlength = 0;
	int c;

	while ((c = getc (this->infile_)) != EOF && c != 0x0d && c!=0x0a)
		if (this->delimiter (c))
		{
			if (wordlength > 0)
				break;
		}
		else
			buf[wordlength++] = c;
	buf[wordlength] = '\0';
	if (c == EOF)
	{
		if (wordlength > 0)
		{
			ungetc (c, this->infile_);
			return FPRT::RT_SUCCESS;
		}
		else
			return FPRT::RT_EOFILE;
	}
	else if (c == 0x0d || c==0x0a)
	{
		if (wordlength > 0)
			ungetc (c, this->infile_);
		else
			return FPRT::RT_EOLINE;
	}
	if (this->comments (buf[0]))
	{
		if (this->skipline () == EOF)
			return FPRT::RT_EOFILE;
		else
			return FPRT::RT_COMMENT;
	}
	else
		return FPRT::RT_SUCCESS;
}
template <class ENTRY> int File_Parser<ENTRY>::delimiter (char ch)
{
	return ch == ' ' || ch == ',' || ch == '\t';
}
template <class ENTRY> int File_Parser<ENTRY>::comments (char ch)
{
	return ch == '#';
}
template <class ENTRY> int File_Parser<ENTRY>::skipline (void)
{
	int c;

	while ((c = getc (this->infile_)) != '\n' && c != EOF)
		continue;
	return c;
}

void vCfgsLTrim( char * sOprt )
{
	int		nHead, nCurr;

	if( NULL == sOprt )
		return ;
	nHead = nCurr = 0;
	while( 0 != isspace((int)*(sOprt+nCurr)) )
		nCurr++;
	while( '\0' != (*(sOprt+nCurr)) )
	{
		*(sOprt+nHead) = *(sOprt+nCurr);
		nHead++, nCurr++;
	}
	*(sOprt+nHead) = *(sOprt+nCurr);
	return ;
}
void vCfgsRTrim( char * sOprt )
{
	char *	sTail;

	if( NULL == sOprt )
		return ;
	if( 0 == (int)strlen(sOprt) )
		return ;
	sTail = sOprt+strlen(sOprt)-1;
	while( 0 != isspace((int)*sTail) && sTail >= sOprt )
	{
		*sTail = '\0';
		sTail--;
	}
	return ;
}
void vCfgsATrim( char * sOprt )
{
	if( NULL == sOprt )
		return ;
	vCfgsRTrim( sOprt );
	vCfgsLTrim( sOprt );
	return ;
}
void vCfgsEscCh( char * sOprt,bool bDivide )
{
	char	* sSrc, * sDst;
	int		nModified;

	if( NULL == sOprt )
		return ;
	sSrc = sDst = sOprt;
	nModified = 0;
	while( '\0' != *sSrc )
	{
		if( bDivide && MacCFGS_CharDivide == *sSrc )
		{
			*sDst = '\0';
			return ;
		}
		else if( '\\' != *sSrc && 0 == nModified )
		{
			sSrc++;
			sDst++;
		}
		else if( '\\' != *sSrc && 0 != nModified )
		{
			*sDst = *sSrc;
			sSrc++;
			sDst++;
		}
		else
		{
			switch( *(sSrc+1) )
			{
			case	'b':
				*sDst = '\b';
				sSrc += 2;
				sDst += 1;
				break;
			case	'n':
				*sDst = '\n';
				sSrc += 2;
				sDst += 1;
				break;
			case	'r':
				*sDst = '\r';
				sSrc += 2;
				sDst += 1;
				break;
			case	't':
				*sDst = '\t';
				sSrc += 2;
				sDst += 1;
				break;
			case	MacCFGS_CharDivide:
				*sDst = MacCFGS_CharDivide;
				sSrc += 2;
				sDst += 1;
				break;
			case	'\0':
				sSrc += 1;
				break;
			default:
				sSrc += 1;
				break;
			}
			nModified += 1;
		}
	}
	*sDst = '\0';
	return ;
}
char *	sCfgsReadString(const char * sFileName, char * sGroupName,char * sIdentName, char * sRet, int nSize )
{
	FILE	* lfd_src;
	char	sGName[ MacCFGS_LineLength ], sIName[ MacCFGS_LineLength ];
	char	sIdent[ MacCFGS_LineLength ];
	char	sGroup[ MacCFGS_LineLength ];
	char	* sTmp;
	int		nTmp, nLen;

	if( NULL == sFileName || NULL == sGroupName || NULL == sIdentName || NULL == sRet )
		return NULL;
	lfd_src = fopen( sFileName, "r" );
	if( NULL == lfd_src )
		return NULL;
	strcpy( sRet, "" );
	sTmp = NULL;
#ifdef WIN32
	sprintf( sIName,"%s", sGroupName );
#else
	snprintf( sIName, sizeof(sIName), "%s", sGroupName );
#endif
	vCfgsATrim( sIName );
#ifdef WIN32
	sprintf( sGName,"[%s]", sIName );
	sprintf( sIName,"%s", sIdentName );
#else
	snprintf( sGName, sizeof(sGName), "[%s]", sIName );
	snprintf( sIName, sizeof(sIName), "%s", sIdentName );
#endif
	vCfgsATrim( sIName );
	if( 0 == strlen(sGName) || 0 == strlen(sIName) )
	{
		fclose( lfd_src );
		return NULL;
	}

	nLen = (int)strlen(sGName);
	for( nTmp=0; nTmp<nLen; nTmp++ )
		sGName[nTmp] = (char)toupper(sGName[nTmp]);
	nLen = (int)strlen(sIName);
	for( nTmp=0; nTmp<nLen; nTmp++ )
		sIName[nTmp] = (char)toupper(sIName[nTmp]);
	nLen = (int)strlen(sGName);
	while( 1 )
	{
		if( NULL == fgets(sGroup,sizeof(sGroup),lfd_src) )
		{
			fclose( lfd_src );
			return NULL;
		}
		vCfgsEscCh( sGroup, 1 ); 
		vCfgsATrim( sGroup );
		if( (int)strlen(sGroup) != nLen )
			continue;
		for( nTmp=0; nTmp<nLen; nTmp++ )
			sGroup[nTmp] = (char)toupper(sGroup[nTmp]);
		if( 0 == strcmp(sGroup,sGName) )
			break;
	}
	nLen = (int)strlen(sIName);
	while( 1 )
	{
		if( NULL == fgets(sIdent,sizeof(sIdent),lfd_src) )
		{
			fclose( lfd_src );
			return NULL;
		}
		if( (int)strlen(sIdent) <= nLen )
			continue;
		if( '\n' == sIdent[strlen(sIdent)-1] )
			sIdent[strlen(sIdent)-1] = '\0';
		vCfgsEscCh( sIdent, 1 );
		vCfgsLTrim( sIdent );
		if( MacCFGS_LeftsQuote == sIdent[0] &&
			NULL != strchr(sIdent,MacCFGS_RightQuote) )
		{
			fclose( lfd_src );
			return NULL;
		}
		for( nTmp=0; nTmp<nLen; nTmp++ )
			sIdent[nTmp] = (char)toupper(sIdent[nTmp]);
		if( 0 == strncmp(sIdent,sIName,nLen) )
		{
			sTmp = sIdent+nLen;
			while( isspace(*sTmp) )
				sTmp++;
			if( MacCFGS_CharAssign != *sTmp )
				continue;
			else
			{
				sTmp++;
				break;
			}
		}
	}
	fclose( lfd_src );
#ifdef WIN32
	sprintf( sRet,"%s", sTmp );
#else
	snprintf( sRet, nSize, "%s", sTmp );
#endif
	return sRet;
}
char* sCfgsReadNumber(const char * sFileName, char * sGroupName,char * sIdentName, int * pnRet )
{
	static char	sRst[ MacCFGS_LineLength ], * sRet;

	if( NULL == pnRet )
		return NULL;
	*pnRet = 0;
	memset(sRst,0, sizeof(sRst) );
	sRet = sCfgsReadString( sFileName, sGroupName,sIdentName, sRst, sizeof(sRst) );
	vCfgsATrim( sRst );
	if( NULL != sRet )
		*pnRet = atoi( sRst );
	return sRet;
}
char *	sCfgsReadLongNu(const char * sFileName, char * sGroupName,char * sIdentName, long * plRet )
{
	static char	sRst[ MacCFGS_LineLength ], * sRet;

	if( NULL == plRet )
		return NULL;
	*plRet = 0;
	memset( sRst,0, sizeof(sRst) );
	sRet = sCfgsReadString( sFileName, sGroupName,sIdentName, sRst, sizeof(sRst) );
	vCfgsATrim( sRst );
	if( NULL != sRet )
		*plRet = atol( sRst );
	return sRet;
}
char *	sCfgsReadGrpOff(const char * sFileName, int nGrpOff,char * sGrpNam, int nSize )
{
	FILE	* lfp_src;
	char	sGName[ MacCFGS_LineLength ];
	int		i, nLen;

	if( NULL == sFileName || nGrpOff < 0 ||	NULL == sGrpNam   || nSize   < 0 )
		return NULL;
	lfp_src = fopen( sFileName, "r" );
	if( NULL == lfp_src )
		return NULL;
	strcpy( sGrpNam, "" );
	for( i=0; i<=nGrpOff; )
	{
		if( NULL == fgets(sGName,sizeof(sGName),lfp_src) )
		{
			fclose( lfp_src );
			return NULL;
		}
		vCfgsEscCh( sGName, 1 );
		vCfgsATrim( sGName );
		nLen = (int)strlen(sGName);
		if( nLen <= 2 )
			continue;
		if( MacCFGS_LeftsQuote != sGName[0] || MacCFGS_RightQuote != sGName[nLen-1] )
			continue;
		if( i < nGrpOff )
		{
			i += 1;
			continue;
		}
		fclose( lfp_src );
#ifdef WIN32
		sprintf( sGrpNam,"%.*s", nLen-2, sGName+1 );
#else
		snprintf( sGrpNam, nSize, "%.*s", nLen-2, sGName+1 );
#endif
		return sGrpNam;
	}
	fclose( lfp_src );
	return NULL;
}
char *	sCfgsReadItmLin(const char * sFileName, char * sSect, int nItmLin,char * sItmLin, int nSize )
{
	FILE	* lfp_src;
	char	sGName[ MacCFGS_LineLength ], sIName[ MacCFGS_LineLength ];
	char	sIdent[ MacCFGS_LineLength ];
	char	sGroup[ MacCFGS_LineLength ];
	char	* sTmp;
	int		i, nTmp, nLen;

	if( NULL == sFileName || NULL == sSect || nItmLin < 0 || NULL == sItmLin || nSize < 0 )
		return NULL;

	lfp_src = fopen( sFileName, "r" );
	if( NULL == lfp_src )
		return NULL;
	strcpy( sItmLin, "" );
	sTmp = NULL;
#ifdef WIN32
	sprintf( sIName,"%s", sSect );
#else
	snprintf( sIName, sizeof(sIName), "%s", sSect );
#endif
	vCfgsATrim( sIName );
#ifdef WIN32
	nLen = sprintf( sGName,"[%s]", sIName );
#else
	nLen = snprintf( sGName, sizeof(sGName), "[%s]", sIName );
#endif
	if( 2 >= (int)strlen(sGName) )
	{
		fclose( lfp_src );
		return NULL;
	}
	nLen = (int)strlen(sGName);
	for( nTmp=0; nTmp<nLen; nTmp++ )
		sGName[nTmp] = (char)toupper(sGName[nTmp]);
	while( 1 )
	{
		if( NULL == fgets(sGroup,sizeof(sGroup),lfp_src) )
		{
			fclose( lfp_src );
			return NULL;
		}
		vCfgsEscCh( sGroup, 1  );
		vCfgsATrim( sGroup );
		if( (int)strlen(sGroup) != nLen )
			continue;
		for( nTmp=0; nTmp<nLen; nTmp++ )
			sGroup[nTmp] = (char)toupper(sGroup[nTmp]);
		if( 0 == strcmp(sGroup,sGName) )
			break;
	}
	for( i=0; i<= nItmLin; )
	{
		if( NULL == fgets(sIdent,sizeof(sIdent),lfp_src) )
		{
			fclose( lfp_src );
			return NULL;
		}
		nLen = (int)strlen(sIdent);
		if( nLen <= 1 )
			continue;
		if( '\n' == sIdent[nLen-1] )
			sIdent[nLen-1] = (char)0;
		vCfgsEscCh( sIdent, 1 );
		vCfgsLTrim( sIdent );
		if( MacCFGS_LeftsQuote == sIdent[0] )
		{
			fclose( lfp_src );
			return NULL;
		}
		sTmp = strchr( sIdent, MacCFGS_CharAssign );
		if( NULL == sTmp || sTmp == sIdent )
			continue;
		if( i <  nItmLin )
		{
			i += 1;
			continue;
		}
		fclose( lfp_src );
#ifdef WIN32
		sprintf( sItmLin,"%s", sIdent );
#else
		snprintf( sItmLin, nSize, "%s", sIdent );
#endif
		return sItmLin;
	}
	fclose( lfp_src );
	return NULL;
}
int nvCfgsReadItmDat(const char * sFile, char * sSect, char * sIdnt,char * sDatFmt, ... )
{
	sFile = 0;
	sSect = 0;
	sIdnt = 0;
	sDatFmt = 0;

	return -1;
}
int nvCfgsReadItmOff(const char * sFile, char * sSect, int nItmOff,char * sItmFmt, ... )
{
	sFile = 0;
	sSect = 0;
	nItmOff = 0;
	sItmFmt = 0;

	return -1;
}

//End part IniFileHandle
//**********************************************************************

