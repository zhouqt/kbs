/* user cache 处理
   用hask table 保存所有ID的id名,加快查找速度
   2001.5.4 KCN
*/


#include "bbs.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <netinet/in.h>
#ifdef FREEBSD
#include <sys/socket.h>
#endif

struct UTMPHEAD {
    int next[USHM_SIZE];
    int hashhead[UTMP_HASHSIZE + 1];    /* use UCACHE_HASHSIZE/32 */
    int number;
    int listhead;
    int list_prev[USHM_SIZE];   /* sorted list prev ptr */
    int list_next[USHM_SIZE];   /* sorted list next ptr */
    time_t uptime;
};

static int rebuild_list(struct user_info *up, char *arg, int p);
static struct UTMPHEAD *utmphead;

struct UTMPFILE *get_utmpshm_addr()
{
    return utmpshm;
}

static void longlock(int signo)
{
    bbslog("5system", "utmp lock for so long time!!!.");
    exit(-1);
}

static int utmp_lock()
{
    int utmpfd = 0;

    utmpfd = open(ULIST, O_RDWR | O_CREAT, 0600);
    if (utmpfd < 0) {
        exit(-1);
    }
    signal(SIGALRM, longlock);
    alarm(10);
    if (flock(utmpfd, LOCK_EX) == -1) {
        exit(-1);
    }
    signal(SIGALRM, SIG_IGN);
    return utmpfd;
}

static void utmp_unlock(int fd)
{
    flock(fd, LOCK_UN);
    close(fd);
}

static void utmp_setreadonly(int readonly)
{
    int iscreate;

    shmdt(utmphead);
    utmphead = (struct UTMPHEAD *) attach_shm1(NULL, 3698, sizeof(struct UTMPHEAD), &iscreate, readonly, utmphead);     /*attach user tmp head */
}

void detach_utmp()
{
    shmdt(utmphead);
    shmdt(utmpshm);
    utmphead = NULL;
    utmpshm = NULL;
}

void resolve_utmp()
{
    int iscreate;

    if (utmpshm == NULL) {
        utmpshm = (struct UTMPFILE *) attach_shm("UTMP_SHMKEY", 3699, sizeof(*utmpshm), &iscreate);     /*attach user tmp cache */
        if (iscreate) {
            int i, utmpfd;
            utmphead = (struct UTMPHEAD *) attach_shm("UTMPHEAD_SHMKEY", 3698, sizeof(struct UTMPHEAD), &iscreate);     /*attach user tmp cache */
            utmpfd = utmp_lock();
            bzero(utmpshm, sizeof(struct UTMPFILE));
            bzero(utmphead, sizeof(struct UTMPHEAD));
            utmphead->number = 0;
            utmphead->hashhead[0] = 1;
            for (i = 0; i < USHM_SIZE - 1; i++)
                utmphead->next[i] = i + 2;
            utmphead->next[USHM_SIZE - 1] = 0;
/*
        	utmphead->listhead=0;
*/
            utmp_unlock(utmpfd);
        } else
            utmphead = (struct UTMPHEAD *) attach_shm1("UTMPHEAD_SHMKEY", 3698, sizeof(struct UTMPHEAD), &iscreate, 1, NULL);   /*attach user tmp head */
    }
}

static int utmp_hash(const char *userid)
{
    int hash;

    hash = ucache_hash(userid);
    if (hash == 0)
        return 0;
    hash = (hash / 3) % UTMP_HASHSIZE;
    if (hash == 0)
        return 1;
    return hash;
}

struct requesthdr {
    int command;
    union {
        struct user_info utmp;
        int uent;
    } u_info;
} utmpreq;

