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

extern ipnode ipbuf[];

extern char strbuf[];
extern int strindex[];


int LookIP(const char *ipstr, char ** parea, char **plocation)
{
	int currnode = 0;
	int i;
	unsigned short area = ipbuf[0].area;
	unsigned short location = ipbuf[0].location;
	unsigned ip;
	int ret = -1;
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
		if (ipbuf[currnode].area) {
			ret = 0;
			area = ipbuf[currnode].area;
		}
		if (ipbuf[currnode].location) 
			location = ipbuf[currnode].location;
	}
	
	*parea = strbuf + strindex[area -1]; 
	*plocation = strbuf + strindex[location -1];
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

