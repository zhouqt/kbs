/* user cache 处理
   用hask table 保存所有ID的id名,加快查找速度
   2001.5.4 KCN
*/


#include "bbs.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include <signal.h>

#include "uhashgen.h"

struct UCACHE {
    ucache_hashtable hashtable;
    ucache_hashtable hashusage;
    int hashhead[UCACHE_HASHSIZE + 1];
    int next[MAXUSERS];
    time_t uptime;
    int number;
#ifdef HAVE_CUSTOM_USER_TITLE
    char user_title[255][USER_TITLE_LEN]; //定义用户的称号字符串。
#endif
    struct userec passwd[MAXUSERS];
};

static struct UCACHE *uidshm = NULL;

static int ucache_lock()
{
    int lockfd;

    lockfd = open(ULIST, O_RDWR | O_CREAT, 0600);
    if (lockfd < 0) {
        bbslog("3system", "CACHE:lock ucache:%s", strerror(errno));
        return -1;
    }
    flock(lockfd, LOCK_EX);
    return lockfd;
}

static void ucache_unlock(int fd)
{
    flock(fd, LOCK_UN);
    close(fd);
}

/* do init hashtable , read from uhashgen.dat -- wwj*/
static void ucache_hashinit()
{
    FILE *fp;
    char line[256];
    int i, j, ptr, data;


    fp = fopen("uhashgen.dat", "rt");
    if (!fp) {
        bbslog("3system", "UCACHE:load uhashgen.dat fail");
        exit(0);
    }
    i = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#')
            continue;
        j = 0;
        ptr = 0;
        while ((line[ptr] >= '0' && line[ptr] <= '9') || line[ptr] == '-') {
            data = ptr;
            while ((line[ptr] >= '0' && line[ptr] <= '9') || line[ptr] == '-')
                ptr++;
            line[ptr++] = 0;
            if (i == 0) {
                if (j >= 26) {
                    bbslog("3system", "UCACHE:hash0>26");
                    exit(0);
                }
                uidshm->hashtable.hash0[j++] = atoi(&line[data]);
            } else {
                if (j >= 36) {
                    bbslog("3system", "UCACHE:hash0>26");
                    exit(0);
                }
                uidshm->hashtable.hash[i - 1][j++] = atoi(&line[data]);
            }
        }
        i++;
        if (i > (int) sizeof(uidshm->hashtable.hash) / 36) {
            bbslog("3system", "hashline %d exceed", i);
            exit(0);
        }
    }
    fclose(fp);

    newbbslog(BBSLOG_USIES,"HASH load");
}

/*
   CaseInsensitive ucache_hash, assume
   isalpha(userid[0])
   isahpha(userid[n]) || isnumber(userid[n]) n>0
   01/5/5 wwj
 */
unsigned int ucache_hash_deep(const char *userid)
{
    int n1, n2, n;
    ucache_hashtable *hash;
    ucache_hashtable *usage;

    if (!*userid)
        return 0;
    hash = &uidshm->hashtable;
    usage = &uidshm->hashusage;

    n1 = *userid++;
    if (n1 >= 'a' && n1 <= 'z')
        n1 &= 0xdf;
    n1 -= 'A';
    if (n1 < 0 || n1 >= 26)
        return 0;

    n1 = hash->hash0[n1];

    n = 1;
    while (n1 < 0) {
        n1 = -n1 - 1;
        if (!*userid) {
/* disable it            usage->hash[n1][0]++; */
            n1 = hash->hash[n1][0];
        } else {
            n2 = *userid++;
            if (n2 >= 'a' && n2 <= 'z') {
                n2 -= 'a' - 10;
            } else if (n2 >= 'A' && n2 <= 'Z') {
                n2 -= 'A' - 10;
            } else {
                n2 -= '0';
            }
            if (n2 < 0 || n2 >= 36)
                return 0;
            n1 = hash->hash[n1][n2];
        }
        n++;
    }
    return n;
}


/*
   CaseInsensitive ucache_hash, assume 
   isalpha(userid[0]) 
   isahpha(userid[n]) || isnumber(userid[n]) n>0
   01/5/5 wwj
 */