/* disable KCN
int sendutmpreq(struct requesthdr *req)
{

        int m_socket;
        struct sockaddr_in sin;
        fd_set rfds;
        int result;
        struct  timeval tv;
        m_socket = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
        if (m_socket<0) return -1;
        sin.sin_family=AF_INET;
        sin.sin_port=htons(60002);
        inet_aton("127.0.0.1",&sin.sin_addr);
        if (connect(m_socket,(struct sockaddr*)&sin,sizeof(sin))!=0) {
                close(m_socket);
                return -1;
        }
        write(m_socket,req,sizeof(*req));
        FD_ZERO(&rfds);
        FD_SET(m_socket,&rfds);
        tv.tv_sec=5;
        tv.tv_usec=0;
        result = select(m_socket+1,&rfds,NULL,NULL,&tv);
        if (result)
        {
                int len=read(m_socket,&result,sizeof(result));
                close(m_socket);
                if (len!=sizeof(result)) return -1 ;
                return result ;
        } 
        close(m_socket);
        return -1;
}
*/

static void logloop()
{
    int i;
    int ptr;
    int data[USHM_SIZE];
    char buf[255];

    buf[0] = 0;
    ptr = utmphead->listhead;
    for (i = 0; i < USHM_SIZE; i++) {
        int j;
        char buf1[20];

        sprintf(buf1, "%d  ", ptr - 1);
        strcat(buf, buf1);
        data[i] = ptr - 1;
        for (j = 0; j < i; j++) {
            if (data[j] == data[i]) {
                bbslog("3system", "%s", buf);
                bbslog("3system", "find loop!");
                exit(0);
            }
        }
        if ((i - 1) % 10 == 0) {
            bbslog("3system", "%s", buf);
            buf[0] = 0;
        }
        ptr = utmphead->list_next[ptr - 1];
        if (ptr == utmphead->listhead) {
            bbslog("3system", "ok!");
            break;
        }
    }
}

#if 0
int getnewutmpent(struct user_info *up)
{
    utmpreq.command = 1;
    memcpy(&utmpreq.u_info.utmp, up, sizeof(*up));
    /* connect and send request */
    return sendutmpreq(&utmpreq);
}
#endif
int getnewutmpent(struct user_info *up)
{
    struct user_info *uentp;
    time_t now;
    int pos, n, i;
    int utmpfd, hashkey;

    utmpfd = utmp_lock();
    utmp_setreadonly(0);
    up->utmpkey=rand() % 100000000;
    pos = utmphead->hashhead[0] - 1;
    if (pos == -1) {
        utmp_setreadonly(1);
        utmp_unlock(utmpfd);
        return -1;
    }
    /* add to sorted list  */

    if (!utmphead->listhead) {  /* init the list head  */
        utmphead->list_prev[pos] = pos + 1;
        utmphead->list_next[pos] = pos + 1;
        utmphead->listhead = pos + 1;
    } else {
        int i;

        i = utmphead->listhead;
        if (strcasecmp(utmpshm->uinfo[i - 1].userid, up->userid) >= 0) {
            /* add to head */
            utmphead->list_prev[pos] = utmphead->list_prev[i - 1];
            utmphead->list_next[pos] = i;

            utmphead->list_prev[i - 1] = pos + 1;

            utmphead->list_next[utmphead->list_prev[pos] - 1] = pos + 1;

            utmphead->listhead = pos + 1;
        } else {
            int count;

            count = 0;
            i = utmphead->list_next[i - 1];
            while ((strcasecmp(utmpshm->uinfo[i - 1].userid, up->userid) < 0) && (i != utmphead->listhead)) {
                i = utmphead->list_next[i - 1];
                count++;
                if (count > USHM_SIZE) {
                    utmphead->listhead = 0;
                    bbslog("3system", "UTMP:maybe loop rebuild!");
                    apply_ulist((APPLY_UTMP_FUNC) rebuild_list, NULL);
                    utmp_setreadonly(1);
                    utmp_unlock(utmpfd);
                    exit(-1);
                }
            }

            utmphead->list_prev[pos] = utmphead->list_prev[i - 1];
            utmphead->list_next[pos] = i;

            utmphead->list_prev[i - 1] = pos + 1;

            utmphead->list_next[utmphead->list_prev[pos] - 1] = pos + 1;
        }
    }
/*    */

    utmphead->hashhead[0] = utmphead->next[pos];

    if (utmpshm->uinfo[pos].active)
        if (utmpshm->uinfo[pos].pid) {
            bbslog("3system", "utmp: alloc a active utmp! old:%s new:%s", utmpshm->uinfo[pos].userid, up->userid);
            kill(utmpshm->uinfo[pos].pid, SIGHUP);
        }
    utmpshm->uinfo[pos] = *up;
    hashkey = utmp_hash(up->userid);

    i = utmphead->hashhead[hashkey];
    /* not need sort */
    utmphead->next[pos] = i;
    utmphead->hashhead[hashkey] = pos + 1;

    utmphead->number++;
    if (get_utmp_number() + getwwwguestcount()>get_publicshm()->max_user) {
        setpublicshmreadonly(0);
        save_maxuser();
        setpublicshmreadonly(1);
    }
    now = time(NULL);
    if ((now > utmphead->uptime + 120) || (now < utmphead->uptime - 120)) {
        utmphead->uptime = now;
        newbbslog(BBSLOG_USIES, "UTMP:Clean user utmp cache");
        for (n = 0; n < USHM_SIZE; n++) {
            utmphead->uptime = now;
            uentp = &(utmpshm->uinfo[n]);
            if ((uentp->mode == WEBEXPLORE)
                && ((now - uentp->freshtime) < 360)) {
                continue;
            }
            if (uentp->active && uentp->pid && kill(uentp->pid, 0) == -1) {     /*uentp检查 */
                char buf[STRLEN];

                strncpy(buf, uentp->userid, IDLEN + 2);
                clear_utmp2(n + 1);     /* 不需要再lock了 */
                RemoveMsgCountFile(buf);
            }
        }
    }
    utmp_setreadonly(1);
    utmp_unlock(utmpfd);
    return pos + 1;
}

