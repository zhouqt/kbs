#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>


#define BtoGtablefile "etc/b2g_table"
#define GtoBtablefile "etc/g2b_table"

#define	BtoG_bad1 0xa1
#define	BtoG_bad2 0xf5
#define	GtoB_bad1 0xa1
#define	GtoB_bad2 0xbc

unsigned char* GtoB,* BtoG;
#define GtoB_count 7614
#define BtoG_count 13973

extern int convcode;
char gb2big_savec[2];
char big2gb_savec[2];
extern void redoscr();

#ifdef BBSMAIN
int switch_code()
{
    convcode=!convcode;
    redoscr();
}
#endif

void resolv_file(char* buf)
{
    int fd;
    int i;
    fd=open(BtoGtablefile,O_RDONLY);
    if (fd==-1)
        for (i=0;i< BtoG_count; i++) {
            buf[i*2]=BtoG_bad1;
            buf[i*2+1]=BtoG_bad2;
        }
    else
    {
        read(fd,buf,BtoG_count*2);
        close(fd);
    }
    fd=open(GtoBtablefile,O_RDONLY);
    if (fd==-1)
        for (i=0;i< GtoB_count; i++) {
            buf[BtoG_count*2+i*2]=GtoB_bad1;
            buf[BtoG_count*2+i*2+1]=GtoB_bad2;
        }
    else
    {
        read(fd,buf+BtoG_count*2,GtoB_count*2);
        close(fd);
    }
}

void *
attach_shm2( shmstr, defaultkey, shmsize ,resolv_file)
char*    shmstr;
int     defaultkey, shmsize;
void(*resolv_file)(char*);
{
    void        *shmptr;
    int         shmid;
    int		shmkey;

#ifndef STANDCONV
    /*shmkey = sysconf_eval( shmstr );
    if( shmkey < 1024 )
*/
    shmkey = get_shmkey( shmstr );
    if( shmkey < 1024 )
#endif
        shmkey = defaultkey;
    shmid = shmget( shmkey, shmsize, 0 );
    if( shmid < 0 ) {
        shmid = shmget( shmkey, shmsize, IPC_CREAT | 0600 );
        if( shmid < 0 )
            attach_err( shmkey, "shmget" );
        shmptr = (void *) shmat( shmid, NULL, 0 );
        if( shmptr == (void *)-1 )
            attach_err( shmkey, "shmat" );
        resolv_file(shmptr);
    } else {
        shmptr = (void *) shmat( shmid, NULL, 0 );
        if( shmptr == (void *)-1 )
            attach_err( shmkey, "shmat" );
    }
    return shmptr;
}

void conv_init()
{
    struct stat st;
    int cachesize;

    BtoG=attach_shm2("CONVTABLE_SHMKEY",3013,GtoB_count*2+BtoG_count*2,resolv_file);
    GtoB=BtoG+BtoG_count*2;
    gb2big_savec[0]=0;
    big2gb_savec[0]=0;
    gb2big_savec[1]=0;
    big2gb_savec[1]=0;
}

#define	c1	(unsigned char)(s[0])
#define	c2	(unsigned char)(s[1])

static void g2b(s)
register unsigned char *s;
{
    register unsigned int i;

    if ((c2 >= 0xa1) && (c2 <= 0xfe)) {
        if ((c1 >= 0xa1) && (c1 <= 0xa9)) {
            i = ((c1 - 0xa1) * 94 + (c2 - 0xa1)) * 2;
            s[0] = GtoB[i++];  s[1] = GtoB[i];
            return;
        } else if ((c1 >= 0xb0) && (c1 <= 0xf7)) {
            i = ((c1 - 0xb0 + 9) * 94 + (c2 - 0xa1)) * 2;
            s[0] = GtoB[i++];  s[1] = GtoB[i];
            return;
        }
    }
    s[0] = GtoB_bad1;  s[1] = GtoB_bad2;
}

static void b2g(s)
register unsigned char *s;
{
    register int i;

    if ((c1 >= 0xa1) && (c1 <= 0xf9)) {
        if ((c2 >= 0x40) && (c2 <= 0x7e)) {
            i = ((c1 - 0xa1) * 157 + (c2 - 0x40)) * 2;
            s[0] = BtoG[i++];  s[1] = BtoG[i];
            return;
        } else if ((c2 >= 0xa1) && (c2 <= 0xfe)) {
            i = ((c1 - 0xa1) * 157 + (c2 - 0xa1) + 63) * 2;
            s[0] = BtoG[i++];  s[1] = BtoG[i];
            return;
        }
    }
    s[0] = BtoG_bad1;  s[1] = BtoG_bad2;
}

#undef c1
#undef c2

char *hzconvert (s, plen, psaved, dbcvrt)
char *s;
int *plen;
char *psaved;              /* unprocessed char buffer pointer */
void (*dbcvrt)();          /* 2-byte conversion func for a hanzi */
{
    char *p, *pend;

    if (*plen == 0)
        return (s);
    if (*psaved) {                  /* previous buffered char */
        *(--s) = *psaved;       /* put the unprocessed char down */
        (*plen) ++;
        *psaved = 0;            /* clean this char buffer */
    }
    p = s;  pend = s + (*plen);     /* begin/end of the buffer string */
    while (p < pend) {
        if ((*p) & 0x80)        /* hi-bit on: hanzi */
            if (p < pend-1)         /* not the last one */
                dbcvrt (p++);
            else {                  /* the end of string */
                *psaved = *p;   /* save the unprocessed char */
                (*plen) --;
                break;
            }
        p++;
    }
    return (s);
}


char *gb2big (s,plen,inst)
char *s;
int *plen;
int inst;
{
    return (hzconvert (s, plen, &gb2big_savec[inst], g2b));
}

char *big2gb (s,plen,inst)
char *s;
int *plen;
int inst;
{
    return (hzconvert (s, plen, &big2gb_savec[inst], b2g));
}

/*
main()
{
	char buf[256];
	char *obuf;
	int len;
	buf[0]=1;
	conv_init();
	while (buf[0]!=0) {
	scanf("%s",buf+1);
	len=strlen(buf+1);
	printf("input len:%d",len);
	obuf=gb2big(buf+1,&len);
	printf("output len:%d",len);
	printf("save char:%2X\n",gb2big_savec);
	puts(obuf);
	}
}*/

