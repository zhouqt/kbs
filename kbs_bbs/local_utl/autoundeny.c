/*    自动解封系统    KCN 1999.7.26 		  */

#include "bbs.h"
struct userec deliveruser;

unsigned long atoul(char *p)
{
    unsigned long s;
    char *t;

    t = p;
    s = 0;
    while ((*t >= '0') && (*t <= '9')) {
        s = s * 10 + *t - '0';
        t++;
    }
    return s;
}

int canundeny(char *linebuf, unsigned long nowtime)
{
    char *p;
    unsigned long time2;

    p = linebuf;
    while ((*p != 0) && (*p != 0x1b))
        p++;
    if (*p == 0)
        return 0;
    if (!strncmp(p - 2, "后", 2))
        return 0;
    p++;
    if (*p == 0)
        return 0;
    p++;
    if (*p == 0)
        return 0;
    time2 = atoul(p);
    return nowtime > time2;
}

int sgetline(char *buf, char *linebuf, int *idx, int maxlen)
{
    int len = 0;

    while (len < maxlen) {
        char ch;

        linebuf[len] = buf[*idx];
        ch = buf[*idx];
        (*idx)++;
        if (ch == 0x0d) {
            linebuf[len] = 0;
            if (buf[*idx] == 0x0a)
                (*idx)++;
            break;
        }
        if (ch == 0x0a) {
            linebuf[len] = 0;
            break;
        }
        if (ch == 0)
            break;
        len++;
    }
    return len;
}

int undenyboard(struct boardheader *bh,void* arg)
{
    int d_fd;
    char denyfile[256];
    static char *buf;
    static int bufsize;
    struct stat st;
    time_t nowtime;
    int idx1, idx2;
    char linebuf[256];

    nowtime = time(NULL);
    if (bh->filename[0]) {
        sprintf(denyfile, "boards/%s/deny_users", bh->filename);
        if (stat(denyfile, &st) == 0) {
            if (st.st_size != 0) {
                if (bufsize < st.st_size + 1) {
                    if (buf)
                        free(buf);
                    buf = malloc(st.st_size + 1);
                    buf[st.st_size] = 0;
                }
                if ((d_fd = open(denyfile, O_RDWR)) != -1) {
                    flock(d_fd, LOCK_EX);
                    if (read(d_fd, buf, st.st_size) == st.st_size) {
                        idx1 = 0;
                        idx2 = 0;
                        while (idx2 < st.st_size) {
                            int len = sgetline(buf, linebuf, &idx2, 255);

                            if (!canundeny(linebuf, nowtime)) {
                                if (idx1 != 0) {
                                    buf[idx1] = 0x0a;
                                    idx1++;
                                }
                                memcpy(buf + idx1, linebuf, len);
                                idx1 += len;
                            } else {
                                char uid[IDLEN + 1], *p;

                                memcpy(uid, linebuf, IDLEN);
                                uid[IDLEN] = 0;
                                for (p = uid; *p; p++)
                                    if (*p == ' ') {
                                        *p = 0;
                                        break;
                                    }
                                //printf("%s %s\n", bh->filename, uid);
                                deldeny(&deliveruser, bh->filename, uid, 1, getSession());
                            }
                        }
                        buf[idx1] = 0x0a;
                        idx1++;
                        lseek(d_fd, 0, SEEK_SET);
                        write(d_fd, buf, idx1);
                        ftruncate(d_fd, idx1);
                    }
                    flock(d_fd, LOCK_UN);
                    close(d_fd);
                }
            }
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (init_all()) {
        printf("init data fail\n");
        return -1;
    }

    bzero(&deliveruser, sizeof(struct userec));
    strcpy(deliveruser.userid, "deliver");
    deliveruser.userlevel = -1;
    strcpy(deliveruser.username, "自动发信系统");
    setCurrentUser(&deliveruser);
    strcpy(getSession()->fromhost, "天堂");
    apply_boards(undenyboard,NULL);
    return 0;
}