/* same as getnewutmpent() except no updating of utmpshm 
 * only called in www
 */
int getnewutmpent2(struct user_info *up)
{
    int pos, i;
    int utmpfd, hashkey;

    utmpfd = utmp_lock();
    utmp_setreadonly(0);
    up->utmpkey = rand() % 100000000;
    pos = utmphead->hashhead[0] - 1;
    if (pos == -1) {
        utmp_setreadonly(1);
        utmp_unlock(utmpfd);
        return -1;
    }
    /* add to sorted list  */

    if (!utmphead->listhead) {  /* init the list head  */
        utmphead->list_prev[pos] = pos + 1;
        utmphead->list_next[pos] = pos + 1;
        utmphead->listhead = pos + 1;
    } else {
        int i;

        i = utmphead->listhead;
        if (strcasecmp(utmpshm->uinfo[i - 1].userid, up->userid) >= 0) {
            /* add to head */
            utmphead->list_prev[pos] = utmphead->list_prev[i - 1];
            utmphead->list_next[pos] = i;

            utmphead->list_prev[i - 1] = pos + 1;

            utmphead->list_next[utmphead->list_prev[pos] - 1] = pos + 1;

            utmphead->listhead = pos + 1;
        } else {
            int count;

            count = 0;
            i = utmphead->list_next[i - 1];
            while ((strcasecmp(utmpshm->uinfo[i - 1].userid, up->userid) < 0) && (i != utmphead->listhead)) {
                i = utmphead->list_next[i - 1];
                count++;
                if (count > USHM_SIZE) {
                    utmphead->listhead = 0;
                    bbslog("3system", "UTMP:maybe loop rebuild..!");
                    apply_ulist((APPLY_UTMP_FUNC) rebuild_list, NULL);
                    utmp_setreadonly(1);
                    utmp_unlock(utmpfd);
                    return -1;
                }
            }

            utmphead->list_prev[pos] = utmphead->list_prev[i - 1];
            utmphead->list_next[pos] = i;

            utmphead->list_prev[i - 1] = pos + 1;

            utmphead->list_next[utmphead->list_prev[pos] - 1] = pos + 1;
        }
    }

    utmphead->hashhead[0] = utmphead->next[pos];

    if (utmpshm->uinfo[pos].active)
        if (utmpshm->uinfo[pos].pid) {
            bbslog("3system", "utmp: alloc a active utmp! old:%s new:%s", utmpshm->uinfo[pos].userid, up->userid);
            kill(utmpshm->uinfo[pos].pid, SIGHUP);
        }
    utmpshm->uinfo[pos] = *up;
    hashkey = utmp_hash(up->userid);

    i = utmphead->hashhead[hashkey];
    /* not need sort */
    utmphead->next[pos] = i;
    utmphead->hashhead[hashkey] = pos + 1;

    utmphead->number++;
    utmp_setreadonly(1);
    utmp_unlock(utmpfd);
    return pos + 1;
}

