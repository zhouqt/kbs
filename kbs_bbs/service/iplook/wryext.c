/*
 *  QQWry.Dat  dumper by kxn@SMTH
 *  I'm too lazy to write a decode routine, so I use reversed assembly code.
 *  thus it can not compile on non-i386 boxes, heihei
 */

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
char *ip2str(unsigned int v)
{
	static char buf[255];
	unsigned char *p = (unsigned char *)&v;
	sprintf(buf,"%03d.%03d.%03d.%03d",p[0],p[1],p[2],p[3]);
	return buf;
}	
void dumpcallback(unsigned int start, unsigned int end, char *location, char *area)
{
	char sbuf[20], ebuf[20];
	strcpy(sbuf,ip2str(ntohl(start)));
	strcpy(ebuf,ip2str(ntohl(end)));
	if (strstr(area,"CZ88") ) area = "未知数据";
	if (location[0] == 0 || strstr(location,"CZ88")) location = "未知数据";
	if (strstr(area,"纯真网络")) {
		FILE *fp = fopen("ipinfo_version","wb");
		fwrite(location,strlen(location)+1,1,fp);
		fclose(fp);
		return;
	};
	printf("%s %s %s %s\n",sbuf,ebuf,area,location);
}

extern void dumpstr(char *buffer);
int main(int argc, char*argv[])
{
	
	FILE *fp;
	struct stat st;
	char *buf,*filename;
	if (argc <2) exit(0);
	filename = argv[1];
	stat (filename,&st);
	buf = (char *)malloc(st.st_size);
	fp = fopen(filename,"rb");
	fread(buf,st.st_size,1,fp);
	fclose(fp);
	dumpstr(buf);
 	
}
