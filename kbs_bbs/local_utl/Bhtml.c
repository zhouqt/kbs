/* Making articles in a board into HTML format and then compress them.       */
/*                                                                           */
/* Leeward 1998.01.05 (Modified 1998.08.28)                                  */
/*                                                                           */
/* This little program scans a board's directory for its articles            */
/* and converts all of them into HTML format new files                       */
/* and finally compress the new into a UNIX tgz format package.              */
/*                                                                           */
/* Also this program duplicates all source files and renames all of them     */
/* into digital MS-DOS 8.3 format file names (avoiding easily re-produce).   */
/*                                                                           */
/* Build:                                                                    */
/*        make Bhtml                                                         */
/* Syntax:                                                                   */
/*        Bhtml [-aAuthor] [-tTitle] [-dDay] [-sStart] [-eEnd]               */
/*                                                                           */
/* Known shortcomings: do not check if disk space is enough                  */
/*                                                                           */
                                                                             

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>


#define MAXSTRLEN 8192
#define STRLEN 80

#define DOTDIR ".DIR"
#define ROOTHTML "index.htm"
#define INDEXHTML "AIXindex.htm"
#define FRAMENAME "SMTHBAIX"

#define HEADER "BBS水木清华站∶讨论区"
#define FOOTER "BBS水木清华站∶讨论区"  

/* Leeward 98.08.28: Do not process those boards not public */
#define XBOARDNUM 6
char Xboard[XBOARDNUM][24] = { "deleted", "junk", "syssecurity", "Registry",
                               "sys_discuss", "Filter"};
typedef struct fileheader {             /* This structure is used to hold data in */
        char filename[STRLEN];     /* the DIR files */
        char owner[STRLEN-4];
        long ldReadCount;     /* 阅读计数 Luzi 99/01/13 减少了owner 4个字节 */
        char title[STRLEN];
        unsigned level;
        unsigned char accessed[ 12 ];   /* struct size = 256 bytes */
} fileheader;

