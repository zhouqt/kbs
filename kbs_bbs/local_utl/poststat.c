/*-------------------------------------------------------*/
/* util/poststat.c      ( NTHU CS MapleBBS Ver 2.36 )    */
/*-------------------------------------------------------*/
/* target : Õ≥º∆ΩÒ»’°¢÷‹°¢‘¬°¢ƒÍ»»√≈ª∞Ã‚                 */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------
#define DEBUG*/

#include <stdio.h>
#include <time.h>
#include "../bbs.h"
#define	DELETE

char *myfile[] = {"day", "week", "month", "year"};
int mycount[4] = {7, 4, 12};
int mytop[] = {10, 50, 100, 100};
char *mytitle[] = {"»’ Æ", "÷‹ŒÂ Æ", "‘¬∞Ÿ", "ƒÍ∂»∞Ÿ"};


#define HASHSIZE 1024
#define TOPCOUNT 200

struct fileheader              /* This structure is used to hold data in */
 fh[1];

struct postrec
{
  char author[13];              /* author name */
  char board[IDLEN+6];               /* board name */
  char title[66];               /* title name */
  time_t date;                  /* last post's date */
  int number;                   /* post number */
  struct postrec *next;         /* next rec */
}      *bucket[HASHSIZE];


/* 100 bytes */
struct posttop
{
  char author[13];              /* author name */
  char board[IDLEN+6];               /* board name */
  char title[66];               /* title name */
  time_t date;                  /* last post's date */
  int number;                   /* post number */
}       top[TOPCOUNT], postlog;


int
hash(key)
  char *key;
{
  int i, value = 0;

  for (i = 0; key[i] && i < 80; i++)
    value += key[i] < 0 ? -key[i] : key[i];

  value = value % HASHSIZE;
  return value;
}


/* ---------------------------------- */
/* hash structure : array + link list */
/* ---------------------------------- */


void
search(t)
  struct posttop *t;
{
  struct postrec *p, *q, *s;
  int i, found = 0;

  i = hash(t->title);
  q = NULL;
  p = bucket[i];
  while (p && (!found))
  {
    if (!strcmp(p->title, t->title) && !strcmp(p->board, t->board))
      found = 1;
    else
    {
      q = p;
      p = p->next;
    }
  }

  if (found)
  {
    p->number += t->number;
    if (p->date < t->date)      /* »°ΩœΩ¸»’∆⁄ */
      p->date = t->date;
  }
  else
  {
    s = (struct postrec *) malloc(sizeof(struct postrec));
    memcpy(s, t, sizeof(struct posttop));
    s->next = NULL;
    if (q == NULL)
      bucket[i] = s;
    else
      q->next = s;
  }
}


int
sort(pp, count)
  struct postrec *pp;
{
  int i, j;

  for (i = 0; i <= count; i++)
  {
    if (pp->number > top[i].number)
    {
      if (count < TOPCOUNT - 1)
        count++;
      for (j = count - 1; j >= i; j--)
        memcpy(&top[j + 1], &top[j], sizeof(struct posttop));

      memcpy(&top[i], pp, sizeof(struct posttop));
      break;
    }
  }
  return count;
}


void
load_stat(fname)
  char *fname;
{
  FILE *fp;

  if (fp = fopen(fname, "r"))
  {
    int count = fread(top, sizeof(struct posttop), TOPCOUNT, fp);
    fclose(fp);
    while (count)
      search(&top[--count]);
  }
}


