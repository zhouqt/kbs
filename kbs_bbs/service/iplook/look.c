/* 
 * 
 *   IP location lookup by kxn@smth
 *   
 *   specially optimized for speed and memory usage, now we only use 3M of memory, 
 *   while performing 600000 queries per second on an athlon XP2500+,
 *   the average iteration depth is 8.75 (result of test).
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
extern char ipinfo_version[];
extern int ipinfo_count;

char *GetIPInfoVersion()
{
	return ipinfo_version;
}

int GetIPInfoCount()
{
	return ipinfo_count;
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
		} else if (!(ip & 0x80000000) && ipinfo_tree[currnode].left) 
		{
			currnode = ipinfo_tree[currnode].left;
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

#ifdef IPLOOK_MAIN
main()
{	
	char *area, *location;
	LookIP("166.111.8.238",&area, &location);
	printf("166.111.8.238 is in %s at %s\n",area,location);
	stress(6000000);
	
}
#endif 

