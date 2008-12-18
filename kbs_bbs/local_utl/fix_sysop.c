#include "bbs.h"

void p_help();

int main(int argc, char*argv[])
{
    struct userec uinfo;
    int unum,ret;
    struct userec *lookupuser;
    char buf[80];
    char * passwd;

    chdir(BBSHOME);
    resolve_boards();
    load_ucache();

    ret=getopt(argc,argv,"hp");
    if (ret=='h') {
        p_help();
        return 0;
    }
    strcpy(buf,"SYSOP");
    unum = getuser(buf, &lookupuser);
    uinfo = *lookupuser;
    if (ret=='p') {
        passwd = getpass("Input a password:");
        setpasswd(passwd,&uinfo);
        printf("SYSOP's password is restored.\n");
    }
    if ((ret==-1 && argc<2) || ret=='p') {
        uinfo.userlevel=0xDAFFDFFF;//»Ö¸´SYSOPÄ¬ÈÏÈ¨ÏÞ
        printf("SYSOP's PERM is restored.\n");
        update_user(&uinfo, unum, 1);
    } else if (ret==-1 && argc>1) {
        p_help();
        return 0;
    }
    return 0;
}

void p_help()
{
    printf("SYSOP's password and PERM restoration tool.\n\nusage:fix_sysop [option]\n-h Print this message.\n-p restore PERM and password.\n Default action is PERM restoration.\n");
    return;
}
