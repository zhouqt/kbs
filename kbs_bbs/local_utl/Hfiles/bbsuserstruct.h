#include <time.h>

struct userec {                  /* Structure used to hold information in */
	char            userid[14];   /* PASSFILE */
	char            fill[30];
	time_t          firstlogin;
	char            lasthost[16];
	unsigned int    numlogins;
	unsigned int    numposts;
	char            flags[2];
	char            passwd[14];
	char            username[40];
	char            ident[40];
	char            termtype[80];
	unsigned        userlevel;
	time_t          lastlogin;
	time_t          stay;
	char            realname[40];
	char            address[80];
	char            email[80];
	int             signature;
	unsigned int    userdefine;
	time_t          notedate;
	int             noteline;
	int             notemode;
	int             unuse1;/* no use*/
	int             unuse2;/* no use*/
};

