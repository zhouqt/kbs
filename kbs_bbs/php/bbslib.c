/*
 * $Id$
 */
#include "bbslib.h"

void f_append(FILE *fp, char *buf)
{
    char *ptr;

    ptr = buf;
    while (*ptr != '\0') {
        if (*ptr == '\x09')     /* horizontal tab */
            fprintf(fp, "    ");
        else
            fputc(*ptr, fp);
        ptr++;
    }
}

int file_exist(char *file)
{
    struct stat buf;

    if (stat(file, &buf) == -1) return 0;
    if (!S_ISREG(buf.st_mode)) return 0;
    return 1;
}

int del_mail(int ent, struct fileheader *fh, char *direct)
{
    char buf[PATHLEN];
    char *t;
    char genbuf[PATHLEN];
    struct stat st;

    strcpy(buf, direct);
    if ((t = strrchr(buf, '/')) != NULL)
        *t = '\0';
    if (!delete_record(direct, sizeof(*fh), ent, (RECORD_FUNC_ARG) cmpname, fh->filename)) {
        sprintf(genbuf, "%s/%s", buf, fh->filename);
        if (strstr(direct, ".DELETED") || HAS_MAILBOX_PROP(getSession()->currentuinfo, MBP_FORCEDELETEMAIL)) {
            if (lstat(genbuf, &st) == 0 && S_ISREG(st.st_mode) && st.st_nlink == 1) {
                if (getCurrentUser()->usedspace > st.st_size)
                    getCurrentUser()->usedspace -= st.st_size;
                else
                    getCurrentUser()->usedspace = 0;
            }
            unlink(genbuf);
        } else {
            strcpy(buf, direct);
            t = strrchr(buf, '/') + 1;
            strcpy(t, ".DELETED");
            append_record(buf, fh, sizeof(*fh));
        }
        return 0;
    }
    return 1;
}

int is_BM(const struct boardheader *board,const struct userec *user)
{
    char BM[STRLEN];

    strncpy(BM, board->BM, sizeof(BM) - 1);
    BM[sizeof(BM) - 1] = '\0';
    return chk_currBM(BM, (struct userec *)user);
}

/* Convert string to Unix format */
char *unix_string(char *str)
{
    char *ptr1, *ptr2;

    ptr1 = ptr2 = str;
    while (*ptr1 != '\0') {
        if (*ptr1 == '\r' && *(ptr1 + 1) == '\n') {
            ptr1++;
            continue;
        }
        if (ptr1 != ptr2)
            *ptr2 = *ptr1;
        ptr1++;
        ptr2++;
    }
    *ptr2 = '\0';

    return str;
}

int send_msg(char *srcid, int srcutmp, char *destid, int destpid, char *msg)
{
    uinfo_t *uin;

    process_control_chars(msg,"\n");
    uin = t_search(destid, destpid);
    if (uin == NULL)
        return -1;
    if (strcasecmp(uin->userid, destid))
        return -1;
    strcpy(getSession()->MsgDesUid, uin->userid);
    return sendmsgfunc(uin, msg, 2, 1, getSession());
}

int isfriend(char *id)
{
    if (getCurrentUser() && strcmp(getCurrentUser()->userid, "guest")) {
        return myfriend(searchuser(id), NULL, getSession());
    } else {
        return false;
    }
}



/* 以下的代码是cgi和php都使用的*/
static struct user_info www_guest_uinfo;

static int www_guest_calc_hashkey(struct in_addr *fromhostn)
{
	unsigned int i=ntohl(fromhostn->s_addr);
	unsigned int j;
			        
    j =  i & 0x0000FFFF;
    j |= (((i&0xFF000000)>>8) + (i&0x00FF0000)) & 0x000F0000;

    return j;
}

static int www_guest_start_map(int key)
{
	return ( key % MAX_WWW_MAP_ITEM + 1 );
}

#define WWW_GUEST_HASHTAB(key) wwwguest_shm->hashtab[key>>16][(key&0x0000FF00)>>8][key&0x000000FF]

/*   stiger:  1 guest entry per IP
return:
	<0: error
	0: 正常登录,idx
	1: 有重复,使用idx的entry
**************/
static int www_new_guest_entry(struct in_addr *fromhostn, int * idx)
{
    struct public_data *pub;
    int oldidx, num, fd, i, j = 0;
    time_t now;
    struct userec *user;
	int hashkey;
	int startkey;

    fd = www_guest_lock();
    if (fd == -1)
        return -1;
    setpublicshmreadonly(0);
    pub = get_publicshm();
    if (pub->www_guest_count >= MAX_WWW_GUEST) {
    	www_guest_unlock(fd);
        setpublicshmreadonly(1);
        return -1;
    }
    user = getCurrentUser();
    getuser("guest", &getCurrentUser());

    if (getCurrentUser() == NULL){
    	www_guest_unlock(fd);
    	setpublicshmreadonly(1);
        return -1;
	}
    now = time(NULL);
    if ((now > wwwguest_shm->uptime + 240) || (now < wwwguest_shm->uptime - 240)) {
        newbbslog(BBSLOG_USIES, "WWW guest:Clean guest table:%d", wwwguest_shm->uptime);
        wwwguest_shm->uptime = now;
        for (i = 0; i < MAX_WWW_GUEST; i++) {
	    struct user_info guestinfo;
            if (!(wwwguest_shm->use_map[i / 32] & (1 << (i % 32))) || (now - wwwguest_shm->guest_entry[i].freshtime < MAX_WWW_GUEST_IDLE_TIME))
                continue;
            newbbslog(BBSLOG_USIES, "EXIT: Stay:%3ld (guest)[%d %d](www)", now - wwwguest_shm->guest_entry[i].freshtime, wwwguest_shm->guest_entry[i].key);
            /*
             * 清除use_map 
             */
	    guestinfo.currentboard=wwwguest_shm->guest_entry[i].currentboard;
	    do_after_logout(getCurrentUser(), &guestinfo, i, 1,false);
	    do_after_logout(getCurrentUser(), &guestinfo, i, 1,true);

            wwwguest_shm->use_map[i / 32] &= ~(1 << (i % 32));
			/* 清除hashtab */
			WWW_GUEST_HASHTAB(www_guest_calc_hashkey(& wwwguest_shm->guest_entry[i].fromip)) = 0;
            if (pub->www_guest_count > 0) {
                pub->www_guest_count--;
	        	pub->wwwguestlogoutcount++;
	       		pub->wwwgueststaytime += now - wwwguest_shm->guest_entry[i].logintime;
                /*
                 * 清除数据 
                 */
                bzero(&wwwguest_shm->guest_entry[i], sizeof(struct WWW_GUEST_S));
            }
        } //for
    }//if need kick www guest

	hashkey = www_guest_calc_hashkey(fromhostn);
	oldidx = WWW_GUEST_HASHTAB(hashkey);

	num=0;
    pub->wwwguestlogincount++;

/* 如果已经有相同的登录 */
if( oldidx != 0 && fromhostn->s_addr == wwwguest_shm->guest_entry[oldidx].fromip.s_addr ){

	*idx = oldidx;
	num=-1;
}else{

	startkey = www_guest_start_map(hashkey);

/* 如果hashtab有值但是IP不同，遍历 */
	if( oldidx != 0 ){
		for ( num = 0, i = startkey; num < MAX_WWW_MAP_ITEM; num++, i++){
			if( i>= MAX_WWW_MAP_ITEM)
				i=1;
        	if (wwwguest_shm->use_map[i] != 0) {
            	int map = wwwguest_shm->use_map[i];
            	for (j = 0; j < 32; j++){
                	if ((map & 1) != 0) {
						/* 找到相同的IP了 */
						if( wwwguest_shm->guest_entry[i*32+j].fromip.s_addr == fromhostn->s_addr ){
							num = -1;
							*idx = i*32+j;
							break;
						}
					}
					map = map >> 1;
				}
        	}
			if( num == -1 )
				break;
		}
	}

/* 如果遍历发现没有相同IP的 */
	if( num != -1 ){
		/* 找一个新的空位 */
	    for (num=0, i = startkey; num < MAX_WWW_MAP_ITEM; num++, i++){
			if( i>= MAX_WWW_MAP_ITEM)
				i=1;
        	if (wwwguest_shm->use_map[i] != 0xFFFFFFFF) {
            	int map = wwwguest_shm->use_map[i];

            	for (j = 0; j < 32; j++)
                	if ((map & 1) == 0) {
                    	wwwguest_shm->use_map[i] |= 1 << j;
                    	wwwguest_shm->guest_entry[i * 32 + j].freshtime = time(0);
						/* 设置hashtab */
						WWW_GUEST_HASHTAB(hashkey) = i*32+j;
                    	/*
                     	* 避免被kick下去 
                     	*/
                    	break;
                	} else
                    	map = map >> 1;
            	break;
        	}
		}
    	if (num != MAX_WWW_MAP_ITEM) {
        	pub->www_guest_count++;
        	if (get_utmp_number() + getwwwguestcount() > get_publicshm()->max_user) {
            	save_maxuser();
        	}
    	}
		*idx = i*32+j;
	}
}
    setCurrentUser(user);
    setpublicshmreadonly(1);
    www_guest_unlock(fd);
    if (num == MAX_WWW_MAP_ITEM)
        return -1;
	if (num == -1)
		return 1;
    return 0;
}

