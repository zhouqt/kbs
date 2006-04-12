#include "innbbsconf.h"
#include "bbslib.h"
#include "inn_funcs.h"
#include "his.h"

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s history-file\n", argv[0]);
        exit(1);
    }
    initial_bbs(NULL);
    mkhistory(argv[1]);
    return 0;
}