unsigned int ucache_hash(const char *userid)
{
    int n1, n2, n, len;
    ucache_hashtable *hash;
    ucache_hashtable *usage;

    if (!*userid)
        return 0;
    hash = &uidshm->hashtable;
    usage = &uidshm->hashusage;

    n1 = *userid++;
    if (n1 >= 'a' && n1 <= 'z')
        n1 &= 0xdf;
    n1 -= 'A';
    if (n1 < 0 || n1 >= 26)
        return 0;

/* disable it    usage->hash0[n1]++;*/
    n1 = hash->hash0[n1];

    while (n1 < 0) {
        n1 = -n1 - 1;
        if (!*userid) {
/* disable it            usage->hash[n1][0]++; */
            n1 = hash->hash[n1][0];
        } else {
            n2 = *userid++;
            if (n2 >= 'a' && n2 <= 'z') {
                n2 -= 'a' - 10;
            } else if (n2 >= 'A' && n2 <= 'Z') {
                n2 -= 'A' - 10;
            } else {
                n2 -= '0';
            }
            if (n2 < 0 || n2 >= 36)
                return 0;
/* disable it            usage->hash[n1][n2]++; */
            n1 = hash->hash[n1][n2];
        }
    }
    n1 = (n1 * UCACHE_HASHBSIZE) % UCACHE_HASHSIZE + 1;
    if (!*userid)
        return n1;

    n2 = 0;
    len = strlen(userid);
    while (*userid) {
        n = *userid++;
        if (n >= 'a' && n <= 'z') {
            n -= 32;
        }
        n2 += (n - 47) * len;
        len--;
    }
    n1 = (n1 + n2 % UCACHE_HASHBSIZE) % UCACHE_HASHSIZE + 1;
    return n1;
}

static int fillucache(struct userec *uentp, int *number, int *prev)
{
    if (*number < MAXUSERS) {
        int hashkey;

        if (id_invalid(uentp->userid))
            hashkey = 0;
        else
            hashkey = ucache_hash(uentp->userid);
        if (hashkey < 0 || hashkey > UCACHE_HASHSIZE) {
            bbslog("3system", "UCACHE:hash(%s) %d error", uentp->userid, hashkey);
            exit(0);
        }
      addempty:
        if (hashkey == 0) {     /* empty user add in recurise sort 
                                   int i=uidshm->hashhead[0];
                                   uidshm->next[*number] = 0;
                                   if (i==0) uidshm->hashhead[0]=++(*number);
                                   else {
                                   int prev;
                                   while (i) {
                                   prev=i;
                                   i=uidshm->next[i-1];
                                   };
                                   uidshm->next[prev-1]=++(*number);
                                   } */
            uidshm->next[*number] = 0;
            (*number)++;
            if (!(*prev)) {
                uidshm->hashhead[0] = *number;
            } else {
                uidshm->next[(*prev) - 1] = *number;
            }
            *prev = *number;
        } else {
/* check multi-entry of user */
            int i, prev;

            i = uidshm->hashhead[hashkey];
            prev = -1;
            while (i != 0) {
                struct userec *uentp;

                uentp = &uidshm->passwd[i - 1];
                if (!strcasecmp(uidshm->passwd[*number].userid, uentp->userid)) {
                    if (uidshm->passwd[*number].numlogins > uentp->numlogins) {
                        bbslog("3passwd", "deleted %s in %d", uentp->userid, i - 1);
                        if (prev == -1)
                            uidshm->hashhead[hashkey] = uidshm->next[i - 1];
                        else
                            uidshm->next[prev - 1] = uidshm->next[i - 1];
                        uentp->userid[0] = 0;
                        uidshm->next[i - 1] = 0;
                        uidshm->hashhead[0] = i;
                    } else {
                        bbslog("3passwd", "deleted %s in %d", uidshm->passwd[*number].userid, *number);
                        uidshm->passwd[*number].userid[0] = 0;
                        hashkey = 0;
                        goto addempty;
                    }
                }
                prev = i;
                i = uidshm->next[i - 1];
            }
            uidshm->next[*number] = uidshm->hashhead[hashkey];
            uidshm->hashhead[hashkey] = ++(*number);
        }
    }
    return 0;
}

#ifdef HAVE_CUSTOM_USER_TITLE
void flush_user_title();
void load_user_title();
#endif

int flush_ucache()
{
    int ret;
    ret= substitute_record(PASSFILE, uidshm->passwd, MAXUSERS * sizeof(struct userec), 1);
#ifdef HAVE_CUSTOM_USER_TITLE
    flush_user_title();
#endif
    return ret;
}