struct WWW_GUEST_S* www_get_guest_entry(int idx)
{
    return  &wwwguest_shm->guest_entry[idx];
}

static int www_free_guest_entry(int idx)
{
    int fd;
    struct public_data *pub;
    struct user_info guestinfo;

    if ((idx < 0) || (idx > MAX_WWW_GUEST))
        return -1;
    guestinfo.currentboard=wwwguest_shm->guest_entry[idx].currentboard;
    do_after_logout(getCurrentUser(), &guestinfo, idx, 1,false);
    do_after_logout(getCurrentUser(), &guestinfo, idx, 1,true);
    setpublicshmreadonly(0);
    pub = get_publicshm();
    fd = www_guest_lock();
    if (wwwguest_shm->use_map[idx / 32] & (1 << (idx % 32))) {
		time_t staytime = time(NULL)-wwwguest_shm->guest_entry[idx].logintime;
        wwwguest_shm->use_map[idx / 32] &= ~(1 << (idx % 32));
		WWW_GUEST_HASHTAB(www_guest_calc_hashkey(&wwwguest_shm->guest_entry[idx].fromip))=0;
    	bzero(&wwwguest_shm->guest_entry[idx], sizeof(struct WWW_GUEST_S));
        if (pub->www_guest_count > 0){
	       	pub->wwwguestlogoutcount++;
	       	pub->wwwgueststaytime += staytime;
            pub->www_guest_count--;
		}
    }
    www_guest_unlock(fd);
    setpublicshmreadonly(1);
    return 0;
}

void www_data_detach()
{
    shmdt((void*)wwwguest_shm);
    wwwguest_shm = NULL;
}

int www_data_init()
{
    struct userec *guest;

    /*
     * www_guest_info目前先使用一个全局变量来做，这样
     * 会导致线程不安全:P   但是对于进程模型的cgi 和php
     * 足够了
     */
    bzero(&www_guest_uinfo, sizeof(www_guest_uinfo));
    www_guest_uinfo.active = true;
    www_guest_uinfo.uid = getuser("guest", &guest);
    if (www_guest_uinfo.uid == 0)
        return -1;
    www_guest_uinfo.invisible = true;
    www_guest_uinfo.pid = 1;
    www_guest_uinfo.mode = WEBEXPLORE;
    strcpy(www_guest_uinfo.username, guest->username);
    strcpy(www_guest_uinfo.userid, guest->userid);
    www_guest_uinfo.pager = 0;
    {
        struct userdata ud;

        read_userdata(guest->userid, &ud);
        strcpy(www_guest_uinfo.realname, ud.realname);
    }
    www_guest_uinfo.utmpkey = 0;

    /*
     * destuid 将被用来存放www guest表的入口 
     */
    www_guest_uinfo.destuid = 0;

    if (resolve_guest_table() != 0)
        return -1;
    return 0;
}


static void set_idle_time(struct user_info * uentp, time_t t)
{
    if (strcasecmp(uentp->userid, "guest"))
        uentp->freshtime = t;
    else {
        int idx;

        idx = uentp->destuid;
        if (idx >= 1 && idx < MAX_WWW_GUEST)
            wwwguest_shm->guest_entry[uentp->destuid].freshtime = t;
    }
}

int www_user_init(int useridx, char *userid, int key, struct userec **x, struct user_info **y,long compat_telnet)
{
    /*
     * printf("utmpuserid = %s\n", id);
     */
    /*
     * printf("utmpnum = %s\n", num); 
     */
    if (userid&&!strcasecmp(userid, "new"))
        return -1;

    if ((userid==NULL)||strcasecmp(userid, "guest")) {
        /*
         * 非guest在线用户处理 
         */
        if (useridx < 1 || useridx >= MAXACTIVE) {
            return -1;
        }
        (*y) = get_utmpent(useridx);
        if (                    /*(strncmp((*y)->from, getSession()->fromhost, IPLEN))|| */
               ((*y)->utmpkey != key))
            return -2;

        if ((((*y)->active == 0)) || ((*y)->userid[0] == 0)
            || ((compat_telnet==0)&&((*y)->pid != 1)))
            return -3;

        if (userid&&strcmp((*y)->userid, userid))
            return -4;
        getuser((*y)->userid, x);

        if (*x == 0)
            return -5;

        strncpy(getSession()->fromhost, (*y)->from, IPLEN);
        getSession()->fromhost[IPLEN] = '\0';
    } else {
        /*
         * guest用户处理 
         */
        struct WWW_GUEST_S *guest_info;

        if ((useridx < 0) || (useridx >= MAX_WWW_GUEST))
            return -1;
        guest_info = &wwwguest_shm->guest_entry[useridx];
        if (guest_info->key != key) {
            return -2;
        }

        inet_ntop(AF_INET, &guest_info->fromip, getSession()->fromhost, IPLEN);
        strcpy(www_guest_uinfo.from, getSession()->fromhost);
        www_guest_uinfo.freshtime = guest_info->freshtime;
        www_guest_uinfo.utmpkey = key;
        www_guest_uinfo.destuid = useridx;
        www_guest_uinfo.logintime = guest_info->logintime;

        *y = &www_guest_uinfo;

        getuser("guest", x);
        if (*x == NULL)
            return -1;
    }
    set_idle_time(*y, time(0));
    return 0;
}

