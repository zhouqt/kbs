/* 创建精华区打包文件 
   KCN 2001.3.21
*/

#include "bbs.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef AIX
#include <sys/mode.h>
#else
#ifdef LINUX
#include <stdlib.h>
#endif /*LINUX*/
#endif /*AIX*/
#include <errno.h>
#include <time.h>
#define DOTNAMES ".Names"
#define INDEXHTML "index.htm"
#define HEADER BBS_FULL_NAME "∶精华区"
#define FOOTER BBS_FULL_NAME "∶精华区"
/* #define MAXLINELEN 512 */
#define MAXLINELEN 512
#ifdef AIX
#define GNUTAR "/usr/local/bin/tar"
#elif defined FREEBSD
#define GNUTAR "/usr/bin/tar"
#else
#define GNUTAR "/bin/tar"
#endif
    typedef struct __tagDIR_DATA {
    char dir[MAXLINELEN];
    struct __tagDIR_DATA *next;
} DIR_DATA, *LP_DIR_DATA;

LP_DIR_DATA task_head, task_tail;
char WorkDir[MAXLINELEN];
char OutDir[MAXLINELEN];
extern int errno;

char prevprevHtml[MAXLINELEN];
char prevHtml[MAXLINELEN];
char nextHtml[MAXLINELEN];
FILE *pdstFile;
FILE *chmcontentFile;
FILE *attachFile;

int DealParameters(int argc, char **argv)
{
    extern char *optarg;
    extern int optind, opterr, optopt;

    char c;
    int flag = 1;

    chmcontentFile = NULL;
    while (flag) {
        int this_option_optind = optind ? optind : 1;
        int option_index;

        c = getopt(argc, argv, "w:d:o:c:");
        if (c == -1)
            break;
        switch (c) {
        case 'd':
            printf("change to directory %s\n", optarg);
            chdir(optarg);
            break;
        case 'w':
            printf("Set Work Directory:%s\n", optarg);
            strncpy(WorkDir, optarg, MAXLINELEN - 1);
            break;
        case 'o':
            printf("Set OutputDirectory:%s\n", optarg);
            strncpy(OutDir, optarg, MAXLINELEN - 1);
            if (OutDir[strlen(OutDir) - 1] == '/')
                OutDir[strlen(OutDir) - 1] = 0;
            break;
        case 'c':
            if (chmcontentFile) {
                printf("Too many -c!\n");
                return -1;
            }
            chmcontentFile = fopen(optarg, "wt");
            if (!chmcontentFile) {
                printf("Can't open %s:%s", optarg, strerror(errno));
                return -1;
            }
            printf("Set CHM content file:%s\n", optarg);
            break;
        default:
            flag = 0;
        }
    }

    for (; optind < argc; optind++) {
        char *p;

        if (task_head) {
            printf("too many argument!\n");
            return -1;
        }
        task_head = (LP_DIR_DATA) malloc(sizeof(DIR_DATA));

        p = strrchr(argv[optind], '/');
        if (p)
            if (*(p + 1) == '/') {
                *p = 0;
                p = strrchr(argv[optind], '/');
            };
        if (p) {
            *p = 0;
            chdir(argv[optind]);
            printf("change to Directory:%s\n", argv[optind]);
            p++;
        } else
            p = argv[optind];

        strncpy(task_head->dir, p, MAXLINELEN - 1);
        task_head->next = NULL;
    }
    if (!task_head) {
        printf("usage:%s -o out_directory -d announce_dir directory_need_deal_with\n", argv[0]);
        return -1;
    }
    return 0;
}

