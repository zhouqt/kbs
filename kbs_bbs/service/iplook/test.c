#include <stdio.h>
#include "iplook.h"

void dolook(char *str)
{
	char *area, *location;
	if (0 != LookIP(str,&area,&location)) {
		printf("%s 位置不明\n",str);
	}
	else printf("%s 位于 %s 的 %s\n", str,area,location);
}
int main(int argc, char *argv[])
{
	char buf[255];
	printf("IP 信息查询，%s，共有 IP 数据 %d 条\n", GetIPInfoVersion(), GetIPInfoCount()); 
	if (argc>1) {
		dolook(argv[1]);
	} else {
		fgets(buf,255,stdin);
		strtok(buf,"\r\n \t");
		dolook(buf);
	}
}
