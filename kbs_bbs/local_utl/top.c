#include <stdio.h>
#define REAL_INFO
#include "bbs.h"

struct userec aman;
struct userec allman[MAXUSERS];
char passwd_file[256];
char *home_path;

int login_cmp(b, a)
    struct userec *a, *b;
{
    return (a->numlogins - b->numlogins);
}

int post_cmp(b, a)
    struct userec *a, *b;
{
    return (a->numposts - b->numposts);
}

int stay_cmp(b, a)
    struct userec *a, *b;
{
    return (a->stay - b->stay);
}

int perm_cmp(b, a)
    struct userec *a, *b;
{
    return (a->numlogins / 3 + a->numposts + a->stay / 3600) - (b->numlogins / 3 + b->numposts + b->stay / 3600);
}

top_login(num)
{
    int i, j, rows = (num + 1) / 2;
    char buf1[80], buf2[80];

    printf("\n");
    printf("%s", "\
\033[1;37m              ===========  \033[1;36m  上站次数排行榜 \033[37m   ============ \n\n\
名次 代号       昵称           次数    名次 代号       昵称            次数 \n\
==== ===============================  ===== ================================\n\
");
    for (i = 0; i < rows; i++) {
        sprintf(buf1, "\033[1;3%1dm[%2d] %-10.10s %-14.14s %3d", i % 7 + 1, i + 1, allman[i].userid, allman[i].username, allman[i].numlogins);
        j = i + rows;
        sprintf(buf2, "[%2d] %-10.10s %-14.14s   %3d", j + 1, allman[j].userid, allman[j].username, allman[j].numlogins);

        printf("%-46.46s%-39.39s\033[m\n", buf1, buf2);
    }
}

top_stay(num)
{
    int i, j, rows = (num + 1) / 2;
    char buf1[80], buf2[80];

    printf("\n");
    printf("%s", "\
\033[1;37m              ===========   \033[36m 上站总时数排行榜 \033[37m   ============ \n\n\
名次 代号       昵称           总时数  名次 代号       昵称           总时数 \n\
==== ================================  ==== ================================\n\
");
    for (i = 0; i < rows; i++) {
        sprintf(buf1, "\033[1;3%1dm[%2d] %-10.10s %-14.14s%4d:%2d", i % 7 + 1, i + 1, allman[i].userid, allman[i].username, allman[i].stay / 3600, (allman[i].stay % 3600) / 60);
        j = i + rows;
        sprintf(buf2, "[%2d] %-10.10s %-14.14s%4d:%2d", j + 1, allman[j].userid, allman[j].username, allman[j].stay / 3600, (allman[j].stay % 3600) / 60);

        printf("%-46.46s%-39.39s\033[m\n", buf1, buf2);
    }
}

top_post(num)
{
    int i, j, rows = (num + 1) / 2;
    char buf1[80], buf2[80];

    printf("\n");
    printf("%s", "\
              \033[1;37m===========  \033[36m  讨论次数排行榜 \033[37m   ============ \n\n\
名次 代号       昵称           次数    名次 代号       昵称            次数 \n\
==== ===============================  ===== ================================\n\
");
    for (i = 0; i < rows; i++) {
        sprintf(buf1, "\033[1;3%1dm[%2d] %-10.10s %-14.14s %3d", i % 7 + 1, i + 1, allman[i].userid, allman[i].username, allman[i].numposts);
        j = i + rows;
        sprintf(buf2, "[%2d] %-10.10s %-14.14s   %3d", j + 1, allman[j].userid, allman[j].username, allman[j].numposts);

        printf("%-46.46s%-39.39s\033[m\n", buf1, buf2);
    }
}

top_perm(num)
{
    int i, j, rows = (num + 1) / 2;
    char buf1[80], buf2[80];

    printf("\n");
    printf("%s", "\
              \033[1;37m===========    \033[36m总表现积分排行榜\033[37m    ============ \n\
                   \033[32m 公式：上站次数/3+文章数+上站几小时\033[37m\n\
名次 代号       □称            积分   名次 代号       □称              积分 \n\
==== ===============================   ==== =================================\n\
");
    for (i = 0; i < rows; i++) {
        sprintf(buf1, "\033[1;3%1dm[%2d] %-10.10s %-14.14s %5d", i % 7 + 1, i + 1, allman[i].userid, allman[i].username, (allman[i].numlogins / 3) + allman[i].numposts + (allman[i].stay / 3600));
        j = i + rows;
        sprintf(buf2, "[%2d] %-10.10s %-14.14s   %5d", j + 1, allman[j].userid, allman[j].username, (allman[j].numlogins / 3) + allman[j].numposts + (allman[j].stay / 3600));

        printf("%-46.46s%-39.39s\033[m\n", buf1, buf2);
    }
}

int post_in_tin(char *name)
{
    char buf[256];
    FILE *fh;
    int counter = 0;

    sprintf(buf, "%s/home/%s/.tin/posted", home_path, name);
    fh = fopen(buf, "r");
    if (fh == NULL)
        return 0;
    else {
        while (fgets(buf, 255, fh) != NULL)
            counter++;
        fclose(fh);
        return counter;
    }

}

main(argc, argv)
    int argc;
    char **argv;
{
    FILE *inf;
    int i, no = 0, mode = 0;

    if (argc < 4) {
        printf("Usage: %s bbs_home num_top mode\nmode=(0All 1Logins 2Posts 3Stay)\n", argv[0]);
        exit(1);
    }

    home_path = argv[1];
    sprintf(passwd_file, "%s/.PASSWDS", home_path);

    no = atoi(argv[2]);
    mode = atoi(argv[3]);
    if (mode > 4 || mode < 1)
        mode = 0;
    if (no == 0)
        no = 20;

    inf = fopen(passwd_file, "rb");

    if (inf == NULL) {
        printf("Sorry, the data is not ready.\n");
        exit(0);
    }

    for (i = 0;; i++) {
        if (fread(&allman[i], sizeof(aman), 1, inf) <= 0)
            break;
        if (strcmp(allman[i].userid, "guest") == 0) {
            i--;
            continue;
        }
        allman[i].numposts += post_in_tin(allman[i].userid);
    }

    if (mode == 1 || mode == 0) {
        qsort(allman, i, sizeof(aman), login_cmp);
        top_login(no);
    }

    if (mode == 2 || mode == 0) {
        qsort(allman, i, sizeof(aman), post_cmp);
        top_post(no);
    }

    if (mode == 3 || mode == 0) {
        qsort(allman, i, sizeof(aman), stay_cmp);
        top_stay(no);
    }
    if (mode == 4 || mode == 0) {
        qsort(allman, i, sizeof(aman), perm_cmp);
        top_perm(no);
    }

}