int www_user_login(struct userec *user, int useridx, int kick_multi, char *fromhost, char *fullfrom, struct user_info **ppuinfo, int *putmpent)
{
    int ret;
    char buf[255];
    struct userdata ud;

    if (user != NULL && strcasecmp(user->userid, "guest")) {
        struct user_info ui;
        int utmpent;
        time_t t;
        int multi_ret = 1;
 
        while (multi_ret != 0) {
            int num;
            struct user_info uin;

            multi_ret = multilogin_user(user, useridx,1);
            if ((multi_ret != 0) && (!kick_multi))
                return -1;
            if (multi_ret == 0)
                break;
            if (!(num = search_ulist(&uin, cmpuids2, useridx)))
                continue;       /* user isn't logged in */
            kick_user_utmp(useridx, get_utmpent(num), 0);
            sleep(1);
        }

        if (!HAS_PERM(user, PERM_BASIC))
            return 3;
        t = user->lastlogin;
        if (abs(t - time(0)) < 5)
            return 5;
        user->lastlogin = time(0);
        user->numlogins++;
        strncpy(user->lasthost, fromhost, IPLEN);
        user->lasthost[IPLEN - 1] = '\0';       /* add by binxun ,fix the bug */
        read_userdata(user->userid, &ud);
	user->userlevel &= (~PERM_SUICIDE);
        if (!HAS_PERM(user, PERM_LOGINOK) && !HAS_PERM(user, PERM_SYSOP)) {
            if (strchr(ud.realemail, '@')
                && valid_ident(ud.realemail)) {
                user->userlevel |= PERM_DEFAULT;
                /*
                 * if (HAS_PERM(user, PERM_DENYPOST)  )
                 * user->userlevel &= ~PERM_POST;
                 */
            }
        }

        memset(&ui, 0, sizeof(struct user_info));
        ui.active = true;
        /*
         * Bigman 2000.8.29 智囊团能够隐身
         */
        if ((HAS_PERM(user, PERM_CHATCLOAK)
             || HAS_PERM(user, PERM_CLOAK))
            && (user->flags & CLOAK_FLAG))
            ui.invisible = true;
        ui.pager = 0;
        if (DEFINE(user, DEF_FRIENDCALL)) {
            ui.pager |= FRIEND_PAGER;
        }
        if (user->flags & PAGER_FLAG) {
            ui.pager |= ALL_PAGER;
            ui.pager |= FRIEND_PAGER;
        }
        if (DEFINE(user, DEF_FRIENDMSG)) {
            ui.pager |= FRIENDMSG_PAGER;
        }
        if (DEFINE(user, DEF_ALLMSG)) {
            ui.pager |= ALLMSG_PAGER;
            ui.pager |= FRIENDMSG_PAGER;
        }
        ui.uid = useridx;
        strncpy(ui.from, fromhost, IPLEN);
        ui.logintime = time(0); /* for counting user's stay time */
        /*
         * refer to bbsfoot.c for details 
         */
        ui.freshtime = time(0);
        ui.mode = WEBEXPLORE;
        
        strncpy(ui.userid, user->userid, 20);
        strncpy(ui.realname, ud.realname, 20);
        strncpy(ui.username, user->username, 40);
        utmpent = getnewutmpent2(&ui, 1);
        if (utmpent == -1)
            ret = 1;
        else {
            struct user_info *u;

            u = get_utmpent(utmpent);
            u->pid = 1;
			/*
            if (addto_msglist(utmpent, user->userid) < 0) {
                bbslog("3system", "can't add msg:%d %s!!!\n", utmpent, user->userid);
                *ppuinfo = u;
                *putmpent = utmpent;
                ret = 2;
            } else {
			*/
                *ppuinfo = u;
                *putmpent = utmpent;
                ret = 0;
				/*
            }
			*/
    	    /* Load getCurrentUser()'s mailbox properties, added by atppp */
    	    u->mailbox_prop = load_mailbox_prop(user->userid);

            getfriendstr(getCurrentUser(), u, getSession());
            do_after_login(getCurrentUser(),utmpent,0);
        }
    } else {
        /*
         * TODO:alloc guest table 
         */
		int idx = 0;
		int exist;
		struct in_addr fromhostn;

#ifdef HAVE_INET_ATON
    inet_aton(fromhost, &fromhostn);
#elif defined HAVE_INET_PTON
	inet_pton(AF_INET, fromhost, &fromhostn);
#else
    my_inet_aton(fromhost, &fromhostn);
#endif

		exist = www_new_guest_entry(&fromhostn, &idx);
		/* exist:
<0: error
0: 正常登录,idx
1: 有重复,使用idx的entry
		*/

        if (exist < 0)
            ret = 5;
        else {

			if( ! exist ){
            	int tmp = rand() % 100000000;
            	wwwguest_shm->guest_entry[idx].key = tmp;
				wwwguest_shm->guest_entry[idx].fromip.s_addr = fromhostn.s_addr;
            	wwwguest_shm->guest_entry[idx].logintime = time(0);
			}

            wwwguest_shm->guest_entry[idx].freshtime = time(0);
            www_guest_uinfo.freshtime = wwwguest_shm->guest_entry[idx].freshtime;
            www_guest_uinfo.logintime = wwwguest_shm->guest_entry[idx].logintime;
            www_guest_uinfo.destuid = idx;
            www_guest_uinfo.utmpkey = wwwguest_shm->guest_entry[idx].key;
            *ppuinfo = &www_guest_uinfo;
            *putmpent = idx;
            getuser("guest", &getCurrentUser());
            ret = 0;
			if( ! exist )
            	do_after_login(getCurrentUser(),idx,1);
        }
    }

    if ((ret == 0) || (ret == 2)) {
        snprintf(buf, sizeof(buf), "ENTER ?@%s (ALLOC %d) [www]", fullfrom, *putmpent);
        newbbslog(BBSLOG_USIES, "%s", buf);
    }
    return ret;
}

static void setflags(struct userec *u, int mask, int value)
{
    if (((u->flags & mask) && 1) != value) {
        if (value)
            u->flags |= mask;
        else
            u->flags &= ~mask;
    }
}
int www_user_logoff(struct userec *user, int useridx, struct user_info *puinfo, int userinfoidx)
{
    int stay = 0;

    stay = time(0) - puinfo->logintime;
    if (stay < 0) stay = 0;
#if 0 // atppp 20060221
    /*
     * 上站时间超过 2 小时按 2 小时计 
     */
    if (stay > 7200)
        stay = 7200;
#endif
    if(stay < 300) {
        if (user->numlogins > 5)
            user->numlogins--;
    } else {
        user->stay += stay;
    }
    user->exittime = time(0);
    if (strcmp(user->userid, "guest")) {
        newbbslog(BBSLOG_USIES, "EXIT: Stay:%3ld (%s)[%d %d](www)", stay / 60, user->username, getSession()->utmpent, useridx);
        if (!puinfo->active)
            return 0;
        setflags(user, PAGER_FLAG, (puinfo->pager & ALL_PAGER));

        if ((HAS_PERM(user, PERM_CHATCLOAK) || HAS_PERM(user, PERM_CLOAK)))
            setflags(user, CLOAK_FLAG, puinfo->invisible);
#if defined(HAVE_BRC_CONTROL)
        init_brc_cache(user->userid,false,getSession());
        brc_update(user->userid, getSession());
#endif

        clear_utmp(userinfoidx, useridx, 1);

    } else {
        newbbslog(BBSLOG_USIES, "EXIT: Stay:%3ld (guest)[%d %d](www)", stay / 60, puinfo->destuid, useridx);
        www_free_guest_entry(puinfo->destuid);
    }
    return 0;
}













