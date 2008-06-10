#include "bbs.h"

int main(int argc, char **argv)
{
    struct userec *user;
    unsigned char accessed[2];
    char path[PATHLEN];
    int ret;
    if (argc != 5) {
        printf("usage: %s FILENAME USERID TITLE BOARD\n", argv[0]);
        return 0;
    }
    if (!getcwd(path, PATHLEN))
        return -1;
    chdir(BBSHOME);
    resolve_boards();
    resolve_ucache();
    if (!getuser(argv[2], &user)) {
        printf("user %s not found!\n", argv[2]);
        return -2;
    }
    accessed[0] = 0; accessed[1] = 0;
    if (argv[1][0] != '/') {
        strcat(path, "/");
        strcat(path, argv[1]);
    } else {
        memcpy(path, argv[1], PATHLEN);
    }
    if (access(path, R_OK)) {
        printf("file %s not found!\n", argv[1]);
        return -3;
    }
    ret = post_file_alt(path, user, argv[3], argv[4], NULL, 0x04, accessed);
    printf("done, return value = %d\n", ret);
    return 0;
}