int load_ucache()
{
    int iscreate;
    int usernumber, i;
    int passwdfd;
    int prev;

    int fd;

    fd=ucache_lock();
    uidshm = (struct UCACHE *) attach_shm("UCACHE_SHMKEY", 3696, sizeof(*uidshm), &iscreate);   /*attach to user shm */

    if (!iscreate) {
        bbslog("4system", "load a exitist ucache shm!");
        ucache_unlock(fd);
        return 0;
    }

#ifdef HAVE_CUSTOM_USER_TITLE
    load_user_title();
#endif
    if ((passwdfd = open(PASSFILE, O_RDWR | O_CREAT, 0644)) == -1) {
        bbslog("3system", "Can't open " PASSFILE "file %s", strerror(errno));
        exit(-1);
    }
    ftruncate(passwdfd, MAXUSERS * sizeof(struct userec));
    if (get_records(PASSFILE, uidshm->passwd, sizeof(struct userec), 1, MAXUSERS) != MAXUSERS) {
        bbslog("4system", "PASS file!");
        ucache_unlock(fd);
        return -1;
    }
    bzero(uidshm->hashhead, UCACHE_HASHSIZE * sizeof(int));
    usernumber = 0;

    ucache_hashinit();

    prev = 0;
    for (i = 0; i < MAXUSERS; i++)
        fillucache(&uidshm->passwd[i], &usernumber, &prev);

    newbbslog(BBSLOG_USIES, "CACHE:reload ucache for %d users", usernumber);
    uidshm->number = usernumber;

    ucache_unlock(fd);
    return 0;
}

int resolve_ucache()
{
    int iscreate;
    int fd;

    fd=ucache_lock();
    iscreate = 0;
    if (uidshm == NULL) {
        uidshm = (struct UCACHE *) attach_shm("UCACHE_SHMKEY", 3696, sizeof(*uidshm), &iscreate);
        /*attach to user shm,readonly */
        if (iscreate) {         /* shouldn't load passwd file in this place */
            bbslog("4system", "passwd daemon havn't startup");
            remove_shm("UCACHE_SHMKEY",3696,sizeof(*uidshm));
            ucache_unlock(fd);
            return -1;
        }
        
    }
    ucache_unlock(fd);
	return 0;
}

void detach_ucache()
{
    shmdt(uidshm);
    uidshm=NULL;
}

/*---	period	2000-10-20	---*/
int getuserid(char *userid, int uid)
{
    if (uid > uidshm->number || uid <= 0)
        return 0;
    strncpy(userid, (char *) uidshm->passwd[uid - 1].userid, IDLEN + 1);
    userid[IDLEN] = 0;
    return uid;
}

static int setuserid_internal(int num, const char *userid)
{                               /* 设置user num的id为user id */
    if (num > 0 && num <= MAXUSERS) {
        int oldkey, newkey, find;

        oldkey = ucache_hash((char *) uidshm->passwd[num - 1].userid);
        newkey = ucache_hash(userid);
        find = uidshm->hashhead[newkey];
        while ((newkey!=0)&&find) { //check duplicate
            if (!strcasecmp(uidshm->passwd[find-1].userid,userid))
                return -1;
            find = uidshm->next[find-1];
        }
        if (num > uidshm->number)
            uidshm->number = num;
/*        if (oldkey!=newkey) { disable,为了加强兼容性*/
        find = uidshm->hashhead[oldkey];

        if (find == num)
            uidshm->hashhead[oldkey] = uidshm->next[find - 1];
        else {                  /* find and remove the hash node */
            int i = 0;

            while (uidshm->next[find - 1] && uidshm->next[find - 1] != num) {
                find = uidshm->next[find - 1];
                i++;
                if (i > MAXUSERS) {
                    bbslog("3system", "UCACHE:uhash loop! find %d,%s", num, userid);
                    exit(0);
                }
            }
            if (!uidshm->next[find - 1]) {
                if (oldkey != 0) {
                    bbslog("3system", "UCACHE:can't find %s in hash table", uidshm->passwd[num - 1].userid);
/*		          	exit(0);*/
                }
                return -1;
            } else
                uidshm->next[find - 1] = uidshm->next[num - 1];
        }

        uidshm->next[num - 1] = uidshm->hashhead[newkey];
        uidshm->hashhead[newkey] = num;
/*        }	        */
        strncpy(uidshm->passwd[num - 1].userid, userid, IDLEN + 1);
    }
    return 0;
}

