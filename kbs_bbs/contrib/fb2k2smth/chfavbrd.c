/*
 * This program converts FavBoards Data from Firebird 2000 to smth_bbs
 */

#include "bbs.h"

struct favbrd_struct favbrd_list[FAVBOARDNUM];
int favbrd_list_t;

void my_save_favboard(char *userid)
{
    int fd, i, j;
    char fname[MAXPATH];

    sethomefile(fname, userid, "favboard");
    if ((fd = open(fname, O_WRONLY | O_CREAT, 0600)) != -1) {
        i = 0x8081;
        write(fd, &i, sizeof(int));
        write(fd, &favbrd_list_t, sizeof(int));
        for (i = 0; i < favbrd_list_t; i++) {
            write(fd, &favbrd_list[i], sizeof(struct favbrd_struct));
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
	int i=0;

	bzero(favbrd_list, sizeof(struct favbrd_struct)*FAVBOARDNUM);

    favbrd_list_t = 1;
    favbrd_list[0].father = -1;

    sethomefile(goodbrdfile, userid, ".goodbrd");
    if ((fp = fopen(goodbrdfile, "r")) == NULL) {
        return;
    }

    while (!feof(fp)) {
        int k = 0;

        if (i == MAXBOARDPERDIR)
            break;
	bzero(buf,256);
        fgets(buf, 30, fp);
        if (tmp = strstr(buf, "\n"))
            *tmp = 0;
	if(*buf==0) break;
	puts(buf);
        k = getbnum(buf);
        if (k) {
            favbrd_list[0].bid[i] = k - 1;
            i++;
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