char *DealLink(char *directory, char *Link, int index, int *isDir, char *date, char *title)
{
    static char filename[MAXLINELEN];
    struct stat st;
    struct tm *tmstruct;

    sprintf(filename, "%s/%s", directory, Link);
    if (stat(filename, &st) == -1) {
        printf("Directory or File %s is not exist!\n", filename);
        return NULL;
    }
    tmstruct = localtime(&st.st_mtime);
    sprintf(date, "%04d.%02d.%02d", tmstruct->tm_year + 1900, tmstruct->tm_mon + 1, tmstruct->tm_mday);
    if (S_ISDIR(st.st_mode)) {
        LP_DIR_DATA data = (LP_DIR_DATA) malloc(sizeof(DIR_DATA));

        data->next = NULL;
        strncpy(data->dir, filename, MAXLINELEN);
        if (task_tail) {
            task_tail->next = data;
            task_tail = data;
        } else {
            task_head = data;
            task_tail = data;
        }

        sprintf(filename, "%s/index.htm", Link);
        *isDir = 1;
        return filename;
    } else {
        /*
         * 处理一般的精华区文件 
         */
        FILE *psrcFile;
        FILE *pBBSFile;
        int i, j, k;
        int attach = 0;

        char srcLine[MAXLINELEN], dstLine[MAXLINELEN * 20];
        char Buf2[MAXLINELEN * 4] = "";
        char *ptr;

        if (NULL == (psrcFile = fopen(filename, "rt"))) {
            printf("Unexpected error: Can not open file \"%s\"\n", filename);
            return NULL;
        }

        sprintf(filename, "%s/%s/%08d.htm", WorkDir, directory, index);

        if (pdstFile) {
            /*
             * 关闭上一个HTML文件 
             */
            if (prevprevHtml[0]) {
                fprintf(pdstFile, "<a href=\"%s\">上一篇</a>\n", prevprevHtml);
            }
            fputs("<a href=\"javascript:history.go(-1)\">返回上一页</a>\n", pdstFile);
            fputs("<a href=\"index.htm\">回到目录</a>\n", pdstFile);
            fputs("<a href=\"#top\">回到页首</a>\n", pdstFile);
            fprintf(pdstFile, "<a href=\"%08d.htm\">下一篇</a>\n", index);
            fputs("</h1></center>\n", pdstFile);
            fputs("<center><h1>", pdstFile);
            fputs(FOOTER, pdstFile);
            fputs("</h1></center>\n", pdstFile);
            fputs("</body></html>", pdstFile);
            fclose(pdstFile);
            pdstFile = NULL;
        }
        if (NULL == (pdstFile = fopen(filename, "wt"))) {
            printf("Unexpected error: Can not write file \"%s\"\n", filename);
            fclose(pdstFile);
            return NULL;
        }

        fputs("<?xml version=\"1.0\" encoding=\"gb2312\"?>\n", pdstFile);
        fputs("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n", pdstFile);
        fputs("<html xmlns=\"http://www.w3.org/1999/xhtml\">\n<head>\n", pdstFile);
        fputs("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\"/>\n", pdstFile);
        fputs("<title>", pdstFile);
        fputs(title, pdstFile);
        fputs("</title>\n</head>\n<body>\n<center><h1>", pdstFile);
        fputs(HEADER, pdstFile);
        fputs("</h1></center>\n", pdstFile);
        fputs("<a name=\"top\"></a>\n", pdstFile);

        sprintf(filename, "%s/bbs/%s/%s", WorkDir, directory, Link);
        if (NULL == (pBBSFile = fopen(filename, "wt")))
            printf("Unexpected error: Can not open file \"%s\"\n", filename);

        while (!feof(psrcFile)) {
            long attach_len, size;
            char *attach_ptr, *attach_filename, *p;

            if (fgets(srcLine, MAXLINELEN, psrcFile) == 0)
                break;
            if (fputs(srcLine, pBBSFile) == EOF)
                perror("fputs error bbs file:");

            if (attach == 0) {
                p = srcLine;
                size = strlen(srcLine);
                if (NULL != (attach_filename = checkattach(p, size, &attach_len, &attach_ptr))) {
                    sprintf(filename, "%s/%s/%s", WorkDir, directory, index);
                    if (mkdir(filename, 0700) == -1) {
                        if (errno != EEXIST) {
                            fprintf(stderr, "Create Directory %s failed:%s", filename, strerror(errno));
                            return;
                        }
                    }
                    strcat(filename, "/");
                    strcat(filename, attach_filename);
                    if (NULL == (attachFile = fopen(filename, "wt"))) {
                        printf("Unexpected error: Can not write file \"%s\"\n", filename);
                        fclose(attachFile);
                        return NULL;
                    }
                    fputs(attach_ptr, attachFile);
                    fclose(attachFile);
                    if (((p = strrchr(attach_filename, '.')) != NULL) && !strcasecmp(p, ".bmp") && !strcasecmp(p, ".jpg") && !strcasecmp(p, ".gif") && !strcasecmp(p, ".jpeg"))
                        sprintf(dstLine, "附图:\r\n<img src=\"%d/%s\"></img>\r\n", index, attach_filename);
                    else
                        sprintf(dstLine, "附件:\r\n<a href=\"%d/%s\">%s</a>\r\n", index, attach_filename, attach_filename);
                } else {
                    if ('\n' == srcLine[strlen(srcLine) - 1])
                        srcLine[strlen(srcLine) - 1] = ' ';

                    for (j = 0; srcLine[j]; j++) {
                        if (ptr = strchr(srcLine + j, '@')) {
                            j = ptr - srcLine;
                            if (strchr(ptr, '.')) {
                                if (strchr(ptr, ' ') - strchr(ptr, '.') > 0) {
                                    for (k = j - 1; k >= 0; k--)
                                        if (!((srcLine[k] >= '0' && srcLine[k] <= '9')
                                              || (srcLine[k] >= 'A' && srcLine[k] <= 'Z')
                                              || (srcLine[k] >= 'a' && srcLine[k] <= 'z')
                                              || '.' == srcLine[k]))
                                            break;

                                    strcpy(Buf2, srcLine + k + 1);
                                    sprintf(srcLine + k + 1, "mailto:%s", Buf2);
                                    ptr += 7;   /* strlen("mailto:") */
                                    j = strchr(ptr, ' ') - srcLine - 1;
                                }       /* End if (strchr(ptr, ' ') - strchr(ptr, '.') > 0) */
                            }   /* End if (strchr(ptr, '.')) */
                        }       /* End if (ptr = strchr(srcLine + j, '@')) */
                    }           /* for (j = 0; srcLine[j]; j ++) */

                    for (j = Buf2[0] = 0; srcLine[j]; j++) {
                        switch (srcLine[j]) {
                        case '>':
                            strcat(Buf2, "&gt;");
                            break;

                        case '<':
                            strcat(Buf2, "&lt;");
                            break;

                        case '&':
                            strcat(Buf2, "&amp;");
                            break;

                        case '"':
                            strcat(Buf2, "&quot;");
                            break;

                        case ' ':
                            strcat(Buf2, "&nbsp;");
                            break;

                        case 27:
                            ptr = strchr(srcLine + j, 'm');
                            if (ptr)
                                j = ptr - srcLine;
                            break;

                        case 'h':
                        case 'H':
                        case 'f':
                        case 'F':
                        case 'n':
                        case 'N':
                        case 'm':
                        case 'M':
                            if (!strncasecmp(srcLine + j, "http://", 7)
                                || !strncasecmp(srcLine + j, "ftp://", 6)
                                || !strncasecmp(srcLine + j, "news://", 7)
                                || !strncasecmp(srcLine + j, "mailto:", 7)) {
                                ptr = strchr(srcLine + j, ' ');

                                if (ptr) {
                                    *ptr = 0;
                                    k = strlen(Buf2);
                                    sprintf(Buf2 + k, "<a href=\"%s\">%s</a>", srcLine + j, srcLine + j + 7 * (!strncasecmp(srcLine + j, "mailto:", 7)));
                                    *ptr = ' ';
                                    j += ptr - (srcLine + j) - 1;
                                    break;
                                }
                            }
                            /*
                             * no break here ! 
                             */

                        default:
                            Buf2[k = strlen(Buf2)] = srcLine[j];
                            Buf2[k + 1] = 0;
                        }
                    }

                    if (':' == srcLine[0])
                        sprintf(dstLine, "∶<i>%s</i><br />\n", Buf2 + 1);
                    else if ('>' == srcLine[0])
                        sprintf(dstLine, "＞<i>%s</i><br />\n", Buf2 + 4);
                    else
                        sprintf(dstLine, "%s<br />\n", Buf2);
                }
            } else {
                sprintf(filename, "%s/%s/%s", WorkDir, directory, index);
                if (mkdir(filename, 0700) == -1) {
                    if (errno != EEXIST) {
                        fprintf(stderr, "Create Directory %s failed:%s", filename, strerror(errno));
                        return;
                    }
                }
                strcat(filename, "/");
                strcat(filename, attach_filename);
                if (NULL == (attachFile = fopen(filename, "wt"))) {
                    printf("Unexpected error: Can not write file \"%s\"\n", filename);
                    fclose(attachFile);
                    return NULL;
                }
                fputs(attach_ptr, attachFile);
                fclose(attachFile);
            }

            fputs(dstLine, pdstFile);
        }

        fclose(psrcFile);
        if (pBBSFile)
            fclose(pBBSFile);
        sprintf(filename, "%08d.htm", index);
        strcpy(prevprevHtml, prevHtml);
        strcpy(prevHtml, filename);
    }
    *isDir = 0;
    return filename;
}