int setuserid2(int num, const char *userid)
{
    int lockfd, ret;

    lockfd = ucache_lock();
    ret = setuserid_internal(num, userid);
    ucache_unlock(lockfd);
    return ret;
}

void setuserid(int num, const char *userid)
{

    int m_socket;
    char cmdbuf[255];
    struct sockaddr_in sin;
    fd_set rfds;
    int result;
    struct timeval tv;

    m_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket < 0)
        return;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(60001);
#ifdef HAVE_INET_ATON
    inet_aton("127.0.0.1", &sin.sin_addr);
#elif defined HAVE_INET_PTON
	inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr);
#else
	/* Is it OK? */
    my_inet_aton("127.0.0.1", &sin.sin_addr);
#endif
    if (connect(m_socket, (struct sockaddr *) &sin, sizeof(sin)) != 0) {
        close(m_socket);
        return;
    }
    if (userid[0] == 0)
        sprintf(cmdbuf, "DEL %s %d", userid, num);
    else
        sprintf(cmdbuf, "SET %s %d", userid, num);
    write(m_socket, cmdbuf, strlen(cmdbuf));
    FD_ZERO(&rfds);
    FD_SET(m_socket, &rfds);
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    result = select(m_socket + 1, &rfds, NULL, NULL, &tv);
    if (result) {
        int len = read(m_socket, &result, sizeof(result));

        close(m_socket);
        if (len != sizeof(result))
            return;
        return;
    }
    close(m_socket);
    return;
}

static int searchnewuser()
{                               /* 找cache中 空闲的 user num */
    if (uidshm->hashhead[0])
        return uidshm->hashhead[0];
    if (uidshm->number < MAXUSERS)
        return uidshm->number + 1;
    return 0;
}
int searchuser(const char *userid)
{
    register int i;

    i = uidshm->hashhead[ucache_hash(userid)];
    while (i)
        if (!strcasecmp(userid, uidshm->passwd[i - 1].userid))
            return i;
        else
            i = uidshm->next[i - 1];
    return 0;
}

int getuser(const char *userid, struct userec **user)
{                               /* 取用户信息 */
    int uid = searchuser(userid);

    if (uid == 0) {
        if (user)
            *user = NULL;
        return 0;
    }
    if (user)
        *user = &uidshm->passwd[uid - 1];
    return uid;
}

char *getuserid2(int uid)
{
    if (uid > uidshm->number || uid <= 0)
        return NULL;
    return uidshm->passwd[uid - 1].userid;
}

char *u_namearray(char buf[][IDLEN + 1], int *pnum, char *tag)
/* 根据tag ,生成 匹配的user id 列表 (针对所有注册用户)*/
{
    register struct UCACHE *reg_ushm = uidshm;
    register int n, num, i;
    int hash, len, ksz;
    char tagv[IDLEN + 1];

    *pnum = 0;

    len = strlen(tag);
    if (len > IDLEN)
        return NULL;
    if (!len) {
        return NULL;
    }
    ksz = ucache_hash_deep(tag);

    strcpy(tagv, tag);


    if (len >= ksz) {
        tagv[ksz] = 0;
        hash = ucache_hash(tagv) - 1;
        for (n = 0; n < UCACHE_HASHBSIZE; n++) {
            num = reg_ushm->hashhead[(hash + n % UCACHE_HASHBSIZE) % UCACHE_HASHSIZE + 1];
            while (num) {
                if (!strncasecmp(uidshm->passwd[num - 1].userid, tag, len)) {
                    strcpy(buf[(*pnum)++], uidshm->passwd[num - 1].userid);     /*如果匹配, add into buf */
                }
                num = reg_ushm->next[num - 1];
            }
        }
    } else {
        for (i = len; i < ksz; i++)
            tagv[i] = '0';
        tagv[ksz] = 0;

        while (1) {
            hash = ucache_hash(tagv) - 1;

            for (n = 0; n < UCACHE_HASHBSIZE; n++) {
                num = reg_ushm->hashhead[(hash + n % UCACHE_HASHBSIZE) % UCACHE_HASHSIZE + 1];  /* see hash() */
                while (num) {
                    if (!strncasecmp(uidshm->passwd[num - 1].userid, tagv, ksz)) {
                        strcpy(buf[(*pnum)++], uidshm->passwd[num - 1].userid); /*如果匹配, add into buf */
                    }
                    num = reg_ushm->next[num - 1];
                }
            }

            i = ksz - 1;
            while (i >= len) {
                if (tagv[i] == 'Z') {
                    tagv[i] = '0';
                    i--;
                } else if (tagv[i] == '9') {
                    tagv[i] = 'A';
                    break;
                } else {
                    tagv[i]++;
                    break;
                }
            }
            if (i < len)
                break;
        }

    }
    return buf[0];
}

