//
//  convert: reads in slightly modified output of QQwry.exe from soff
//           outputs ip location info in network/mask pair format
//
//  Note: I know it is not C++ style code, please do not blame my poor C++.
// 
//                                 ----    kxn@smth
//

#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string>
#include <string.h>
#include <iostream>
#include <stdio.h>
using namespace std;

typedef int (*splitcallback)(unsigned ip, unsigned mask, void *param);

inline unsigned int makelowbound(unsigned int value,unsigned int maskinv) {
	return value  & (~maskinv);
}
inline unsigned int makeupbound(unsigned int value, unsigned int maskinv) {
	return value | maskinv;
}

int _splitrange(unsigned begin, unsigned end, splitcallback cb, void *param)
{
	unsigned maskinv;
	if (begin > end) return 0;
	if (begin == end) {
		(*cb)(htonl(begin),0xFFFFFFFF,param);
		return 1;
	}
	for (maskinv =  0;; maskinv = maskinv *2+1){
		if (makelowbound(begin,maskinv) < begin || makeupbound(begin,maskinv) > end) {
			maskinv = (maskinv -1) /2;
			(*cb)(htonl(makelowbound(begin,maskinv)),htonl(~maskinv),param);
			begin = makeupbound(begin,maskinv) +1;
			break;
		}
	}
	return _splitrange(begin,end,cb,param) + 1;
}

int splitrange(unsigned begin, unsigned end, splitcallback cb, void *param)
{
	return _splitrange(htonl(begin), htonl(end), cb,param);
}

unsigned int my_inet_addr(const char *s)
{
	char buf[255];
	strcpy(buf,s);
	char *p = buf, *q = buf;
	int i;
	unsigned int r = 0;
	for (i=0;i<4;i++) {
		while (*q && *q !='.') q++;
		*q++ = '\0';
		r = (r << 8) + atoi(p);
		p = q;
	}
	return ntohl(r);
}
static int testcb(unsigned ip, unsigned mask, void *param)
{
	struct in_addr inr;
	inr.s_addr = ip;
	string ips = string(inet_ntoa(inr));
	inr.s_addr = mask;
	string masks = string(inet_ntoa(inr));
	printf("%s:%s\n",ips.c_str(),masks.c_str());
	return 0;
}

typedef struct _ipparam {
	string area;
	string location;
} ipparam;

static FILE *outf;
static int ipcount;
static int writecb(unsigned ip, unsigned mask, void *param)
{
	struct in_addr inr;
	ipparam *p = (ipparam*)param;
	inr.s_addr = ip;
	string ips = string(inet_ntoa(inr));
	inr.s_addr = mask;
	string masks = string(inet_ntoa(inr));
	ipcount ++;
	fprintf(stdout,"%s\t%s\t%s\t%s\n",ips.c_str(),masks.c_str(),p->area.c_str(),p->location.c_str());
	return 0;
}
int AddInfo(unsigned int start, unsigned int end, const string & area, const string & location)
{
	if (area == "Î´ÖªÊý¾Ý") return 0;
	ipparam par;
	par.area = area;
	par.location = location;
	splitrange(start,end,writecb, (void *)&par);
	return 0;
}
string trim(string s)
{
	if (s.size() == 0) return s;
	char *p = (char *)s.c_str();
	while (p && *p == ' ') p++;
	char *q = (char *) s.c_str() + s.size() -1;
	while (q > p && *q ==' ') *q-- = '\0';
	return string(p);
}
int AddLine(char *buf)
{
	char *s = strtok(buf," \n\r");
	unsigned start,end;
	string area, location;
	if (s) start = my_inet_addr(s);
	s = strtok(NULL, "\n\r ");
	if (s) end = my_inet_addr(s);
	s = strtok(NULL, "\n\r ");
	if (s) area = string(s); else area = "";
	s = strtok(NULL, "\n\r");
	if (s) location = trim(string(s)); else location = "";
	AddInfo(start,end,area,location);
	return 0;
}
int main(int argc, char* argv[])
{
	char buf[255];
	for (fgets(buf,255,stdin);!feof(stdin);fgets(buf,255,stdin))
		AddLine(buf);
	FILE *fp = fopen("ipinfo_count","wb");
	fwrite(&ipcount,sizeof(int),1,fp);
	fclose(fp);
	return 0; 
}