static void print_font_style(unsigned int style, buffered_output_t * output)
{
    char font_class[8];
    char font_style[STRLEN];
    char font_str[256];
    unsigned int bg;
	int len;

    if (STYLE_ISSET(style, FONT_BG_SET)) {
        bg = 8;
    } else
        bg = STYLE_GET_BG(style);
    sprintf(font_class, "f%01d%02d", bg, STYLE_GET_FG(style));
    font_style[0] = '\0';
    if (STYLE_ISSET(style, FONT_STYLE_UL))
        strcat(font_style, "text-decoration: underline; ");
    if (STYLE_ISSET(style, FONT_STYLE_ITALIC))
        strcat(font_style, "font-style: italic; ");
    if (font_style[0] != '\0')
        sprintf(font_str, "<font class=\"%s\" style=\"%s\">", font_class, font_style);
    else
        sprintf(font_str, "<font class=\"%s\">", font_class);
	len = strlen(font_str);
    BUFFERED_OUTPUT(output, font_str, len);
}

/*
static void html_output(char *buf, size_t buflen, buffered_output_t * output)
*/
#define html_output(buf, buflen, output) \
do { \
    size_t _ho_i; \
	const char *_ho_ptr = buf; \
\
    for (_ho_i = 0; _ho_i < buflen; _ho_i++) { \
        switch (_ho_ptr[_ho_i]) { \
        case '&': \
            BUFFERED_OUTPUT(output, "&amp;", 5); \
            break; \
        case '<': \
            BUFFERED_OUTPUT(output, "&lt;", 4); \
            break; \
        case '>': \
            BUFFERED_OUTPUT(output, "&gt;", 4); \
            break; \
        case ' ': \
            BUFFERED_OUTPUT(output, "&nbsp;", 6); \
            break; \
        default: \
            BUFFERED_OUTPUT(output, &(_ho_ptr[_ho_i]), 1); \
        } \
    } \
} while(0)

static void print_raw_ansi(char *buf, size_t buflen, buffered_output_t * output)
{
    size_t i;

    for (i = 0; i < buflen; i++) {
        if (buf[i] == 0x1b)
		{
            html_output("*", 1, output);
		}
        else if (buf[i]=='\n') {
			BUFFERED_OUTPUT(output, " <br /> ", 8);
        } else {
            html_output(&buf[i], 1, output);
		}
    }
}

static void generate_font_style(unsigned int *style, unsigned int *ansi_val, size_t len)
{
    size_t i;
    unsigned int color;

    for (i = 0; i < len; i++) {
        if (ansi_val[i] == 0)
            STYLE_ZERO(*style);
        else if (ansi_val[i] == 1)
            STYLE_SET(*style, FONT_FG_BOLD);
        else if (ansi_val[i] == 4)
            STYLE_SET(*style, FONT_STYLE_UL);
        else if (ansi_val[i] == 5)
            STYLE_SET(*style, FONT_STYLE_BLINK);
        else if (ansi_val[i] >= 30 && ansi_val[i] <= 37) {
            color = ansi_val[i] - 30;
            STYLE_SET_FG(*style, color);
        } else if (ansi_val[i] >= 40 && ansi_val[i] <= 47) {
            /*
             * user explicitly specify background color 
             */
            /*
             * STYLE_SET(*style, FONT_BG_SET); 
             */
            color = ansi_val[i] - 40;
            STYLE_SET_BG(*style, color);
        }
    }
}

/*
static void js_output(char *buf, size_t buflen, buffered_output_t * output)
*/
#define js_output(buf, buflen, output) \
do { \
    size_t _js_i; \
	const char *_js_ptr = buf; \
\
    for (_js_i = 0; _js_i < buflen; _js_i++) { \
        switch (_js_ptr[_js_i]) { \
        case '&': \
            BUFFERED_OUTPUT(output, "&amp;", 5); \
            break; \
        case '<': \
            BUFFERED_OUTPUT(output, "&lt;", 4); \
            break; \
        case '>': \
            BUFFERED_OUTPUT(output, "&gt;", 4); \
            break; \
        case ' ': \
            BUFFERED_OUTPUT(output, "&nbsp;", 6); \
            break; \
        case '\'': \
            BUFFERED_OUTPUT(output, "\\\'", 2); \
            break; \
        case '\\': \
            BUFFERED_OUTPUT(output, "\\\\", 2); \
            break; \
		case '\r': \
            BUFFERED_OUTPUT(output, "\\r", 2); \
			break; \
        default: \
            BUFFERED_OUTPUT(output, &(_js_ptr[_js_i]), 1); \
        } \
    } \
} while (0)

