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
         fprintf(fp,"À´  Ô´: %s\n",tmpitem->server);
         fprintf(fp,"µµ  Ãû: %s(Ê¹ÓÃ %d ²º)\n",tmpitem->file,tmpitem->port);
         fprintf(fp,"±ê  Ìâ: %s\n\n",tmpitem->title+1);
        while(1)
        {
         if((cc=read(a,buf,255))>0)
         {
             buf[cc]='\0';
             fprintf(fp,"%s",buf);
         }else
         {
                fclose(fp);
                break;
         }
        }
        return 1;

}

int
enterdir(path)
char path[];
{
        char buf[256];
        sprintf(buf,"%s\r\n",path==NULL?"":path);
        write(a,buf,sizeof(buf));
}

int
get_con(servername,port)
char *servername;
int port;
{
    struct hostent *h,*h2 ;
    char buf[1024] ;
    char hostname[81] ;
    struct sockaddr_in sin ;
    char ch[1];
    int  get;

    gethostname(hostname,80);
    if(!(h = gethostbyname(hostname))) {
        perror("gethostbyname") ;
        return -1 ;
    }

    if(!(h2 = gethostbyname(servername)))
    {
        perror("gethostbyname");
        return -1;
    }
    sin.sin_family=AF_INET;
    memcpy(&sin.sin_addr, h2->h_addr, h2->h_length);
    if(!(a=socket(AF_INET,SOCK_STREAM,0)))
    {
        perror("Socket:");
        return -1;
    }
    sin.sin_port = htons(port);
    bind(a,(struct sockaddr *)h,sizeof h);
    if((connect(a, (struct sockaddr *)&sin, sizeof sin))) {
        perror("connect err") ;
        return -1 ;
    }
    return 1;
}

int
showout()
{
    int i,len,lines,i2;
    char cch[5],tmpbar[80];
    char hostname[81] ,tmpfile[STRLEN],foo[1024];
    struct sockaddr_in sin ;
    char ch,model[]="-/|\\",buf[20];
    int  get;
    FILE *fp,*fp2;
    GOPHER *newitem,*tmpnode;

    i=0;
    if(get_con(g_main[gopher_position]->server,g_main[gopher_position]->port)==-1)
    {
        printf("ÎÞ·¨Á¬ÉÏ....\n");
    }
    enterdir(g_main[gopher_position]->file);
for(i=0;i<MAXGOPHERITEMS;i++)
{
     newitem = (GOPHER *) malloc( sizeof(GOPHER) );
     if (readfield(a, foo, 1024)<= 0) {
          free(newitem);
          break;
     }
     if (foo[0]=='.'&&foo[1] == '\r' && foo[2] == '\n')
     {
          free(newitem);
          break;
     }
        strncpy(newitem->title,foo,70);
     if (readfield(a, foo, 1024) == 0)
     {
          free(newitem);
          break;
     }
        strncpy(newitem->file,foo,80);
     if (readfield(a, foo, 1024) == 0)
     {
          free(newitem);
          break;
     }
        strncpy(newitem->server,foo,40);
     if (readline(a, foo, 1024)==0)
     {
          free(newitem);
          break;
     }
        newitem->port=atoi(foo);
     if(newitem->title[0]!='0'&&newitem->title[0]!='1')
     {
          free(newitem);
          i--;
          continue;
     }
     newitem->next==NULL;
     if(i==0)
     {
        topitem->next=newitem;
        tmpnode=newitem;
     }else
     {
        tmpnode->next=newitem;
        tmpnode=newitem;
     }
}
     
     system("cp /home/bbs/etc/movie /home/bbs/etc/movie2");
     if((fp=fopen("/home/bbs/etc/movie","w"))==NULL)
        return;
     if((fp2=fopen("/home/bbs/etc/movie2","r"))==NULL)
        return;
     lines=0;
     while(fgets( foo,sizeof(foo), fp2)!=NULL)
     {
                if(strstr(foo,TOPSTR))
                        break;
                lines++;
                fputs(foo,fp);
     }
     fclose(fp2);
     sprintf(foo,"%79.79s\n"," ");
     while(lines%MAXnettyLN!=0)
     {
        fputs(foo,fp);
        lines++;
     }
     lines=ACBOARD_MAXLINE-lines;
     lines=(lines/5)*3;
     newitem=find_kth(topitem,i-lines+1);
     unlink("/home/bbs/etc/movie2");
     sprintf(foo,"%s\n",TOPSTR);
     fputs(foo,fp);
                     sprintf(foo,"[1;30;4%dm¡õ¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡õ[m",COLOR);
                     fprintf(fp,"%s\n",foo);

     i=1;
     while(newitem!=NULL)
     {
             i++;
             if(i%3==1)
             {
                     sprintf(foo,"[1;30;4%dm¡õ¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡õ[m",COLOR);
                     fprintf(fp,"%s\n",foo);
             }
             len=strlen(newitem->title+1);
             if(len%2==1)
             {
                len++;
                strcat(newitem->title+1," ");
             }
             len=(80-len)/2-1-2;
             sprintf(tmpbar,"%*.*s",len,len,"");
             sprintf(foo,"[1;4%d;30m©¦[1;3%d;4%dm%s%s%s[m[1;3%d;4%dm©¦[m\n",COLOR,(i%2)+6,COLOR,tmpbar,newitem->title+1,tmpbar,COLOR,COLOR);
             fprintf(fp,"%s",foo);
/*             if(i%3!=0)
             {
                sprintf(foo,"[41m%78.78s[m","");
                fprintf(fp,"%s\n",foo);
             }*/
             if(i%3==0)
             {
                     sprintf(foo,"[1;3%d;4%dm¡õ¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡õ[m",COLOR,COLOR);
                     fprintf(fp,"%s\n",foo);
             }
             newitem=next(newitem);
     }
     fclose(fp);
}

main()
{
         gopher("gopher.nsysu.edu.tw","nntp ls cna.today",4320,"test");
 /*        gopher("news.csie.nctu.edu.tw","gonnrp -T cna.today",4870,"test");*/
}

gopher(serv,dire,port,title)
char serv[],dire[],title[];
int port;
{
        GOPHER *newitem;
        char buf[80];


        gopher_position=0;
        sprintf(TOPSTR,"[4%dm                            [1;3%dm¡¾[37mÖÐÑëÉç¼´Ê±ÐÂÎÅ±¨µ¼[3%dm¡¿[m[4%dm                            [m",COLOR,COLOR,COLOR,COLOR);
        newitem = (GOPHER *) malloc( sizeof(GOPHER) );
        strncpy(newitem->server,serv,40);
        strncpy(newitem->file,dire,80);
        sprintf(buf," %s",title);
        strncpy(newitem->title,buf,70);
        newitem->port=port;
        newitem->position=0;
        g_main[gopher_position]=newitem;

        topitem = (GOPHER *) malloc( sizeof(GOPHER) );
        topitem->next=NULL;
        showout();
        close(a);
}
