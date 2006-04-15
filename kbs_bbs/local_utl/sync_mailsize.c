/********
atppp: 同步邮件字节数
新发的邮件都会计算了，这个程序主要用于计算以前的邮件的字节数
并会改写相应.DIR里的effsize
*/


#include "bbs.h"
#define FORCE_SYNC /* 强制重新扫描信件并且重设总使用空间 */

/* return total size */
int calc_mailsize(char *userid, char *dirname)
{
    char fn[256];
    int fd, total, i;
    struct stat buf;
    struct flock ldata;
    struct fileheader * ptr1;
    char * ptr;
    int totalsize = 0;
    int size=sizeof(struct fileheader);

    setmailfile(fn, userid, dirname);

    if ((fd = open(fn, O_RDWR, 0664)) == -1) {
        /* bbslog("user", "%s", "recopen err"); */
        return 0;      /* 创建文件发生错误*/
    }
    fstat(fd, &buf);
    total = buf.st_size / size;
    if (total == 0) {
        close(fd);
        return 0;
    }
    ldata.l_type = F_RDLCK;
    ldata.l_whence = 0;
    ldata.l_len = 0;
    ldata.l_start = 0;
    fcntl(fd, F_SETLKW, &ldata);

    if((i=safe_mmapfile_handle(fd,PROT_READ|PROT_WRITE,MAP_SHARED, &ptr, &buf.st_size))!=1){
        if (i == 2)
            end_mmapfile((void *) ptr, buf.st_size, -1);
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        return 0;
    }
    ptr1 = (struct fileheader *) ptr;
    for (i = 0; i < total; i++) {
        struct stat st;
        char ffn[256];
#ifndef FORCE_SYNC
        if (ptr1->eff_size > 0) {
            totalsize += ptr1->eff_size;
            ptr1++;
            continue;
        }
#endif
        setmailfile(ffn, userid, ptr1->filename);
        if (lstat(ffn, &st) == -1 || !S_ISREG(st.st_mode)) ptr1->eff_size = 0;
        else {
            ptr1->eff_size = st.st_size;
            totalsize += ptr1->eff_size;
        }

        ptr1++;
    }
    end_mmapfile((void *) ptr, buf.st_size, -1);
    ldata.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &ldata);
    close(fd);
    return totalsize;
}

int sync_mailsize(struct userec *user, void *arg)
{
    char buf[40];
    int i;
    int totalsize = 0;
    struct _mail_list mail;

    if (!user->userid[0]) return 0;
    totalsize += calc_mailsize(user->userid, DOT_DIR);
    totalsize += calc_mailsize(user->userid, ".SENT");
    totalsize += calc_mailsize(user->userid, ".DELETED");
    load_mail_list(user, &mail);
    for (i = 0; i < mail.mail_list_t; i++) {
        sprintf(buf, ".%s", mail.mail_list[i] + 30);
        totalsize += calc_mailsize(user->userid, buf);
    }
    printf("%s ok.\n", user->userid);
    if (totalsize != user->usedspace) {
        fprintf(stderr, "%s new:%d old:%d\n", user->userid, totalsize, user->usedspace);
#ifdef FORCE_SYNC
        user->usedspace = totalsize;
#endif
    }
    return 0;
}

static void 
usage()
{
    fprintf(stderr, "Usage: sync_mailsize <-a|-u userid>\n\n");
    fprintf(stderr, "    If -a parameter is provided, this program will reset all userids' mail sizes,\n");
    fprintf(stderr, "    else only reset the specified userid's mail size.\n");
}

int 
main(int argc, char ** argv)
{
    struct userec *user = NULL;

    chdir(BBSHOME);
    resolve_ucache();
    if (argc == 2 && !strcmp(argv[1], "-a"))
        apply_users(sync_mailsize, NULL);
    else if (argc == 3 && !strcmp(argv[1], "-u"))
    {
        getuser(argv[2], &user);
        if (user == NULL)
        {
            fprintf(stderr, "User %s not found.\n", argv[2]);
            return -1;
        }
        sync_mailsize(user, NULL);
    }
    else
        usage();

    return 0;
}

