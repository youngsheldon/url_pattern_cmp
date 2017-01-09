#include <sys/stat.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <time.h>
#include <sys/time.h>
#include <dirent.h>
#include "commonlib.h"
#include "cryptodes.h"
using namespace std;
typedef vector<string> FILELIST;
typedef vector<string> CDRINFO;
typedef vector<CDRINFO> CDRLIST;

FILELIST				file_list;
CDRLIST					cdr_list;
int						msgCount;
int						pos;
int						pos2;
int						output_format_=1;	//0=txt显示格式;1=execl显示格式
fstream					outfile;
int getFileList(char *dirname);
int getInfo(char *fileName);
int decodeMsg(CDES1* encrypt, char *contentE, unsigned char *content);
int splitString(char *inbuf, int inlen, CDRINFO &cdr_info, char delimiter, int splitCount, int splitSize);
int checkMsg(const char *inbuf);
int main(int argc, char* argv[])
{
	CDES1* encrypt = new CDES1();
	encrypt->init("3QjMYs!?");

	CDRINFO tmp;
	int msgnum=0;
	unsigned char msg[3080];
	char buf[6660];
	int fileNum = 0;
	int fileCount = 0;
	char fileName[100];
	int status;
	if(argc == 3)
	{
		msgCount = atoi(argv[1]);
		pos = atoi(argv[2]) - 1;
	}
	else
	{
		msgCount = 3;
		pos = 2;
	}
	
	file_list.clear();
	//获取CDR文件名
	if(getFileList("./data/") < 0)
	{
		printf("getFilelist num < 0");
		return 0;
	}
	sprintf(fileName, "./output/suspicion_phone_trace_%03d.csv", fileCount);
	//outfile.open("./output/nti_cdr_tmp.xls",ios::out | ios::app);
	outfile.open(fileName, ios::out | ios::app);
//	strcpy(buf, "\"   \",\"ROWNUM\",\"ALARM_TIME\",\"SRC_ADDR\",\"DES_ADDR\",\"NAME\",\"NAME\",\"SRCTYPE\",\"DESTTYPE\",\"SM_TEXT\",\"ALARM_TYPE\",\"GROUP_NAME\"");
//	outfile << buf << endl;
	for(int i = 0; i < (int)file_list.size(); i++)
	{
		cdr_list.clear();
		//读取CDR信息
		getInfo((char*)file_list[i].c_str());
		printf("fileName = %s\tmsgcount = %d\n", file_list[i].c_str(), (int)cdr_list.size());
		for(int j = 0; j < (int)cdr_list.size(); j++)
		{
			tmp = cdr_list[j];
			status = -1;
			if(((int)tmp.size() > pos) && (tmp[pos].length() >= 2))
			{
				//status = checkMsg(tmp[pos].c_str());
				//if(status >= 0)
					decodeMsg(encrypt, (char*)tmp[pos].c_str(), msg);
			}
			memset(buf, 0, sizeof(buf));
			for(int k = 0; k < (int)tmp.size(); k++)
			{
				if(k == pos)
				{
					if(output_format_ == 0)
					{
						strcat(buf, (char*)msg);
						strcat(buf, "\t");
					}
					else
					{
						strcat(buf, "\"");
					//	if(status < 0)
					//		strcat(buf, tmp[k].c_str());
					//	else
							strcat(buf, (char*)msg);
						strcat(buf, "\"");
						if(k < (int)tmp.size() - 1)
							strcat(buf, ",");
					}
				}
				else
				{
					if(output_format_ == 0)
					{
						strcat(buf, tmp[k].c_str());
						strcat(buf, "\t");
					}
					else
					{
						strcat(buf, tmp[k].c_str());
						if(k < (int)tmp.size() - 1)
							strcat(buf, ",");
					}
				}
			}
			fileNum++;
			if(fileNum >= 500000)
			{
				fileCount++;
				fileNum = 0;
				outfile.close();
				sprintf(fileName, "./output/suspicion_phone_trace_%03d.csv", fileCount);
				outfile.open(fileName, ios::out | ios::app);
			}
			if(strlen(buf) <= 8)
				continue;
			outfile << buf << endl;
		}
		msgnum += cdr_list.size();
	}

	printf("totalmsg = %d\n", msgnum);
	outfile.close();
	return 0;
}