/* disable by KCN 
int
set_safe_record()
{
    struct userec tmp;
    extern int ERROR_READ_SYSTEM_FILE;

    if(get_record(PASSFILE,&tmp,sizeof(struct userec),usernum)==-1)
    {
        char buf[STRLEN];

        sprintf(buf,"Error:Read Passfile %4d %12.12s",usernum,currentuser->userid);
        bbslog("user","%s",buf);
        ERROR_READ_SYSTEM_FILE=true;
        abort_bbs(0);
        return -1;
    }
    currentuser->numposts=tmp.numposts;
    currentuser->numlogins=tmp.numlogins;
    currentuser->stay=tmp.stay;
    currentuser->userlevel=tmp.userlevel;
}
*/

int getnewuserid3(char *userid)
{

    int m_socket;
    char cmdbuf[255];
    struct sockaddr_in sin;
    fd_set rfds;
    int result;
    struct timeval tv;

    m_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket < 0)
        return -1;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(60001);
#ifdef HAVE_INET_ATON
    inet_aton("127.0.0.1", &sin.sin_addr);
#elif defined HAVE_INET_PTON
	inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr);
#else
	/* Is it OK? */
    my_inet_aton("127.0.0.1", &sin.sin_addr);
#endif
    if (connect(m_socket, (struct sockaddr *) &sin, sizeof(sin)) != 0) {
        close(m_socket);
        return -1;
    }
    sprintf(cmdbuf, "NEW %s", userid);
    write(m_socket, cmdbuf, strlen(cmdbuf));
    FD_ZERO(&rfds);
    FD_SET(m_socket, &rfds);
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    result = select(m_socket + 1, &rfds, NULL, NULL, &tv);
    if (result) {
        int len = read(m_socket, &result, sizeof(result));

        close(m_socket);
        if (len != sizeof(result))
            return -1;
        return result;
    }
    close(m_socket);
    return -1;
}

int getnewuserid2(char *userid)
{
    int result = getnewuserid3(userid);

    if (result >= 0)
        return result;
    return -1;
}

struct userec *getuserbynum(int num)
{
    if (num <= 0 || num >= MAXUSERS)
        return NULL;
    return &uidshm->passwd[num - 1];
}

int getnewuserid(char *userid)
{
    struct userec utmp;
    int fd, i;
    time_t system_time;

    system_time = time(NULL);
/*
    if( (fd = open( PASSFILE, O_RDWR|O_CREAT, 0600 )) == -1 )
        return -1;
    flock( fd, LOCK_EX );
*/
    fd = ucache_lock();

    while (1) {
        int ret;

        i = searchnewuser();
        if (i <= 0 || i > MAXUSERS) {
            ucache_unlock(fd);
            return -1;
        }
        memset(&utmp, 0, sizeof(utmp));
        strcpy(utmp.userid, userid);
        utmp.lastlogin = time(NULL);
        ret = setuserid_internal(i, userid);    /* added by dong, 1998.12.2 */
        if (ret == 0)
            break;
        uidshm->passwd[i - 1].userid[0] = 0;
    }
    update_user(&utmp, i, 0);
    ucache_unlock(fd);
    return i;
}

int update_user(struct userec *user, int num, int all)
{
    struct userec tmpuser;

    if (!all) {
        if (strncasecmp(user->userid, uidshm->passwd[num - 1].userid, IDLEN))
            return -1;
        tmpuser = *user;
#ifdef CONV_PASS
        memcpy(tmpuser.passwd, uidshm->passwd[num - 1].passwd, IDLEN + 2);
#endif
        memcpy(tmpuser.md5passwd, uidshm->passwd[num - 1].md5passwd, IDLEN + 2);
    } else {
        tmpuser = *user;
        memcpy(tmpuser.userid, uidshm->passwd[num - 1].userid, IDLEN + 2);
    }
    memcpy(&uidshm->passwd[num - 1], &tmpuser, sizeof(struct userec));
    return 0;
}

