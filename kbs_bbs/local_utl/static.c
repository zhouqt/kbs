/* static.c : ÉÏÕ¾ÈË´ÎÍ³¼Æ */
/*Modify By SmallPig*/
#include <time.h>
#include <stdio.h>
#define MAX_LINE        15
#include "../bbs.h"
struct
{
  int no[24];                   /* ´ÎÊý */
  int sum[24];                  /* ×ÜºÏ */
}      st;


main(argc, argv)
  char *argv[];
{
  char *progmode;
  FILE *fp;
  char buf[256], *p;
  char date[80];
  int now;
  int hour, max = 0, item, total = 0;
  int totaltime = 0;
  int i, j;
  struct tm * date_tm;
  char    *blk[10] =
  {
      "  ", "  ", "  ", "  ", "  ",
      "¡õ", "¡õ", "¡õ", "¡õ", "¡õ",
  };

  if ((fp = fopen(BBSHOME"/usies", "r")) == NULL)
  {
    printf("cann't open usies\n");
    return 1;
  }

  now=time(0);
  date_tm = localtime(&now);
  sprintf(date,"%02u/%02u",date_tm->tm_mon+1,date_tm->tm_mday);

  while (fgets(buf, 256, fp))
  {
    hour = atoi(buf+7);
    if (hour < 0 || hour > 23)
    {
       printf("%s", buf);
       continue;
    }
    if(strncmp(buf+1,date,5))
        continue;
    if ( strstr(buf, "ENTER"))
    {
      st.no[hour]++;
      continue;
    }
    if ( p = (char *)strstr(buf+40, "Stay:"))
    {
      st.sum[hour] += atoi( p + 6);
      continue;
    }
  }
  fclose(fp);

  for (i = 0; i < 24; i++)
  {
    total += st.no[i];
    totaltime += st.sum[i];
    if (st.no[i] > max)
      max = st.no[i];
  }

  item = max / MAX_LINE + 1;

  if ((fp = fopen(BBSHOME"/0Announce/bbslists/countlogins", "w")) == NULL)
  {
    printf("Cann't open countlogins\n");
    return 1;
  }

  fprintf(fp,"\n[36m    ©°¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª ³¬¹ý \033[01m\033[37m1000\033[00m\033[36m ½«²»ÏÔÊ¾Ç§Î»Êý×Ö ¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©´\n");
  for (i = max/item+1 ; i >= 0; i--)
  {
    fprintf(fp, "[34m%4d[36m©¦[33m",(i)*item);
    for (j = 0; j < 24; j++)
    {
      if ((item * (i) > st.no[j]) && (item * (i-1) <= st.no[j]) && st.no[j])
      {
        if(st.no[j]>=2000)
                /*fprintf(fp, "[35m###[33m"); Leeward 97.12.08 */
                fprintf(fp, "\033[1m[33m%-3d\033[m[33m", (st.no[j]) % 1000);
        else if (st.no[j] >= 1000) /* Leeward 98.02.27 */
                fprintf(fp, "\033[1m[37m%-3d\033[m[33m", (st.no[j]) % 1000);
        else
                fprintf(fp, "[35m%-3d[33m", (st.no[j]));
        continue;
      }
      if(st.no[j]-item*i<item && item*i<st.no[j])
              fprintf(fp,"%s ", blk[((st.no[j]-item * i)*10)/item]);
      else if(st.no[j]-item * i>=item)
              fprintf(fp,"%s ",blk[9]);
      else
           fprintf(fp,"   ");
    }
    fprintf(fp, "[36m©¦\n");
  }
  fprintf(fp, "   [36m"
    " ©¸¡ª¡ª¡ª[37m   "NAME_BBS_CHINESE NAME_BBS_NICK"  ÉÏÕ¾ÈË´Î±í   [36m¡ª¡ª¡ª[37m%s[36m¡ª¡ª©¼\n"
    /*"    [34m  0  1  2  3  4  5  6  7  8  9  10 11 [31m12 13 14 15 16 17 18 19 20 21 22 23 \n\n"*/ /* Leeward 98.02.27 */
    "    [34m  1  2  3  4  5  6  7  8  9  10 11 12 [31m13 14 15 16 17 18 19 20 21 22 23 24\n\n"
    "               [36m 1 [33m¡õ[36m = [37m%-5d [36m×Ü¹²ÉÏÕ¾ÈË´Î£º[37m%-9d[36mÆ½¾ùÊ¹ÓÃÊ±¼ä£º[37m%d[m \n\n", Ctime(&now),item,total, totaltime / total + 1); /* Leeward 98.09.24 add the 2nd \n for SHARE MEM in ../main.c */

  fclose(fp);
}
