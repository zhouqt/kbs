#include "bbs.h"

int generate_board(struct boardheader *bh,void* arg)
{
    char buf[256];

    if (bh==NULL || bh->filename[0]=='\0')
        return 0;

    if (check_see_perm(NULL, bh)==0)
        return 0;

    sprintf(buf,"%s/boards/%s.all",BONLINE_LOGDIR, bh->filename);

    if (!dashf(buf))
        return 0;

    sprintf(buf,"sed -e \"s/BOARD/%s/g\" boards.plt > boardstmp.plt", bh->filename);
    system(buf);
    system("gnuplot boardstmp.plt");

    return 0;
}

int generate_all()
{
    apply_boards(generate_board,NULL);
    return 0;
}

int main(int argc,char** argv)
{
    struct boardheader bh;

    chdir(BBSHOME);
    resolve_boards();

    if (argc < 2) {
        printf("./genbplot [-a|boardname]\n");
        exit(0);
    }
    if (argv[1][0]!='-') {
        bzero(&bh, sizeof(bh));
        strncpy(bh.filename,argv[1],STRLEN);
        generate_board(&bh,NULL);
    } else {
        if (!strcmp(argv[1],"-a")) {
            generate_all();
        } else {
            printf("./genbplot [-a|boardname]\n");
        }
    }

    return 0;
}