int apply_users(int (*fptr) (struct userec *, char *), char *arg)
{
    int i;
    int count;

    count = 0;
    for (i = 0; i < uidshm->number; i++)
        if (fptr) {
            int ret;

            ret = (*fptr) (&uidshm->passwd[i], arg);
            if (ret == QUIT)
                break;
            if (ret == COUNT)
                count++;
        } else
            count++;
    return count;
}

int get_giveupinfo(char* userid,int* basicperm,int s[10][2])
{
	int lcount;
	FILE* fn;
	char buf[255];
    *basicperm = 0;
    lcount=0;
    sethomefile(buf, userid, "giveup");
    fn = fopen(buf, "rt");
    if (fn) {
        while (!feof(fn)) {
	    int i,j;
            if (fscanf(fn, "%d %d", &i, &j) <= 0)
                break;
            s[lcount][0] = i;
            s[lcount][1] = j;
            switch (i) {
            case 1:
                *basicperm |= PERM_BASIC;
                break;
            case 2:
                *basicperm |= PERM_POST;
                break;
            case 3:
                *basicperm |= PERM_CHAT;
                break;
            case 4:
                *basicperm |= PERM_PAGE;
                break;
            case 5:
                *basicperm |= PERM_DENYMAIL;
                break;
            case 6:
                *basicperm |= PERM_DENYRELAX;
                break;
            }
            lcount++;
        }
        fclose(fn);
    }
    return lcount;
}

void save_giveupinfo(struct userec* lookupuser,int lcount,int s[10][2])
{
/*Bad 2002.7.6 受限与戒网问题*/
	int kcount,tcount,i,j;
	char buf[255];
	FILE* fn;
    sethomefile(buf, lookupuser->userid, "giveup");
    kcount = lcount;
    for (i = 0; i < lcount; i++) {
        j = 0;
        switch (s[i][0]) {
        case 1:
            j = lookupuser->userlevel & PERM_BASIC;
            break;
        case 2:
            j = lookupuser->userlevel & PERM_POST;
            break;
        case 3:
            j = lookupuser->userlevel & PERM_CHAT;
            break;
        case 4:
            j = lookupuser->userlevel & PERM_PAGE;
            break;
        case 5:
            j = !(lookupuser->userlevel & PERM_DENYMAIL);
            break;
        case 6:
            j = !(lookupuser->userlevel & PERM_DENYRELAX);
            break;
        }
        if (j) {
            kcount--;
            s[i][1] = 0;
        }
    }
    if (kcount != lcount) {
        if (kcount == 0)
            unlink(buf);
        else {
            fn = fopen(buf, "wt");
            for (i = 0; i < lcount; i++)
                if (s[i][1] > 0)
                    fprintf(fn, "%d %d\n", s[i][0], s[i][1]);
            fclose(fn);
        }
    }
	tcount = 0;
    if (lookupuser->userlevel & PERM_BASIC)
        tcount++;
    if (lookupuser->userlevel & PERM_POST)
        tcount++;
    if (lookupuser->userlevel & PERM_CHAT)
        tcount++;
    if (lookupuser->userlevel & PERM_PAGE)
        tcount++;
    if (!(lookupuser->userlevel & PERM_DENYMAIL))
        tcount++;

    if (!(lookupuser->userlevel & PERM_DENYRELAX))
        tcount++;

    if (kcount + tcount == 6 && kcount > 0)
        lookupuser->flags |= GIVEUP_FLAG;
    else
        lookupuser->flags &= ~GIVEUP_FLAG;
}

#if USE_TMPFS==1

void setcachehomefile(char* path,char* user,int unum,char* file)
{
    if (unum==-1) {
    if (file==NULL)
      sprintf(path, "%s/home/%c/%s",TMPFSROOT,toupper(user[0]),user);
    else
      sprintf(path, "%s/home/%c/%s/%s",TMPFSROOT, toupper(user[0]), user,file);
    } else {
    if (file==NULL)
      sprintf(path, "%s/home/%c/%s/%d",TMPFSROOT,toupper(user[0]),user,unum);
    else
      sprintf(path, "%s/home/%c/%s/%d/%s",TMPFSROOT, toupper(user[0]), user,unum,file);
    }
}

