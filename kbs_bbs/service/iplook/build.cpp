//
//  accepts convert's output, outputs ipinfo_tree string_index string_pool
//
//  Note: I know it is not C++ style code, please do not blame me.
//  
//  kxn@smth
//
//
#include <vector>
#include <stdio.h>
#include <string>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <arpa/inet.h>
#include <map>
using namespace std;

#pragma pack (push, enter, 1)
typedef struct _ipNode{
	int left, right;
        unsigned short area, location;
} ipnode;
#pragma pack (pop, enter)

vector <ipnode> ipbuf;

class strpool
{
	public:
	strpool () : id(0), offset(0) {} ;
	int str2id(const string &s)
	{
		// first look up in the pool;
		map<string,int> ::iterator iter = pool.find(s);
		if (iter != pool.end()) {
			return iter->second;
		}
		pool.insert(pair<string, int>(s,++id));
		revpool.push_back(s);
		indexes.push_back(offset);
		offset += s.size() + 1; // null terminator
		return id;
	}
	void dumpidx(const string &fn)
	{
		FILE *fp = fopen(fn.c_str(),"wb");
		fwrite(&indexes[0], sizeof(int), indexes.size(),fp);
		fclose(fp);
	}
	void dumpstr(const string &fn)
	{
		FILE *fp = fopen(fn.c_str(),"wb");
		for (int i=0;i< indexes.size(); i++)
			fwrite(revpool[i].c_str(), revpool[i].size()+1 , 1, fp);
		fclose(fp);
	}
	void dump(const string &fn)
	{
		FILE *fp = fopen(fn.c_str(), "wb");
		int count = indexes.size();
		fwrite(&count, sizeof(int), 1, fp); 
		fwrite(&indexes[0], sizeof(int), indexes.size(),fp);
		fwrite(&offset, sizeof(int), 1, fp);
		for (int i=0;i< indexes.size(); i++)
			fwrite(revpool[i].c_str(), revpool[i].size()+1 , 1, fp);
		fclose(fp);
	}
	private:
		
	map <string, int> pool;
	vector <string> revpool;
	vector <int> indexes;
	int id, offset;
};

strpool pool;

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

int AddInfoPair(unsigned int ip, unsigned int mask, const string & area, const string & location)
{
	int currnode = 0;
	for (;;mask <<= 1 , ip <<= 1)
	{
		if ((mask & 0x80000000) == 0) {
			if (ipbuf[currnode].area != 0) cout << "warning: info already exists" << endl;
			ipbuf[currnode].area = pool.str2id(area);
			ipbuf[currnode].location = pool.str2id(location);
			return 0;
		}
		ipnode n;
		n.left = n.right = 0;
		n.area = n.location = 0;
		if (ip & 0x80000000) {
			if (ipbuf[currnode].right != 0) {
				currnode = ipbuf[currnode].right;
			} else {
				// create a node;
				int next = ipbuf.size();
				ipbuf.push_back(n);
				ipbuf[currnode].right = next;
				currnode = next;
			}
		} else {
			if (ipbuf[currnode].left != 0) {
				currnode = ipbuf[currnode].left;
			} else {
				// create a node;
				int next = ipbuf.size();
				ipbuf.push_back(n);
				ipbuf[currnode].left = next;
				currnode = next;
			}
		}
	}
}


int AddLine(char *buf)
{
	char *s = strtok(buf,"\t\n\r");
	unsigned start,end;
	string area, location;
	if (s) start = my_inet_addr(s);
	s = strtok(NULL, "\t\n\r");
	if (s) end = my_inet_addr(s);
	s = strtok(NULL, "\n\r\t");
	if (s) area = string(s); else area = "";
	s = strtok(NULL, "\n\r\t");
	if (s) location = string(s); else location = "";
	AddInfoPair(htonl(start),htonl(end),area,location);
	return 0;
}
int InitInfo()
{
	ipbuf.clear();
	ipnode n;
	n.left = n.right = 0;
	n.area = pool.str2id("未知数据");
	n.location = pool.str2id("未知数据");
	ipbuf.push_back(n);
	return 0;
}
int main(int argc, char* argv[])
{
	char buf[255];
	InitInfo();
	for (fgets(buf,255,stdin);!feof(stdin);fgets(buf,255,stdin))
		AddLine(buf);
	//pool.dump(string("ipinfo.dat"));
	pool.dumpidx(string("string_index"));
	pool.dumpstr(string("string_pool"));
	FILE *fp = fopen("ipinfo_tree","wb");
//	int c = ipbuf.size();
//	fwrite(&c,sizeof(int),1,fp);
	fwrite(&ipbuf[0],sizeof(ipnode),ipbuf.size(),fp);
	fclose(fp);
	return 0; 
}