void output_ansi_text(char *buf, size_t buflen, 
							buffered_output_t * output, char* attachlink)
{
    unsigned int font_style = 0;
    unsigned int ansi_state;
    unsigned int ansi_val[STRLEN];
    int ival = 0;
    size_t i;
    char *ansi_begin = NULL;
    char *ansi_end;
    int attachmatched;
	long attachPos[MAXATTACHMENTCOUNT];
	long attachLen[MAXATTACHMENTCOUNT];
	char* attachFileName[MAXATTACHMENTCOUNT];
	enum ATTACHMENTTYPE attachType[MAXATTACHMENTCOUNT];
	int attachShowed[MAXATTACHMENTCOUNT];
	char outbuf[512];
	int outbuf_len;
	size_t article_len = buflen;

    if (buf == NULL)
        return;

    STATE_ZERO(ansi_state);
    bzero(ansi_val, sizeof(ansi_val));
    bzero(attachShowed, sizeof(attachShowed));
    attachmatched = 0;
	if (attachlink != NULL)
	{
		long attach_len;
		char *attachptr, *attachfilename;
		for (i = 0; i < buflen ; i++ )
		{
			if (attachmatched >= MAXATTACHMENTCOUNT)
				break;

			if (((attachfilename = checkattach(buf + i, buflen - i, 
									&attach_len, &attachptr)) != NULL))
			{
				i += (attachptr-buf-i) + attach_len - 1;
				if (i > buflen)
					continue;
				attachPos[attachmatched] = attachfilename - buf;
				attachLen[attachmatched] = attach_len;
				attachFileName[attachmatched] = (char*)malloc(256);
				strncpy(attachFileName[attachmatched], attachfilename, 255);
				attachFileName[attachmatched][255] = '\0';
				attachType[attachmatched] = get_attachment_type(attachfilename);
				attachmatched++;
			}
		}
	}

	if (attachmatched > 0)
		article_len = attachPos[0] - ATTACHMENT_SIZE;

    for (i = 0; i < article_len; i++)
	{
        if (STATE_ISSET(ansi_state, STATE_NEW_LINE)) {
            STATE_CLR(ansi_state, STATE_NEW_LINE);
            if (i < (buflen - 1) && (buf[i] == ':' && buf[i + 1] == ' ')) {
                STATE_SET(ansi_state, STATE_QUOTE_LINE);
                if (STATE_ISSET(ansi_state, STATE_FONT_SET))
                    BUFFERED_OUTPUT(output, "</font>", 7);
                /*
                 * set quoted line styles 
                 */
                STYLE_SET(font_style, FONT_STYLE_QUOTE);
                STYLE_SET_FG(font_style, FONT_COLOR_QUOTE);
                STYLE_CLR_BG(font_style);
                print_font_style(font_style, output);
                BUFFERED_OUTPUT(output, &buf[i], 1);
                STATE_SET(ansi_state, STATE_FONT_SET);
                STATE_CLR(ansi_state, STATE_ESC_SET);
                /*
                 * clear ansi_val[] array 
                 */
                bzero(ansi_val, sizeof(ansi_val));
                ival = 0;
                continue;
            } else
                STATE_CLR(ansi_state, STATE_QUOTE_LINE);
        }
        if (buf[i] == 0x1b) {
            STATE_SET(ansi_state, STATE_ESC_SET);
        }
		else if (STATE_ISSET(ansi_state, STATE_ESC_SET))
		{
            if (isalpha(buf[i]))
				STATE_CLR(ansi_state, STATE_ESC_SET);
        }
		else if (buf[i] == '\n')
		{
            if (STATE_ISSET(ansi_state, STATE_ESC_SET)) {
                /*
                 *[\n or *[13;24\n */
                size_t len;

                ansi_end = &buf[i - 1];
                len = ansi_end - ansi_begin + 1;
                /*print_raw_ansi(ansi_begin, len, output);*/
                STATE_CLR(ansi_state, STATE_ESC_SET);
            }
            if (STATE_ISSET(ansi_state, STATE_QUOTE_LINE)) {
                /*
                 * end of a quoted line 
                 */
                BUFFERED_OUTPUT(output, "</font>", 7);
                STYLE_CLR(font_style, FONT_STYLE_QUOTE);
                STATE_CLR(ansi_state, STATE_FONT_SET);
            }
            BUFFERED_OUTPUT(output, "<br />\n", 7);
            STATE_CLR(ansi_state, STATE_QUOTE_LINE);
            STATE_SET(ansi_state, STATE_NEW_LINE);
        }
		else
			print_raw_ansi(&buf[i], 1, output);
    }
    if (STATE_ISSET(ansi_state, STATE_FONT_SET)) {
        BUFFERED_OUTPUT(output, "</font>", 7);
        STATE_CLR(ansi_state, STATE_FONT_SET);
    }
	for ( i = 0; i<attachmatched ; i++ ){
		if (!attachShowed[i]) { 
			switch(attachType[i]) {
			case ATTACH_IMG:
		 		snprintf(outbuf, 511, "<br /><img src=\"images/img.gif\" border=\"0\" />此主题相关图片如下：%s (%ld 字节)<br /><a href=\"%s&amp;ap=%ld\" target=\"_blank\"><img src=\"%s&amp;ap=%ld\" border=\"0\" title=\"按此在新窗口浏览图片\" onload=\"javascript:resizeImg(this)\" /></a> ",attachFileName[i], attachLen[i], attachlink, attachPos[i],attachlink, attachPos[i]);
				break;
			case ATTACH_FLASH:
		        snprintf(outbuf, 511, "<br />Flash动画: " "<a href=\"%s&amp;ap=%ld\">%s</a> (%ld 字节)<br />" "<object classid=\"clsid:D27CDB6E-AE6D-11cf-96B8-444553540000\" codebase=\"http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=5,0,0,0\" > <param name=\"MOVIE\" value=\"%s&amp;ap=%ld\" />" "<embed src=\"%s&amp;ap=%ld\"></embed></object><br />", attachlink, attachPos[i], attachFileName[i], attachLen[i], attachlink, attachPos[i], attachlink, attachPos[i]);
				break;
			case ATTACH_OTHERS:
				 snprintf(outbuf, 511, "<br />附件: <a href=\"%s&amp;ap=%ld\">%s</a> (%ld 字节)<br />", attachlink, attachPos[i], attachFileName[i], attachLen[i]);
				 break;
			}	
			outbuf_len = strlen(outbuf);
			BUFFERED_OUTPUT(output, outbuf, outbuf_len);
			attachShowed[i]=1;
		}
		free(attachFileName[i]);
	}

    BUFFERED_FLUSH(output);

}

#define JS_BUFFERED_OUTPUT(buf, buflen, output) \
do { \
    size_t _jbo_i; \
	const char *_jbo_ptr = buf; \
\
    for (_jbo_i = 0; _jbo_i < buflen; _jbo_i++) { \
        switch (_jbo_ptr[_jbo_i]) { \
        case '\'': \
            BUFFERED_OUTPUT(output, "\\\'", 2); \
            break; \
        case '\\': \
            BUFFERED_OUTPUT(output, "\\\\", 2); \
            break; \
		case '\r': \
            BUFFERED_OUTPUT(output, "\\r", 2); \
			break; \
        default: \
            BUFFERED_OUTPUT(output, &(_jbo_ptr[_jbo_i]), 1); \
        } \
    } \
} while (0)

