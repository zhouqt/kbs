/* 
 * 
 *   IP location lookup by kxn@smth
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
#pragma pack (push, enter, 1)
typedef struct _ipNode{
	int left, right;
	unsigned short area, location;
} ipnode;
#pragma pack (pop, enter)

extern ipnode ipinfo_tree[];
extern char string_pool[];
extern int string_index[];

int titer = 0;
void stress(int count)
{
	char buf[255];
	int i;
	double start, end, elasp;
	struct timeval tv;
	gettimeofday(&tv,NULL);
	srand(tv.tv_usec);
	titer = 0;
	start = tv.tv_sec + (double) tv.tv_usec /1000000.0;
	for (i=0;i<count;i++)
	{
		char *a,*b;
		sprintf(buf,"%d.%d.%d.%d",rand()%256,rand()%256,rand()%256,rand()%256);
		LookIP(buf,&a,&b);
	}
	gettimeofday(&tv,NULL);
	end = tv.tv_sec + (double) tv.tv_usec /1000000.0;
	elasp  = end- start;
	printf("Completed %d queries in %f seconds, average speed is %f q/s, average iterating depth is %f\n", count, elasp, (double)count/elasp, (double)titer /(double) count);
}
int LookIP(const char *ipstr, char ** parea, char **plocation)
{
	int currnode = 0;
	int i;
	unsigned short area = ipinfo_tree[0].area;
	unsigned short location = ipinfo_tree[0].location;
	unsigned ip;
	int ret = -1;
	ip = htonl(inet_addr(ipstr));
	for (i=0;i<32;i++,ip<<=1)
	{
		if (ip & 0x80000000  && ipinfo_tree[currnode].right) 
		{
			currnode = ipinfo_tree[currnode].right;
			titer ++;
		} else if (!(ip & 0x80000000) && ipinfo_tree[currnode].left) 
		{
			currnode = ipinfo_tree[currnode].left;
			titer++;
		} else {
			// can not advance more, bail out
			break;
		}
		// update the information if ok;
		// dumpnode(currnode);
		if (ipinfo_tree[currnode].area) {
			ret = 0;
			area = ipinfo_tree[currnode].area;
		}
		if (ipinfo_tree[currnode].location) 
			location = ipinfo_tree[currnode].location;
	}
	
	*parea = string_pool + string_index[area -1]; 
	*plocation = string_pool + string_index[location -1];
	return ret;
}

main()
{	
	char *area, *location;
	LookIP("166.111.8.238",&area, &location);
	printf("166.111.8.238 is in %s at %s\n",area,location);
	stress(300000);
	
}

