/* 
 * 
 *   IP location lookup by kxn@smth
 *   
 *   Note: this can still speeded up with AVL tree,  but current speed is good enough.
 *
 *   specially optimized for memory usage, now we only use 3M of memory
 * 
 *   kxn@smth
 *
 */
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/mman.h>
#pragma pack (push, enter, 1)
typedef struct _ipNode{
	int left, right;
	unsigned short area, location;
} ipnode;
#pragma pack (pop, enter)

ipnode *ipbuf;

char *strbuf;
int strcount;
int *strindex;

int LoadIPInfo(const char *fn)
{
	FILE *fp;
	int count;
	fp = fopen(fn,"rb");
	fread(&strcount,sizeof(int),1,fp);
	printf("Load string pool with %d strings \n", strcount);
	strindex = (int *) malloc(strcount * sizeof(int));
	fread(strindex,sizeof(int), strcount,fp);
	fread(&count,sizeof(int),1,fp);
	printf("Load string buffer with %d bytes \n", count);
	strbuf = (char *) malloc(count);
	fread(strbuf, count, 1, fp);
	fread(&count,sizeof(int),1,fp);
	printf("Load tree with %d nodes\n", count);
	ipbuf = (ipnode *) malloc( sizeof(ipnode) * count);
	fread(ipbuf,sizeof(ipnode), count, fp);
	fclose(fp);
}

int dumpnode(int n)
{
	printf("goto node %d, area %s, location %s\n", n, ipbuf[n].area, ipbuf[n].location);
	printf("left child %d, right child %d\n", ipbuf[n].left, ipbuf[n].right);
}
void LookIP(const char *ipstr, char ** parea, char **plocation)
{
	int currnode = 0;
	int i;
	unsigned short area = ipbuf[0].area;
	unsigned short location = ipbuf[0].location;
	unsigned ip;
	ip = htonl(inet_addr(ipstr));
	for (i=0;i<32;i++,ip<<=1)
	{
		if (ip & 0x80000000  && ipbuf[currnode].right) 
		{
			currnode = ipbuf[currnode].right;
		} else if (!(ip & 0x80000000) && ipbuf[currnode].left) 
		{
			currnode = ipbuf[currnode].left;
		} else {
			// can not advance more, bail out
			break;
		}
		// update the information if ok;
		// dumpnode(currnode);
		if (ipbuf[currnode].area) 
			area = ipbuf[currnode].area;
		if (ipbuf[currnode].location) 
			location = ipbuf[currnode].location;
	}
	
	*parea = strbuf + strindex[area -1]; 
	*plocation = strbuf + strindex[location -1];
}
void stress(int count)
{
	int i;
	struct timeval tv;
	char buf[20];
	double start,end,all;
	char *p, *q;
	printf("Stressing...\n");
	gettimeofday(&tv,NULL);
	start = (double) tv.tv_sec + (double)tv.tv_usec / 1000000.0;
	srand(time(NULL));
	for (i=0;i<count;i++)
	{
		sprintf(buf,"%d.%d.%d.%d",rand() % 256, rand() % 256, rand() %256, rand() % 256);
		LookIP(buf, &p, &q);
	}
	gettimeofday(&tv,NULL);
	end = (double) tv.tv_sec + (double)tv.tv_usec / 1000000.0;
	all = end - start;
	printf("Completed %d queries in %f seconds, average speed is %f queries/second\n", count, all, (double) count / all);
	
}
main()
{	
	char *area, *location;
	LoadIPInfo("ipinfo.dat");
	LookIP("1.2.3.4",&area, &location);
	printf("166.111.8.238 is in %s at %s\n",area,location);
	stress(6000000);
	
}