int getFileList(char *dirname)
{
	DIR *dp;
	struct dirent *dirp;
	string filename;
	dp = opendir(dirname);
	if(dp == NULL)
	{
		printf("can't open the %s\n", dirname);
		return -1;
	}
	while((dirp = readdir(dp)) != NULL)
	{
		//if(strncmp("nti_cdr_", dirp->d_name, strlen("nti_cdr_")) != 0 )
		if(strcmp(".", dirp->d_name) == 0 || strcmp("..", dirp->d_name) == 0)
			continue;
		filename = dirname;
		filename += dirp->d_name;
		file_list.push_back(filename);
	}
	closedir(dp);
	return 0;
}

int getInfo(char *fileName)
{
	fstream infile;
	char buf[6660];
	CDRINFO cdrtmp;
	int count = 0;
	if(strlen(fileName) <= 0)
	{
		return -1;
	}
	infile.open(fileName, std::ios::in);
	if(infile == NULL)
	{
		return -1;
	}
	while(!infile.eof())
	{
		memset(buf, 0, sizeof(buf));
		infile.getline(buf, sizeof(buf), '\n');
		if(strlen(buf) < 10)
		{
			printf("sizeof(buf):%s < 10, count=%d", buf, count);
			continue;
		}
		cdrtmp.clear();
		if(splitString(buf, strlen(buf), cdrtmp, ',', msgCount, 6660) < 0)
		//if(splitString(buf, strlen(buf), cdrtmp, ' ', msgCount, 3080) < 0)
		{
			continue;
		}
		cdr_list.push_back(cdrtmp);
		count++;
	}
	infile.close();
	return 0;
}

int decodeMsg(CDES1* encrypt, char *contentE, unsigned char *content)
{
	int datalen;
	unsigned char s[6660]; 
	unsigned char hightmp; //高位字节
	unsigned char lowtmp;//低位字节
	int i = 0,j = 0;
	
	datalen = strlen(contentE);
	memset(s, 0, sizeof(s));
//	printf("contentE = %s\n", contentE);
//	printf("msg :\n");
	int len = encrypt->doDecrypt((const unsigned char *)contentE,s,datalen);
	for (; i<len; i=i+2,j++)
	{
		hightmp = zs_decode(*(s+i));
		lowtmp = zs_decode(*(s+i+1));
		
		content[j] = (hightmp<<4) | lowtmp;
//		printf("h:%02X,l:%02X,content:%02X\n", hightmp, lowtmp, content[j]);
	//	printf("%02X ", content[j]);
		//if(content[j] =='\"')
		//if(content[j] == '\r' || content[j] == '\n' ||  content[j] == '\t')
		if(content[j] == '\r' || content[j] == '\n' || content[j] == '\"' ||  content[j] == '\t')
		{
			content[j] = '\0';
			j--;
		}
	}
//	printf("\n");
	//printf("contentE = %s\nlen=%d\ncontent = %s\n", contentE, len, content);
//	printf("len = %d, content = %s\n\n", len, content);
	return j;
}
int splitString(char *inbuf, int inlen, CDRINFO &divString, char delimiter, int splitCount, int splitSize)
{
	int i = 0, j= 0,u = 0;
	char tmp[6660];
	if((int)strlen(inbuf) != inlen)
	{
		return -1;
	}
	for(; i < inlen; i++)
	{
		if(inbuf[i] == delimiter)
		{
		//	if(j > 0)
			{
				if(j == 0)
				{
					tmp[0] = ' ';
					tmp[1] = 0;
				}
				else
					tmp[j] = 0; 
				divString.push_back(tmp);
				j = 0;
				u++;
			}
			//while(i+1 < inlen && inbuf[i+1] == delimiter)
			//{
			//	i++;
			//}
			continue;
		}
		tmp[j++] = inbuf[i];
	}
	if(j > 0)
	{
		tmp[j] = 0;
		divString.push_back(tmp);
		u++;
	}
/*	if(splitCount != -1 && u != splitCount)
	{
		printf("u = %d, splitCount = %d, inbuf = %s\n", u, splitCount, inbuf);
		return -1;
	}
*/
	return 0;
}
int checkMsg(const char *inbuf)
{
	int status = 0;
	int pos = 0;
	int len = (int)strlen(inbuf);
	while(pos < len - 1)
	{
		if(inbuf[pos] >= '0' && inbuf[pos] <= '9')
		{
			pos++;
			continue;
		}
		if(inbuf[pos] >= 'A' && inbuf[pos] <= 'F')
		{
			pos++;
			continue;
		}
		status = -1;
		break;
	}
	return status;
}