void init_cachedata(char* userid,int unum)
{
    char path1[MAXPATH],path2[MAXPATH];
    int fd,logincount;
    int count;
    struct stat st;
    struct flock ldata;
    setcachehomefile(path1, userid, -1, NULL);
    mkdir(path1,0700);
    setcachehomefile(path1, userid, unum, NULL);
    if (mkdir(path1,0700)==-1)
        bbslog("3error","mkdir %s errorno %d",path1,errno);
    
    setcachehomefile(path1, userid, -1, "logincount");
    if ((fd = open(path1, O_RDWR, 0664)) != -1) {
        ldata.l_type = F_RDLCK;
        ldata.l_whence = 0;
        ldata.l_len = 0;
        ldata.l_start = 0;
        if (fcntl(fd, F_SETLKW, &ldata) == -1) {
            bbslog("3error", "%s", "logincount err");
            close(fd);
            return;              /* lock error*/
        }
        count=read(fd,path2,MAXPATH);
        path2[count]=0;
        logincount=atoi(path2);
    } else {
        if ((fd = open(path1, O_WRONLY|O_CREAT, 0664)) != -1) {
            logincount=0;
        } else {
            bbslog("3error", "%s", "write logincount err");
            return;              /* lock error*/
        }
    }
    logincount++;
    lseek(fd,0,SEEK_SET);
    sprintf(path2,"%d",logincount);
    write(fd,path2,strlen(path2));
    close(fd);
}
void flush_cachedata(char* userid)
{
}

int clean_cachedata(char* userid,int unum)
{
    char path1[MAXPATH],path2[MAXPATH];
    int fd,logincount;
    int count;
    struct flock ldata;

    setcachehomefile(path1, userid, unum, NULL);
    f_rm(path1);
    //todo: check the dir
    setcachehomefile(path1, userid, -1, "logincount");
    if ((fd = open(path1, O_RDWR, 0664)) != -1) {
    ldata.l_type = F_RDLCK;
    ldata.l_whence = 0;
    ldata.l_len = 0;
    ldata.l_start = 0;
    if (fcntl(fd, F_SETLKW, &ldata) == -1) {
        bbslog("3error", "%s", "logincount err");
        close(fd);
        return;              /* lock error*/
    }
    count=read(fd,path2,MAXPATH);
    path2[count]=0;
    logincount=atoi(path2);
    logincount--;
    lseek(fd,0,SEEK_SET);
    sprintf(path2,"%d",logincount);
    write(fd,path2,strlen(path2));
    close(fd);
    } else logincount=0;
    if (logincount==0) {
	if ((userid[0]==0)||(userid==NULL)) {
	  bbslog("3error","error in clean cache");
	} else {
        setcachehomefile(path1, userid, -1, "entry");
        unlink(path1);
        setcachehomefile(path1, userid, -1, NULL);
        f_rm(path1);
	}
    }
}
#endif

int do_after_login(struct userec* user,int unum,int mode)
{
#if USE_TMPFS==1
  if (mode==0)
    init_cachedata(user->userid,unum);
  else //www guest,使用负数来和telnet guest区分
    init_cachedata(user->userid,-unum);
#endif
}

int do_after_logout(struct userec* user,struct user_info* userinfo,int unum,int mode)
{
    char buf[MAXPATH];

    if (userinfo&&(mode==0)) {
#if USE_TMPFS==0
      if (userinfo->utmpkey!=0) {
        snprintf(buf,MAXPATH,"%s/%s_%d",ATTACHTMPPATH,userinfo->userid,unum);
        f_rm(buf);
      }
#endif
	  if(userinfo -> pid > 1 ){
		snprintf(buf,MAXPATH,"rm -rf tmp/%d/%s/", userinfo->pid, userinfo->userid);
		system(buf);
	  }
    }
    if (user) {
#if USE_TMPFS==1
        if (mode==0)
            clean_cachedata(user->userid,unum);
        else //www guest,使用负数来和telnet guest区分
	{
            clean_cachedata(user->userid,-unum);
	}
#endif
    }
    if (userinfo&&userinfo->currentboard)
        board_setcurrentuser(userinfo->currentboard,-1);
}

#ifdef HAVE_CUSTOM_USER_TITLE
/**
 * user_title数组是1 base,所以idx都要减一
 * 当title==0的时候，应该用原来的显示体系结构
 */

