#include "bbs.h"
#ifdef lint
#include <sys/uio.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define COLOR (4)
char TOPSTR[160];
int a;

typedef struct node GOPHER;
struct node {
    char        file[81],title[71],server[41];
    int         port;
    int         position;
    GOPHER      *next;
};

GOPHER *g_main[100];/*100 directories to move in to*/
GOPHER *tmpitem;
GOPHER *topitem;
int    gopher_position=0;


GOPHER *
next(node)
GOPHER *node;
{
        if(node==NULL)
                return NULL;
        else
                return node->next;
}


GOPHER *
find_kth(node,n)
GOPHER *node;
int n;
{
        int i;
        GOPHER *tmpnode;

        tmpnode=node;
        for(i=0;i<=n;i++)
        {
                tmpnode=next(tmpnode);
        }
        return tmpnode;
}

int
readfield(fd, ptr, maxlen)
  int   fd;
  char  *ptr;
  int   maxlen;
{
     int n;
     int rc;
     char c;

     for (n=1; n < maxlen; n++) {
          if ( (rc = read(fd, &c, 1)) == 1) {
               *ptr++ = c;
               if (c == '\t') {
                    *(ptr - 1) = '\0';
                    break;
               }
          }
          else if (rc == 0) {
               if (n == 1)
                    return(0);  /* EOF, no data read */
               else
                    break;              /* EOF, some data was read */
          }
          else
               return(-1);              /* error */
     }

     *ptr = 0;                          /* Tack a NULL on the end */
     return(n);
}

int readline(fd, ptr, maxlen)
  int   fd;
  char  *ptr;
  int   maxlen;
{
     int n;
     int rc;
     char c;


     for (n=1; n < maxlen; n++) {
          if ( (rc = read(fd, &c, 1)) == 1) {
               *ptr++ = c;
               if (c == '\n')
                    break;
          }
          else if (rc == 0) {
               if (n == 1)
                    return(0);  /* EOF, no data read */
               else
                    break;              /* EOF, some data was read */
          }
          else
               return(-1);              /* error */
     }

     *ptr = 0;                          /* Tack a NULL on the end */
     return(n);
}

int
savetmpfile(tmpname)
char tmpname[];
{
        char ch;
        char buf[256];
        FILE *fp;
        int cc;

    if((fp=fopen(tmpname,"w"))==NULL)
        return -1;
         fprintf(fp,"®”  ∑Ω: %s\n",tmpitem->server);
         fprintf(fp,"¿…  