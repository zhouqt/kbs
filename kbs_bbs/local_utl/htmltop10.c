/* Leeward 1997.12.13 */

#include <stdio.h>

#define MAXLINELEN 1024

/* Copied from ~/bbs2www/util.c by Leeward 98.10.22
   Encode the string, change character to %XX */
char *escape_word(unsigned char *word) 
{
    int i, index = 0 ;
    char *result = (char *) malloc( 3 * sizeof(char) * (strlen(word) + 1));

    for (i=0; word[i]; i ++) {
      if (((word[i] >= '0') && (word[i] <= '9')) ||
          ((word[i] >= 'a') && (word[i] <= 'z')) ||
          ((word[i] >= 'A') && (word[i] <= 'Z')) ||
          (word[i] >= 0X80)) {
            result[index++] = word[i];
      } else if(word[i] != '\n') {
            result[index++] = '%';
            sprintf(result+index, "%X", word[i]);
            index += 2;
      }
    }
    result[index] = '\0';
    return result;
}

void main(int argc, char **argv)
{
  FILE *fp, *html;
  char buf[MAXLINELEN], buf2[MAXLINELEN], board[MAXLINELEN];
  int i, j, k;
  char *title; /* Leeward 98.10.22 */


  if (argc < 2)
    exit(1);

  sprintf(buf, "%s/0Announce/bbslists/day", argv[1]);
  if (NULL == (fp = fopen(buf, "rt")))
    exit(2);

  sprintf(buf, "%s/0Announce/bbslists/day.html", argv[1]);
  if (NULL == (html = fopen(buf, "wt")))
  {
    fclose(fp);
    exit(2);    
  }

 
  k = 0;
  fputs("<PRE>", html);
  while (!feof(fp))
  {
    fgets(buf, MAXLINELEN, fp);
    if (feof(fp))
      break;
    else k ++;
 
    for (j = i = 0; i < strlen(buf);)
    {
      if ('\033' == buf[i])
        i += 5;
      else
        buf2[j ++] = buf[i ++]; 
    }
    buf2[j] = 0;
    strcpy(buf, buf2);

    if (k > 2)
    {
      if (k % 2)
      {
        for (j = 0, i = 16; i < 40;)
        {
          if (' ' == buf2[i])
          {
            board[j] = 0;
            break;
          }
          else
            board[j ++] = buf2[i ++];
        }
      }
      else
      {
        for (i = strlen(buf2) - 2; ' ' == buf2[i]; i --)
          buf2[i] = 0;
        buf2[11] = 0;
        title = escape_word(buf2 + 12);
        sprintf(buf, "%s <A HREF=\"/cgi-bin/bbsArticleSearch?board=%s&title=%s\">%s</A>\n", buf2, board, /*buf2 + 12*/ title, buf2 + 12);
        free(title);
      }
    } /* if (k > 2) */

    fputs(buf, html);
  }
  fputs("</PRE>", html);

  fclose(fp);
  fclose(html);
}
