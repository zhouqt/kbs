/*
 * This program converts FavBoards Data from Firebird 2000 to smth_bbs
 */

#include "bbs.h"

struct favbrd_struct {
    int flag;
    char *title;
    int father;
};

struct favbrd_struct favbrd_list[FAVBOARDNUM];
int favbrd_list_t;

void my_save_favboard(char *userid)
{
    int fd, i, j;
    char fname[MAXPATH];

    sethomefile(fname, userid, "favboard");
    if ((fd = open(fname, O_WRONLY | O_CREAT, 0600)) != -1) {
        i = 0x8080;
        write(fd, &i, sizeof(int));
        write(fd, &favbrd_list_t, sizeof(int));
        for (i = 0; i < favbrd_list_t; i++) {
            j = favbrd_list[i].flag;
            write(fd, &j, sizeof(int));
            if (j == -1) {
                char len = strlen(favbrd_list[i].title) + 1;
                write(fd, &len, sizeof(char));
                write(fd, favbrd_list[i].title, len);
            }
            j = favbrd_list[i].father;
            write(fd, &j, sizeof(int));
        }
        close(fd);
    }
}
void create_favboard(char *userid)
{
    FILE *fp;
    char goodbrdfile[256];
    char buf[256];
    char *tmp;

    favbrd_list_t = 0;

    sethomefile(goodbrdfile, userid, ".goodbrd");
    if ((fp = fopen(goodbrdfile, "r")) == NULL) {
        return;
    }

    while (!feof(fp)) {
        int k = 0;

        if (favbrd_list_t == FAVBOARDNUM)
            break;
	bzero(buf,256);
        fgets(buf, 30, fp);
        if (tmp = strstr(buf, "\n"))
            *tmp = 0;
	if(*buf==0) break;
	puts(buf);
        k = getbnum(buf);
        if (k) {
            favbrd_list[favbrd_list_t].flag = k - 1;
            favbrd_list[favbrd_list_t].father = -1;
            favbrd_list_t++;
        }
    }
    fclose(fp);
    unlink(goodbrdfile);
    if (favbrd_list_t)
        my_save_favboard(userid);
}

int main()
{
    struct userec *users = NULL;
    struct userec *ptr = NULL;
    struct stat fs;
    int fd, i, records;

    chdir(BBSHOME);
    resolve_boards();

    if ((fd = open(PASSFILE, O_RDONLY, 0644)) == -1) {
        perror("open passwd");
        return -1;
    }
    fstat(fd, &fs);
    records = fs.st_size / sizeof(struct userec);
    users = (struct userec *) mmap(NULL, fs.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (users == MAP_FAILED) {
        perror("mmap");
        return -1;
    }
    for (i = 0; i < records; i++) {
        ptr = users + i;
        if (ptr->userid[0] == '\0')
            continue;
        if (!strcmp(ptr->userid, "New"))
            continue;
        create_favboard(ptr->userid);
    }
    munmap(users, fs.st_size);

    return 0;
}
