#include <time.h>
#include <stdio.h>
#include "bbs.h"
#include "config.h"

struct binfo
{
  char  boardname[15];
  char  expname[25];
  int times;
  int sum;
} st[MAXBOARD];

int numboards=0;

int 
brd_cmp(b, a)
struct binfo *a, *b;
{
    if(a->times!=b->times)
            return (a->times - b->times);
    return a->sum-b->sum;
}

int
total_cmp(b, a)
struct binfo *a, *b;
{
    if(a->sum!=b->sum)
	return (a->sum - b->sum);
    return a->times - b->times;
}

int
average_cmp(b, a)
struct binfo *a, *b;
{
    int a_ave,b_ave;
    if(a->times)
   	 a_ave = a->sum/a->times;
    else
	 a_ave = 0;
    if(b->times)
    	 b_ave = b->sum/b->times;
    else
	 b_ave = 0;
   
    if(a_ave != b_ave)
	return (a_ave - b_ave);
    return a->sum - b->sum;
}

int
record_data(board,sec)
char *board;
int sec;
{
        int i;
        for(i=0;i<numboards;i++)
        {
                if(!strcmp(st[i].boardname,board))
                {
                        st[i].times++;
                        st[i].sum+=sec;
                        return;
                }
        }
        return ;
}

int
fillbcache(fptr)
struct boardheader *fptr ;
{

    if( numboards >= MAXBOARD )
        return 0;
    if(fptr->level!=0||strlen(fptr->filename)==0)
        return;
    strcpy(st[numboards].boardname,fptr->filename);
    strcpy(st[numboards].expname,fptr->title+13);
    st[numboards].times=0;
    st[numboards].sum=0;
    numboards++;
    return 0 ;
}

int 
fillboard()
{
        apply_record( BOARDS, fillbcache, sizeof(struct boardheader) );
}

char *
timetostr(i)
int i;
{
        static char str[30];
        int minute,sec,hour;

        minute=(i/60);
        hour=minute/60;
        minute=minute%60;
        sec=i&60;
        sprintf(str,"%2d:%2d:%2d",hour,minute,sec);
        return str;
}