void DealDirectory(char *directory)
{
    FILE *IndexHtmlFile;
    FILE *DotFile;
    FILE *BBSDotFile;
    char filename[MAXLINELEN];
    int index;

    printf("Dealing Directory %s\n", directory);
    if (strstr(directory, "install")) {
        prevHtml[1] = 1;
    }
    prevHtml[0] = 0;
    prevprevHtml[0] = 0;

    sprintf(filename, "%s/%s", WorkDir, directory);
    if (mkdir(filename, 0700) == -1) {
        if (errno != EEXIST) {
            fprintf(stderr, "Create Directory %s failed:%s", filename, strerror(errno));
            return;
        }
    }
    sprintf(filename, "%s/bbs/%s", WorkDir, directory);
    if (mkdir(filename, 0700) == -1) {
        if (errno != EEXIST)
            fprintf(stderr, "Create Directory %s failed:%s", filename, strerror(errno));
    };

    sprintf(filename, "%s/%s", directory, DOTNAMES);
    if ((DotFile = fopen(filename, "rt")) == NULL) {
        fprintf(stderr, "can't open %s file", filename);
        return;
    }
    sprintf(filename, "%s/bbs/%s/%s", WorkDir, directory, DOTNAMES);
    if ((BBSDotFile = fopen(filename, "wt")) == NULL) {
        fprintf(stderr, "can't open %s file", filename);
        return;
    }

    sprintf(filename, "%s/%s/%s", WorkDir, directory, INDEXHTML);
    if ((IndexHtmlFile = fopen(filename, "wt")) == NULL) {
        fclose(DotFile);
        if (BBSDotFile)
            fclose(BBSDotFile);
        perror("can't open Index Html file");
        return;
    }

    index = 0;
    while (!feof(DotFile)) {
        char Buf[MAXLINELEN];
        char anchor[MAXLINELEN] = "";
        char *ptr;

        if (fgets(Buf, MAXLINELEN, DotFile) == 0)
            break;
        if (BBSDotFile)
            if (!(strstr(Buf, "Name=") && (strstr(Buf, "(BM: BMS)") || strstr(Buf, "(BM: SYSOPS)"))))
                if (fputs(Buf, BBSDotFile) == EOF)
                    perror("fputs bbs .Name:");
        Buf[strlen(Buf) - 1] = 0;
        if (ptr = strstr(Buf, "Title=")) {
            fputs("<?xml version=\"1.0\" encoding=\"gb2312\"?>\n", IndexHtmlFile);
            fputs("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n", IndexHtmlFile);
            fputs("<html xmlns=\"http://www.w3.org/1999/xhtml\">\n<head>\n", IndexHtmlFile);
            fputs("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\">\n", IndexHtmlFile);
            fputs("<title>", IndexHtmlFile);
/*          fputs("<meta http-equiv='Content-Type' content='text/html; charset=gb2312'>");
		  fputs("<link rel=stylesheet type=text/css href='/bbs.css'>");
*/
            fputs(ptr + 6, IndexHtmlFile);
            fputs("</title>\n</head>\n\n<body>\n\n<center><h1>", IndexHtmlFile);
            fputs(HEADER, IndexHtmlFile);
            fputs("</h1></center>\n\n", IndexHtmlFile);
            fputs("<center>\n", IndexHtmlFile);
            fputs("<table>", IndexHtmlFile);
            fputs("<tr><th>编号<th>类别<th class=\"body\">标题<th class=\"body\">编辑日期</tr>", IndexHtmlFile);
        } else if (ptr = strstr(Buf, "Name=")) {
            char Name[256];

            if (strstr(Buf, "(BM: BMS)"))
                continue;
            if (strstr(Buf, "(BM: SYSOPS)"))
                continue;
            strcpy(Name, ptr + 5);

            strcpy(anchor, ptr + 5);
            while (1) {
                fgets(Buf, MAXLINELEN, DotFile);
                if (feof(DotFile)) {
                    printf("Unexpected error: Incorrect format in \"%s\" file\n\tName not match path", DOTNAMES);
                    break;
                } else {
                    if (BBSDotFile)
                        fputs(Buf, BBSDotFile);
                    if (ptr = strstr(Buf, "Path=~/")) {
                        char *herfname;
                        char datestr[25];
                        int isDir;

                        ptr[strlen(ptr) - 1] = 0;
                        herfname = DealLink(directory, ptr + 7, index, &isDir, datestr, Name);
                        if (herfname) {
                            index++;
                            fprintf(IndexHtmlFile, "<tr><td>%d</td><td>%s</td>", index, isDir ? "目录" : "文件");
                            fprintf(IndexHtmlFile, "<td><a href=\"%s\">%s</a></td>", herfname, anchor);
                            fprintf(IndexHtmlFile, "<td>%s</td></tr>\n", datestr);
                        }
                        break;
                    }
                }               /* feof */
            }                   /* while (1) */
        }                       /* if Buf has "Name" */
    };                          /* while feof(DotFile) */
    fclose(DotFile);
    if (BBSDotFile)
        fclose(BBSDotFile);

    if (pdstFile) {
/*    fputs("</CENTER>\n",pdstFile);*/
        if (prevHtml[0]) {
            fprintf(pdstFile, "<a href=\"%s\">上一篇</a>\n", prevHtml);
        }
        fputs("<a href=\"javascript:history.go(-1)\">返回上一页</a>\n", pdstFile);
        fputs("<a href=\"index.htm\">回到目录</a>\n", pdstFile);
        fputs("<a href=\"#top\">回到页首</a>\n", pdstFile);
        fputs("</center>\n", pdstFile);
        fputs("<center><h1>", pdstFile);
        fputs(FOOTER, pdstFile);
        fputs("</h1></center>\n", pdstFile);
        fputs("</body></html>", pdstFile);
        fclose(pdstFile);
        pdstFile = NULL;
    }
    fputs("</table>\n", IndexHtmlFile);

    fputs("<a href=\"javascript:history.go(-1)\">返回上一页</a>\n", IndexHtmlFile);
    fputs("<a href=\"../index.htm\">回到上一级目录</a>\n", IndexHtmlFile);
    fputs("</center>\n", IndexHtmlFile);
    fputs("<center><h1>", IndexHtmlFile);
    fputs(FOOTER, IndexHtmlFile);
    fputs("</h1></center>\n", IndexHtmlFile);
    fputs("</body></html>", IndexHtmlFile);

    fclose(IndexHtmlFile);
}