void output_ansi_javascript(char *buf, size_t buflen, 
							buffered_output_t * output, char* attachlink)
{
    unsigned int font_style = 0;
    unsigned int ansi_state;
    unsigned int ansi_val[STRLEN];
    int ival = 0;
    size_t i;
    char *ansi_begin = NULL;
    char *ansi_end;
    int attachmatched;
	long attachPos[MAXATTACHMENTCOUNT];
	long attachLen[MAXATTACHMENTCOUNT];
	char* attachFileName[MAXATTACHMENTCOUNT];
	enum ATTACHMENTTYPE attachType[MAXATTACHMENTCOUNT];
	int attachShowed[MAXATTACHMENTCOUNT];
	char outbuf[512];
	int outbuf_len;
	size_t article_len = buflen;

    if (buf == NULL)
        return;

    STATE_ZERO(ansi_state);
    bzero(ansi_val, sizeof(ansi_val));
    bzero(attachShowed, sizeof(attachShowed));
    attachmatched = 0;
	if (attachlink != NULL)
	{
		long attach_len;
		char *attachptr, *attachfilename;
		for (i = 0; i < buflen ; i++ )
		{
			if (attachmatched >= MAXATTACHMENTCOUNT)
				break;

			if (((attachfilename = checkattach(buf + i, buflen - i, 
									&attach_len, &attachptr)) != NULL))
			{
				i += (attachptr-buf-i) + attach_len - 1;
				if (i > buflen)
					continue;
				attachPos[attachmatched] = attachfilename - buf;
				attachLen[attachmatched] = attach_len;
				attachFileName[attachmatched] = (char*)malloc(256);
				strncpy(attachFileName[attachmatched], attachfilename, 255);
				attachFileName[attachmatched][255] = '\0';
                attachType[attachmatched] = get_attachment_type(attachfilename);
				attachmatched++;
			}
		}
	}

	if (attachmatched > 0)
		article_len = attachPos[0] - ATTACHMENT_SIZE;

	BUFFERED_OUTPUT(output, "document.write('", 16);
    for (i = 0; i < article_len; i++)
	{
        if (STATE_ISSET(ansi_state, STATE_NEW_LINE)) {
			BUFFERED_OUTPUT(output, "document.write('", 16);
            STATE_CLR(ansi_state, STATE_NEW_LINE);
            if (i < (buflen - 1) && (buf[i] == ':' && buf[i + 1] == ' ')) {
                STATE_SET(ansi_state, STATE_QUOTE_LINE);
                if (STATE_ISSET(ansi_state, STATE_FONT_SET))
                    BUFFERED_OUTPUT(output, " </font>", 8);
                /*
                 * set quoted line styles 
                 */
                STYLE_SET(font_style, FONT_STYLE_QUOTE);
                STYLE_SET_FG(font_style, FONT_COLOR_QUOTE);
                STYLE_CLR_BG(font_style);
                print_font_style(font_style, output);
                BUFFERED_OUTPUT(output, &buf[i], 1);
                STATE_SET(ansi_state, STATE_FONT_SET);
                STATE_CLR(ansi_state, STATE_ESC_SET);
                /*
                 * clear ansi_val[] array 
                 */
                bzero(ansi_val, sizeof(ansi_val));
                ival = 0;
                continue;
            } else
                STATE_CLR(ansi_state, STATE_QUOTE_LINE);
        }
        if (buf[i] == 0x1b) {
            STATE_SET(ansi_state, STATE_ESC_SET);
        }
		else if (STATE_ISSET(ansi_state, STATE_ESC_SET))
		{
            if (isalpha(buf[i]))
				STATE_CLR(ansi_state, STATE_ESC_SET);
        }
		else if (buf[i] == '\n')
		{
            if (STATE_ISSET(ansi_state, STATE_ESC_SET)) {
                /*
                 *[\n or *[13;24\n */
                size_t len;

                ansi_end = &buf[i - 1];
                len = ansi_end - ansi_begin + 1;
                /*print_raw_ansi(ansi_begin, len, output);*/
                STATE_CLR(ansi_state, STATE_ESC_SET);
            }
            if (STATE_ISSET(ansi_state, STATE_QUOTE_LINE)) {
                /*
                 * end of a quoted line 
                 */
                BUFFERED_OUTPUT(output, " </font>", 8);
                STYLE_CLR(font_style, FONT_STYLE_QUOTE);
                STATE_CLR(ansi_state, STATE_FONT_SET);
            }
            BUFFERED_OUTPUT(output, " <br/>');\n", 10);
            STATE_CLR(ansi_state, STATE_QUOTE_LINE);
            STATE_SET(ansi_state, STATE_NEW_LINE);
        }
		else
			js_output(&buf[i], 1, output);
    }
    if (STATE_ISSET(ansi_state, STATE_FONT_SET)) {
        BUFFERED_OUTPUT(output, " </font>", 8);
        STATE_CLR(ansi_state, STATE_FONT_SET);
    }
	if (!STATE_ISSET(ansi_state, STATE_NEW_LINE)) {
		BUFFERED_OUTPUT(output, " <br/>');\n", 10);
	}
	for ( i = 0; i<attachmatched ; i++ ){
		if (!attachShowed[i]) { 
			switch(attachType[i]) {
			case ATTACH_IMG:
		 		snprintf(outbuf, 511, "<br /><img src=\"images/img.gif\" border=\"0\" />此主题相关图片如下：%s (%ld 字节)<br /><a href=\"%s&amp;ap=%ld\" target=\"_blank\"><img src=\"%s&amp;ap=%ld\" border=\"0\" title=\"按此在新窗口浏览图片\" onload=\"javascript:resizeImg(this)\" /></a> ",attachFileName[i], attachLen[i], attachlink, attachPos[i],attachlink, attachPos[i]);
				break;
			case ATTACH_FLASH:
		        snprintf(outbuf, 511, "<br />Flash动画: " "<a href=\"%s&amp;ap=%ld\">%s</a> (%ld 字节)<br />" "<object classid=\"clsid:D27CDB6E-AE6D-11cf-96B8-444553540000\" codebase=\"http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=5,0,0,0\" > <param name=\"MOVIE\" value=\"%s&amp;ap=%ld\" />" "<embed src=\"%s&amp;ap=%ld\"></embed></object><br />", attachlink, attachPos[i], attachFileName[i], attachLen[i], attachlink, attachPos[i], attachlink, attachPos[i]);
				break;
			case ATTACH_OTHERS:
				 snprintf(outbuf, 511, "<br />附件: <a href=\"%s&amp;ap=%ld\">%s</a> (%ld 字节)<br />", attachlink, attachPos[i], attachFileName[i], attachLen[i]);
				 break;
			}	
			BUFFERED_OUTPUT(output, "document.write('", 16);
			outbuf_len = strlen(outbuf);
			JS_BUFFERED_OUTPUT(outbuf, outbuf_len, output);
			BUFFERED_OUTPUT(output, "');\n", 4);
			attachShowed[i]=1;
		}
		free(attachFileName[i]);
	}

    BUFFERED_FLUSH(output);

}


/**
 * Warning: Use of this function is deprecated. It's kept only for compatible
 * purpose. Use output_ansi_text() instead.
 *
 * This function is a core function in wForum, do not modify unless you know what you are doing. - atppp
 *
 * 原来的 UBB 处理代码过于 buggy，现在全盘去掉换成只支持解析 [upload=%d][/upload] 一种，如果以后 ubb 替换要放到这里，
 * 可以参考这个时间点之前的 CVS 代码。- atppp 200408
 */