main(argc, argv)
char *argv[];
{
  char *progmode;
  FILE *fp;
  FILE *op,*op1,*op2;
  char buf[256],buf1[256],buf2[256], *p,bname[20];
  char *q;//added by Czz 010614
  char date[80];
  int mode;
  int c[3];
  int max[3];
  unsigned int ave[3];
  int now,sec;
  int i, j,k;
  char    *blk[10] =
  {
      "  ","  ", "  ", "  ", "  ",
      "¡õ","¡õ", "¡õ", "¡õ", "¡õ",
  };

  if(argc<=1) exit(0);
  mode=atoi(argv[1]);
  if(mode==1)
  {
        strcpy(buf,BBSHOME"/0Announce/bbslists/board2");
	strcpy(buf1,BBSHOME"/0Announce/bbslists/totaltime");
	strcpy(buf2,BBSHOME"/0Announce/bbslists/averagetime");
  }
  else
  {
        strcpy(buf,BBSHOME"/0Announce/bbslists/board1");
  }
  if ((fp = fopen(BBSHOME"/boardusage.log", "r")) == NULL)
  {
    printf("cann't open boardusage.log\n");
    return 1;
  }
  if(mode==1){
  if ((op = fopen(buf, "w")) == NULL || (op1 = fopen(buf1, "w")) == NULL || (op2 = fopen(buf2, "w")) == NULL)
  {
    printf("Can't Write file\n");
    return 1;
  }}
  else if((op = fopen(buf, "w")) == NULL)
  {
    printf("Can't Write file\n");
    return 1;
  }


  fillboard();
  now=time(0);
  sprintf(date,"%6.6s",Ctime(&now)+4);
  printf("%6.6s",date);/**/
  while (fgets(buf, 256, fp))
  {
    if(strlen(buf)<57)
        continue;
/*
    if ( !strncmp(buf+21, "USE", 3))
    {
      p=strstr(buf,"USE");
      p+=4;
      p=strtok(p," ");
      strcpy(bname,p);
    if ( p = (char *)strstr(buf+46, "Stay: "))
    {
      sec=atoi( p + 6);
    }
    else
        sec=0;
*/
/* modified by Czz 010614 */
    if ( p = (char *)strstr(buf, "Stay: "))
    {
	q = p-21;
	q=strtok(q," ");
	strcpy(bname,q);
	sec=atoi( p + 6);
    } 
    record_data(bname,sec);
    }
/* modified end */
/*
   }
*/
   fclose(fp);
   qsort(st, numboards, sizeof( st[0] ), brd_cmp);
   printf("%d",numboards);/**/
   ave[0]=0;
   ave[1]=0;
   ave[2]=0;
   max[1]=0;
   max[0]=0;
   max[2]=0;
   for(i=0;i<numboards;i++)
   {
        ave[0]+=st[i].times;
        ave[1]+=st[i].sum;
        ave[2]+=st[i].times==0?0:st[i].sum/st[i].times;
        if(max[0]<st[i].times)
        {
                max[0]=st[i].times;
        }
        if(max[1]<st[i].sum)
        {
                max[1]=st[i].sum;
        }
        if(max[2]<(st[i].times==0?0:st[i].sum/st[i].times))
        {
                max[2]=(st[i].times==0?0:st[i].sum/st[i].times);
        }
   }
   c[0]=max[0]/30+1;
   c[1]=max[1]/30+1;
   c[2]=max[2]/30+1;
   numboards++;
   st[numboards-1].times=ave[0]/numboards;
   st[numboards-1].sum=ave[1]/numboards;
   strcpy(st[numboards-1].boardname,"Average");
   strcpy(st[numboards-1].expname,"×ÜÆ½¾ù");
   if(mode==1)
   {
        fprintf(op,"Ãû´Î %-15.15s%-25.25s %5s %8s %10s\n","ÌÖÂÛÇøÃû³Æ","ÖÐÎÄÐðÊö","ÈË´Î","ÀÛ»ýÊ±¼ä","Æ½¾ùÊ±¼ä");
   }else
   {
        fprintf(op,"      [37m1 [m[34m%2s[37m= %d (×ÜÈË´Î) [37m1 [m[32m%2s[37m= %s (ÀÛ»ý×ÜÊ±Êý) [37m1 [m[31m%2s[37m= %d Ãë(Æ½¾ùÊ±Êý)\n\n",
                blk[9],c[0],blk[9],timetostr(c[1]),blk[9],c[2]);
   }

   for(i=0;i<numboards;i++)
   {
      if(mode==1)
      {
        fprintf(op,"%4d[m %-15.15s%-25.25s %5d %-.8s %10d\n",i+1,st[i].boardname,st[i].expname,st[i].times,timetostr(st[i].sum),st[i].times==0?0:st[i].sum/st[i].times);
      }
      else
      {
        fprintf(op,"      [37mµÚ[31m%3d [37mÃû ÌÖÂÛÇøÃû³Æ£º[31m%s [35m%s[m\n",i+1,st[i].boardname,st[i].expname);
        fprintf(op,"[37m    ©°¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª\n");
        fprintf(op,"[37mÈË´Î©¦[m[34m");
        for(j=0;j<st[i].times/c[0];j++)
        {
                fprintf(op,"%2s",blk[9]);
        }
                fprintf(op,"%2s [37m%d[m\n",blk[(st[i].times%c[0])*10/c[0]],st[i].times);
        fprintf(op,"[1;37mÊ±¼ä©¦[m[32m");
        for(j=0;j<st[i].sum/c[1];j++)
        {
                fprintf(op,"%2s",blk[9]);
        }
                fprintf(op,"%2s [37m%s[m\n",blk[(st[i].sum%c[1])*10/c[1]],timetostr(st[i].sum));
        j=st[i].times==0?0:st[i].sum/st[i].times;
        fprintf(op,"[37mÆ½¾ù©¦[m[31m");
        for(k=0;k<j/c[2];k++)
        {
                fprintf(op,"%2s",blk[9]);
        }
                fprintf(op,"%2s [37m%s[m\n",blk[(j%c[2])*10/c[2]],timetostr(j));
        fprintf(op,"[37m    ©¸¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª[m\n\n");
      }
   }
   fclose(op);
   if(mode==1){
   qsort(st, numboards-1, sizeof( st[0] ), total_cmp);
   fprintf(op1,"Ãû´Î %-15.15s%-25.25s %8s %5s %10s\n","ÌÖÂÛÇøÃû³Æ","ÖÐÎÄÐðÊö","ÀÛ»ýÊ±¼ä","ÈË´Î","Æ½¾ùÊ±¼ä");
   for(i=0;i<numboards;i++)
      fprintf(op1,"%4d %-15.15s%-25.25s %-.8s %5d %10d\n",i+1,st[i].boardname,st[i].expname,timetostr(st[i].sum),st[i].times,st[i].times==0?0:st[i].sum/st[i].times);
   fclose(op1);
   qsort(st, numboards-1, sizeof( st[0] ), average_cmp);
   fprintf(op2,"Ãû´Î %-15.15s%-25.25s %10s %5s %8s\n","ÌÖÂÛÇøÃû³Æ","ÖÐÎÄÐðÊö","Æ½¾ùÊ±¼ä","ÀÛ»ýÊ±¼ä","ÈË´Î");
   for(i=0;i<numboards;i++)
	fprintf(op2,"%4d %-15.15s%-25.25s %10d %-.8s %5d\n",i+1,st[i].boardname,st[i].expname,st[i].times==0?0:st[i].sum/st[i].times,timetostr(st[i].sum),st[i].times);
   fclose(op2);}
}