int
main(int argc, char **argv)
{
  FILE *fpDOTDIR, *fpIn, *fpOut, *fpIndex;
  char szBuf[MAXSTRLEN], szBufX[MAXSTRLEN], *ptr, szBoard[48], szAuthor[48] = "", szTitle[128] = "";
  int nArticle, i, j, k, nStart = 0, nEnd = 20480, nDay = -1, nTotal = 0;
  fileheader FH;

  
  printf("This application creates compressed HTML package for BBS.\n");


  printf("Initializing...\n");      
  getcwd(szBuf, MAXSTRLEN);
  for (i = 0; i < XBOARDNUM; i ++)
  {
    ptr = strrchr(szBuf, '/');
    ptr = ptr ? ptr + 1 : szBuf;
    if (!strcmp(ptr, Xboard[i]))
    {
      printf("Board \"%s\" forbidden\n", ptr);
      exit(-4);
    }
  }
  for (i = 1; i < argc; i ++)
  {
    if (!strncasecmp(argv[i], "-d", 2))
    {
      sscanf(argv[i], "-d%d", &nDay);
    }
    else if (!strncasecmp(argv[i], "-s", 2))
    {
      sscanf(argv[i], "-s%d", &nStart);
    }
    else if (!strncasecmp(argv[i], "-e", 2))
    {
      sscanf(argv[i], "-e%d", &nEnd);
    }
    else if (!strncasecmp(argv[i], "-a", 2))
    {
      sscanf(argv[i], "-a%24s", szAuthor);
    }
    else if (!strncasecmp(argv[i], "-t", 2))
    {
      sscanf(argv[i], "-t%96s", szTitle);
    }
    else
    {
      printf("Syntax: Bhtml [-aAuthor] [-tTitle] [-dDay] [-sStart] [-eEnd]\n");
      exit(1998);
    }
  }

  if (nDay < 0)
    nDay = - 1;
  if (nStart < 0)
    nStart = 0;
  if (nEnd < nStart)
    nEnd = nStart;

  printf("Executing: %s -a%s -t%s -d%d -s%d -e%d...\n", argv[0], szAuthor, szTitle, nDay, nStart, nEnd);
 

  getcwd(szBuf, MAXSTRLEN);
  ptr = strrchr(szBuf, '/');
  if (NULL == ptr)
  {
    printf("Directory error?\n");
    exit(-1);
  }
  else
    strcpy(szBoard, ptr + 1);


  sprintf(szBuf, "rm -fr %s.AIX", szBoard);
  system(szBuf);
  sprintf(szBuf, "%s.AIX", szBoard);
  if (0 == chdir(szBuf))
  {
    printf("Can not erase %s\n", szBuf);
    exit(-3);
  }             

  if (mkdir(szBuf, 0000755)) /* drwxr-xr-x */
  {
    printf("Can not create %s\n", szBuf);
    exit(-2);
  }             


  fpDOTDIR = fopen(DOTDIR, "rb");
  if (NULL == fpDOTDIR)
  {
    printf("%s not found\n", DOTDIR);
    exit(1);
  }
 
  sprintf(szBuf, "%s.AIX/%s", szBoard, INDEXHTML);
  fpIndex = fopen(szBuf, "wt");
  if (NULL == fpIndex)
  {
    printf("Cannot create %s\n", szBuf);
    exit(2);    
  }

  fseek(fpDOTDIR, 0L, 2);
  nArticle = ftell(fpDOTDIR) / sizeof(fileheader);
  printf("Currently total %d articles found\n", nArticle);
  rewind(fpDOTDIR);

  fprintf(fpIndex, "<HTML>\n<HEAD>\n  <TITLE>%s</TITLE>\n</HEAD>\n\n<BODY>\n<CENTER>\n\n<H2>%s - %s</H2>\n</CENTER>\n\n", HEADER, HEADER, szBoard);

  for (i = 0; i < nArticle; i ++)
  {
    fread(&FH, sizeof(fileheader), 1, fpDOTDIR);

    if (i + 1 < nStart)
      continue;
    if (i + 1 > nEnd)
      break;
    if (nDay >= 0)
    {
      struct stat st;
      time_t      timeCurrent;
      struct tm   *ptm; 

      timeCurrent = time(NULL);
      ptm = localtime(&timeCurrent);
      ptm->tm_hour = 23;
      ptm->tm_min = ptm->tm_sec = 59;
      timeCurrent = mktime(ptm);

      stat(FH.filename, &st);  
      if (difftime(timeCurrent, st.st_mtime) > (double)(nDay + 1) * 3600 * 24)
        continue;
    }
    if (szAuthor[0])
    {
      if (strcmp(szAuthor, FH.owner))
        continue;
    }
    if (szTitle[0])
    {
      if ( !(  (   !strncasecmp(FH.title, "Re: ", 4) 
                && !strcmp( 4 + FH.title, szTitle  )  
               )
            || (   !strcmp(     FH.title, szTitle  )
               )
            )
         )
        continue;
    }

    printf("Processing article No.%08d: %s...\n", i + 1, FH.filename);

    fpIn = fopen(FH.filename, "rt");
    if (NULL == fpIn)
    {
      printf("%s not found！！！\n", FH.filename);
      continue;
      /*fclose(fpIndex);
      fclose(fpDOTDIR);    
      exit(3);*/
    }

    sprintf(szBuf, "%s.AIX/%08d.htm", szBoard, i);
    fpOut = fopen(szBuf, "wt");
    if (NULL == fpOut)
    {
      printf("Cannot create %s\n", szBuf);
      fclose(fpIndex);
      fclose(fpDOTDIR);  
      exit(4);
    } 

    nTotal ++;
    fprintf(fpOut, "<HTML>\n<HEAD>\n  <TITLE>%s - %s</TITLE>\n</HEAD>\n<BODY>\n\n", HEADER, szBoard);

    while (!feof(fpIn))
    {
      fgets(szBuf, MAXSTRLEN, fpIn);
      if (feof(fpIn))
        break;

      if ('\n' == szBuf[strlen(szBuf) - 1])
        szBuf[strlen(szBuf) - 1] = ' ';
      if (!strncmp(szBuf, "标  题:", 7))
        szBuf[2] = szBuf[3] = 161;
      else if (!strncmp(szBuf, "来  源:", 7))
        szBuf[2] = szBuf[3] = 161;    

      for (j = 0; szBuf[j]; j ++)
      {
        if (ptr = strchr(szBuf + j, '@'))
        {
          j = ptr - szBuf;
          if (strchr(ptr, '.'))
          {
            if (strchr(ptr, ' ') - strchr(ptr, '.') > 0)
            {
              for (k = j - 1; k >= 0; k --)
                if (!(  (szBuf[k] >= '0' && szBuf[k] <= '9')
                      ||(szBuf[k] >= 'A' && szBuf[k] <= 'Z')
                      ||(szBuf[k] >= 'a' && szBuf[k] <= 'z')
                      || '.' == szBuf[k])  )
                  break;
            
              strcpy(szBufX, szBuf + k + 1);
              sprintf(szBuf + k + 1, "mailto:%s", szBufX);
              ptr += 7; /* strlen("mailto:") */
              j = strchr(ptr, ' ') - szBuf - 1;   
            } /* End if (strchr(ptr, ' ') - strchr(ptr, '.') > 0) */
          } /* End if (strchr(ptr, '.')) */
        } /* End if (ptr = strchr(szBuf + j, '@')) */
      } /* for (j = 0; szBuf[j]; j ++) */


      for (j = szBufX[0] = 0; szBuf[j]; j ++)
      {
        switch (szBuf[j])
        {
          case '>':
            strcat(szBufX, "&gt;");
          break;

          case '<':
            strcat(szBufX, "&lt;");
          break;

          case '&':
            strcat(szBufX, "&amp;");
          break;

          case '"':
            strcat(szBufX, "&quot;");
          break;

          case ' ':
            strcat(szBufX, "&nbsp;");
          break;

          case 27:
            ptr = strchr(szBuf + j, 'm');  
            if (ptr)
              j = ptr - szBuf;
          break;
                                                     
          case 'h':
          case 'H':
          case 'f':
          case 'F':
          case 'n':
          case 'N':
          case 'm':
          case 'M':
            if (!strncasecmp(szBuf + j, "http://", 7)
            ||  !strncasecmp(szBuf + j, "ftp://",  6)
            ||  !strncasecmp(szBuf + j, "news://", 7)
            ||  !strncasecmp(szBuf + j, "mailto:", 7))
            {
              ptr = strchr(szBuf + j, ' ');

              if (ptr)
              {
                *ptr = 0;
                k = strlen(szBufX);
                sprintf(szBufX + k, "<A HREF=\"%s\">%s</A>", szBuf + j, szBuf + j + 7 * (!strncasecmp(szBuf + j, "mailto:", 7)));
                *ptr = ' ';
                j += ptr - (szBuf + j) - 1;
                break;
              }
            }
            /* no break here ! */
                                            
          default:
            szBufX[k = strlen(szBufX)] = szBuf[j];
            szBufX[k + 1] = 0;  
        }
      }

      if (':' == szBuf[0])
        sprintf(szBuf, "∶<I>%s</I><BR>\n", szBufX + 1);
      else if ('>' == szBuf[0])
        sprintf(szBuf, "＞<I>%s</I><BR>\n", szBufX + 4);
      else
        sprintf(szBuf, "%s<BR>\n", szBufX);        

      fputs(szBuf, fpOut);
    }

    fprintf(fpOut, "\n</BODY>\n</HTML>\n");
    fclose(fpIn);
    fclose(fpOut);

    fprintf(fpIndex, "No.%d&nbsp;&nbsp;<A HREF=\"%08d.htm\" TARGET=\"%s\">%s </A>&nbsp;&nbsp;&nbsp;&nbsp;&lt;%s&gt;<BR>\n", i + 1, i, FRAMENAME, FH.title, FH.owner); /* Do NOT erase the appended ' ' after the 2nd %s ! (for IE 4) */
  }

  fprintf(fpIndex, "\n<CENTER>\n<HR>\n<H2>%s - %s</H2>\n</CENTER>\n\n</BODY>\n</HTML>\n", FOOTER, szBoard);

  fclose(fpIndex);
  fclose(fpDOTDIR);


  sprintf(szBuf, "%s.AIX/%s", szBoard, ROOTHTML);
  fpIndex = fopen(szBuf, "wt");
  if (NULL == fpIndex)
  {
    printf("Cannot create %s\n", szBuf);
    exit(5);
  }

  fprintf(fpIndex, "<HTML>\n  <TITLE>%s - %s</TITLE>\n\n<FRAMESET ROWS=25%,75%>\n  <FRAME SRC=\"%s\" FRAMEBORDER=1>\n  <FRAME SRC=\"null.htm\" NAME=\"%s\" FRAMEBORDER=0>\n</FRAMESET>\n\n</HTML>\n", HEADER, szBoard, INDEXHTML, FRAMENAME);

  fclose(fpIndex);


  sprintf(szBuf, "%s.AIX/null.htm", szBoard);
  fpIndex = fopen(szBuf, "wt");
  if (NULL == fpIndex)
  {
    printf("Cannot create %s\n", szBuf);
    exit(5);
  }

  fprintf(fpIndex, "<HTML>\n<BODY>\n<CENTER><BR>\n<H2>阅读本版文章</H2><BR>\n<H2>请点击上面窗口内的文章标题</H2></CENTER>\n</BODY>\n</HTML>\n");

  fclose(fpIndex);         


  printf("Compressing HTML files...\n");
  printf("Calling tar...\n");
  sprintf(szBuf, "%s.board.html.tar", szBoard);
  unlink(szBuf);
  sprintf(szBuf, "tar cf %s.board.html.tar %s.AIX", szBoard, szBoard);
  system(szBuf);
  printf("Calling gzip...\n");
  sprintf(szBuf, "%s.board.html.tar.gz", szBoard);
  unlink(szBuf);
  sprintf(szBuf, "gzip %s.board.html.tar", szBoard);
  system(szBuf);

  sprintf(szBuf, "rm -fr %s.AIX", szBoard);
  system(szBuf);
  sprintf(szBuf, "%s.AIX", szBoard);
  if (0 == chdir(szBuf))
  {
    printf("Can not erase %s\n", szBuf);
    exit(-3);
  }

  sprintf(szBuf, "mv -f %s.board.html.tar.gz %s.board.html.tgz", szBoard, szBoard);
  system(szBuf);


  printf("Finished Bhtml: %s.board.html.tgz (Total %d articles, %d files)\n", szBoard, nTotal, nTotal + 3);
  return 0;
}