void output_ansi_html(char *buf, size_t buflen, buffered_output_t * output,char* attachlink, int is_tex, char* preview_attach_dir)
{
    unsigned int font_style = 0;
    unsigned int ansi_state;
    unsigned int ansi_val[STRLEN];
    int ival = 0;
    size_t i;
    char *ansi_begin = 0;
    char *ansi_end;
    int attachmatched;
    long attachPos[MAXATTACHMENTCOUNT];
    long attachLen[MAXATTACHMENTCOUNT];
    char* attachFileName[MAXATTACHMENTCOUNT];
    enum ATTACHMENTTYPE attachType[MAXATTACHMENTCOUNT];
    int attachShowed[MAXATTACHMENTCOUNT];
    char outbuf[512];
    int outbuf_len;
    size_t article_len = buflen;

    /* hardcode everything for supporting [upload=%d][/upload], hehe. - atppp */
    char *UBBUpload1 = "upload=";
    int UBBUpload1_len = 7;
    char *UBBUpload2 = "][/upload]";
    int UBBUpload2_len = 10;

    if (buf == NULL)
        return;

    STATE_ZERO(ansi_state);
    bzero(ansi_val, sizeof(ansi_val));
    bzero(attachShowed, sizeof(attachShowed));
    attachmatched = 0;
    if (attachlink != NULL)
    {
        long attach_len;
        char *attachptr, *attachfilename;
        if (preview_attach_dir) {
            char filepath[MAXPATH];
            char inputbuf[256];
            FILE *fp2;
            snprintf(filepath, MAXPATH, "%s/.index", preview_attach_dir);
            if ((fp2 = fopen(filepath, "r")) != NULL) {
                while (!feof(fp2)) {
                    char *name;
                    char *ptr;
                    struct stat st;

                    if (attachmatched >= MAXATTACHMENTCOUNT)
                        break;                    

                    fgets(inputbuf, 256, fp2);
                    name = strchr(inputbuf, ' ');
                    if (name == NULL)
                        continue;
                    *name = 0;
                    name++;
                    ptr = strchr(name, '\n');
                    if (ptr)
                        *ptr = 0;

                    attachPos[attachmatched] = attachmatched+1;
                    if (stat(inputbuf, &st) < 0) attachLen[attachmatched] = 0;
                    else attachLen[attachmatched] = st.st_size;
                    attachFileName[attachmatched] = (char*)malloc(256);
                    strncpy(attachFileName[attachmatched], name, 255);
                    attachFileName[attachmatched][255] = '\0';
                    attachType[attachmatched] = get_attachment_type(name);

                    attachmatched++;
                }
    			fclose(fp2);
            }
        } else {
            for (i = 0; i < buflen ; i++ )
            {
                if (attachmatched >= MAXATTACHMENTCOUNT)
                    break;

                if (((attachfilename = checkattach(buf + i, buflen - i, 
                                        &attach_len, &attachptr)) != NULL))
                {
                    i += (attachptr-buf-i) + attach_len - 1;
                    if (i > buflen)
                        continue;
                    attachPos[attachmatched] = attachfilename - buf;
                    attachLen[attachmatched] = attach_len;
                    attachFileName[attachmatched] = (char*)malloc(256);
                    strncpy(attachFileName[attachmatched], attachfilename, 255);
                    attachFileName[attachmatched][255] = '\0';
                    attachType[attachmatched] = get_attachment_type(attachfilename);
                    attachmatched++;
                }
            }
            if (attachmatched > 0)
                article_len = attachPos[0] - ATTACHMENT_SIZE;
        }
    }


    for (i = 0; i < article_len; i++) {
        if (STATE_ISSET(ansi_state, STATE_NEW_LINE)) {
            STATE_CLR(ansi_state, STATE_NEW_LINE);
            if (i < (buflen - 1) && !STATE_ISSET(ansi_state,STATE_TEX_SET) && (buf[i] == ':' && buf[i + 1] == ' ')) {
                STATE_SET(ansi_state, STATE_QUOTE_LINE);
                if (STATE_ISSET(ansi_state, STATE_FONT_SET))
                    BUFFERED_OUTPUT(output, "</font>", 7);
                /*
                 * set quoted line styles 
                 */
                STYLE_SET(font_style, FONT_STYLE_QUOTE);
                STYLE_SET_FG(font_style, FONT_COLOR_QUOTE);
                STYLE_CLR_BG(font_style);
                print_font_style(font_style, output);
                BUFFERED_OUTPUT(output, &buf[i], 1);
                STATE_SET(ansi_state, STATE_FONT_SET);
                STATE_CLR(ansi_state, STATE_ESC_SET);
                /*
                 * clear ansi_val[] array 
                 */
                bzero(ansi_val, sizeof(ansi_val));
                ival = 0;
                continue;
            } else
                STATE_CLR(ansi_state, STATE_QUOTE_LINE);
        }
        if (is_tex) {
            if ((buf[i] == '$') && ((i==0) || (buf[i-1] != '\\'))) {
                if (STATE_ISSET(ansi_state, STATE_TEX_SET)) {
                    STATE_CLR(ansi_state, STATE_TEX_SET);
                } else {
                    if (STATE_ISSET(ansi_state, STATE_QUOTE_LINE)) {
                        BUFFERED_OUTPUT(output, "</font>", 7);
                        STYLE_CLR(font_style, FONT_STYLE_QUOTE);
                        STATE_CLR(ansi_state, STATE_QUOTE_LINE);
                        STATE_CLR(ansi_state, STATE_FONT_SET);
                    } else if (STATE_ISSET(ansi_state, STATE_FONT_SET)) {
                        BUFFERED_OUTPUT(output, "</font>", 7);
                        STATE_CLR(ansi_state, STATE_FONT_SET);
                    }
                    STATE_SET(ansi_state, STATE_TEX_SET);
                }
            }
        }
        /*
        * is_tex 情况下，\[upload 优先匹配 \[ 而不是 [upload
        * is_tex 情况下应该还有一个问题是 *[\[ 等，不过暂时不管了 - atppp
        */
        if (i < (buflen - 1) && is_tex && !STATE_ISSET(ansi_state,STATE_ESC_SET) && (buf[i] == '\\' && buf[i + 1] == '[')) {
            if (STATE_ISSET(ansi_state, STATE_QUOTE_LINE)) {
                BUFFERED_OUTPUT(output, "</font>", 7);
                STYLE_CLR(font_style, FONT_STYLE_QUOTE);
                STATE_CLR(ansi_state, STATE_QUOTE_LINE);
                STATE_CLR(ansi_state, STATE_FONT_SET);
            } else if (STATE_ISSET(ansi_state, STATE_FONT_SET)) {
                BUFFERED_OUTPUT(output, "</font>", 7);
                STATE_CLR(ansi_state, STATE_FONT_SET);
            }
            STATE_SET(ansi_state, STATE_TEX_SET);
        }
        if (buf[i] == '[' && !STATE_ISSET(ansi_state, STATE_ESC_SET)) {
            if (is_tex && (i > 0) && (buf[i-1] == '\\')) {
            } else if (attachmatched > 0) {
                /*
                 * determine if this is correct attachment [upload], if correct, output html, and skip i to the end of [/upload]
                 * otherwise, do nothing and keep i unchanged
                 */
                int num = 0;
                int k;
                char *buf_p;
                char *end_buf_p;
                char *UBB_p;
                buf_p = &buf[i];
                end_buf_p = &buf[buflen];
                buf_p++;
                if (buf_p + UBBUpload1_len - 1 >= end_buf_p) {
                    goto atppp_never_use_goto;
                }
                for (UBB_p = UBBUpload1, k = 0; k < UBBUpload1_len; k++, UBB_p++, buf_p++) {
                    if (*UBB_p != *buf_p) {
                        goto atppp_never_use_goto;
                    }
                }
                while(buf_p < end_buf_p) {
                    if ((*buf_p >= '0') && (*buf_p <= '9')) {
                        num = num * 10 + (*buf_p - '0');
                    } else {
                        break;
                    }
                    buf_p++;
                }
                if (buf_p + UBBUpload2_len - 1 >= end_buf_p) {
                    goto atppp_never_use_goto;
                }
                for (UBB_p = UBBUpload2, k = 0; k < UBBUpload2_len; k++, UBB_p++, buf_p++) {
                    if (*UBB_p != *buf_p) {
                        goto atppp_never_use_goto;
                    }
                }
                if ((num > 0) && (num <= attachmatched)) {
                    switch(attachType[num-1]) {
                    case ATTACH_IMG:
                        snprintf(outbuf, 511, "<a href=\"%s&amp;ap=%ld\" target=\"_blank\"><img src=\"%s&amp;ap=%ld\" border=\"0\" title=\"按此在新窗口浏览图片\" onload=\"javascript:resizeImg(this)\" /></a> ", attachlink, attachPos[num-1],attachlink, attachPos[num-1]);
                        break;
                    case ATTACH_FLASH:
                        snprintf(outbuf, 511, "<object classid=\"clsid:D27CDB6E-AE6D-11cf-96B8-444553540000\" codebase=\"http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=5,0,0,0\" > <param name=\"MOVIE\" value=\"%s&amp;ap=%ld\" />" "<embed src=\"%s&amp;ap=%ld\"></embed></object>", attachlink, attachPos[num-1], attachlink, attachPos[num-1]);
                        break;
                    case ATTACH_OTHERS:
                         snprintf(outbuf, 511, "<br />附件: <a href=\"%s&amp;ap=%ld\">%s</a> (%ld 字节)<br />", attachlink, attachPos[num-1], attachFileName[num-1], attachLen[num-1]);
                         break;
                    }   
                    outbuf_len = strlen(outbuf);
                    BUFFERED_OUTPUT(output, outbuf, outbuf_len);
                    attachShowed[num-1]=1;
                    i = buf_p - buf - 1;
                    continue;
                }
atppp_never_use_goto:
                ;
            }
        } else if (is_tex && (buf[i] == ']') && (i > 0) && (buf[i-1] == '\\')) {
            STATE_CLR(ansi_state, STATE_TEX_SET);
        }
        if (i < (buflen - 1) && !STATE_ISSET(ansi_state,STATE_TEX_SET) && (buf[i] == 0x1b && buf[i + 1] == '[')) {
            if (STATE_ISSET(ansi_state, STATE_ESC_SET)) {
                /*
                 *[*[ or *[13;24*[ */
                size_t len;

                ansi_end = &buf[i - 1];
                len = ansi_end - ansi_begin + 1;
                print_raw_ansi(ansi_begin, len, output);
            }
            STATE_SET(ansi_state, STATE_ESC_SET);
            ansi_begin = &buf[i];
            i++;                /* skip the next '[' character */
        } else if (buf[i] == '\n') {
            if (STATE_ISSET(ansi_state, STATE_ESC_SET)) {
                /*
                 *[\n or *[13;24\n */
                size_t len;

                ansi_end = &buf[i - 1];
                len = ansi_end - ansi_begin + 1;
                print_raw_ansi(ansi_begin, len, output);
                STATE_CLR(ansi_state, STATE_ESC_SET);
            }
            if (STATE_ISSET(ansi_state, STATE_QUOTE_LINE)) {
                /*
                 * end of a quoted line 
                 */
                BUFFERED_OUTPUT(output, "</font>", 7);
                STYLE_CLR(font_style, FONT_STYLE_QUOTE);
                STATE_CLR(ansi_state, STATE_FONT_SET);
            }
            if (!STATE_ISSET(ansi_state,STATE_TEX_SET)) {
                BUFFERED_OUTPUT(output, " <br /> ", 8);
            }
            STATE_CLR(ansi_state, STATE_QUOTE_LINE);
            STATE_SET(ansi_state, STATE_NEW_LINE);
        } else {
            if (STATE_ISSET(ansi_state, STATE_ESC_SET)) {
                if (buf[i] == 'm') {
                    /*
                     *[0;1;4;31m */
                    if (STATE_ISSET(ansi_state, STATE_FONT_SET)) {
                        BUFFERED_OUTPUT(output, "</font>", 7);
                        STATE_CLR(ansi_state, STATE_FONT_SET);
                    }
                    if (i < buflen - 1) {
                        generate_font_style(&font_style, ansi_val, ival + 1);
                        if (STATE_ISSET(ansi_state, STATE_QUOTE_LINE))
                            STYLE_SET(font_style, FONT_STYLE_QUOTE);
                        print_font_style(font_style, output);
                        STATE_SET(ansi_state, STATE_FONT_SET);
                        STATE_CLR(ansi_state, STATE_ESC_SET);
                        /*
                         * STYLE_ZERO(font_style);
                         */
                        /*
                         * clear ansi_val[] array 
                         */
                        bzero(ansi_val, sizeof(ansi_val));
                        ival = 0;
                    }
                } else if (isalpha(buf[i])) {
                    /*
                     *[23;32H */
                    /*
                     * ignore it 
                     */
                    STATE_CLR(ansi_state, STATE_ESC_SET);
                    STYLE_ZERO(font_style);
                    /*
                     * clear ansi_val[] array 
                     */
                    bzero(ansi_val, sizeof(ansi_val));
                    ival = 0;
                    continue;
                } else if (buf[i] == ';') {
                    if (ival < sizeof(ansi_val) - 1) {
                        ival++; /* go to next ansi_val[] element */
                        ansi_val[ival] = 0;
                    }
                } else if (buf[i] >= '0' && buf[i] <= '9') {
                    ansi_val[ival] *= 10;
                    ansi_val[ival] += (buf[i] - '0');
                } else {
                    /*
                     *[1;32/XXXX or *[* or *[[ */
                    /*
                     * not a valid ANSI string, just output it 
                     */
                    size_t len;

                    ansi_end = &buf[i];
                    len = ansi_end - ansi_begin + 1;
                    print_raw_ansi(ansi_begin, len, output);
                    STATE_CLR(ansi_state, STATE_ESC_SET);
                    /*
                     * clear ansi_val[] array 
                     */
                    bzero(ansi_val, sizeof(ansi_val));
                    ival = 0;
                }

            } else
                print_raw_ansi(&buf[i], 1, output);
        }
    }
    if (STATE_ISSET(ansi_state, STATE_FONT_SET)) {
        BUFFERED_OUTPUT(output, "</font>", 7);
        STATE_CLR(ansi_state, STATE_FONT_SET);
    }
    for ( i = 0; i<attachmatched ; i++ ){
        if (!attachShowed[i]) { 
            switch(attachType[i]) {
            case ATTACH_IMG:
                snprintf(outbuf, 511, "<br /><img src=\"images/img.gif\" border=\"0\" />此主题相关图片如下：%s (%ld 字节)<br /><a href=\"%s&amp;ap=%ld\" target=\"_blank\"><img src=\"%s&amp;ap=%ld\" border=\"0\" title=\"按此在新窗口浏览图片\" onload=\"javascript:resizeImg(this)\" /></a> ",attachFileName[i], attachLen[i], attachlink, attachPos[i],attachlink, attachPos[i]);
                break;
            case ATTACH_FLASH:
                snprintf(outbuf, 511, "<br />Flash动画: " "<a href=\"%s&amp;ap=%ld\">%s</a> (%ld 字节)<br />" "<object classid=\"clsid:D27CDB6E-AE6D-11cf-96B8-444553540000\" codebase=\"http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=5,0,0,0\" > <param name=\"MOVIE\" value=\"%s&amp;ap=%ld\" />" "<embed src=\"%s&amp;ap=%ld\"></embed></object><br />", attachlink, attachPos[i], attachFileName[i], attachLen[i], attachlink, attachPos[i], attachlink, attachPos[i]);
                break;
            case ATTACH_OTHERS:
                 snprintf(outbuf, 511, "<br />附件: <a href=\"%s&amp;ap=%ld\">%s</a> (%ld 字节)<br />", attachlink, attachPos[i], attachFileName[i], attachLen[i]);
                 break;
            }
            outbuf_len = strlen(outbuf);
            BUFFERED_OUTPUT(output, outbuf, outbuf_len);
            attachShowed[i]=1;
        }
        free(attachFileName[i]);
    }

    BUFFERED_FLUSH(output);

}

/* ent 是 1-based 的*/
int del_post(int ent, struct fileheader *fileinfo, const struct boardheader *bh)
{
    struct write_dir_arg delarg;
    int ret;
    char direct[PATHLEN];

    ret = deny_del_article(bh, fileinfo, getSession());
    switch(ret) {
        case -3:
            return 4;
            break;
        case -6:
            return 1;
            break;
        default:
            if (ret)
                return 1;
            break;
    }
    init_write_dir_arg(&delarg);
    setbdir(DIR_MODE_NORMAL, direct, bh->filename);
    delarg.filename=direct;
    delarg.ent=ent;
    ret=do_del_post(getCurrentUser(),&delarg,fileinfo,bh->filename,DIR_MODE_NORMAL,0,getSession());
    free_write_dir_arg(&delarg);
    if (ret != 0)
        return 3;
    return 0;

}