static int rebuild_list(struct user_info *up, char *arg, int p)
{
    int pos = p - 1;

    /* add to sorted list  */
    if ((up->userid[0] == 0) || (!up->active))
        return COUNT;

    if (!utmphead->listhead) {  /* init the list head  */
        utmphead->list_prev[pos] = pos + 1;
        utmphead->list_next[pos] = pos + 1;
        utmphead->listhead = pos + 1;
    } else {
        int i;

        i = utmphead->listhead;
        if (strcasecmp(utmpshm->uinfo[i - 1].userid, up->userid) >= 0) {
            /* add to head */
            utmphead->list_prev[pos] = utmphead->list_prev[i - 1];
            utmphead->list_next[pos] = i;

            utmphead->list_prev[i - 1] = pos + 1;

            utmphead->list_next[utmphead->list_prev[pos] - 1] = pos + 1;

            utmphead->listhead = pos + 1;
        } else {
            int count;

            count = 0;
            i = utmphead->list_next[i - 1];
            while ((strcasecmp(utmpshm->uinfo[i - 1].userid, up->userid) < 0) && (i != utmphead->listhead)) {
                i = utmphead->list_next[i - 1];
                count++;
                if (count > USHM_SIZE) {
                    bbslog("3system", "UTMP:rebuild maybe loop???");
                    f_cat("NOLOGIN", "系统故障，请稍后再来");
                    logloop();
                    exit(-1);
                }
            }

            utmphead->list_prev[pos] = utmphead->list_prev[i - 1];
            utmphead->list_next[pos] = i;

            utmphead->list_prev[i - 1] = pos + 1;

            utmphead->list_next[utmphead->list_prev[pos] - 1] = pos + 1;
        }
    }
    return COUNT;
}

int apply_ulist(APPLY_UTMP_FUNC fptr, void *arg)
{                               /* apply func on user list */
    struct user_info *uentp, utmp;
    int i, max;

    max = USHM_SIZE - 1;
    while (max > 0 && utmpshm->uinfo[max].active == 0)  /*跳过后段 非active的user */
        max--;
    for (i = 0; i <= max; i++) {
        uentp = &(utmpshm->uinfo[i]);
        utmp = *uentp;
        if ((*fptr) (&utmp, arg, i + 1) == QUIT)
            return QUIT;
    }
    return 0;
}

int apply_ulist_addr(APPLY_UTMP_FUNC fptr, void *arg)
{                               /* apply func on user list */
    int i;
    int num;

    i = utmphead->listhead;
    if (!i)
        return 0;
    num = 0;
    if (utmpshm->uinfo[i - 1].active) {
        if (fptr) {
            int ret;

            ret = (*fptr) (&utmpshm->uinfo[i - 1], arg, num);
            if (ret == QUIT)
                return num;
            if (ret == COUNT)
                num++;
        } else
            num++;
    }
    i = utmphead->list_next[i - 1];
    while (i != utmphead->listhead) {
        if (utmpshm->uinfo[i - 1].active) {
            if (fptr) {
                int ret;

                ret = (*fptr) (&utmpshm->uinfo[i - 1], arg, num);
                if (ret == QUIT)
                    return num;
                if (ret == COUNT)
                    num++;
            } else
                num++;
        }
        i = utmphead->list_next[i - 1];
        if (num >= USHM_SIZE) {
            utmphead->listhead = 0;
            bbslog("3system", "UTMP:maybe loop rebuild!!");
            apply_ulist((APPLY_UTMP_FUNC) rebuild_list, NULL);

            exit(0);
        };
    }
    return 0;
}

