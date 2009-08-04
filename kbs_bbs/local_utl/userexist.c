
#include "bbs.h"

int main(int argc, char* argv[])
{

    /*    int mode;
        bool dodel; */
    struct userec *user;

    chdir(BBSHOME);
    load_ucache();

    if ((getuser(argv[1], &user)))
        return 1;
    else
        return 19;
}