int main(int argc, char **argv)
{
    char maindir[MAXLINELEN];
    char Buf[MAXLINELEN];
    char basedir[MAXLINELEN];

    strcpy(basedir, getcwd(NULL, 0));
    task_head = NULL;
    WorkDir[0] = 0;
    if (DealParameters(argc, argv) != 0)
        return -1;

    if (chmcontentFile) {
        fputs("<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML//EN\">\n", chmcontentFile);
        fputs("<HTML>\n<HEAD>\n<meta name=\"GENERATOR\" content=\"Microsoft&reg; HTML Help Workshop 4.1\">\n", chmcontentFile);
        fputs("<!-- Sitemap 1.0 -->\n</HEAD><BODY>\n<UL>\n", chmcontentFile);
    }
    if (WorkDir[0] == 0) {
        sprintf(WorkDir, "%s.AIX", task_head->dir);
    } else {
        strcat(WorkDir, task_head->dir);
        strcat(WorkDir, ".AIX");
    }

    if (OutDir[0] == 0) {
        strcpy(OutDir, basedir);
    } else if (OutDir[0] != '/') {
        strcpy(Buf, basedir);
        if (Buf[strlen(Buf) - 1] != '/')
            strcat(Buf, "/");
        strcat(Buf, OutDir);
        strcpy(OutDir, Buf);
    }

    pdstFile = NULL;
    strcpy(maindir, task_head->dir);
    if (mkdir(WorkDir, 0700) == -1) {
        fprintf(stderr, "mkdir %s:%s", WorkDir, strerror(errno));
        return 0;
    }
    sprintf(Buf, "%s/bbs", WorkDir);
    if (mkdir(Buf, 0700) == -1) {
        fprintf(stderr, "mkdir %s/bbs:%s", WorkDir, strerror(errno));
        return 0;
    }
    printf("Begin %s\n", task_head->dir);
    task_tail = task_head;
    while (task_head) {
        LP_DIR_DATA dirdata = task_head;

        task_head = task_head->next;
        if (!task_head)
            task_tail = NULL;
        DealDirectory(dirdata->dir);
        free(dirdata);
    }
    free(task_head);

    printf("Finished creating HTML files...\n");

    printf("Compressing BBS files...\n");
    printf("Calling \"gnu tar\"...\n");
    sprintf(Buf, "%s/%s.bbs.tgz", OutDir, maindir);
    unlink(Buf);
    sprintf(Buf, "cd %s/bbs; %s zcf %s/%s.bbs.tgz %s", WorkDir, GNUTAR, OutDir, maindir, maindir);
    printf("%s\n", Buf);
    system(Buf);

    printf("Compressing HTML files...\n");
    printf("Calling \"gnu tar\"...\n");
    sprintf(Buf, "%s/%s.html.tgz", OutDir, maindir);
    unlink(Buf);
    sprintf(Buf, "cd %s; %s zcf %s/%s.html.tgz %s", WorkDir, GNUTAR, OutDir, maindir, maindir);
    printf("%s\n", Buf);
    system(Buf);

    printf("Cleaning working directory/data...\n");
    sprintf(Buf, "rm -fr %s", WorkDir);
    system(Buf);
    printf("Finished Xhtml: %s/%s.html.tgz\n", OutDir, maindir);
    return 0;
}