int apply_utmpuid(APPLY_UTMP_FUNC fptr, int uid, void *arg)
{
    int num;
    char userid[IDLEN + 1];

    num = 0;

    if (uid <= 0)
        return 0;
    strcpy(userid, getuserid2(uid));
    return apply_utmp(fptr, 0, userid, arg);
}

int apply_utmp(APPLY_UTMP_FUNC fptr, int maxcount, char *userid, void *arg)
{
    int i, num;
    int hashkey;

    num = 0;
    hashkey = utmp_hash(userid);
    i = utmphead->hashhead[hashkey];
    while (i) {
        if ((utmpshm->uinfo[i - 1].active) && (!strcasecmp(utmpshm->uinfo[i - 1].userid, userid))) {
            int ret;

            if (fptr) {
                ret = (*fptr) (&utmpshm->uinfo[i - 1], arg, i);
                if (ret == QUIT)
                    break;
                if (ret == COUNT)
                    num++;
                if (maxcount && (num > maxcount))
                    break;
            } else
                num++;
        }
        i = utmphead->next[i - 1];
    }
    return num;
}

int search_ulist(struct user_info *uentp, int (*fptr) (int, struct user_info *), int farg)
 /* ulist 中 search 符合fptr函数的 user */
{
    int i;

    for (i = 0; i < USHM_SIZE; i++) {
        *uentp = utmpshm->uinfo[i];
        if ((*fptr) (farg, uentp))
            return i + 1;
    }
    return 0;
}

#if 0
void clear_utmp(int uent)
{
    utmpreq.command = 3;
    utmpreq.u_info.uent = uent;
    /* connect and clear */
    sendutmpreq(&utmpreq);
}
#endif

void clear_utmp2(int uent)
{
    int hashkey, find;
    struct user_info zeroinfo;
    char buf[MAXPATH];
    struct userec* user;

#ifdef BBSMAIN
    if (!uent) {
        if (!CHECK_UENT(uinfo.uid))
            return;
        uent = utmpent;
    }
#endif
    user=getuserbynum(utmpshm->uinfo[uent-1].uid);
    do_after_logout(user,get_utmpent(uent),uent,0);
    hashkey = utmp_hash(utmpshm->uinfo[uent - 1].userid);
    find = utmphead->hashhead[hashkey];

    if (find == uent)
        utmphead->hashhead[hashkey] = utmphead->next[uent - 1];
    else {
        while (utmphead->next[find - 1] && utmphead->next[find - 1] != uent)
            find = utmphead->next[find - 1];
        if (!utmphead->next[find - 1])
            bbslog("3system", "UTMP:Can't find %s [%d]", utmpshm->uinfo[uent - 1].userid, uent);
        else
            utmphead->next[find - 1] = utmphead->next[uent - 1];
    }

    /* remove from sorted list */
    if (utmphead->listhead == uent) {
        utmphead->listhead = utmphead->list_next[uent - 1];
        if (utmphead->listhead == uent)
            utmphead->listhead = 0;
    }

    utmphead->list_next[utmphead->list_prev[uent - 1] - 1] = utmphead->list_next[uent - 1];
    utmphead->list_prev[utmphead->list_next[uent - 1] - 1] = utmphead->list_prev[uent - 1];
/*	*/

    newbbslog(BBSLOG_USIES,"UTMP:clean %s(%d)", utmpshm->uinfo[uent - 1].userid, uent);
    utmphead->next[uent - 1] = utmphead->hashhead[0];
    utmphead->hashhead[0] = uent;
    /* Delete the user's msglist entry from webmsgd,
     * if the user is login from web. */
    if (utmpshm->uinfo[uent - 1].mode == WEBEXPLORE)
        delfrom_msglist(uent, utmpshm->uinfo[uent - 1].userid);
    zeroinfo.active = false;
    zeroinfo.pid = 0;
    zeroinfo.invisible = true;
    zeroinfo.sockactive = false;
    zeroinfo.sockaddr = 0;
    zeroinfo.destuid = 0;

    if (utmpshm->uinfo[uent - 1].active != false)
        utmphead->number--;
    utmpshm->uinfo[uent - 1] = zeroinfo;
}