void
poststat(mytype)
  int mytype;
{
  static char *logfile = ".post";
  static char *oldfile = ".post.old";

  FILE *fp;
  char buf[40], curfile[40] = "etc/posts/day.0", *p;
  struct postrec *pp;
  int i, j;

          /*Haohmaru.99.11.20.ºÏ≤È «∑Ò“—±ª…æ*/
          FILE* fp1;
          char  dirfile[80];
          int exist,real;

  /*Bigman.2000.8.28: –ﬁ∏ƒÕ≥º∆∑Ω Ω */
	int m,n;
	char BoardName[10][13];
	
  if (mytype < 0)
  {
    /* --------------------------------------- */
    /* load .post and statictic processing     */
    /* --------------------------------------- */

    remove(oldfile);
    rename(logfile, oldfile);
    if ((fp = fopen(oldfile, "r")) == NULL)
      return;
    mytype = 0;
    load_stat(curfile);

    while (fread(top, sizeof(struct posttop), 1, fp))
      search(top);
    fclose(fp);
  }
  else
  {
    /* ---------------------------------------------- */
    /* load previous results and statictic processing */
    /* ---------------------------------------------- */

    i = mycount[mytype];
    p = myfile[mytype];
    while (i)
    {
      sprintf(buf, "etc/posts/%s.%d", p, i);
      sprintf(curfile, "etc/posts/%s.%d", p, --i);
      load_stat(curfile);
      rename(curfile, buf);
    }
    mytype++;
  }

  /* ---------------------------------------------- */
  /* sort top 100 issue and save results            */
  /* ---------------------------------------------- */

  memset(top, 0, sizeof(top));
  for (i = j = 0; i < HASHSIZE; i++)
  {
    for (pp = bucket[i]; pp; pp = pp->next)
    {

#ifdef  DEBUG
      printf("Title : %s, Board: %s\nPostNo : %d, Author: %s\n"
        ,pp->title
        ,pp->board
        ,pp->number
        ,pp->author);
#endif

      j = sort(pp, j);
    }
  }

  p = myfile[mytype];
  sprintf(curfile, "etc/posts/%s.0", p);
  if (fp = fopen(curfile, "w"))
  {
    fwrite(top, sizeof(struct posttop), j, fp);
    fclose(fp);
  }

  sprintf(curfile, "etc/posts/%s", p);
  if (fp = fopen(curfile, "w"))
  {
    /*fprintf(fp, "\t\t[34m-----[37m=====[41m ±æ%s¥Û»»√≈ª∞Ã‚ [40m=====[34m-----[0m\n\n", mytitle[mytype]);*/
    fprintf(fp, "                [34m-----[37m=====[41m ±æ%s¥Û»»√≈ª∞Ã‚ [40m=====[34m-----[0m\n\n", mytitle[mytype]);
    /* Leeward 98.09.24 replace the \t\t to spaces for SHARE MEM in ../main.c */

    i = mytop[mytype];
    if (j > i)
      j = i;
#ifdef  DEBUG
      printf("i : %d, j: %d \n", i, j);
#endif
    real = 0;
    for (i = 0; !mytype ? (real < 10) : (i < j); i++)
    {
      strcpy(buf, ctime(&top[i].date));
      buf[20] = NULL;
      p = buf + 4;
/*      if (!strcmp(top[i].board,"Birthday")) continue; *//*»ı£¨ªπ“™’‚√¥◊ˆ£¨∏ƒ∏ƒ∞Ê√Ê ∂¡»®œﬁ≤ªæÕµ√¡À*/
#ifdef	DELETE
	  /*Haohmaru.99.11.20.ºÏ≤È «∑Ò“—±ª…æ*/
      if( mytype == 0 )/*÷ª”–µ±»’ Æ¥Û≤≈◊ˆœ¬√ÊµƒºÏ≤È*/
	{
	  sprintf(dirfile,"boards/%s/.DIR",top[i].board);
	  exist = 0;
	  if((fp1 = fopen(dirfile,"r") )!= NULL)
	  {
		  exist = 1;
		  while(fread(fh, sizeof(struct fileheader), 1, fp1))
		  {
				if(strstr(fh->title,top[i].title)/* || strstr(top[i].title,fh->title)*/)
				{
					exist = 0;
					break;
				}
		  }
		  fclose(fp1);
	  }
	  if(exist)
		  continue;

	/* Bigman 2000.8.28 –ﬁ∏ƒÕ≥º∆∑Ω Ω */
		m=0;
		for (n=0;n<real;n++) 
		{
			if (!strcmp(top[i].board,BoardName[n]))
				m++;
		}
		
		if (m>=3) continue; 

		strcpy(BoardName[real],top[i].board);
	}
	real++;
#endif
      fprintf(fp,
        "[37mµ⁄[31m%3d[37m √˚ [37m–≈«¯ : [33m%-16s[37m°æ[32m%s[37m°ø[36m%4d [37m»À[35m%+16s\n"
        "     [37m±ÍÃ‚ : [44m[37m%-60.60s[40m\n"
        ,!mytype ? real : (i + 1) , top[i].board, p, top[i].number, top[i].author, top[i].title);
    }
    fclose(fp);
  }

  /* free statistics */

  for (i = 0; i < HASHSIZE; i++)
  {
    for (pp = bucket[i]; pp; pp = pp->next)
      free(pp);
    bucket[i] = NULL;
  }
}


main(argc, argv)
  char *argv[];
{
  time_t now;
  struct tm *ptime;

  if (argc < 2)
  {
    printf("Usage:\t%s bbshome [day]\n", argv[0]);
    return (-1);
  }

  chdir(argv[1]);

  if (argc == 3)
  {
    poststat(atoi(argv[2]));
    return (0);
  }

  time(&now);
  ptime = localtime(&now);
  if (ptime->tm_hour == 0)
  {
    if (ptime->tm_mday == 1)
      poststat(2);
    if (ptime->tm_wday == 0)
      poststat(1);
    poststat(0);
  }
  poststat(-1);
}
