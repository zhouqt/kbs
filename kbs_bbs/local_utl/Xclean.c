/* Leeward 98.04.22 Cleaning "dead" files */
/* Leeward 99.04.22 Fixing a bug: 
                    Error erasing files in a directory without .DIR */
                                                                            
#include <stdio.h>
#include <string.h>

#include "bbs.h"

void
main(int argc, char **argv)
{
  char fn[256], dn[256];
  char *p;
  FILE *fp;
  struct fileheader fh;
  int  in;


  if (argc < 2) return;

  p = strrchr(argv[1], '/');
  if (!p) { printf("ERROR process %s\n", argv[1]);  return; }

  strcpy(fn, p + 1);
  *p = 0;
  strcpy(dn, argv[1]);

  if (!strcmp(fn, ".DIR")) return;
  else if (!strcmp(fn, ".DIGEST")) return;
  else if (!strcmp(fn, "deny_users")) return;
  else if (!strcmp(fn, ".badword")) return;

  in = 0;

  chdir(dn);

{ int i;
  for (i = 0; i < 2 && 0 == in; i ++) {

  if (0 == i) fp = fopen(".DIR", "r");
  else if (1 == i) fp = fopen(".DIGEST", "r");

  if (fp)
  {
    while (!feof(fp))
    {
      fread(&fh, sizeof(struct fileheader), 1, fp);
      if (feof(fp))  break;

      if (!strcmp(fh.filename, fn)) { in = 1; break; }
    }
    fclose(fp);
  }
  else if (0 == i)
  {
    printf(".DIR not found! You MUST NOT run clean in directory %s!!\n", dn);
    return;
  }

  } /* End for */
}

  if (!in)  
  {
    char mv[1024];

    sprintf(mv, "%s/%s", dn, fn);
    printf("Unlinking %s\n", mv);
    unlink(mv);
    /*p = strstr(dn, "/mail/");
    if (!p) p = strstr(dn, "/boards/");
    sprintf(mv, "/opt/bbsbackup/clean%s", p);
    mkdir(mv, 0000755);
    sprintf(mv, "/bin/mv %s/%s /opt/bbsbackup/clean%s", dn, fn, p);
    printf("%s\n", mv);
    system(mv);*/
  }
}