void clear_utmp(int uent, int useridx, int pid)
{
    int lockfd;

    lockfd = utmp_lock();
    utmp_setreadonly(0);

    if (((useridx == 0) || (utmpshm->uinfo[uent - 1].uid == useridx)) && pid == utmpshm->uinfo[uent - 1].pid)
        clear_utmp2(uent);

    utmp_setreadonly(1);
    utmp_unlock(lockfd);
}

int get_utmp_number()
{
    return utmphead->number;
}

struct user_info *get_utmpent(int utmpnum)
{
    if (utmpnum <= 0)
        return NULL;
    return utmpshm->uinfo + (utmpnum - 1);
}

int get_utmpent_num(struct user_info *uent)
{
    if (uent == NULL)
        return -1;
    return uent - utmpshm->uinfo + 1;
}

static int cmpfuid(struct friends *a, struct friends *b)
{
    return strncasecmp(a->id, b->id, IDLEN);
}


struct friends_info* topfriend;
int getfriendstr(struct userec* user,struct user_info* puinfo)
{
    int nf;
    int i;
    struct friends *friendsdata;
    char buf[60];

    puinfo->friendsnum=0;
    if (topfriend != NULL) {
        free(topfriend);
        topfriend = NULL;
    }
    sethomefile(buf, user->userid, "friends");
    nf = get_num_records(buf, sizeof(struct friends));
    if (nf <= 0)
        return 0;
    if (nf>MAXFRIENDS)
    	nf=MAXFRIENDS;
    friendsdata = (struct friends *) calloc(nf,sizeof(struct friends));
    get_records(buf, friendsdata, sizeof(struct friends), 1, nf);
    for (i = 0; i < nf; i++) {
        friendsdata[i].id[IDLEN] = 0;
        friendsdata[i].exp[LEN_FRIEND_EXP-1] = 0;
        if (id_invalid(friendsdata[i].id)||(searchuser(friendsdata[i].id)==0)) {
            memcpy(&friendsdata[i], &friendsdata[nf - 1], sizeof(struct friends));
            nf--;
        }
    }
    qsort(friendsdata, nf, sizeof(friendsdata[0]), (int (*)(const void *, const void *)) cmpfuid);      /*For Bi_Search */
    topfriend = (struct friends_info *) calloc(nf,sizeof(struct friends_info));
    for (i = 0; i < nf; i++) {
        puinfo->friends_uid[i] = searchuser(friendsdata[i].id);
        strcpy(topfriend[i].exp, friendsdata[i].exp);
    }
    free(friendsdata);
    puinfo->friendsnum=nf;
    return 0;
}

int utmpent=-1;
int myfriend(int uid, char *fexp)
{
    int i, found = false;
    struct user_info* u;

    u = get_utmpent(utmpent);
    /*
     * char buf[IDLEN+3]; 
     */
    if (u == NULL || u->friendsnum<= 0) {
        return false;
    }
    for (i = 0; i < u->friendsnum; i++) {
        if (u->friends_uid[i] == uid) {
            found = true;
            break;
        }
    }
    if ((found) && fexp)
        strcpy(fexp, topfriend[i].exp);
    return found;
}

bool hisfriend(int uid,struct user_info* him)
{
    int i, found = false;
    if (him->friendsnum<= 0) {
        return false;
    }
    for (i = 0; i < him->friendsnum; i++) {
        if (him->friends_uid[i] == uid) {
            found = true;
            break;
        }
    }
    return found;
}