/**
  * 读入文件中保存的user title
  */
void load_user_title()
{
    FILE* titlefile;
    bzero(uidshm->user_title,sizeof(uidshm->user_title));
    if ((titlefile = fopen(USER_TITLE_FILE, "r")) == NULL) {
        bbslog("3system", "Can't open " USER_TITLE_FILE "file %s", strerror(errno));
    } else {
        int i;
        for (i=0;i<256;i++) {
            fgets(uidshm->user_title[i],USER_TITLE_LEN,titlefile);
            
            if ((uidshm->user_title[i][0]!=0)&&(uidshm->user_title[i][strlen(uidshm->user_title[i])-1]=='\n'))
                uidshm->user_title[i][strlen(uidshm->user_title[i])-1]=0;
        }
        fclose(titlefile);
    }
}

/**
  *  把user_title数组写入磁盘
  */
static void flush_user_title()
{
    FILE* titlefile;
    if ((titlefile = fopen(USER_TITLE_FILE, "w")) == -1) {
        bbslog("3system", "Can't open " USER_TITLE_FILE "file %s", strerror(errno));
    } else {
        int i;
        for (i=0;i<256;i++) {
            fprintf(titlefile,"%s\n",uidshm->user_title[i]);
        }
        fclose(titlefile);
    }
}

/**
 * 获得title对应的字符串
 * @param titleidx 1base的title
 * @return 用于显示的title
 */
char* get_user_title(unsigned char titleidx)
{
    if (titleidx==0) return "";
    return uidshm->user_title[titleidx-1];
}

/**
 * 设置title对应的字符串
 * @param titleidx 1base的title
 * @param newtitle 需要设置的title
 */
void set_user_title(unsigned char titleidx,char* newtitle)
{
    int fd;
    fd=ucache_lock();
    if (titleidx==0) return;
    uidshm->user_title[titleidx-1][USER_TITLE_LEN-1]=0;
    strncpy(uidshm->user_title[titleidx-1],newtitle,USER_TITLE_LEN-1);
    flush_user_title();
    ucache_unlock(fd);
}
#endif

#if HAVE_WWW==1

/* WWW GUEST这样做有个同步问题，就是当被清除一个
 * GUEST的时候如果正好这个guest刷新了，那么会重写数据结构
 * 所以，要注意除了key之外的数据如果要变动，必须保证
 * 在这种情况下不会逻辑混乱。freshtime因为即使被错误的
 * client更新了，误差在秒级，而且也不会造成用户问题，所以
 * 不会出错。但是如果需要更新key，那么就有可能导致下一个
 * guest用户的key被错误的覆盖。这个是个问题
 * 另: www guest表使用的idx居然是0 base的。
 */
struct WWW_GUEST_TABLE *wwwguest_shm = NULL;

static void longlock(int signo)
{
    bbslog("5system", "www_guest lock for so long time!!!.");
    exit(-1);
}

int www_guest_lock()
{
    int fd = 0;

    fd = open("www_guest.lock", O_RDWR | O_CREAT, 0600);
    if (fd < 0) {
        return -1;
    }
    signal(SIGALRM, longlock);
    alarm(10);
    if (flock(fd, LOCK_EX) == -1) {
        return -1;
    }
    signal(SIGALRM, SIG_IGN);
    return fd;
}

void www_guest_unlock(int fd)
{
    flock(fd, LOCK_UN);
    close(fd);
}


int resolve_guest_table()
{
    int iscreate = 0;

    if (wwwguest_shm == NULL) {
        wwwguest_shm = (struct WWW_GUEST_TABLE *)
            attach_shm("WWWGUEST_SHMKEY", 4500, sizeof(*wwwguest_shm), &iscreate);      /*attach user tmp cache */
		if (wwwguest_shm==NULL)
			bbslog("3error","can't load guest shm:%d",errno);
        if (iscreate) {
            struct public_data *pub;
            int fd = www_guest_lock();
            setpublicshmreadonly(0);
	    pub=get_publicshm();
            if (fd == -1)
                return -1;
            bzero(wwwguest_shm, sizeof(*wwwguest_shm));
            wwwguest_shm->uptime = time(0);
            www_guest_unlock(fd);
            pub->www_guest_count = 0;
            setpublicshmreadonly(1);
        }
    }
    return 0;
}
#endif
