/* Leeward 1997.12.13 */

#include <stdio.h>

#define MAXLINELEN 1024

void main(int argc, char *argv[])
{
  FILE *fp, *html;
  char buf[MAXLINELEN], buf2[MAXLINELEN], board[MAXLINELEN];
  int i, j;


  if (argc < 2)
    exit(1);

  sprintf(buf, "%s/0Announce/bbslists/countlogins", argv[1]);
  if (NULL == (fp = fopen(buf, "rt")))
    exit(2);

  sprintf(buf, "%s/0Announce/bbslists/countlogins.html", argv[1]);
  if (NULL == (html = fopen(buf, "wt")))
  {
    fclose(fp);
    exit(2);    
  }

 
  fputs("<PRE>", html);
  while (!feof(fp))
  {
    fgets(buf, MAXLINELEN, fp);
    if (feof(fp))
      break;
 
    for (j = i = 0; i < strlen(buf);)
    {
      if (161 == buf[i] && (i < strlen(buf) - 2))
      {
        if (245 == buf[i + 1]) 
        {
          strcpy(buf2 + j, "[]");
          j += 2;
          i += 2;
          continue;
        }
      }

      if ('\033' == buf[i])
      {
        while (buf[i ++] != 'm')
          ;
        continue;
      }
      else
        buf2[j ++] = buf[i ++]; 
    }
    buf2[j] = 0;

    fputs(buf2, html);
  }
  fputs("</PRE>", html);

  fclose(fp);
  fclose(html);
}
