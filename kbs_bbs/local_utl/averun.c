/*
    PIVOT Bulletin Board System
    Copyright (C) 1995, Tseng Kuo-Feng, kftseng@ccnews.nchu.edu.tw

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*Modify by SmallPig*/
#ifndef lint
static  char    sccsid[] = "@(#)averun.c   2.0 3/19/95 (C) 1993 University of \
NCHU, Computer Center and Tseng Kuo-Feng";
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define AVEFLE  "/home/bbs/reclog/ave.src"
#define AVEPIC  "/home/bbs/0Announce/bbslists/today"
#define MAX_LINE (16)

char    *Ctime(date)
time_t  *date;
{
        static char buf[80];

        strcpy(buf, (char *)ctime(date));
        buf[strlen(buf)-1] = '\0';
        return buf;
}

int     draw_pic()
{
        char    *blk[10] =
        {
                "£ß", "¡õ", "¡õ", "¡õ", "¡õ",
                "¡õ", "¡õ", "¡õ", "¡õ", "¡õ",
        };
        FILE    *fp;
        int     max = 0,
                cr = 0,
                tm,
                i, item,
                j, aver;
        int     pic[24];
        char    buf[80];
        time_t  now;

        time(&now);
        if ((fp = fopen(AVEFLE, "r")) == NULL)
                return -1;
        else
        {
                bzero(&pic, sizeof(pic));
                i=0;
                while (fgets(buf, 50, fp) != NULL)
                {
                        cr = atoi(index(buf, ':')+1);
                        tm = atoi(buf);
                        pic[tm] = cr;
                        aver+=cr;
                        i++;
                        max = (max > cr) ? max : cr;
                }
                aver=aver/i;
                fclose(fp);
        }

        if ((fp = fopen(AVEPIC, "w")) == NULL)
                return -1;

        item=(max/MAX_LINE)+1;

        fprintf(fp,"\n [1;36m   ©°¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©´[m\n");

        for (i = max/item+1; i >= 0; i--)
        {
                fprintf(fp, "[1;34m%3d[36m ©¦[32m", i*item);
                for (j = 0; j < 24; j++)
                {
                  if ((item * i > pic[j]) && (item * (i-1) <= pic[j])&& pic[j])
                  {
                    fprintf(fp, "[35m%-3d[32m", (pic[j]));
                    continue;
                  }
                  if(pic[j]-item*i<item && item*i<pic[j])
                          fprintf(fp,"%s ", blk[((pic[j]-item * i)*10)/item]);
                  else if(pic[j]-item * i>=item)
                          fprintf(fp,"%s ",blk[9]);
                  else
                           fprintf(fp,"   ");
                }
                fprintf(fp,"[1;36m©¦[m");
                fprintf(fp, "\n");
        }
        time(&now);
        fprintf(fp, "[1;36m    ©¸¡ª¡ª[37mÖÐÕý×Ê¹¤ËÄ°ÙÄêÀ´µÚÒ»Õ¾¸ºÔØÈËÊýÍ³¼Æ[36m¡ª");
        fprintf(fp, "¡ª¡ª[37m%s[36m¡ª¡ª©¼[m\n", Ctime(&now));
        fprintf(fp, "[1;34m      00 01 02 03 04 05 06 07 08 09 10 11 [31m12 13 14");
        fprintf(fp, " 15 16 17 18 19 20 21 22 23[m\n");
        fprintf(fp, "                         [1;36m    1 [32m¡õ[36m = [37m%3d     [36m Æ½¾ùÉÏÕ¾ÈËÊý£º[37m%3d[m\n",item,aver);
        fclose(fp);
}

int     parse_ave(time, ave)
int     time,
        ave;
{
        FILE    *fp;

        if ((fp = fopen(AVEFLE, "a+")) == NULL)
                return -1;
        fprintf(fp, "%d:%d\n", time, ave);
        fclose(fp);
}

int     gain_hour(buf)
char    *buf;
{
        int     retm;

        retm = atoi(buf);
        if (strstr(buf, "pm") && retm != 12)
                retm +=12;
        if (strstr(buf, "am") && retm == 12)
                retm = 0;
        return retm;
}

int     init_base(file, time)
char    *file;
int     *time;
{
        FILE    *fp;
        char    buf[80],
                *p;
        int     ave = 0,
                tmp = 0,
                i;

        if ((fp = fopen(file, "r")) == NULL)
        {
                printf("File: %s cannot be opened\n", file);
                exit(-2);
        }

        for (i = 0; i < 12; i++)
        {
                int     once = 0;

                if (fgets(buf, 99, fp) == NULL)
                        break;
                if (strstr(buf, "day"))
                        once = 1;
                if (i == 0)
                        *time = gain_hour(buf);
                strtok(buf, ",");
                if (once)
                        strtok(NULL, ",");
                p = strtok(NULL, ",");
                ave = ave + atoi(p);
        }
        tmp = ave/i;
        if (tmp*i != ave)
                tmp++;
        fclose(fp);
        return tmp;
}

void    main(argc, argv)
int     argc;
char    **argv;
{
        FILE    *fp;
        char    file[80],
                buf[100],
                start[10],
                end[10],
                *p;
        int     i,
                tmp,
                ave,
                time;


        if (argc < 2)
        {
                printf("Usage: %s crontab_output_filename\n", argv[0]);
                exit(-1);
        }
        ave = init_base(argv[1], &time);
        parse_ave(time, ave);
        draw_pic(time);
}
