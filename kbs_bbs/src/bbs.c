/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
/* ËùÓĞ µÄ×¢ÊÍ ÓÉ Alex&Sissi Ìí¼Ó £¬ alex@mars.net.edu.cn */

#include "bbs.h"
#include "read.h"
#include <time.h>
#include <dlfcn.h>
#include "md5.h"

/*#include "../SMTH2000/cache/cache.h"*/

int scrint = 0;
int local_article = 0;  //0:×ªĞÅ°æÃæÄ¬ÈÏ×ªĞÅ£¬1:·Ç×ªĞÅ°æÃæ£¬2:×ªĞÅ°æÃæÄ¬ÈÏ²»×ªĞÅ - atppp
int readpost;
int helpmode = 0;
struct boardheader* currboard=NULL;
int currboardent;
char currBM[BM_LEN - 1];
int selboard = 0;
int check_upload = 0; //·¢±íÎÄÕÂÊ±ÊÇ·ñÒª¼ì²éÌí¼Ó¸½¼ş

int Anony;
char genbuf[1024];
unsigned int tmpuser = 0;
char quote_title[120], quote_board[120];
char quote_user[120];

#ifndef NOREPLY
char replytitle[STRLEN];
#endif

char *filemargin();

/* bad 2002.8.1 */

int deny_user();

int m_template();

/*int     b_jury_edit();  stephen 2001.11.1*/
int add_author_friend();
int m_read();                   /*Haohmaru.2000.2.25 */
int Goodbye();

void RemoveAppendedSpace();     /* Leeward 98.02.13 */
extern time_t login_start_time;

extern struct screenline *big_picture;
extern struct userec *user_data;
int post_reply(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg);
extern int b_vote(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg);
extern int b_vote_maintain(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg);
extern int b_jury_edit(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg);
extern int mainreadhelp(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg);

int check_readonly(char *checked)
{                               /* Leeward 98.03.28 */
    if (checkreadonly(checked)) {       /* Checking if DIR access mode is "555" */
        if (!strcmp(currboard->filename,checked)) {
            move(0, 0);
            clrtobot();
            move(8, 0);
            prints("                                        "); /* 40 spaces */
            move(8, (80 - (24 + strlen(checked))) / 2); /* Set text in center */
            prints("\033[1m\033[33mºÜ±§Ç¸£º\033[31m%s °æÄ¿Ç°ÊÇÖ»¶ÁÄ£Ê½\033[33m\n\n                          Äú²»ÄÜÔÚ¸Ã°æ·¢±í»òÕßĞŞ¸ÄÎÄÕÂ\033[m\n", checked);
            pressreturn();
            clear();
        }
        return true;
    } else
        return false;
}

/* Undel an article   pig2532 2005.12.18 */

/* etnlegend, 2006.10.06, Ôö¼ÓÓÃ»§´¦Àí×ÔÉ¾ÎÄÕÂ¹¦ÄÜ... */
int UndeleteArticle(struct _select_def *conf,struct fileheader *fh, void *varg){
    struct read_arg *arg=(struct read_arg*)conf->arg;
    /* ²»ÔÙÔÚ´Ë´¦¼ì²â°æÖ÷È¨ÏŞ, ´¦Àí DIR_MODE µÄµØ·½À´×öÕâ¼şÊÂÇé... */
    if(!fh||!(arg->mode==DIR_MODE_DELETED||arg->mode==DIR_MODE_JUNK
#ifndef DENY_SELF_UNDELETE
        /* ÊÇ·ñÔÊĞíÓÃ»§»Ö¸´×ÔÉ¾ÎÄÕÂÓÉ DENY_SELF_UNDELETE ºê¿ØÖÆ... */
        ||arg->mode==DIR_MODE_SELF
#endif /* DENY_SELF_UNDELETE */
        ))
        return DONOTHING;
    switch(do_undel_post(currboard->filename,arg->direct,conf->pos,fh,NULL,getSession())){
        case -1:
            move(t_lines-1,0);
            clrtoeol();
            prints("\033[1;31;47m\t%s\033[K\033[m","´ËÎÄÕÂÒÑ±»»Ö¸´»òÁĞ±í´íÎó, °´ <Enter> ¼ü¼ÌĞø...");
            WAIT_RETURN;
            break;
        case 1:
            move(t_lines-1,0);
            clrtoeol();
            prints("\033[1;34;47m\t%s\033[K\033[m","»Ö¸´³É¹¦, °´ <Enter> ¼ü¼ÌĞø!");
            WAIT_RETURN;
            break;
        default:
            return DONOTHING;
    }
    return FULLUPDATE;
}

int check_stuffmode()
{
    if (uinfo.mode == RMAIL)
        return true;
    else
        return false;
}

/*Add by SmallPig*/
int shownotepad()
{                               /* ÏÔÊ¾ notepad */
    modify_user_mode(NOTEPAD);
    ansimore("etc/notepad", true);
    clear();
    return 1;
}

void printutitle()
{                               /* ÆÁÄ»ÏÔÊ¾ ÓÃ»§ÁĞ±í title */
    /*---	modified by period	2000-11-02	hide posts/logins	---*/

    move(2, 0);
    clrtoeol();
    prints(
              "\033[44m ±à ºÅ  Ê¹ÓÃÕß´úºÅ     %-19s  #ÉÏÕ¾ #ÎÄÕÂ %4s    ×î½ü¹âÁÙÈÕÆÚ   \033[m\n",
#if defined(ACTS_REALNAMES)
              "ÕæÊµĞÕÃû",
#else
              "Ê¹ÓÃÕßêÇ³Æ",
#endif
              "Éí·İ");
}


int g_board_names(struct boardheader *fhdrp,void* arg)
{
    if (check_read_perm(getCurrentUser(), fhdrp)) {
        AddNameList(fhdrp->filename);
    }
    return 0;
}

void make_blist(int addfav)
{                               /* ËùÓĞ°æ °æÃû ÁĞ±í */
    CreateNameList();
    apply_boards((int (*)()) g_board_names,NULL);
	if(addfav){
		int i;
		for( i=0; i<bdirshm->allbrd_list_t; i++){
			if( bdirshm->allbrd_list[i].ename[0] && HAS_PERM(getCurrentUser(), bdirshm->allbrd_list[i].level) )
				AddNameList( bdirshm->allbrd_list[i].ename );
		}
	}
}

int ReadBoard(void){
    int returnmode;
    while(1){
        returnmode=Read();
        if((returnmode==-2)||(returnmode==CHANGEMODE)){ //is directory or select another board
            if(currboard->flag&BOARD_GROUP){
                choose_board(0,NULL,currboardent,0);
                break;
            }
        }
        else
            break;
    }
    return 0;
}

/* etnlegend, 2006.04.07, ĞÂÑ¡ÔñÔÄ¶ÁÌÖÂÛÇø... */
int Select(void){
    return (do_select(NULL,NULL,NULL)==CHANGEMODE?ReadBoard():0);
}

int Post()
{                               /* Ö÷²Ëµ¥ÄÚµÄ ÔÚµ±Ç°°æ POST ÎÄÕÂ */
    if (!selboard) {
        prints("\n\nÏÈÓÃ (S)elect È¥Ñ¡ÔñÒ»¸öÌÖÂÛÇø¡£\n");
        pressreturn();          /* µÈ´ı°´return¼ü */
        clear();
        return 0;
    }
#ifndef NOREPLY
    *replytitle = '\0';
#endif
    do_post(NULL,NULL,NULL);
    return 0;
}

int get_a_boardname(char *bname, char *prompt)
{                               /* ÊäÈëÒ»¸ö°æÃû */
    /*
     * struct boardheader fh; 
     */

    make_blist(0);
    namecomplete(prompt, bname);        /* ¿ÉÒÔ×Ô¶¯ËÑË÷ */
    if (*bname == '\0') {
        return 0;
    }
    /*---	Modified by period	2000-10-29	---*/
    if (getbnum_safe(bname,getSession()) <= 0)
        /*---	---*/
    {
        move(1, 0);
        prints("´íÎóµÄÌÖÂÛÇøÃû³Æ\n");
        pressreturn();
        move(1, 0);
        return 0;
    }
    return 1;
}

int top_move(struct _select_def* conf,struct fileheader *fileinfo)
{
    struct read_arg* arg=(struct read_arg*)conf->arg;
    char ans[3];
	int d;
	int alltop;
	int ret;

	alltop = arg->boardstatus->toptitle;
	if (alltop <= 0)
		return DONOTHING;
	getdata(t_lines - 1, 0, "ÒÆ¶¯µ½ĞÂÎ»ÖÃ: ", ans, 3, DOECHO, NULL, true);
	if (ans[0] == '\0' || ans[0] == '\n' || ans[0] == '\r')
		return SHOW_REFRESH;
	d = atoi(ans);
	if (d < 1)
		d = 1;
	else if (d > alltop)
		d = alltop;
	if (d == conf->pos - arg->filecount)
		return SHOW_REFRESH;

	ret = move_record(arg->dingdirect, sizeof(struct fileheader), conf->pos-arg->filecount, d,
					(RECORD_FUNC_ARG) cmpname, fileinfo->filename);
	if (ret==0) {
		ret=DIRCHANGED;
		board_update_toptitle(arg->bid, true);
	} else {
		char buf[STRLEN];
		a_prompt(-1, "²Ù×÷Ê§°Ü, Çë°´ Enter ¼ÌĞø << ", buf);
		ret=SHOW_REFRESH;
		board_update_toptitle(arg->bid, true);
	}
	return ret;
}

int set_article_flag(struct _select_def* conf,struct fileheader *fileinfo,long flag)
{
    struct read_arg* arg=(struct read_arg*)conf->arg;
    bool    isbm=chk_currBM(currboard->BM, getCurrentUser());
    struct write_dir_arg dirarg;
    struct fileheader data;
    int ret;
    struct actionlist{
        int action;
        int pos;
        long flag;
        char* name;
    };
    const struct actionlist *ptr;
    static const struct actionlist flaglist[]= {
            {FILE_MARK_FLAG,0,FILE_MARKED,"±ê¼Çm"},
            {FILE_NOREPLY_FLAG,1,FILE_READ,"²»¿ÉRe"},
            {FILE_SIGN_FLAG,0,FILE_SIGN,"±ê¼Ç#"},
            {FILE_PERCENT_FLAG,0,FILE_PERCENT,"±ê¼Ç%"},
            {FILE_DELETE_FLAG,1,FILE_DEL,"±ê¼ÇX"},
            {FILE_DIGEST_FLAG,0,FILE_DIGEST,"ÎÄÕª"},
            {FILE_TITLE_FLAG,0,0,"Ô­ÎÄ"},
            {FILE_IMPORT_FLAG,0,FILE_IMPORTED,"ÊÕÈë¾«»ªÇø"},
#ifdef FILTER
            {FILE_CENSOR_FLAG,0,0,"ÉóºËÍ¨¹ı"},
#endif
#ifdef COMMEND_ARTICLE
            {FILE_COMMEND_FLAG,1,FILE_COMMEND,"ÉóºËÍ¨¹ı"},
#endif
            {0,0,0,NULL}
    };
    if (fileinfo==NULL)
        return DONOTHING;
    
#ifdef FILTER
#ifdef SMTH
    if (!strcmp(currboard->filename,"NewsClub")&&haspostperm(getCurrentUser(), currboard->filename)) 
            isbm=true;
#endif
#endif

    if (!isbm
#ifdef OPEN_NOREPLY
		&& (flag!=FILE_NOREPLY_FLAG || strcmp(fileinfo->owner,getCurrentUser()->userid) )
#endif
					)
        return DONOTHING;
    if (conf->pos > arg->filecount && flag == FILE_MARK_FLAG)
        return top_move(conf, fileinfo);
    data=*fileinfo;
    init_write_dir_arg(&dirarg);
    dirarg.fd=arg->fd;
    dirarg.ent = conf->pos;
    ptr=flaglist;
    while (ptr->action!=0) {
        if (ptr->action==flag) {
            data.accessed[ptr->pos] = (fileinfo->accessed[ptr->pos] & ptr->flag)?0 : ptr->flag;
            break;
        }
        ptr++;
    }
    if (ptr->action!=0) {
        ret=change_post_flag(&dirarg, arg->mode, currboard,  
            fileinfo,flag, &data,isbm,getSession());
        if (ret==0) {
//prompt...
            ret=DIRCHANGED;
        } else {
            char buf[STRLEN];
            a_prompt(-1, "²Ù×÷Ê§°Ü, Çë°´ Enter ¼ÌĞø << ", buf);
            ret=FULLUPDATE;
        }
    } else
        ret=DONOTHING;
    free_write_dir_arg(&dirarg);
    return ret;
}

#ifdef COMMEND_ARTICLE
/* stiger, ÍÆ¼öÎÄÕÂ */
int do_commend(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{

    char ispost[3];
    struct read_arg* arg=conf->arg;

    if (fileinfo==NULL)
        return DONOTHING;
    if (arg->mode!=DIR_MODE_NORMAL) {
        return DONOTHING;
    }
    if ((fileinfo->accessed[1] & FILE_COMMEND) && !HAS_PERM(getCurrentUser(), PERM_SYSOP)) {
        clear();
        move(1, 0);
        prints("±¾ÎÄÕÂÒÑ¾­ÍÆ¼ö¹ı£¬¸ĞĞ»ÄúµÄÈÈĞÄÍÆ¼ö");
        move(2, 0);
        pressreturn();
        return FULLUPDATE;
    }
    if( ! normal_board(currboard->filename) ){
        clear();
        move(1, 0);
        prints("¶Ô²»Æğ£¬ÇëÎğÍÆ¼öÄÚ²¿°æÃæÎÄÕÂ");
        move(2, 0);
        pressreturn();
        return FULLUPDATE;
    }
    if ( deny_me(getCurrentUser()->userid, COMMEND_ARTICLE) ) {
        clear();
        move(1, 0);
        prints("¶Ô²»Æğ£¬Äú±»Í£Ö¹ÁËÍÆ¼öµÄÈ¨Á¦");
        move(2, 0);
        pressreturn();
        return FULLUPDATE;
    }

    clear();
    move(0, 0);
    prints("ÍÆ¼ö ' %s '", fileinfo->title);
    clrtoeol();
    move(1, 0);
    getdata(1, 0, "È·¶¨? [y/N]: ", ispost, 2, DOECHO, NULL, true);
    if (ispost[0] == 'y' || ispost[0] == 'Y') {
        if (post_commend(getCurrentUser(), currboard->filename, fileinfo ) == -1) {
        	prints("ÍÆ¼öÊ§°Ü£¬ÏµÍ³´íÎó \n");
            pressreturn();
            move(2, 0);
            return FULLUPDATE;
        }
        move(2, 0);
        prints("ÍÆ¼ö³É¹¦ \n");
	set_article_flag(conf,fileinfo,FILE_COMMEND_FLAG);
	/*
        fileinfo->accessed[1] |= FILE_COMMEND;
        substitute_record(direct, fileinfo, sizeof(*fileinfo), ent);
	*/
    } else {
        prints("È¡Ïû");
    }
    move(2, 0);
    pressreturn();
    return FULLUPDATE;
}
#endif	/* COMMEND_ARTICLE */

/* Add by SmallPig */
int do_cross(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{                               /* ×ªÌù Ò»Æª ÎÄÕÂ */
    char bname[STRLEN];
    char ispost[10];
    char q_file[STRLEN];
    struct read_arg* arg=(struct read_arg*)conf->arg;
    int outgo_board = true, proceed = false;
    const struct boardheader* bh;

    if (fileinfo==NULL)
        return DONOTHING;
    if (!HAS_PERM(getCurrentUser(), PERM_POST)) {    /* ÅĞ¶ÏÊÇ·ñÓĞPOSTÈ¨ */
        return DONOTHING;
    }
#if 0 //atppp 20051230
    if ((fileinfo->accessed[0] & FILE_FORWARDED) && !HAS_PERM(getCurrentUser(), PERM_SYSOP)) {
        clear();
        move(1, 0);
        prints("±¾ÎÄÕÂÒÑ¾­×ªÌù¹ıÒ»´Î£¬ÎŞ·¨ÔÙ´Î×ªÌù");
        move(2, 0);
        pressreturn();
        return FULLUPDATE;
    }
#endif //atppp 20051230
    if (uinfo.mode != RMAIL)
        sprintf(q_file, "boards/%s/%s", currboard->filename, fileinfo->filename);
    else
        setmailfile(q_file, getCurrentUser()->userid, fileinfo->filename);
    strcpy(quote_title, fileinfo->title);

    clear();

    move(4, 0);                 /* Leeward 98.02.25 */
    prints
        ("\033[1m\033[33mÇë×¢Òâ£º\033[31m±¾Õ¾Õ¾¹æ¹æ¶¨£ºÍ¬ÑùÄÚÈİµÄÎÄÕÂÑÏ½ûÔÚ 5 (º¬) ¸öÒÔÉÏÌÖÂÛÇøÄÚÖØ¸´ÕÅÌù¡£\n\nÎ¥·´Õß\033[33m³ıËùÌùÎÄÕÂ»á±»É¾³ıÖ®Íâ£¬»¹½«±»\033[31m°ş¶á¼ÌĞø·¢±íÎÄÕÂµÄÈ¨Á¦¡£\033[33mÏêÏ¸¹æ¶¨Çë²ÎÕÕ£º\n\n    Announce °æµÄÕ¾¹æ£º¡°¹ØÓÚ×ªÌùºÍÕÅÌùÎÄÕÂµÄ¹æ¶¨¡±¡£\n\nÇë´ó¼Ò¹²Í¬Î¬»¤ BBS µÄ»·¾³£¬½ÚÊ¡ÏµÍ³×ÊÔ´¡£Ğ»Ğ»ºÏ×÷¡£\n\n\033[m");

    move(1, 0);
    if (!get_a_boardname(bname, "ÇëÊäÈëÒª×ªÌùµÄÌÖÂÛÇøÃû³Æ: ")) {
        return FULLUPDATE;
    }

    if (!strcmp(bname, currboard->filename) && (uinfo.mode != RMAIL)) {
        move(3, 0);
        clrtobot();
        prints("\n\n                          ±¾°æµÄÎÄÕÂ²»ĞèÒª×ªÌùµ½±¾°æ!");
        pressreturn();
        clear();
        return FULLUPDATE;
    }

    if (!haspostperm(getCurrentUser(), bname)) {
        move(3, 0);
        clrtobot();
        prints("ÄúÉĞÎŞÈ¨ÏŞÔÚ %s ·¢±íÎÄÕÂ.\n", bname);
        prints("Èç¹ûÄúÉĞÎ´×¢²á£¬ÇëÔÚ¸öÈË¹¤¾ßÏäÄÚÏêÏ¸×¢²áÉí·İ\n");
        prints("Î´Í¨¹ıÉí·İ×¢²áÈÏÖ¤µÄÓÃ»§£¬Ã»ÓĞ·¢±íÎÄÕÂµÄÈ¨ÏŞ¡£\n");
        prints("Ğ»Ğ»ºÏ×÷£¡ :-) \n");
        pressreturn();
        clear();
        return FULLUPDATE;
    }

    if(!(bh=getbcache(bname)))
        return FULLUPDATE;

#ifdef NEWSMTH
    if(!check_score_level(getCurrentUser(),bh)){
        move(3,0);
        clrtobot();
        prints("\n\n    \033[1;33m%s\033[0;33m<Enter>\033[m",
            "ÄúµÄ»ı·Ö²»·ûºÏµ±Ç°ÌÖÂÛÇøµÄÉè¶¨, ÔİÊ±ÎŞ·¨ÔÚµ±Ç°ÌÖÂÛÇø·¢±íÎÄÕÂ...");
        WAIT_RETURN;
        return FULLUPDATE;
    }
#endif

        /* Leeward 98.01.13 ¼ì²é×ªÌùÕßÔÚÆäÓû×ªµ½µÄ°æÃæÊÇ·ñ±»½ûÖ¹ÁË POST È¨ */


        if (normal_board(bname) && !normal_board(currboard->filename) && (uinfo.mode != RMAIL)) {
            char ans[4];
            move(2,0);clrtobot();
            move(4,4);prints("\033[1;31m×¢Òâ:\033[m ÄúÊÔÍ¼½«ÄÚ²¿°æÃæ(%s)µÄÎÄÕÂ×ªÔØµ½¹«¿ª°æÃæ£¬",currboard->filename);
            getdata(5,4,"ÕâÖÖ×ö·¨Í¨³£ÊÇ\033[1;33m²»ÔÊĞí\033[mµÄ»ò\033[1;33m²»½¨Òé\033[mµÄ, ÄúÈ·ÈÏ×ªÔØ²Ù×÷? [y/N]: ",ans,2,DOECHO,NULL,true);
            if(!(ans[0]=='y'||ans[0]=='Y'))
                return FULLUPDATE;
        }
        if ((fileinfo->attachment!=0)&&!(bh->flag&BOARD_ATTACH)) {
            char ans[4];
            move(3, 0);
            clrtobot();
            prints("\n\n  ÄúÒª×ªÌùµÄÎÄÕÂ´øÓĞ¸½¼ş£¬¶ø%s°æ²»ÄÜÕ³Ìù¸½¼ş¡£", bh->filename);
            getdata(8, 2, "ÊÇ·ñ¶ªÆú¸½¼ş¼ÌĞø×ªÌù [y/N]: ", ans, 2, DOECHO, NULL, true);
            if(!(ans[0]=='y'||ans[0]=='Y'))
                return FULLUPDATE;
        }
        if (deny_me(getCurrentUser()->userid, bname) ) {    /* °æÖ÷½ûÖ¹POST ¼ì²é */
			if( HAS_PERM(getCurrentUser(), PERM_SYSOP) ){
				char buf[3];

				clear();
                getdata(3,0,"ÄúÒÑ±»È¡ÏûÔÚÄ¿µÄ°æÃæµÄ·¢ÎÄÈ¨ÏŞ, ÊÇ·ñÇ¿ÖÆ×ªÔØ? [y/N]: ",buf,2,DOECHO,NULL,true);
				if( buf[0]!='y' && buf[0]!='Y' ){
					clear();
					return FULLUPDATE;
				}
			}else{
            	move(3, 0);
            	clrtobot();
                prints("\n\n\t\tÄúÒÑ±»¹ÜÀíÈËÔ±È¡ÏûÔÚÄ¿µÄ°æÃæµÄ·¢ÎÄÈ¨ÏŞ...\n");
            	pressreturn();
            	clear();
            	return FULLUPDATE;
			}
        }
		if (true == check_readonly(bname)) { /* Leeward 98.03.28 */
            return FULLUPDATE;
        }
        outgo_board = (bh->flag & BOARD_OUTFLAG);


    move(0, 0);
    prints("×ªÌù ' %s ' µ½ %s °æ ", quote_title, bname);
    clrtoeol();
    move(1, 0);
    if (outgo_board) {
        getdata(1, 0, "(S)×ªĞÅ (L)±¾Õ¾ (A)È¡Ïû? [A]: ", ispost, 9, DOECHO, NULL, true);
        proceed = (ispost[0] == 's' || ispost[0] == 'S' || ispost[0] == 'L' || ispost[0] == 'l');
    } else {
        getdata(1, 0, "(L)±¾Õ¾ (A)È¡Ïû? [A]: ", ispost, 9, DOECHO, NULL, true);
        proceed = (ispost[0] == 's' || ispost[0] == 'S' || ispost[0] == 'L' || ispost[0] == 'l');
    }
    if (proceed) {
		if(!outgo_board)
			ispost[0]='l';
        /*add by stiger
        if(conf->pos<=arg->filecount) {
            int i=0;
            for (i = conf->pos-arg->filecount; i > 0; i--) {
                if (0 == strcmp(arg->boardstatus->topfh[i].filename, fileinfo->filename)) {
                    conf->new_pos=i;
                    break;
                }
           }
	    if (i==0){
                move(2, 0);
	        prints("ÄÕÂÁĞ±í·¢Éú±ä»¯£¬È¡Ïû");
		move(2,0);
		pressreturn();
		return FULLUPDATE;
	    }
        } else conf->new_pos=conf->pos; */
        /*add old*/
        if (post_cross(getCurrentUser(), bh, currboard->filename, 
            quote_title, q_file, Anony, 
            arg->mode==DIR_MODE_MAIL?1:0, 
            ispost[0], 0, getSession()) == -1) { /* ×ªÌù */
            pressreturn();
            move(2, 0);
            return FULLUPDATE;
        }
        move(2, 0);
        prints("' %s ' ÒÑ×ªÌùµ½ %s °æ \n", quote_title, bname);
#if 0 //atppp 20051230
        fileinfo->accessed[0] |= FILE_FORWARDED;        /*added by alex, 96.10.3 */
        if(conf->pos<=arg->filecount)
            substitute_record(arg->direct, fileinfo, sizeof(*fileinfo), conf->pos);
    /* ÖÃ¶¥Ìù¿ÉÒÔ¶à´Î×ªÔØÓ¦¸ÃÃ»ÓĞÎÊÌâ°É...
        else
            substitute_record(arg->dingdirect, fileinfo, sizeof(*fileinfo), conf->new_pos);
    */
#endif //atppp 20051230
        conf->new_pos=0;
    } else {
        prints("È¡Ïû");
    }
    move(2, 0);
    pressreturn();
    return FULLUPDATE;
}

void readtitle(struct _select_def* conf)
{                               /* °æÄÚ ÏÔÊ¾ÎÄÕÂÁĞ±í µÄ title */
    const struct boardheader *bp;
    struct BoardStatus * bs;
    char header[STRLEN*2], title[STRLEN];
    const char *readmode;
    int chkmailflag = 0;
    int bnum;
    struct read_arg* arg=(struct read_arg*)conf->arg;

    bnum = getbid(currboard->filename,&bp);
    bs = getbstatus(bnum);
    memcpy(currBM, bp->BM, BM_LEN - 1);
    if (currBM[0] == '\0' || currBM[0] == ' ') {
        strcpy(header, "³ÏÕ÷°æÖ÷ÖĞ");
    } else {
        //if (HAS_PERM(getCurrentUser(), PERM_OBOARDS)) {
#ifdef OPEN_BMONLINE
		if (1)
#else
        if(check_board_delete_read_perm(getCurrentUser(),currboard,0))
#endif
        {
            char *p1, *p2;

            strcpy(header, "°æÖ÷: ");
            p1 = currBM;
            p2 = p1;
            while (1) {
                if ((*p2 == ' ') || (*p2 == 0)) {
                    int end;

                    end = 0;
                    if (p1 == p2) {
                        if (*p2 == 0)
                            break;
                        p1++;
                        p2++;
                        continue;
                    }
                    if (*p2 == 0)
                        end = 1;
                    *p2 = 0;
                    if (apply_utmp(NULL, 1, p1, NULL)) {
                        sprintf(genbuf, "\x1b[32m%s\x1b[33m ", p1);
                        strcat(header, genbuf);
                    } else {
                        strcat(header, p1);
                        strcat(header, " ");
                    }
                    if (end)
                        break;
                    p1 = p2 + 1;
                    *p2 = ' ';
                }
                p2++;
            }
        } else {
            sprintf(header, "°æÖ÷: %s", currBM);
        }
    }
    chkmailflag = chkmail();
    if (chkmailflag == 2)       /*Haohmaru.99.4.4.¶ÔÊÕĞÅÒ²¼ÓÏŞÖÆ */
        strcpy(title, "[ÄúµÄĞÅÏä³¬¹ıÈİÁ¿,²»ÄÜÔÙÊÕĞÅ!]");
    else if (chkmailflag)       /* ĞÅ¼ş¼ì²é */
        strcpy(title, "[ÄúÓĞĞÅ¼ş]");
    else if ((bp->flag & BOARD_VOTEFLAG))       /* Í¶Æ±¼ì²é */
        sprintf(title, "Í¶Æ±ÖĞ£¬°´ V ½øÈëÍ¶Æ±");
    else
        strcpy(title, bp->title + 13);

    showtitle(header, title);   /* ÏÔÊ¾ µÚÒ»ĞĞ */
    update_endline();
    move(1, 0);
    clrtoeol();
    if (DEFINE(getCurrentUser(), DEF_HIGHCOLOR))
        prints
            ("Àë¿ª[\x1b[1;32m¡û\x1b[m,\x1b[1;32me\x1b[m] Ñ¡Ôñ[\x1b[1;32m¡ü\x1b[m,\x1b[1;32m¡ı\x1b[m] ÔÄ¶Á[\x1b[1;32m¡ú\x1b[m,\x1b[1;32mr\x1b[m] ·¢±íÎÄÕÂ[\x1b[1;32mCtrl-P\x1b[m] ¿³ĞÅ[\x1b[1;32md\x1b[m] ±¸ÍüÂ¼[\x1b[1;32mTAB\x1b[m] ÇóÖú[\x1b[1;32mh\x1b[m]\033[m");
    else
        prints("Àë¿ª[¡û,e] Ñ¡Ôñ[¡ü,¡ı] ÔÄ¶Á[¡ú,r] ·¢±íÎÄÕÂ[Ctrl-P] ¿³ĞÅ[d] ±¸ÍüÂ¼[TAB] ÇóÖú[h]\x1b[m");

    switch(arg->mode){
        case DIR_MODE_NORMAL:
            readmode="Ò»°ã";
            break;
        case DIR_MODE_DIGEST:
            readmode="ÎÄÕª";
            break;
        case DIR_MODE_THREAD:
            readmode="Ö÷Ìâ";
            break;
        case DIR_MODE_MARK:
            readmode="¾«»ª";
            break;
        case DIR_MODE_DELETED:
            readmode="»ØÊÕ";
            break;
        case DIR_MODE_JUNK:
            readmode="Ö½Â¦";
            break;
        case DIR_MODE_ORIGIN:
            readmode="Ô­×÷";
            break;
        case DIR_MODE_AUTHOR:
            readmode="×÷Õß";
            break;
        case DIR_MODE_TITLE:
            readmode="±êÌâ";
            break;
        case DIR_MODE_SUPERFITER:
            readmode="ËÑË÷";
            break;
        case DIR_MODE_SELF:
            readmode="×ÔÉ¾";
            break;
        default:
            readmode="Î´Öª";
            break;
    }

    move(2, 0);
    setfcolor(WHITE, DEFINE(getCurrentUser(), DEF_HIGHCOLOR));
    setbcolor(BLUE);
    clrtoeol();
    prints("  ±àºÅ   %-12s %6s %s", "¿¯ µÇ Õß", "ÈÕ  ÆÚ", " ÎÄÕÂ±êÌâ");
    sprintf(title, "ÔÚÏß:%4d [%4sÄ£Ê½]", bs->currentusers, readmode);
    move(2, -strlen(title)-1);
    prints("%s", title);
    resetcolor();
}

#if defined(COLOR_ONLINE) || defined(LOWCOLOR_ONLINE)

int count_login(struct user_info *uentp, int *arg, int pos)
{
     if (uentp->invisible == 1) {
         return COUNT;
     }
     (*arg)++;
     UNUSED_ARG(pos);
     return COUNT;
}

int isonline(char *s){

	int tuid;
	int logincount=0,seecount=0;
    struct userec *lookupuser;

	if( ! (tuid=getuser(s,&lookupuser)) ){
		return 7;
	}
	logincount = apply_utmp((APPLY_UTMP_FUNC) count_login, 0, lookupuser->userid, &seecount);
	if( logincount == 0 )
		return 7;
	if( seecount != 0 )
		return 2;
    if (HAS_PERM(getCurrentUser(), PERM_SEECLOAK))
		return 6;
	return 7;
}
#endif

char *readdoent(char *buf, int num, struct fileheader *ent,struct fileheader* readfh,struct _select_def* conf)
{                               /* ÔÚÎÄÕÂÁĞ±íÖĞ ÏÔÊ¾ Ò»ÆªÎÄÕÂ±êÌâ */
    time_t filetime;
    char date[20];
    char TITLE[ARTICLE_TITLE_LEN+30];
    int titlelen;
    int type;
    int manager;
    char typeprefix[20];
    char *typesufix;
    char attachch[20];
    struct read_arg * arg=(struct read_arg*)conf->arg;
	int isreply=0;
	int isthread=0;
	char highstr[10];
	char threadprefix[20];
	char threadprefix1[20];
	char threadsufix[20];

   /* typesufix = typeprefix = "";*/
   typesufix = ""; 
   typeprefix[0]='\0';

    manager = chk_currBM(currBM, getCurrentUser());

    type = get_article_flag(ent, getCurrentUser(), currboard->filename, manager, NULL, getSession());
    if (manager && (ent->accessed[0] & FILE_IMPORTED)) {        /* ÎÄ¼şÒÑ¾­±»ÊÕÈë¾«»ªÇø */
        if (type == ' ') {
            strcpy(typeprefix ,"\x1b[42m");
            typesufix = "\x1b[m";
        } else {
            strcpy(typeprefix ,"\x1b[32m");
            typesufix = "\x1b[m";
        }
    }
    filetime = get_posttime(ent);
    if (filetime > 740000000) {
#ifdef HAVE_COLOR_DATE
        struct tm *mytm;
        char *datestr = ctime(&filetime) + 4;

        mytm = localtime(&filetime);
        sprintf(date, "\033[1;%dm%6.6s\033[m", mytm->tm_wday + 31, datestr);
#else
        strncpy(date, ctime(&filetime) + 4, 6);
        date[6] = 0;
#endif
    }
    /*
     * date = ctime( &filetime ) + 4;   Ê±¼ä -> Ó¢ÎÄ 
     */
    else
        /*
         * date = ""; char *ÀàĞÍ±äÁ¿, ¿ÉÄÜ´íÎó, modified by dong, 1998.9.19 
         */
        /*
         * { date = ctime( &filetime ) + 4; date = ""; } 
         */
        date[0] = 0;

    /*
     * Re-Write By Excellent 
     */
#if defined(OPEN_NOREPLY) && defined(LOWCOLOR_ONLINE)
    if ( ent->accessed[1] & FILE_READ ) {
		if(ent->attachment!=0)
			strcpy(attachch,"\033[0;1;4;33m@\033[m");
		else
			strcpy(attachch,"\033[0;1;4;33mx\033[m");
	}else{
		if(ent->attachment!=0)
			strcpy(attachch,"\033[0;1;33m@\033[m");
		else
			strcpy(attachch," ");
	}
#else
    if (ent->attachment!=0)
        attachch[0]='@';
    else
        attachch[0]=' ';
	attachch[1]='\0';
#endif
    titlelen = scr_cols > 80 ? scr_cols - 80 + 45 : 45;
    if (titlelen > ARTICLE_TITLE_LEN) {
        titlelen = ARTICLE_TITLE_LEN - 1;
    }
    if (! DEFINE(getCurrentUser(), DEF_SHOWSIZE) && arg->mode != DIR_MODE_DELETED && arg->mode != DIR_MODE_JUNK){
		char sizebuf[30];
		strnzhcpy(TITLE, ent->title, titlelen - 7);
		if(ent->eff_size < 1000)
			sprintf(sizebuf,"(%d)", ent->eff_size);
		else if(ent->eff_size < 1000000){
			sprintf(sizebuf,"\033[1;33m(%dk)\033[m", ent->eff_size/1000);
		}else{
			sprintf(sizebuf, "\033[1;31m(%dm)\033[m", (ent->eff_size/1000000)%1000 );
		}
		strcat(TITLE, sizebuf);
	}else {
		strnzhcpy(TITLE, ent->title, titlelen);
	}
//    TITLE = ent->title;         /*ÎÄÕÂ±êÌâTITLE */
//	sprintf(TITLE,"%s(%d)",ent->title,ent->eff_size);
    if ((type=='d')||(type=='D')) { //ÖÃ¶¥ÎÄÕÂ
        sprintf(buf, " \x1b[1;33m[ÌáÊ¾]\x1b[m %-12.12s %s %s" FIRSTARTICLE_SIGN " %s ", ent->owner, date, attachch, TITLE);
        return buf;
    }

    if(toupper(type)=='Y'){
        sprintf(buf," \033[1;33m[±¸·İ]\033[m %-12.12s %s %s" FIRSTARTICLE_SIGN " %s ",ent->owner,date,attachch,TITLE);
        return buf;
    }

    if (uinfo.mode != RMAIL && arg->mode != DIR_MODE_DIGEST && arg->mode != DIR_MODE_DELETED && arg->mode != DIR_MODE_JUNK
        && strcmp(currboard->filename, "sysmail")) { /* ĞÂ·½·¨±È½Ï*/
        if ((ent->groupid != ent->id)&&(arg->mode==DIR_MODE_THREAD||!strncasecmp(TITLE,"Re:",3)||!strncmp(TITLE,"»Ø¸´:",5))) {
			isreply=1;
		}
        if ((readfh&&readfh->groupid == ent->groupid))
			isthread=1;
	}else {
        if (!strncmp("Re:", ent->title, 3)) {
			isreply=1;
            if (readfh&&isThreadTitle(readfh->title, ent->title))
				isthread=1;
		}else
            if ((readfh!=NULL)&&!strcmp(readfh->title, ent->title))
				isthread=1;
	}

    if (DEFINE(getCurrentUser(), DEF_HIGHCOLOR))
		strcpy(highstr,"1;");
	else
		highstr[0]='\0';

	if(isthread){
		if(isreply){
			sprintf(threadprefix,"\033[%s36m", highstr);
			sprintf(threadprefix1,"\033[%s36m.", highstr);
			strcpy(threadsufix,"\033[m");
		}else{
			sprintf(threadprefix,"\033[%s33m", highstr);
			sprintf(threadprefix1,"\033[%s33m.", highstr);
			strcpy(threadsufix,"\033[m");
		}
	}else{
		threadprefix[0]='\0';
		strcpy(threadprefix1," ");
		threadsufix[0]='\0';
	}

#if defined(COLOR_ONLINE)
                        sprintf(buf, " %s%4d%s %s%c%s \033[1;3%dm%-12.12s\033[m %s%s%c%s%s%s ", threadprefix, num, threadsufix, typeprefix, type, typesufix, isonline(ent->owner), ent->owner, date, threadprefix1, attachch[0], isreply?"":FIRSTARTICLE_SIGN" ", TITLE, threadsufix);
#elif defined(LOWCOLOR_ONLINE)
                        sprintf(buf, " %s%4d%s %s%c%s \033[3%dm%-12.12s\033[m %s%s%c%s%s%s ", threadprefix, num, threadsufix, typeprefix, type, typesufix, isonline(ent->owner), ent->owner, date, threadprefix1, attachch[0], isreply?"":FIRSTARTICLE_SIGN" ", TITLE, threadsufix);
#else
                        sprintf(buf, " %s%4d%s %s%c%s %-12.12s %s%s%c%s%s%s ", threadprefix, num, threadsufix, typeprefix, type, typesufix, ent->owner, date, threadprefix1, attachch[0], isreply?"":FIRSTARTICLE_SIGN" ", TITLE, threadsufix);
#endif

    return buf;
}

#define SESSIONLEN 9
void get_telnet_sessionid(char* buf,int unum)
{
    static const char encode[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    struct user_info* pui=get_utmpent(unum);
    int utmpkey=pui->utmpkey;
    buf[0]=encode[unum%36];
    unum/=36;
    buf[1]=encode[unum%36];
    unum/=36;
    buf[2]=encode[unum%36];

    buf[3]=encode[utmpkey%36];
    utmpkey/=36;
    buf[4]=encode[utmpkey%36];
    utmpkey/=36;
    buf[5]=encode[utmpkey%36];
    utmpkey/=36;
    buf[6]=encode[utmpkey%36];
    utmpkey/=36;
    buf[7]=encode[utmpkey%36];
    utmpkey/=36;
    buf[8]=encode[utmpkey%36];
    utmpkey/=36;

    buf[9]=0;
}

char *get_my_webdomain(int force)
{
	static char myurl[80];
	static int urlinit=0;

	if(force) urlinit=0;

	if(!urlinit){
		FILE *fp;
		char buf[256];
		char *c;

		sethomefile(buf, getCurrentUser()->userid, "myurl");
		if((fp=fopen(buf,"r"))!=NULL){
			if(fgets(myurl,80,fp)){
				if((c=strchr(myurl,'\n'))!=NULL) *c='\0';
				if((c=strchr(myurl,'\r'))!=NULL) *c='\0';
				if(strlen(myurl)>5)
					urlinit=1;
			}
			fclose(fp);
		}
		if(!urlinit){
			const char *c=sysconf_str("BBS_WEBDOMAIN");
			if(c!=NULL){
				urlinit=1;
				strncpy(myurl, c, 80);
				myurl[79]='\0';
			}
		}
		if(!urlinit){
			strncpy(myurl, sysconf_str("BBSDOMAIN"), 80);
			myurl[79]='\0';
			urlinit=1;
		}
	}

	return myurl;

}

static void  board_attach_link(char* buf,int buf_len,char *ext,int len,long attachpos,void* arg)
{
    struct fileheader* fh=(struct fileheader*)arg;
    char ftype[12];
    int zd = (POSTFILE_BASENAME(fh->filename)[0] == 'Z');
    ftype[0] = '\0';
    if (attachpos!=-1) {
        char ktype = 's';
        if (!public_board(currboard)) {
#ifdef ATPPP_YMSW_YTJH
            MD5_CTX md5;
            char info[9+19], base64_info[9+25];
            char *ptr = info;
            uint32_t ii; uint16_t is;
            char md5ret[17];
            get_telnet_sessionid(info, getSession()->utmpent);
            ptr = info + 9;
            is = (uint16_t)currboardent; memcpy(ptr, &is, 2), ptr += 2;
            ii = (fh->id);          memcpy(ptr, &ii, 4); ptr += 4;
            ii = (attachpos);       memcpy(ptr, &ii, 4); ptr += 4;
            ii = ((int)time(NULL)); memcpy(ptr, &ii, 4); ptr += 4;
 
            MD5Init(&md5);
            MD5Update(&md5, (unsigned char *) info, 23);
            MD5Final((unsigned char*)md5ret, &md5);

            memcpy(ptr, md5ret, 4);
            memcpy(base64_info, info, 9);
            to64frombits((unsigned char*)base64_info+9, (unsigned char*)info+9, 18);
            snprintf(buf,buf_len,"http://%s/att.php?n.%s%s",
                get_my_webdomain(0),base64_info,ext);
            return;
#else
            ktype = 'n';
#endif
        } else {
            if (len > 51200) ktype = 'p';
        }

        if (zd) sprintf(ftype, ".%d.0", DIR_MODE_ZHIDING);
        
        snprintf(buf,buf_len,"http://%s/att.php?%c.%d.%d%s.%ld%s",
            get_my_webdomain(0),ktype,currboardent,fh->id,ftype,attachpos,ext);
    } else {
        if (zd) sprintf(ftype, "&ftype=%d", DIR_MODE_ZHIDING);
        
        snprintf(buf,buf_len,"http://%s/bbscon.php?bid=%d&id=%d%s",
            get_my_webdomain(0),currboardent,fh->id, ftype);
    }
}

int zsend_attach(int ent, struct fileheader *fileinfo, char *direct)
{
    char *t;
    char buf1[512];
    char *ptr, *p;
    off_t size;
    long left;

    if(fileinfo->attachment==0) return -1;
    strcpy(buf1, direct);
    if ((t = strrchr(buf1, '/')) != NULL)
        *t = '\0';
    snprintf(genbuf, 512, "%s/%s", buf1, fileinfo->filename);
    BBS_TRY {
        if (safe_mmapfile(genbuf, O_RDONLY, PROT_READ, MAP_SHARED, &ptr, &size, NULL) == 0) {
            BBS_RETURN(-1);
        }
        for (p=ptr,left=size;left>0;p++,left--) {
            long attach_len;
            char* file,*attach;
            FILE* fp;
            char name[100];
            if (NULL !=(file = checkattach(p, left, &attach_len, &attach))) {
                left-=(attach-p)+attach_len-1;
                p=attach+attach_len-1;
                setcachehomefile(name, getCurrentUser()->userid,getSession()->utmpent, "attach.tmp");
                fp=fopen(name, "wb");
                fwrite(attach, 1, attach_len, fp);
                fclose(fp);
                if (bbs_zsendfile(name, file)==-1) {
                    unlink(name);
                    break;
                }
                unlink(name);
                continue;
            }
        }
    }
    BBS_CATCH {
    }
    BBS_END;
    end_mmapfile((void *) ptr, size, -1);
    return 0;
}

int showinfo(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
    char slink[256];
    bool isbm;
    char unread_mark;
    if (fileinfo==NULL) return DONOTHING;

    clear();

    if (!deny_modify_article(currboard, fileinfo, DIR_MODE_NORMAL, getSession())
        && ((currboard->flag & BOARD_ATTACH) || HAS_PERM(getCurrentUser(), PERM_SYSOP) || fileinfo->attachment)) {
        char ses[20];
        move(0,0);
        get_telnet_sessionid(ses, getSession()->utmpent);
        snprintf(slink, sizeof(slink), "¸½¼ş±à¼­µØÖ·: (ÇëÎğ½«´ËÁ´½Ó·¢ËÍ¸ø±ğÈË)\n\033[4mhttp://%s/bbseditatt.php?bid=%d&id=%d&sid=%s\033[m\n",
            get_my_webdomain(0), currboardent, fileinfo->id, ses);
        prints("%s", slink);
    }

    move(3,0);
    board_attach_link(slink,255,NULL,-1,-1,fileinfo);
    prints("È«ÎÄÁ´½Ó£º\n\033[4m%s\033[m\n",slink);

    isbm=chk_currBM(currboard->BM, getCurrentUser());
    unread_mark = (DEFINE(getCurrentUser(), DEF_UNREADMARK) ? UNREAD_SIGN : 'N');
    move(6,0);
    prints("ÎÄÕÂ±ê¼Ç: %c%c%c%c%c%c%c%s%c",
        (strcmp(getCurrentUser()->userid, "guest") && brc_unread(fileinfo->id, getSession())) ? unread_mark : ' ',
        (fileinfo->accessed[0] & FILE_DIGEST) ? 'g' : ' ',
        (fileinfo->accessed[0] & FILE_MARKED) ? 'm' : ' ',
        (fileinfo->accessed[1] & FILE_READ) ? ';' : ' ',
        (isbm && (fileinfo->accessed[0] & FILE_SIGN)) ? '#' : ' ',
        (isbm && (fileinfo->accessed[0] & FILE_PERCENT)) ? '%' : ' ',
        (isbm && (fileinfo->accessed[1] & FILE_DEL)) ? 'X' : ' ',
        (isbm && (fileinfo->accessed[0] & FILE_IMPORTED)) ? "\x1b[42m \x1b[0m" : " ",
        (isbm && (fileinfo->accessed[1] & FILE_CENSOR)) ? '@' : ' '
        );
    pressanykey();
    return FULLUPDATE;
}

int jumpReID(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
    int now; // 1-based
    char *data;
    struct fileheader *pFh, *pFh1;
    off_t size;
    struct read_arg *arg = (struct read_arg *) conf->arg;

    if (arg->mode != DIR_MODE_NORMAL) return DONOTHING;
    if (fileinfo->reid == fileinfo->id) return DONOTHING;

    BBS_TRY {
        if (safe_mmapfile_handle(arg->fd, PROT_READ, MAP_SHARED, &data, &size) == 0)
            BBS_RETURN(0);
        pFh = (struct fileheader*)data;
        now = arg->filecount = size/sizeof(struct fileheader);
        if (now >= conf->pos) now = conf->pos - 1;
        for(;now>=1;now--) { /* Ë³ĞòÕÒÄØ£¬»¹ÊÇ yuhuan mm µÄ Search_Bin ÄØ... */
            pFh1 = pFh + now - 1;
            if (fileinfo->reid == pFh1->id) break;
            if (fileinfo->reid > pFh1->id) {
                now = -1;
                break;
            }
        }
    }
    BBS_CATCH {
        now = -1;
    }
    BBS_END;
    end_mmapfile(data, size, -1);
    if(now > 0) {
        conf->new_pos = now;
        return SELCHANGE;
    }
    return DONOTHING;
}

/* etnlegend, 2006.09.14, Ìø×ªµ½Í¬Ö÷ÌâÖĞĞŞ¸Ä±êÌâµÄµÚÒ»ÆªÎÄÕÂ */
static int jump_changed_title(struct _select_def *conf,struct fileheader *fh,void *varg){
    int res,okay,last;
    char *data;
    off_t size;
    struct fileheader *map;
    struct read_arg *arg=(struct read_arg*)conf->arg;
    if(arg->mode!=DIR_MODE_NORMAL)
        return DONOTHING;
    if((fh->id==fh->groupid)||strncmp(fh->title,"Re: ",4))  /* ³õÊ¼ÅĞ¶Ï */
        return DONOTHING;
    BBS_TRY{
        if(!safe_mmapfile_handle(arg->fd,PROT_READ,MAP_SHARED,&data,&size))
            BBS_RETURN(0);
        map=(struct fileheader*)data;
        if((arg->filecount=size/sizeof(struct fileheader))<conf->pos)
            res=-1;
        else{
            res=conf->pos-1;
            if(map[res].id!=fh->id)
                res=-1;
            else{
                do{         /* ´¦ÀíÖ÷ÌâÁ´ÍêÕûµÄÇé¿ö */
                    for(last=res;!(res<0)&&(map[res].id>map[last].reid);res--)
                        continue;
                    if(!(okay=(!(res<0)&&(map[res].id==map[last].reid))))
                        break;
                    if((res==last)||
                        !((!strncmp(map[res].title,"Re: ",4)
                        &&!strncmp(&map[res].title[4],&fh->title[4],54))
                        ||!strncmp(map[res].title,&fh->title[4],54)))
                        break;
                }
                while(1);
                if(!okay){  /* ´¦ÀíÖ÷ÌâÁ´ÆÆËğµÄÇé¿ö */
                    for(res=last;!(res<0)&&!(map[res].id<fh->groupid);res--)
                        if((map[res].groupid==fh->groupid)
                            &&((!strncmp(map[res].title,"Re: ",4)
                            &&!strncmp(&map[res].title[4],&fh->title[4],54))
                            ||!strncmp(map[res].title,&fh->title[4],54)))
                            last=res;
                }
                res=last;
            }
        }
    }
    BBS_CATCH{
        res=-1;
    }
    BBS_END;
    end_mmapfile(data,size,-1);
    if(res==-1)
        return DONOTHING;
    conf->new_pos=(res+1);
    return SELCHANGE;
}

int read_post(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
    char *t;
    char buf[512];
    int ch;
    int ent=conf->pos;
    struct read_arg* arg=conf->arg;
    int ret=FULLUPDATE;

    if (fileinfo==NULL)
        return DONOTHING;
    /* czz 2003.3.4 forbid reading cancelled post in board */
    if ((fileinfo->owner[0] == '-') 
        && (arg->readmode != DIR_MODE_DELETED) 
        && (arg->readmode != DIR_MODE_JUNK) )
	    return ret;

    clear();
    strcpy(buf, read_getcurrdirect(conf));
    if ((t = strrchr(buf, '/')) != NULL)
        *t = '\0';
    sprintf(genbuf, "%s/%s", buf, fileinfo->filename);
/*
    strcpy(quote_file, genbuf);
*/
    strcpy(quote_board, currboard->filename);
    strncpy(quote_title, fileinfo->title, ARTICLE_TITLE_LEN);
/*    quote_file[119] = fileinfo->filename[STRLEN - 2];
*/
    strncpy(quote_user, fileinfo->owner, OWNER_LEN);
    quote_user[OWNER_LEN - 1] = 0;

    register_attach_link(board_attach_link, fileinfo);
#ifndef NOREPLY
    ch = ansimore_withzmodem(genbuf, false, fileinfo->title);   /* ÏÔÊ¾ÎÄÕÂÄÚÈİ */
#else
    ch = ansimore_withzmodem(genbuf, true, fileinfo->title);    /* ÏÔÊ¾ÎÄÕÂÄÚÈİ */
#endif
    register_attach_link(NULL,NULL);
#ifdef HAVE_BRC_CONTROL
    brc_add_read(fileinfo->id,currboardent,getSession());
#endif
#ifndef NOREPLY
    move(t_lines - 1, 0);
    switch (arg->readmode) {
    case READ_THREAD:
        if (DEFINE(getCurrentUser(), DEF_HIGHCOLOR))
	        prints("\x1b[44m\x1b[1;31m[Ö÷ÌâÔÄ¶Á] \x1b[33m »ØĞÅ R ©¦ ½áÊø Q,¡û ©¦ÉÏÒ»·â ¡ü©¦ÏÂÒ»·â <Space>,¡ı");
        else
	        prints("\x1b[44m\x1b[31m[Ö÷ÌâÔÄ¶Á] \x1b[33m »ØĞÅ R ©¦ ½áÊø Q,¡û ©¦ÉÏÒ»·â ¡ü©¦ÏÂÒ»·â <Space>,¡ı");
        break;
    case READ_AUTHOR:
        if (DEFINE(getCurrentUser(), DEF_HIGHCOLOR))
	        prints("\x1b[44m\x1b[1;31m[Í¬×÷ÕßÔÄ¶Á] \x1b[33m »ØĞÅ R ©¦ ½áÊø Q,¡û ©¦ÉÏÒ»·â ¡ü©¦ÏÂÒ»·â <Space>,¡ı");
        else
	        prints("\x1b[44m\x1b[31m[Í¬×÷ÕßÔÄ¶Á] \x1b[33m »ØĞÅ R ©¦ ½áÊø Q,¡û ©¦ÉÏÒ»·â ¡ü©¦ÏÂÒ»·â <Space>,¡ı");
        break;
    default:
        if (DEFINE(getCurrentUser(), DEF_HIGHCOLOR))
            prints("\033[44m\033[1;31m[ÔÄ¶ÁÎÄÕÂ] \033[33m »ØĞÅ R ©¦ ½áÊø Q,¡û ©¦ÉÏÒ»·â ¡ü©¦ÏÂÒ»·â <Space>,¡ı©¦Ö÷ÌâÔÄ¶Á ^X»òp ");
        else
            prints("\033[44m\033[31m[ÔÄ¶ÁÎÄÕÂ] \033[33m »ØĞÅ R ©¦ ½áÊø Q,¡û ©¦ÉÏÒ»·â ¡ü©¦ÏÂÒ»·â <Space>,¡ı©¦Ö÷ÌâÔÄ¶Á ^X»òp ");
    }

    clrtoeol();                 /* ÇåÆÁµ½ĞĞÎ² */
    resetcolor();

    setreadpost(conf, fileinfo);

    if (!(ch == KEY_RIGHT || ch == KEY_UP || ch == KEY_PGUP
        || ch == KEY_DOWN) && (ch <= 0 || strchr("RrEexp", ch) == NULL))
reget:
        ch = igetkey();

    switch (ch) {
    case Ctrl('Z'):
        r_lastmsg();            /* Leeward 98.07.30 support msgX */
        break;
    case Ctrl('Y'):
        read_zsend(conf, fileinfo, NULL);
        break;
    case Ctrl('D'):
        zsend_attach(ent, fileinfo, read_getcurrdirect(conf));
        break;
    case 'Q':
    case 'q':
    case KEY_LEFT:
        break;
    case KEY_REFRESH:
        break;
    case ' ':
    case 'j':
    case 'n':
    case KEY_DOWN:
    case KEY_PGDN:
        return READ_NEXT;
    case KEY_UP:
    case KEY_PGUP:
    case 'k':
    case 'l':
        return READ_PREV;
    case 'Y':
    case 'R':
    case 'y':
    case 'r':
        if ( currboard->flag & BOARD_NOREPLY ) 
        {
            clear();
            move(3, 0);
            clrtobot();
            prints("\n\n                    ºÜ±§Ç¸£¬¸Ã°æ½öÄÜ·¢±íÎÄÕÂ,²»ÄÜ»ØÎÄÕÂ...\n");
            pressreturn();
            break;              /*Haohmaru.98.12.19,²»ÄÜ»ØÎÄÕÂµÄ°æ */
        }
        if (fileinfo->accessed[1] & FILE_READ) {        /*Haohmaru.99.01.01.ÎÄÕÂ²»¿Ére */
            clear();
            move(3, 0);
            prints("\n\n            ºÜ±§Ç¸£¬±¾ÎÄÒÑ¾­ÉèÖÃÎª²»¿ÉreÄ£Ê½,Çë²»ÒªÊÔÍ¼ÌÖÂÛ±¾ÎÄ...\n");
            pressreturn();
            break;
        }
        do_reply(conf,fileinfo);
        ret=DIRCHANGED;
        break;
    case Ctrl('R'):
        post_reply(conf, fileinfo, extraarg);      /* »ØÎÄÕÂ */
        break;
    case 'g':
        ret=set_article_flag(conf , fileinfo, FILE_DIGEST_FLAG);       /* Leeward 99.03.02 */
        break;
    case 'M':
        ret=set_article_flag(conf , fileinfo, FILE_MARK_FLAG);       /* Leeward 99.03.02 */
        break;
    case Ctrl('U'):
        if (arg->readmode==READ_NORMAL) {
            move(t_lines - 1, 0);
            if (DEFINE(getCurrentUser(), DEF_HIGHCOLOR))
                prints("\x1b[44m\x1b[1;31m[Í¬×÷ÕßÔÄ¶Á] \x1b[33m »ØĞÅ R ©¦ ½áÊø Q,¡û ©¦ÉÏÒ»·â ¡ü©¦ÏÂÒ»·â <Space>,¡ı");
            else
	        prints("\x1b[44m\x1b[31m[Í¬×÷ÕßÔÄ¶Á] \x1b[33m »ØĞÅ R ©¦ ½áÊø Q,¡û ©¦ÉÏÒ»·â ¡ü©¦ÏÂÒ»·â <Space>,¡ı");
            clrtoeol();
            arg->readmode=READ_AUTHOR;
            arg->oldpos=0;
            goto reget;
        } else return READ_NEXT;
    case Ctrl('H'):
        if (arg->readmode==READ_NORMAL) {
            move(t_lines - 1, 0);
            if (DEFINE(getCurrentUser(), DEF_HIGHCOLOR))
                prints("\x1b[44m\x1b[1;31m[Í¬×÷ÕßÔÄ¶Á] \x1b[33m »ØĞÅ R ©¦ ½áÊø Q,¡û ©¦ÉÏÒ»·â ¡ü©¦ÏÂÒ»·â <Space>,¡ı");
            else
	        prints("\x1b[44m\x1b[31m[Í¬×÷ÕßÔÄ¶Á] \x1b[33m »ØĞÅ R ©¦ ½áÊø Q,¡û ©¦ÉÏÒ»·â ¡ü©¦ÏÂÒ»·â <Space>,¡ı");
            clrtoeol();
            arg->readmode=READ_AUTHOR;
            arg->oldpos=conf->pos;
            goto reget;
        } else return READ_NEXT;
    case Ctrl('N'):
        list_select_add_key(conf,Ctrl('N'));
        break;
    case Ctrl('S'):
    case 'p':                  /*Add by SmallPig */
        if (arg->readmode==READ_NORMAL) {
           move(t_lines - 1, 0);
           if (DEFINE(getCurrentUser(), DEF_HIGHCOLOR))
	        prints("\x1b[44m\x1b[1;31m[Ö÷ÌâÔÄ¶Á] \x1b[33m »ØĞÅ R ©¦ ½áÊø Q,¡û ©¦ÉÏÒ»·â ¡ü©¦ÏÂÒ»·â <Space>,¡ı");
           else
	        prints("\x1b[44m\x1b[31m[Ö÷ÌâÔÄ¶Á] \x1b[33m »ØĞÅ R ©¦ ½áÊø Q,¡û ©¦ÉÏÒ»·â ¡ü©¦ÏÂÒ»·â <Space>,¡ı");
	   clrtoeol();
            arg->readmode=READ_THREAD;
            arg->oldpos=0;
            goto reget;
        } else return READ_NEXT;
    case Ctrl('X'):            /* Leeward 98.10.03 */
    case KEY_RIGHT:
        if (arg->readmode==READ_NORMAL) {
           move(t_lines - 1, 0);
           if (DEFINE(getCurrentUser(), DEF_HIGHCOLOR))
	        prints("\x1b[44m\x1b[1;31m[Ö÷ÌâÔÄ¶Á] \x1b[33m »ØĞÅ R ©¦ ½áÊø Q,¡û ©¦ÉÏÒ»·â ¡ü©¦ÏÂÒ»·â <Space>,¡ı");
           else
	        prints("\x1b[44m\x1b[31m[Ö÷ÌâÔÄ¶Á] \x1b[33m »ØĞÅ R ©¦ ½áÊø Q,¡û ©¦ÉÏÒ»·â ¡ü©¦ÏÂÒ»·â <Space>,¡ı");
	   clrtoeol();
            arg->readmode=READ_THREAD;
            arg->oldpos=conf->pos;
            goto reget;
        } else return READ_NEXT;
    case '~':            /*Haohmaru.98.12.05,ÏµÍ³¹ÜÀíÔ±Ö±½Ó²é×÷Õß×ÊÁÏ */
        ret=read_authorinfo(conf,fileinfo,NULL);
        break;
    case Ctrl('W'):            /*cityhunter 00.10.18²ì¿´°æÖ÷ĞÅÏ¢ */
        ret=read_showauthorBM(conf, fileinfo, NULL);
        break;
    case Ctrl('O'):
        clear();
        read_addauthorfriend(conf, fileinfo, NULL);
        return READ_NEXT;
    case 'Z':
    case 'z':
        if (!HAS_PERM(getCurrentUser(), PERM_PAGE))
            break;
        read_sendmsgtoauthor(conf, fileinfo, NULL);
        return READ_NEXT;
    case Ctrl('A'):            /*Add by SmallPig */
        clear();
        read_showauthor(conf, fileinfo, NULL);
        return READ_NEXT;
    case 'L':
        if (uinfo.mode != LOOKMSGS) {
            show_allmsgs();
            break;
        }
        return DONOTHING;
    case '!':                  /*Haohmaru 98.09.24 */
        Goodbye();
        break;
    case 'H':                  /* Luzi 1997.11.1 */
        r_lastmsg();
        break;
    case 'w':                  /* Luzi 1997.11.1 */
        if (!HAS_PERM(getCurrentUser(), PERM_PAGE))
            break;
        s_msg();
        break;
    case 'O':
    case 'o':                  /* Luzi 1997.11.1 */
        if (!HAS_PERM(getCurrentUser(), PERM_BASIC))
            break;
        t_friends();
        break;
    case 'u':                  /* Haohmaru 1999.11.28 */
        clear();
        modify_user_mode(QUERY);
        t_query(NULL);
        break;
	case 'U':		/* pig2532 2005.12.10 */
		return(board_query());
    }
#endif
    if ((ret==FULLUPDATE)&&(arg->oldpos!=0)) {
        conf->new_pos=arg->oldpos;
	arg->oldpos=0;
	ret=SELCHANGE;
	list_select_add_key(conf,KEY_REFRESH);
        arg->readmode=READ_NORMAL;
    }
    return ret;
}

int skip_post(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
#ifdef HAVE_BRC_CONTROL
    brc_add_read(fileinfo->id,currboardent,getSession());
#endif
    return GOTO_NEXT;
}

/************
  ³¬¼¶°æÃæÑ¡Ôñ£¬add by stiger
  *****/
#define MAXBOARDSEARCH 100
int super_board_count;
int super_board_now=0;

static int sb_show(struct _select_def *conf, int i)
{
	const struct boardheader *bp;
	int *result = (int *) (conf->arg);
    int dir = 0;
	struct BoardStatus *bptr;

	bp=getboard(result[i-1]);
	if(bp==NULL){
		prints(" ERROR ");
		return SHOW_CONTINUE;
	}
    bptr = getbstatus(result[i-1]);

//	prints(" %2d %-20s %-40s", i, bp->filename, bp->title);
    if (bp->flag&BOARD_GROUP) {
        dir = 1;
    }
    prints(" %4d%s", dir?bp->board_data.group_total:bptr->total, !dir&&bptr->total>9999?"":" ");
    prints("%2s %-16s %s%-36s", dir?"£«":"  ", bp->filename, (bp->flag & BOARD_VOTEFLAG) ? "\033[31;1mV\033[m" : " ",  bp->title+1); 

	return SHOW_CONTINUE;
}

static int sb_prekey(struct _select_def *conf, int *key)
{
    switch (*key) {
    case 'q':
	case 'e':
        *key = KEY_LEFT;
        break;
    case 'p':
    case 'k':
        *key = KEY_UP;
        break;
    case 'N':
        *key = KEY_PGDN;
        break;
    case 'n':
    case 'j':
        *key = KEY_DOWN;
        break;
    }
    return SHOW_CONTINUE;
}

static int sb_key(struct _select_def *conf, int key)
{
	switch(key){
	default:
		break;
	}

	return SHOW_CONTINUE;
}

static int sb_refresh(struct _select_def *conf)
{
    clear();
    docmdtitle("[³¬¼¶°æÃæÑ¡Ôñ]",
               "ÍË³ö[\x1b[1;32m¡û\x1b[0;37m] Ñ¡Ôñ[\x1b[1;32m¡ü\x1b[0;37m,\x1b[1;32m¡ı\x1b[0;37m] ½øÈë°æÃæ[\x1b[1;32mENTER\x1b[0;37m]");
    move(2, 0);
    setfcolor(WHITE, DEFINE(getCurrentUser(), DEF_HIGHCOLOR));
    setbcolor(BLUE);
    clrtoeol();
    prints("   %s   ÌÖÂÛÇøÃû³Æ       V Àà±ğ ×ªĞÅ  %-24s            ", "È«²¿ " , "ÖĞ  ÎÄ  Ğğ  Êö");
    resetcolor();
    update_endline();
    return SHOW_CONTINUE;
}

static int sb_getdata(struct _select_def *conf, int pos, int len)
{
    conf->item_count = super_board_count;
    return SHOW_CONTINUE;
}

static int sb_select(struct _select_def *conf)
{
	super_board_now = conf->pos;
	return SHOW_QUIT;
}

static int do_super_board_select(int result[])
{
	POINT *pts;
    struct _select_def grouplist_conf;
    int i;

    pts = (POINT *) malloc(sizeof(POINT) * BBS_PAGESIZE);
    for (i = 0; i < BBS_PAGESIZE; i++) {
        pts[i].x = 2;
        pts[i].y = i + 3;
    }
    bzero(&grouplist_conf, sizeof(struct _select_def));

	grouplist_conf.arg = result;
    grouplist_conf.item_count = super_board_count;
    grouplist_conf.item_per_page = BBS_PAGESIZE;
    grouplist_conf.flag = LF_VSCROLL | LF_BELL | LF_LOOP | LF_MULTIPAGE;
    grouplist_conf.prompt = "¡ô";
    grouplist_conf.item_pos = pts;
    grouplist_conf.title_pos.x = 0;
    grouplist_conf.title_pos.y = 0;
    grouplist_conf.pos = 1;
    grouplist_conf.page_pos = 1;

    grouplist_conf.show_data = sb_show;
    grouplist_conf.pre_key_command = sb_prekey;
    grouplist_conf.key_command = sb_key;
    grouplist_conf.show_title = sb_refresh;
    grouplist_conf.get_data = sb_getdata;
	grouplist_conf.on_select = sb_select;

    list_select_loop(&grouplist_conf);

	free(pts);

	return 0;
}

int super_select_board(char *bname)
{
	int result[MAXBOARDSEARCH];
	char searchname[STRLEN];

	clear();

	bname[0]='\0';
	searchname[0]='\0';

//	while(bname[0]=='\0'){
    	getdata(1, 0, "ËÑË÷°æÃæ¹Ø¼ü×Ö: ", searchname, STRLEN-1, DOECHO, NULL, false);
		if (searchname[0] == '\0' || searchname[0]=='\n')
			return 0;
		if( ( super_board_count = fill_super_board (getCurrentUser(),searchname, result, MAXBOARDSEARCH) ) <= 0 ){
			move(5,0);
			prints("Ã»ÓĞÕÒµ½ÈÎºÎÏà¹Ø°æÃæ\n");
			pressanykey();
			return 0;
		}
		super_board_now=0;
		do_super_board_select(result);
		if (super_board_now != 0){
			const struct boardheader *bp;
			bp=getboard(result[super_board_now-1]);
			if (bp==NULL){
				bname[0]='\0';
				return 0;
			}else
				strcpy(bname, bp->filename);
		}
//	}

	return 0;
}

extern int in_do_sendmsg;

int do_select(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
        /*
         * ÊäÈëÌÖÂÛÇøÃû Ñ¡ÔñÌÖÂÛÇø 
         */
{
    char bname[STRLEN], bpath[STRLEN];
    int bid;
    struct read_arg* arg=NULL;
	int addfav=0;
    int ret;
    
    if (conf!=NULL) arg=conf->arg;
	if( extraarg ) addfav = * ( (int *) extraarg ) ;

    /*
    move(0, 0);
    prints("Ñ¡ÔñÒ»¸öÌÖÂÛÇø (Ó¢ÎÄ×ÖÄ¸´óĞ¡Ğ´½Ô¿É, °´\033[1;32m#\033[0m½øÈë\033[1;31m¹Ø¼ü×Ö»ò°æÃæÃû³Æ\033[0mËÑË÷)");
    clrtoeol();
    prints("\nÊäÈëÌÖÂÛÇøÃû (°´¿Õ°×¼ü»òTab¼ü×Ô¶¯²¹Æë): ");
    clrtoeol();
    */

    /* etnlegend, 2006.04.07, ÕâµØ·½¸Ä¸Ä... */
    move(0,0);clrtoeol();
    prints("%s","Ñ¡ÔñÌÖÂÛÇø [ \033[1;32m#\033[m - \033[1;31m°æÃæÃû³Æ/¹Ø¼ü×ÖËÑË÷\033[m, "
        "\033[1;32mSPACE/TAB\033[m - ×Ô¶¯²¹È«, \033[1;32mESC\033[m - ÍË³ö ]");
    move(1,0);clrtoeol();
    if(uinfo.mode!=MMENU)
        prints("ÇëÊäÈëÌÖÂÛÇøÃû³Æ: ");
    else
        prints("ÇëÊäÈëÌÖÂÛÇøÃû³Æ [\033[1;32m%s\033[m]: ",currboard->filename);

    make_blist(addfav);               /* Éú³ÉËùÓĞBoardÃû ÁĞ±í */
	in_do_sendmsg=true;
    if((ret=namecomplete(NULL,bname))=='#'){ /* ÌáÊ¾ÊäÈë board Ãû */
		super_select_board(bname);
	}
	in_do_sendmsg=0;

    if(!*bname)
        return ((uinfo.mode!=MMENU||ret==KEY_ESC)?FULLUPDATE:CHANGEMODE);

	if(addfav){
		bid = EnameInFav(bname,getSession());

		if(bid){
			*((int *)extraarg) = bid;
    		board_setcurrentuser(uinfo.currentboard, -1);
    		uinfo.currentboard = 0;
    		UPDATE_UTMP(currentboard,uinfo);
    		return CHANGEMODE;
		}else
			*((int *)extraarg) = 0;
	}

    setbpath(bpath,bname);
    if(!dashd(bpath)||!(bid=getbnum_safe(bname,getSession()))){
        move(2,0);clrtoeol();
        prints("\033[1;37m%s\033[0;33m<ENTER>\033[m","´íÎóµÄÌÖÂÛÇøÃû³Æ...");
        WAIT_RETURN;
        return FULLUPDATE;
    }

    board_setcurrentuser(uinfo.currentboard, -1);
    uinfo.currentboard = bid;
    UPDATE_UTMP(currentboard,uinfo);
    board_setcurrentuser(uinfo.currentboard, 1);
    
    selboard = 1;

    currboardent=bid;
    currboard=(struct boardheader*)getboard(bid);

#ifdef HAVE_BRC_CONTROL
    brc_initial(getCurrentUser()->userid, bname,getSession());
#endif

    move(0, 0);
    clrtoeol();
    move(1, 0);
    clrtoeol();
    if (arg!=NULL) {
    arg->mode=DIR_MODE_NORMAL;
    setbdir(arg->mode, arg->direct, currboard->filename);     /* direct Éè¶¨ Îª µ±Ç°boardÄ¿Â¼ */
    }
    return CHANGEMODE;
}

/* ²éÑ¯°æÃæĞÅÏ¢ by pig2532 on 2005-12-10 */
int board_query()
{
	char bname[STRLEN];
	int bid;
    int oldmode;
    oldmode = uinfo.mode;
    modify_user_mode(QUERYBOARD);
	clear();
	move(2,0);
	clrtobot();
	prints("ÇëÊäÈëÄúÒª²éÑ¯µÄ°æÃæÓ¢ÎÄÃû³Æ£¬°´¿Õ¸ñ¼ü»òTab¼ü²¹Æë¡£");
	move(1,0);
	prints("°æÃæ²éÑ¯£º");
	make_blist(0);
	namecomplete(NULL, bname);
	if(*bname!='\0') {
        bid = getbnum_safe(bname,getSession());
        if (bid == 0)
    	{
            move(2, 0);
            prints("²»ÕıÈ·µÄÌÖÂÛÇø.");
            clrtoeol();
            pressreturn();
        } else {
            show_boardinfo(bname);
        }
	}
    modify_user_mode(oldmode);
	return FULLUPDATE;
}

int digest_mode(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{                               /* ÎÄÕªÄ£Ê½ ÇĞ»» */
    struct read_arg* arg=(struct read_arg*)conf->arg;
    if (arg->mode == DIR_MODE_DIGEST) {
        arg->newmode = DIR_MODE_NORMAL;
        setbdir(arg->newmode, arg->direct, currboard->filename);
    } else {
        arg->newmode = DIR_MODE_DIGEST;
        setbdir(arg->newmode, arg->direct, currboard->filename);
        if (!dashf(arg->direct)) {
            setbdir(arg->mode, arg->direct, currboard->filename);
            return FULLUPDATE;
        }
    }
    return NEWDIRECT;
}

int deleted_mode(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
    struct read_arg* arg=(struct read_arg*)conf->arg;

    if(!check_board_delete_read_perm(getCurrentUser(),currboard,1)){
        return DONOTHING;
    }

    if (arg->mode == DIR_MODE_DELETED) {
        arg->newmode = DIR_MODE_NORMAL;
        setbdir(arg->newmode, arg->direct, currboard->filename);
    } else {
        arg->newmode = DIR_MODE_DELETED;
        setbdir(arg->newmode, arg->direct, currboard->filename);
        if (!dashf(arg->direct)) {
            arg->mode = DIR_MODE_NORMAL;
            setbdir(arg->mode, arg->direct, currboard->filename);
            return DONOTHING;
        }
    }
    return NEWDIRECT;
}

int marked_mode(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
    struct read_arg* arg=NULL;
    if (conf!=NULL)
        arg=(struct read_arg*)conf->arg;
    if (arg==NULL||arg->mode!= DIR_MODE_MARK) {
        if (setboardmark(currboard->filename, -1)) {
            if (board_regenspecial(currboard->filename, DIR_MODE_MARK, NULL) == -1) {
                return FULLUPDATE;
            }
        }
        arg->newmode= DIR_MODE_MARK;
        setbdir(arg->newmode, arg->direct, currboard->filename);
        if (!dashf(arg->direct)) {
            arg->mode=DIR_MODE_NORMAL;
            setbdir(DIR_MODE_NORMAL, arg->direct, currboard->filename);
            return FULLUPDATE;
        }
        return NEWDIRECT;
    }
    return DONOTHING;
}

int title_mode(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
    struct stat st;

    struct read_arg* arg=NULL;
    if (conf!=NULL)
        arg=(struct read_arg*)conf->arg;
    if (!stat("heavyload", &st)) {
        move(t_lines - 1, 0);
        clrtoeol();
        prints("ÏµÍ³¸ºµ£¹ıÖØ£¬ÔİÊ±²»ÄÜÏìÓ¦Ö÷ÌâÔÄ¶ÁµÄÇëÇó...");
        pressanykey();
        return FULLUPDATE;
    }

    arg->newmode = DIR_MODE_THREAD;
    if (setboardtitle(currboard->filename, -1)) {
    	setbdir(arg->newmode, arg->direct, currboard->filename);
        if (gen_title(currboard->filename) == -1) {
            arg->mode=DIR_MODE_NORMAL;
            setbdir(DIR_MODE_NORMAL, arg->direct, currboard->filename);
            return FULLUPDATE;
        }
    }
    setbdir(arg->newmode, arg->direct, currboard->filename);
    if (!dashf(arg->direct)) {
        arg->mode=DIR_MODE_NORMAL;
        setbdir(DIR_MODE_NORMAL, arg->direct, currboard->filename);
        return FULLUPDATE;
    }
    return NEWDIRECT;
}

int self_mode(struct _select_def *conf,struct fileheader *fh,void *varg){
#define SM_QUIT(msg)                                            \
    do{                                                         \
        if(cptr!=MAP_FAILED){                                   \
            if(filedes!=-1){                                    \
                flock(filedes,LOCK_UN);                         \
                close(filedes);                                 \
            }                                                   \
            end_mmapfile(cptr,size,-1);                         \
        }                                                       \
        if(msg){                                                \
            move(t_lines-2,0);                                  \
            clrtoeol();                                         \
            move(t_lines-1,0);                                  \
            clrtoeol();                                         \
            prints("\033[1;34;47m\t%s\033[K\033[m",(msg));      \
            WAIT_RETURN;                                        \
            BBS_RETURN(FULLUPDATE);                             \
        }                                                       \
    }                                                           \
    while(0)
    struct read_arg *arg=(struct read_arg*)conf->arg;
    struct fileheader info;
    const struct fileheader *ptr;
    char dir[STRLEN],*cptr;
    int filedes,count,selected,i;
    const void *data;
    ssize_t length,writen;
    off_t size;
    if(arg->mode==DIR_MODE_SELF){
        arg->newmode=DIR_MODE_NORMAL;
        setbdir(DIR_MODE_NORMAL,arg->direct,currboard->filename);
        return NEWDIRECT;
    }
    BBS_TRY{
        cptr=MAP_FAILED;
        filedes=-1;
        if(!HAS_PERM(getCurrentUser(),PERM_LOGINOK))
            SM_QUIT("µ±Ç°ÓÃ»§²»¾ßÓĞ´¦Àí×ÔÉ¾ÎÄÕÂµÄÈ¨ÏŞ...");
        setbdir(DIR_MODE_JUNK,dir,currboard->filename);
        if(!safe_mmapfile(dir,O_RDONLY,PROT_READ,MAP_SHARED,&cptr,&size,NULL))
            SM_QUIT("Ä¿Ç°Ã»ÓĞ×ÔÉ¾ÎÄÕÂ, °´ <Enter> ¼ü¼ÌĞø...");
        setbdir(DIR_MODE_SELF,dir,currboard->filename);
        if((filedes=open(dir,O_WRONLY|O_CREAT|O_TRUNC,0644))==-1||flock(filedes,LOCK_EX)==-1)
            SM_QUIT("´ò¿ªÎÄ¼şÊ±·¢Éú´íÎó, °´ <Enter> ¼ü¼ÌĞø...");
        ptr=(const struct fileheader*)cptr;
        count=size/sizeof(struct fileheader);
        for(selected=0,i=0;i<count;i++){
            if(getCurrentUser()->firstlogin
                &&(get_posttime(&ptr[i])>getCurrentUser()->firstlogin)
                &&!strcmp(ptr[i].owner,getCurrentUser()->userid)){
                info=ptr[i];
                strnzhcpy(info.title,ptr[i].title,34);
                for(data=&info,length=sizeof(struct fileheader),writen=0;
                    writen!=-1&&length>0;vpm(data,writen),length-=writen){
                    writen=write(filedes,data,length);
                }
                selected++;
            }
        }
        SM_QUIT((!selected?"Ä¿Ç°Ã»ÓĞ×ÔÉ¾ÎÄÕÂ, °´ <Enter> ¼ü¼ÌĞø...":NULL));
    }
    BBS_CATCH{
    }
    BBS_END;
    arg->newmode=DIR_MODE_SELF;
    strcpy(arg->direct,dir);
    return NEWDIRECT;
#undef SM_QUIT
}

int junk_mode(struct _select_def *conf,struct fileheader *fh,void *varg){
    struct read_arg *arg=(struct read_arg*)conf->arg;
    if(!HAS_PERM(getCurrentUser(),PERM_SYSOP))
        return self_mode(conf,fh,varg);
    if(arg->mode==DIR_MODE_JUNK){
        arg->newmode=DIR_MODE_NORMAL;
        setbdir(arg->newmode,arg->direct,currboard->filename);
    }
    else{
        arg->newmode=DIR_MODE_JUNK;
        setbdir(DIR_MODE_JUNK,arg->direct,currboard->filename);
        if(!dashf(arg->direct)){
            arg->newmode=DIR_MODE_NORMAL;
            setbdir(arg->mode,arg->direct,currboard->filename);
            return DONOTHING;
        }
    }
    return NEWDIRECT;
}

static char search_data[STRLEN];

int search_mode(struct _select_def* conf,struct fileheader *fileinfo,int mode, char *index)
/* added by bad 2002.8.8 search mode*/
{
    int count = 0;
    struct read_arg* arg=(struct read_arg*)conf->arg;

    strncpy(search_data, index, STRLEN);
    arg->newmode=mode;
    setbdir(mode,arg->direct, currboard->filename);
    if (mode == DIR_MODE_ORIGIN && !setboardorigin(currboard->filename, -1)) {
        return NEWDIRECT;
    }

    count = board_regenspecial(currboard->filename, mode, index);

    if (count<0)
	return FULLUPDATE;

    if (count==0) {
        clear();
        move(t_lines-2,0);
        prints("Ã»ÓĞÂú×ãÌõ¼şµÄÎÄÕÂ");
		pressanykey();
		unlink(arg->direct);
		arg->newmode=DIR_MODE_NORMAL;
        setbdir(DIR_MODE_NORMAL, arg->direct, currboard->filename);
    }
    return NEWDIRECT;
}

int search_x(char * b, char * s)
{
    void *hdll;
	typedef int (*iquery_board_func)(char *b, char *s);
	iquery_board_func iquery_board;
    int oldmode;

    oldmode = uinfo.mode;
    modify_user_mode(SERVICES);
    hdll=dlopen("service/libiquery.so",RTLD_NOW);
    if(hdll)
    {
        const char* error;
        if((iquery_board = (iquery_board_func)dlsym(hdll,"iquery_board"))!=NULL)
            iquery_board(b, s);
        else
        if ((error = dlerror()) != NULL)  {
            clear();
            prints ("%s\n", error);
            pressanykey();
        }
        dlclose(hdll);
    }
    modify_user_mode(oldmode);
    return 0;
}

/* etnlegend, 2006.10.06, Ôö¼ÓÓÃ»§´¦Àí×ÔÉ¾ÎÄÕÂ¹¦ÄÜ... */
int change_mode(struct _select_def *conf,struct fileheader *fh,int mode){
    static char title[32];
    struct read_arg *arg=(struct read_arg*)conf->arg;
    char buf[STRLEN],echo[STRLEN],ans[4];
    if(!mode){
        move(t_lines-2,0);
        clrtoeol();
        prints("%s","ÇĞ»»Ä£Ê½µ½: 0)È¡Ïû 1)ÎÄÕªÇø 2)Í¬Ö÷Ìâ 3)±£ÁôÇø 4)Ô­×÷ 5)Í¬×÷Õß 6)±êÌâ¹Ø¼ü×Ö");
        move(t_lines-1,0);
        clrtoeol();
        getdata(t_lines-1,12,"7)³¬¼¶ÎÄÕÂÑ¡Ôñ"
#ifdef NEWSMTH
            " 8)±¾°æ¾«»ªÇøËÑË÷"
#endif /* NEWSMTH */
            " 9)×ÔÉ¾ÎÄÕÂ [1]: ",ans,2,DOECHO,NULL,true);
        switch(ans[0]){
            case '0':
                return FULLUPDATE;
            case '1':
                mode=DIR_MODE_DIGEST;
                break;
            case '2':
                mode=DIR_MODE_THREAD;
                break;
            case '3':
                mode=DIR_MODE_MARK;
                break;
            case '4':
                mode=DIR_MODE_ORIGIN;
                buf[0]=0;
                break;
            case '5':
                mode=DIR_MODE_AUTHOR;
                move(t_lines-2,0);
                clrtoeol();
                move(t_lines-1,0);
                clrtoeol();
                sprintf(echo,"ÄúÏ£Íû²éÕÒÄÄÎ»ÓÃ»§µÄÎÄÕÂ[%s]: ",fh->owner);
                getdata(t_lines-1,0,echo,buf,IDLEN+2,DOECHO,NULL,true);
                if(!buf[0])
                    strcpy(buf,fh->owner);
                if(!buf[0])
                    return FULLUPDATE;
                break;
            case '6':
                mode=DIR_MODE_TITLE;
                move(t_lines-2,0);
                clrtoeol();
                move(t_lines-1,0);
                clrtoeol();
                snprintf(echo,STRLEN,"ÄúÏ£Íû²éÕÒµÄ±êÌâ¹Ø¼ü×Ö[%s]: ",title);
                getdata(t_lines-1,0,echo,buf,32,DOECHO,NULL,true);
                if(buf[0])
                    strcpy(title,buf);
                if(!title[0])
                    return FULLUPDATE;
                strcpy(buf,title);
                break;
            case '7':
                mode=DIR_MODE_SUPERFITER;
                break;
#ifdef NEWSMTH
            case '8':
                move(t_lines-2,0);
                clrtoeol();
                move(t_lines-1,0);
                clrtoeol();
                snprintf(echo,STRLEN,"ÄúÏ£Íû²éÕÒµÄÈ«ÎÄ¹Ø¼ü×Ö[%s]: ",title);
                getdata(t_lines-1,0,echo,buf,64,DOECHO,NULL,true);
                if(buf[0])
                    strcpy(title,buf);
                if(title[0]){
                    strcpy(buf,title);
                    search_x(currboard->filename,buf);
                }
                return FULLUPDATE;
#endif /* NEWSMTH */
            case '9':
                mode=DIR_MODE_SELF;
                break;
            default:
                mode=DIR_MODE_NORMAL;
                break;
        }
        if(mode!=DIR_MODE_SUPERFITER&&(arg->mode==DIR_MODE_AUTHOR||arg->mode==DIR_MODE_TITLE||arg->mode==DIR_MODE_SELF))
            unlink(arg->direct);
    }
    switch(mode){
        case DIR_MODE_NORMAL:
        case DIR_MODE_DIGEST:
            return digest_mode(conf,fh,NULL);
        case DIR_MODE_THREAD:
            return title_mode(conf,fh,NULL);
        case DIR_MODE_MARK:
            return marked_mode(conf,fh,NULL);
        case DIR_MODE_ORIGIN:
        case DIR_MODE_AUTHOR:
        case DIR_MODE_TITLE:
            return search_mode(conf,fh,mode,buf);
        case DIR_MODE_SUPERFITER:
            return super_filter(conf,fh,NULL);
        case DIR_MODE_SELF:
            return self_mode(conf,fh,NULL);
        case DIR_MODE_DELETED:
            return deleted_mode(conf,fh,NULL);
        case DIR_MODE_JUNK:
            return junk_mode(conf,fh,NULL);
        default:
            break;
    }
    return DIRCHANGED;
}

#ifdef NEWSMTH
int set_board_rule(struct boardheader *bh, int flag)
{
	int pos;
	struct boardheader newbh;

	pos = getbid(bh->filename, NULL);
	if(!pos) return -1;

    memcpy(&newbh,bh,sizeof(struct boardheader));
	if(flag) newbh.flag|=BOARD_RULES;
	else newbh.flag&=~BOARD_RULES;

    set_board(pos,&newbh,bh);

	if(flag){
		char buf[256];
		char buf2[256];

		setvfile(buf, bh->filename, "rules");
		sprintf(buf2,"%sÍ¨¹ı%sÖÎ°æ·½Õë", getCurrentUser()->userid, bh->filename);
		post_file(getCurrentUser(), "", buf, "BoardRules", buf2, 0, 2, getSession());
		post_file(getCurrentUser(), "", buf, "BoardManager", buf2, 0, 2, getSession());
		sprintf(buf2,"ÉóºËÍ¨¹ı%s°æÖÎ°æ·½Õë", bh->filename);
		post_file(getCurrentUser(), "", buf, bh->filename, buf2, 0, 2, getSession());
	}
	return 0;
}
#endif

#ifndef USE_PRIMORDIAL_TOP10
static int select_top(void);
#endif /* USE_PRIMORDIAL_TOP10 */

int read_hot_info()
{
    char ans[4];
	char prompt[STRLEN];
    move(t_lines - 1, 0);
    clrtoeol();
    snprintf(prompt,STRLEN,"Ñ¡Ôñ: 1)Ê®´ó»°Ìâ "
#ifdef HAPPY_BBS
        "2)ÏµÄÚÈÈµã "
#else
        "2)Ê®´ó×£¸£ "
#endif
        "3)½üÆÚÈÈµã 4)ÏµÍ³ÈÈµã 5)ÈÕÀúÈÕ¼Ç %s[1]: ",
#ifdef NEWSMTH
        ((uinfo.mode==READING)?"6)ÖÎ°æ·½Õë ":"")
#else
        ""
#endif
        );
    getdata(t_lines - 1, 0, prompt, ans, 3, DOECHO, NULL, true);
    switch (ans[0])
	{
    case '2':
        show_help("etc/posts/bless");
        break;
    case '3':
		show_help("0Announce/hotinfo");
        break;
    case '4':
		showsysinfo("0Announce/systeminfo");
        pressanykey();
        break;
    case '5':
            if (getCurrentUser()&&!HAS_PERM(getCurrentUser(),PERM_DENYRELAX))
            exec_mbem("@mod:service/libcalendar.so#calendar_main");
        break;
#ifdef NEWSMTH
	case '6':
	{
		char fpath[256];
		struct stat st;
		if(uinfo.mode!=READING)
			break;
		if(!(currboard->flag & BOARD_RULES) && !HAS_PERM(getCurrentUser(),PERM_SYSOP) && (getCurrentUser()->title==0) && !chk_currBM(currBM, getCurrentUser()) ){
			clear();
			move(3,0);
			prints("%s°æÉĞÎ´ÓĞÍ¨¹ıÉóºËµÄÖÎ°æ·½Õë\n", currboard->filename);
			pressanykey();
			break;
		}
		setvfile(fpath, currboard->filename, "rules");
    	if(stat(fpath, &st)==-1){
			clear();
			move(3,0);
			prints("°æÖ÷ÉĞÎ´Ìá½»ÖÎ°æ·½Õë\n");
			pressanykey();
       		break;
		}
		if(!(currboard->flag & BOARD_RULES)){
			clear();
			move(3,0);
			prints("%s°æÖÎ°æ·½ÕëÉĞÎ´Í¨¹ıÉóºË,ÅúºÅ:%d\n", currboard->filename, st.st_mtime);
			pressanykey();
		}
		show_help(fpath);
		if(!(currboard->flag & BOARD_RULES) && HAS_PERM(getCurrentUser(), PERM_SYSOP) ){
			char ans[4];
			clear();
			move(3,0);
			prints("%s°æÖÎ°æ·½ÕëÉĞÎ´Í¨¹ıÉóºË,ÅúºÅ:%d\n", currboard->filename, st.st_mtime);
        	getdata(t_lines - 1, 0, "ÄúÒªÍ¨¹ı¸Ã°æµÄÖÎ°æ·½ÕëÂğ (Y/N)? [N] ", ans, 3, DOECHO, NULL, true);
			if(ans[0]=='y' || ans[0]=='Y'){
				int ret;
				ret = set_board_rule(currboard, 1);
				move(6,0);
				prints("Í¨¹ı%s:%d\n",(ret==0)?"³É¹¦":"Ê§°Ü",ret);
				pressreturn();
			}
		}
		break;
	}
#endif
	case '1':
	default:

/* etnlegend, 2006.05.30, ÔÄ¶ÁÊ®´ó ... */

#ifndef USE_PRIMORDIAL_TOP10
        select_top();
#else /* USE_PRIMORDIAL_TOP10 */
        show_help("etc/posts/day");
#endif /* USE_PRIMORDIAL_TOP10 */

    }
    return FULLUPDATE;
}

#ifndef NOREPLY
int do_reply(struct _select_def* conf,struct fileheader *fileinfo)
/* reply POST */
{
    char buf[255];

    if (fileinfo==NULL)
        return DONOTHING;
    if (fileinfo->accessed[1] & FILE_READ) {    /*Haohmaru.99.01.01.ÎÄÕÂ²»¿Ére */
        clear();
        move(3, 0);
        prints("\n\n            ºÜ±§Ç¸£¬±¾ÎÄÒÑ¾­ÉèÖÃÎª²»¿ÉreÄ£Ê½,Çë²»ÒªÊÔÍ¼ÌÖÂÛ±¾ÎÄ...\n");
        pressreturn();
        return FULLUPDATE;
    }
    setbfile(buf, currboard->filename, fileinfo->filename);
    strcpy(replytitle, fileinfo->title);
    post_article(conf,buf, fileinfo);
    replytitle[0] = '\0';
    return FULLUPDATE;
}
#endif

int garbage_line(const char *str)
{                               /* ÅĞ¶Ï±¾ĞĞÊÇ·ñÊÇ ÎŞÓÃµÄ */
    int qlevel = 0;

    while (*str == ':' || *str == '>') {
        str++;
        if (*str == ' ')
            str++;
        else
            break;
        if (qlevel++ >= QUOTELEV)
            return 1;
    }
    while (*str == ' ' || *str == '\t')
        str++;
    if (qlevel >= QUOTELEV)
        if (strstr(str, "Ìáµ½:\n") || strstr(str, ": ¡¿\n") || strncmp(str, "==>", 3) == 0 || strstr(str, "µÄÎÄÕÂ Ëµ"))
            return 1;
    return (*str == '\n');
}

/* When there is an old article that can be included -jjyang */
void do_quote(char *filepath, char quote_mode, char *q_file, char *q_user)
{                               /* ÒıÓÃÎÄÕÂ£¬ È«¾Ö±äÁ¿quote_file,quote_user, */
    FILE *inf, *outf;
    char *qfile, *quser;
    char buf[256], *ptr;
    char op;
    int bflag;
    int line_count = 0;         /* Ìí¼Ó¼òÂÔÄ£Ê½¼ÆÊı Bigman: 2000.7.2 */

    qfile = q_file;
    quser = q_user;
    bflag = strncmp(qfile, "mail", 4);  /* ÅĞ¶ÏÒıÓÃµÄÊÇÎÄÕÂ»¹ÊÇĞÅ */
    outf = fopen(filepath, "a");
    if (outf==NULL) {
    	bbslog("user","do_quote() fopen(%s):%s",filepath,strerror(errno));
    	return;
    }
    if (*qfile != '\0' && (inf = fopen(qfile, "rb")) != NULL) {  /* ´ò¿ª±»ÒıÓÃÎÄ¼ş */
        op = quote_mode;
		if (op != 'N') {        /* ÒıÓÃÄ£Ê½Îª N ±íÊ¾ ²»ÒıÓÃ */
            skip_attach_fgets(buf, 256, inf);
            /* È¡³öµÚÒ»ĞĞÖĞ ±»ÒıÓÃÎÄÕÂµÄ ×÷ÕßĞÅÏ¢ */
            if ((ptr = strrchr(buf, ')')) != NULL) {    /* µÚÒ»¸ö':'µ½×îºóÒ»¸ö ')' ÖĞµÄ×Ö·û´® */
                ptr[1] = '\0';
                if ((ptr = strchr(buf, ':')) != NULL) {
                    quser = ptr + 1;
                    while (*quser == ' ')
                        quser++;
                }
            }
            /*---	period	2000-10-21	add '\n' at beginning of Re-article	---*/
            if (bflag)
                fprintf(outf, "\n¡¾ ÔÚ %s µÄ´ó×÷ÖĞÌáµ½: ¡¿\n", quser);
            else
                fprintf(outf, "\n¡¾ ÔÚ %s µÄÀ´ĞÅÖĞÌáµ½: ¡¿\n", quser);

            if (op == 'A') {    /* ³ıµÚÒ»ĞĞÍâ£¬È«²¿ÒıÓÃ */
				int enterflag=1;
                while (skip_attach_fgets(buf, 256, inf) != 0) {
					if(enterflag)
                    	fprintf(outf, ": %s", buf);
					else
                    	fprintf(outf, "%s", buf);
                    if(buf[strlen(buf)-1]!='\n')
						enterflag=0;
					else
						enterflag=1;
                }
            } else if (op == 'R') {
                while (skip_attach_fgets(buf, 256, inf) != 0)
                    if (buf[0] == '\n')
                        break;
                while (skip_attach_fgets(buf, 256, inf) != 0) {
                    if (Origin2(buf))   /* ÅĞ¶ÏÊÇ·ñ ¶à´ÎÒıÓÃ */
                        continue;
                    fprintf(outf, "%s", buf);

                }
            } else {
                while (skip_attach_fgets(buf, 256, inf) != 0)
                    if (buf[0] == '\n')
                        break;
                while (skip_attach_fgets(buf, 256, inf) != 0) {
                    if (strcmp(buf, "--\n") == 0)       /* ÒıÓÃ µ½Ç©ÃûµµÎªÖ¹ */
                        break;
                    if(buf[strlen(buf)-1]!='\n') {
                        char ch;
                        while((ch=fgetc(inf))!=EOF)
                            if(ch=='\n') break;
                    }
                    if (buf[250] != '\0')
                        strcpy(buf + 250, "\n");
                    if (!garbage_line(buf)) {   /* ÅĞ¶ÏÊÇ·ñÊÇÎŞÓÃĞĞ */
                        fprintf(outf, ": %s", buf);
#if defined(QUOTED_LINES) && QUOTED_LINES >= 3
                        if (op == 'S') {        /* ¼òÂÔÄ£Ê½,Ö»ÒıÓÃÇ°¼¸ĞĞ Bigman:2000.7.2 */
                            line_count++;
                            if (line_count >= QUOTED_LINES) {
                                fprintf(outf, ": ...................");
                                break;
                            }
                        }
#endif
                    }
                }
            }
        }

        fprintf(outf, "\n");
        fclose(inf);
    }
    /*
     * *q_file = '\0';
     * *q_user = '\0';
     */

    if ((getSession()->currentmemo->ud.signum > 0) && !(getCurrentUser()->signature == 0 || Anony == 1)) {       /* Ç©ÃûµµÎª0Ôò²»Ìí¼Ó */
        if (getCurrentUser()->signature < 0)
            addsignature(outf, getCurrentUser(), (rand() % getSession()->currentmemo->ud.signum) + 1);
        else
            addsignature(outf, getCurrentUser(), getCurrentUser()->signature);
    }
    fclose(outf);
}

int do_post(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{                               /* ÓÃ»§post */
#ifndef NOREPLY
    *replytitle = '\0';
#endif
    *quote_user = '\0';
    return post_article(conf,"", NULL);
}

int post_reply(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
        /*
         * »ØĞÅ¸øPOST×÷Õß 
         */
{
    char uid[STRLEN];
    char title[STRLEN];
    char *t;
    FILE *fp;
    char q_file[STRLEN];


    if (fileinfo==NULL)
        return DONOTHING;
    if (!HAS_PERM(getCurrentUser(), PERM_LOGINOK) || !strcmp(getCurrentUser()->userid, "guest"))  /* guest ÎŞÈ¨ */
        return 0;
    /*
     * Ì«ºİÁË°É,±»·âpost¾Í²»ÈÃ»ØĞÅÁË
     * if (!HAS_PERM(getCurrentUser(),PERM_POST)) return; Haohmaru.99.1.18 
     */

    /*
     * ·â½ûMail Bigman:2000.8.22 
     */
    if (HAS_PERM(getCurrentUser(), PERM_DENYMAIL)) {
        clear();
        move(3, 10);
        prints("ºÜ±§Ç¸,ÄúÄ¿Ç°Ã»ÓĞMailÈ¨ÏŞ!");
        pressreturn();
        return FULLUPDATE;
    }

    modify_user_mode(SMAIL);

    /*
     * indicate the quote file/user 
     */
    setbfile(q_file, currboard->filename, fileinfo->filename);
    strncpy(quote_user, fileinfo->owner, OWNER_LEN);
    quote_user[OWNER_LEN - 1] = 0;

    /*
     * find the author 
     */
    if (strchr(quote_user, '.')) {
        genbuf[0] = '\0';
        fp = fopen(q_file, "rb");
        if (fp != NULL) {
            skip_attach_fgets(genbuf, 255, fp);
            fclose(fp);
        }

        t = strtok(genbuf, ":");
        if (strncmp(t, "·¢ĞÅÈË", 6) == 0 || strncmp(t, "Posted By", 9) == 0 || strncmp(t, "×÷  ¼Ò", 6) == 0) {
            t = (char *) strtok(NULL, " \r\t\n");
            strcpy(uid, t);
        } else {
            prints("Error: Cannot find Author ... \n");
            pressreturn();
        }
    } else
        strcpy(uid, quote_user);

    /*
     * make the title 
     */
    if (toupper(fileinfo->title[0]) != 'R' || fileinfo->title[1] != 'e' || fileinfo->title[2] != ':')
        strcpy(title, "Re: ");
    else
        title[0] = '\0';
    strncat(title, fileinfo->title, ARTICLE_TITLE_LEN - 5);

    clear();

    switch (do_send(uid, title, q_file)) {
    case -1:
        prints("ÏµÍ³ÎŞ·¨ËÍĞÅ\n");
        break;
    case -2:
        prints("ËÍĞÅ¶¯×÷ÒÑ¾­ÖĞÖ¹\n");
        break;
    case -3:
        prints("Ê¹ÓÃÕß '%s' ÎŞ·¨ÊÕĞÅ\n", uid);
        break;
    case -4:
        prints("¶Ô·½ĞÅÏäÒÑÂú£¬ÎŞ·¨ÊÕĞÅ\n");
        break;
    default:
        prints("ĞÅ¼şÒÑ³É¹¦µØ¼Ä¸øÔ­×÷Õß %s\n", uid);
    }
	/* »Ö¸´ in_mail ±äÁ¿Ô­À´µÄÖµ.
	 * do_send() ÀïÃæ´ó¸´ÔÓ, ¾Í¼ÓÔÚÕâÀï°É, by flyriver, 2003.5.9 */
    pressreturn();
    return FULLUPDATE;
}

static int show_board_notes(char bname[30], int all)
{                               /* ÏÔÊ¾°æÖ÷µÄ»° */
    char buf[256];

    sprintf(buf, "vote/%s/notes", bname);       /* ÏÔÊ¾±¾°æµÄ°æÖ÷µÄ»° vote/°æÃû/notes */
    if (dashf(buf)) {
		if(all)
        	ansimore2(buf, false, 0, 0);
		else
        ansimore2(buf, false, 0, 23 /*19 */ );
        return 1;
    } else if (dashf("vote/notes")) {   /* ÏÔÊ¾ÏµÍ³µÄ»° vote/notes */
        ansimore2("vote/notes", false, 0, 23 /*19 */ );
        return 1;
    }
    return -1;
}

int add_attach(char* file1, char* file2, char* filename)
{
    FILE* fp,*fp2;
    struct stat st;
    uint32_t size;
    char o[8]={0,0,0,0,0,0,0,0};
    char buf[1024*16];
    int i;
    if(stat(file2, &st)==-1)
        return 0;
    if(st.st_size>=MAXATTACHMENTSIZE&&!HAS_PERM(getCurrentUser(), PERM_SYSOP)) {
        unlink(file2);
        return 0;
    }
    size=htonl(st.st_size);
    fp2=fopen(file2, "rb");
    if(!fp2) return 0;
    fp=fopen(file1, "ab");
    fwrite(o,1,8,fp);
    filename = filter_upload_filename(filename);
    fwrite(filename, 1, strlen(filename)+1, fp);
    fwrite(&size,1,4,fp);
    while((i=fread(buf,1,1024*16,fp2))) {
        fwrite(buf,1,i,fp);
    }
    
    fclose(fp2);
    fclose(fp);
    unlink(file2);
    return st.st_size;
}

int process_upload(int nUpload, int maxShow, char *ans, struct ea_attach_info* ai)
{
    char buf[256], ses[20];
    char *upload = NULL;
    int i, totalsize = 0;

    if (ans[0] == 'U' && nUpload<MAXATTACHMENTCOUNT) {
        chdir("tmp");
        upload = bbs_zrecvfile();
        chdir("..");
        if (upload != NULL && *upload) {
            char uploaded_file[PATHLEN];
            snprintf(uploaded_file, PATHLEN, "tmp/%s", upload);
            upload_add_file(uploaded_file, upload, getSession());
        }
    } else if (ans[0] == 'u' && ans[1]) {
        int att = atoi(ans+1);
        if (att > 0 && att <= nUpload) {
            upload_del_file(ai[att-1].name, getSession());
        }
    }

    get_telnet_sessionid(ses, getSession()->utmpent);
    snprintf(buf, sizeof(buf), "¸½¼şÉÏ´«µØÖ·: (ÇëÎğ½«´ËÁ´½Ó·¢ËÍ¸ø±ğÈË)\n\033[4mhttp://%s/bbsupload.php?sid=%s\033[m\n", get_my_webdomain(0), ses);
    clear();
    prints(buf);
    nUpload = upload_read_fileinfo(ai, getSession());
    prints("%s", "ÒÑÉÏ´«¸½¼şÁĞ±í: (°´ \033[1;32mu\033[m Ë¢ĞÂ, \033[1;32mu<ĞòºÅ>\033[m É¾³ıÏàÓ¦ĞòºÅ¸½¼ş"
#ifdef SSHBBS
        ", \033[1;32mU\033[m zmodem ÉÏ´«"
#endif
        ")\n");
    for(i=0;i<nUpload;i++) {
        if (i>=nUpload-maxShow) {
            snprintf(buf, sizeof(buf), "[%02d] %-60.60s (%7d ×Ö½Ú)\n", i+1, ai[i].name, ai[i].size);
            prints("%s", buf);
        }
        totalsize += ai[i].size;
    }
    if (nUpload > maxShow) {
        snprintf(buf, sizeof(buf), "¸½¼şÊıÁ¿³¬¹ı %d ¸ö£¬Ç°ÃæµÄ²»ÏÔÊ¾ÁË£¡\n", maxShow);
        prints("%s", buf);
    }
    snprintf(buf, sizeof(buf), "\n\033[1;36m¹² \033[1;37m%d\033[1;36m ¸½¼ş/¼Æ \033[1;37m%d\033[1;36m ×Ö½Ú\033[m"
             " \033[1;36m(ÉÏÏŞ \033[1;37m%d\033[1;36m ¸½¼ş/ \033[1;37m%d\033[1;36m ×Ö½Ú)\033[m\n",
             nUpload, totalsize, MAXATTACHMENTCOUNT, MAXATTACHMENTSIZE);
    prints("%s", buf);
    check_upload = 1;
    return nUpload;
}

int post_article(struct _select_def* conf,char *q_file, struct fileheader *re_file)
{                               /*ÓÃ»§ POST ÎÄÕÂ */
    struct fileheader post_file;
    char filepath[STRLEN];
    char buf[256], buf2[256], buf3[STRLEN], buf4[STRLEN];
//	char tmplate[STRLEN];
	int use_tmpl=0;
    int aborted, anonyboard;
    int replymode = 1;          /* Post New UI */
    char ans[8], ooo, include_mode = 'S';
    struct boardheader *bp;
    long eff_size;/*ÓÃÓÚÍ³¼ÆÎÄÕÂµÄÓĞĞ§×ÖÊı*/
    int nUpload = 0;
    struct ea_attach_info ai[MAXATTACHMENTCOUNT];
    int mailback = 0;		/* stiger,»Ø¸´µ½ĞÅÏä */
	int ret = DIRCHANGED;

    char direct[PATHLEN];
    int cmdmode;
#ifdef FILTER
    int returnvalue;
#endif

    check_upload = 0;

    if (conf!=NULL)  {
        struct read_arg* arg;
        arg=(struct read_arg*)conf->arg;
        cmdmode=arg->mode;
    }
    else {
	cmdmode=DIR_MODE_NORMAL;
    }

    if (true == check_readonly(currboard->filename))      /* Leeward 98.03.28 */
        return FULLUPDATE;

#ifdef AIX_CANCELLED_BY_LEEWARD
    if (true == check_RAM_lack())       /* Leeward 98.06.16 */
        return FULLUPDATE;
#endif

#ifdef HAPPY_BBS
    anonyboard = anonymousboard(currboard->filename);
    if (!anonyboard)
#endif
    modify_user_mode(POSTING);
    setbdir(DIR_MODE_NORMAL, direct, currboard->filename);
    if(!((cmdmode==DIR_MODE_MARK)||(cmdmode==DIR_MODE_THREAD)||(cmdmode==DIR_MODE_NORMAL)||(cmdmode==DIR_MODE_TOP10)))
    {
        move(3, 0);
        clrtobot();
        prints("\n\n     Ä¿Ç°ÊÇÎÄÕª»òÖ÷ÌâÄ£Ê½, ËùÒÔ²»ÄÜ·¢±íÎÄÕÂ.(°´»Ø³µÀë¿ª)\n");
        pressreturn();
        clear();
        return FULLUPDATE;
    }
    if (!haspostperm(getCurrentUser(), currboard->filename)) { /* POSTÈ¨ÏŞ¼ì²é */
        move(3, 0);
        clrtobot();
            prints("\n\n        ´ËÌÖÂÛÇøÊÇÎ¨¶ÁµÄ, »òÊÇÄúÉĞÎŞÈ¨ÏŞÔÚ´Ë·¢±íÎÄÕÂ.\n");
            prints("        Èç¹ûÄúÉĞÎ´×¢²á£¬ÇëÔÚ¸öÈË¹¤¾ßÏäÄÚÏêÏ¸×¢²áÉí·İ\n");
            prints("        Î´Í¨¹ıÉí·İ×¢²áÈÏÖ¤µÄÓÃ»§£¬Ã»ÓĞ·¢±íÎÄÕÂµÄÈ¨ÏŞ¡£\n");
            prints("        Ğ»Ğ»ºÏ×÷£¡ :-) \n");
        pressreturn();
        clear();
        return FULLUPDATE;
#ifdef NEWSMTH
    }
    else if(!check_score_level(getCurrentUser(),currboard)){
        move(3,0);
        clrtobot();
        prints("\n\n    \033[1;33m%s\033[0;33m<Enter>\033[m",
            "ÄúµÄ»ı·Ö²»·ûºÏµ±Ç°ÌÖÂÛÇøµÄÉè¶¨, ÔİÊ±ÎŞ·¨ÔÚµ±Ç°ÌÖÂÛÇø·¢±íÎÄÕÂ...");
        WAIT_RETURN;
        return FULLUPDATE;
#endif /* NEWSMTH */
    } else if (deny_me(getCurrentUser()->userid, currboard->filename)) { /* °æÖ÷½ûÖ¹POST ¼ì²é */
		if( !HAS_PERM(getCurrentUser(), PERM_SYSOP) ){
        	move(3, 0);
        	clrtobot();
            prints("\n\n\t\tÄúÒÑ±»¹ÜÀíÈËÔ±È¡ÏûÔÚµ±Ç°°æÃæµÄ·¢ÎÄÈ¨ÏŞ...\n");
        	pressreturn();
        	clear();
        	return FULLUPDATE;
		}else{
			clear();
            getdata(3,0,"ÄúÒÑ±»È¡ÏûÔÚµ±Ç°°æÃæµÄ·¢ÎÄÈ¨ÏŞ, ÊÇ·ñÇ¿ÖÆ·¢ÎÄ? [y/N]: ",buf,2,DOECHO,NULL,true);
			if( buf[0]!='y' && buf[0]!='Y' ){
				clear();
				return FULLUPDATE;
			}
		}
    }

    memset(&post_file, 0, sizeof(post_file));
//	tmplate[0]='\0';
    clear();
    show_board_notes(currboard->filename, 0);        /* °æÖ÷µÄ»° */
#ifndef NOREPLY                 /* titleÊÇ·ñ²»ÓÃRe: */
    if (replytitle[0] != '\0') {
        buf4[0] = ' ';
        /*
         * if( strncasecmp( replytitle, "Re:", 3 ) == 0 ) Change By KCN:
         * why use strncasecmp? 
         */
        if (strncmp(replytitle, "Re:", 3) == 0)
            strcpy(buf, replytitle);
        else if (strncmp(replytitle, "©À ", 3) == 0)
            sprintf(buf, "Re: %s", replytitle + 3);
        else if (strncmp(replytitle, "©¸ ", 3) == 0)
            sprintf(buf, "Re: %s", replytitle + 3);
        else
            sprintf(buf, "Re: %s", replytitle);
        buf[78] = '\0';
    } else
#endif
    {
        buf[0] = '\0';
        buf4[0] = '\0';
        replymode = 0;
    }
    if (getCurrentUser()->signature > getSession()->currentmemo->ud.signum)      /*Ç©ÃûµµNo.¼ì²é */
        getCurrentUser()->signature = (getSession()->currentmemo->ud.signum == 0) ? 0 : 1;
    anonyboard = anonymousboard(currboard->filename);     /* ÊÇ·ñÎªÄäÃû°æ */
    if (!strcmp(currboard->filename, "Announce"))
        Anony = 1;
    else if (anonyboard)
        Anony = ANONYMOUS_DEFAULT;
    else
        Anony = 0;

#ifdef FREE
#define RAND_SIG_KEY 'X'
#define RAND_SIG_KEYS "X"
#define ANONY_KEY 'L'
#define ANONY_KEYS "L"
#else
#define RAND_SIG_KEY 'L'
#define RAND_SIG_KEYS "L"
#define ANONY_KEY 'M'
#define ANONY_KEYS "M"
#endif

    while (1) {                 /* ·¢±íÇ°ĞŞ¸Ä²ÎÊı£¬ ¿ÉÒÔ¿¼ÂÇÌí¼Ó'ÏÔÊ¾Ç©Ãûµµ' */
        sprintf(buf3, "ÒıÑÔÄ£Ê½ [%c]", include_mode);
        move(t_lines - 4, 0);
        clrtoeol();
        prints("\033[m·¢±íÎÄÕÂÓÚ %s ÌÖÂÛÇø  %s %s %s\n", currboard->filename, (anonyboard) ? (Anony == 1 ? "\033[1mÒª\033[mÊ¹ÓÃÄäÃû" : "\033[1m²»\033[mÊ¹ÓÃÄäÃû") : "", mailback? "»Ø¸´µ½ĞÅÏä":"",use_tmpl?"Ê¹ÓÃÄ£°å":"");
        clrtoeol();
        prints("Ê¹ÓÃ±êÌâ: %s\n", (buf[0] == '\0') ? "[ÕıÔÚÉè¶¨Ö÷Ìâ]" : buf);

        if (buf4[0] == '\0' || buf4[0] == '\n') {
        	clrtoeol();
        	if (getCurrentUser()->signature < 0)
            	prints("Ê¹ÓÃËæ»úÇ©Ãûµµ     %s", (replymode) ? buf3 : " ");
        	else
            	prints("Ê¹ÓÃµÚ %d ¸öÇ©Ãûµµ     %s", getCurrentUser()->signature, (replymode) ? buf3 : " ");

            move(t_lines - 1, 0);
            clrtoeol();
            strcpy(buf4, buf);
            getdata(t_lines - 1, 0, "±êÌâ: ", buf4, 79, DOECHO, NULL, false);
            if ((buf4[0] == '\0' || buf4[0] == '\n')) {
                if (buf[0] != '\0') {
                    buf4[0] = ' ';
                    continue;
                } else
                    return FULLUPDATE;
            }
            strcpy(buf, buf4);
            continue;
        }

       	clrtoeol();
       	if (getCurrentUser()->signature < 0)
			strcpy(buf2, "Ê¹ÓÃËæ»úÇ©Ãûµµ");
       	else
           	sprintf(buf2, "Ê¹ÓÃµÚ %d ¸öÇ©Ãûµµ", getCurrentUser()->signature);
		prints("%s  %s °´\033[1;32m0\033[m~\033[1;32m%d/V/" RAND_SIG_KEYS "\033[mÑ¡/¿´/Ëæ»úÇ©Ãûµµ", buf2, (replymode) ? buf3 : " ", getSession()->currentmemo->ud.signum);

        move(t_lines - 1, 0);
        clrtoeol();
        /*
         * Leeward 98.09.24 add: viewing signature(s) while setting post head 
         */
        sprintf(buf2, "%s£¬\033[1;32mb\033[m»Ø¸´µ½ĞÅÏä£¬\033[1;32mT\033[m¸Ä±êÌâ£¬%s%s%s\033[1;32mEnter\033[m¼ÌĞø: ", 
                (replymode) ? "\033[1;32mS/Y/N/R/A\033[m ¸ÄÒıÑÔÄ£Ê½" : "\033[1;32mP\033[mÊ¹ÓÃÄ£°å", (anonyboard) ? "\033[1;32m" ANONY_KEYS "\033[mÄäÃû£¬" : "",
				(currboard->flag&BOARD_ATTACH)?"\033[1;32mu\033[m´«¸½¼ş, ":"", "\033[1;32mQ\033[m·ÅÆú, ");
        getdata(t_lines - 1, 0, buf2, ans, 4, DOECHO, NULL, true);
        ooo = toupper(ans[0]);       /* Leeward 98.09.24 add; delete below toupper */
        if ((ooo - '0') >= 0 && ooo - '0' <= 9) {
            int ii = atoi(ans);
            if (ii <= getSession()->currentmemo->ud.signum)
                getCurrentUser()->signature = ii;
        } else if ((ooo == 'S' || ooo == 'Y' || ooo == 'N' || ooo == 'A' || ooo == 'R') && replymode) {
            include_mode = ooo;
        } else if (ooo == 'T') {
            buf4[0] = '\0';
		} else if (ooo == 'P') {
			if( use_tmpl >= 0)
				use_tmpl = use_tmpl ? 0 : 1;
				/*
			if( replymode == 0 ){
				choose_tmpl( tmplate );
				clear();
				if( tmplate[0] ){
					setbfile( buf2, currboard->filename, tmplate );
					ansimore2(buf2, false, 0, 18);
				}
			}*/
		} else if (ooo == 'B') {
			/* if( replymode == 0 ) */
				mailback = mailback ? 0 : 1;
        } else if (ooo == ANONY_KEY) {
            Anony = (Anony == 1) ? 0 : 1;
        } else if (ooo == RAND_SIG_KEY) {
            getCurrentUser()->signature = -1;
		} else if (ooo == 'Q') {
        	return FULLUPDATE;
        } else if (ooo == 'V') {     /* Leeward 98.09.24 add: viewing signature(s) while setting post head */
            sethomefile(buf2, getCurrentUser()->userid, "signatures");
            move(t_lines - 1, 0);
            if (askyn("Ô¤ÉèÏÔÊ¾Ç°Èı¸öÇ©Ãûµµ, ÒªÏÔÊ¾È«²¿Âğ", false) == true)
                ansimore(buf2, 0);
            else {
                clear();
                ansimore2(buf2, false, 0, 18);
            }
        } else if (ooo == 'U') {
            if(currboard->flag&BOARD_ATTACH || HAS_PERM(getCurrentUser(),PERM_SYSOP)) {
                nUpload = process_upload(nUpload, 10, ans, ai);
            }
        } else {
            /*
             * Changed by KCN,disable color title 
             */
            process_control_chars(buf);
            strnzhcpy(post_file.title, buf, ARTICLE_TITLE_LEN);
            strcpy(save_title, post_file.title);
            if (save_title[0] == '\0') {
                return FULLUPDATE;
            }
            break;
        }
    }                           /* ÊäÈë½áÊø */

    setbfile(filepath, currboard->filename, "");
    if ((aborted = GET_POSTFILENAME(post_file.filename, filepath)) != 0) {
        move(3, 0);
        clrtobot();
        prints("\n\nÎŞ·¨´´½¨ÎÄ¼ş:%d...\n", aborted);
        pressreturn();
        clear();
        return FULLUPDATE;
    }

    in_mail = false;

    /*
     * strncpy(post_file.owner,(anonyboard&&Anony)?
     * "Anonymous":getCurrentUser()->userid,STRLEN) ;
     */
    strncpy(post_file.owner, (anonyboard && Anony) ? currboard->filename : getCurrentUser()->userid, OWNER_LEN);
    post_file.owner[OWNER_LEN - 1] = 0;

	/* »Ø¸´µ½ĞÅÏä£¬stiger */
	if(mailback)
		post_file.accessed[1] |= FILE_MAILBACK;

    /*
     * if ((!strcmp(currboard,"Announce"))&&(!strcmp(post_file.owner,"Anonymous")))
     * strcpy(post_file.owner,"SYSOP");
     */

    if ((!strcmp(currboard->filename, "Announce")) && (!strcmp(post_file.owner, currboard->filename)))
        strcpy(post_file.owner, "SYSOP");

    setbfile(filepath, currboard->filename, post_file.filename);

    bp=currboard;
    if (bp->flag & BOARD_OUTFLAG) {
        local_article = 0;
        /* »Ø¸´ local_save µÄÎÄÕÂÄ¬ÈÏ local_save - atppp */
        if (re_file) local_article = (re_file->innflag[0]=='L' && re_file->innflag[1]=='L')?2:0;
    } else
        local_article = 1;
    if (!strcmp(post_file.title, buf) && q_file[0] != '\0')
        if (q_file[119] == 'L') /* FIXME */
            local_article = 1; //Õâ¸öµØ·½Ì«¹îÒìÁË£¬ÍêÈ«¿´²»¶®£¬²»ÖªµÀ¶Ô local_save ÓĞÊ²Ã´Ó°Ïì¡£

#ifdef HAPPY_BBS
    if (!anonyboard)
#endif
    modify_user_mode(POSTING);

	if( use_tmpl > 0 ){
		FILE *fp,*fp1;
		char filepath1[STRLEN];
		char buff[256];
		char title_prefix[STRLEN];

		if( ! strncmp(post_file.title, "Re: ",4) )
			strncpy(title_prefix, post_file.title+4, ARTICLE_TITLE_LEN);
		else
			strncpy(title_prefix, post_file.title, ARTICLE_TITLE_LEN);

		title_prefix[STRLEN-1]='\0';

		sprintf(filepath1,"%s.mbak",filepath);
		aborted =  choose_tmpl( title_prefix , filepath ) ;
		if(aborted != -1){
			if( (fp=fopen( filepath1,"w"))!=NULL){
				if((fp1=fopen(filepath,"r"))==NULL){
					aborted = -1;
				}else{
					if( title_prefix[0] ){
						if( ! strncmp(post_file.title, "Re: ",4) )
							snprintf(save_title, ARTICLE_TITLE_LEN, "Re: %s", title_prefix );
						else
							snprintf(save_title, ARTICLE_TITLE_LEN , "%s", title_prefix );

						save_title[ARTICLE_TITLE_LEN-1]='\0';
            			strnzhcpy(post_file.title, save_title, ARTICLE_TITLE_LEN);
					}

					write_header(fp, getCurrentUser(), 0, currboard->filename, post_file.title, Anony, 0,getSession());
					while(fgets(buff,255,fp1))
						fprintf(fp,"%s",buff);

					fclose(fp);
					fclose(fp1);

					unlink(filepath);
					rename(filepath1,filepath);

				}
			}else{
				aborted = -1;
			}
		}
	}
				

	do_quote(filepath, include_mode, q_file, quote_user);       /*ÒıÓÃÔ­ÎÄÕÂ */

    strnzhcpy(quote_title, save_title, sizeof(quote_title));
    strcpy(quote_board, currboard->filename);

    if( use_tmpl <= 0 )
        aborted = vedit(filepath, true, &eff_size, NULL, 1);    /* ½øÈë±à¼­×´Ì¬ */
    else{
        add_loginfo(filepath, getCurrentUser(), currboard->filename, Anony,getSession());
        eff_size = get_effsize(filepath);
    }

    post_file.eff_size = eff_size;

    strnzhcpy(post_file.title, save_title, ARTICLE_TITLE_LEN);
    if (aborted == 1 || !(bp->flag & BOARD_OUTFLAG)) {  /* local save */
        post_file.innflag[1] = 'L';
        post_file.innflag[0] = 'L';
    } else if (aborted == 0) {
        post_file.innflag[1] = 'S';
        post_file.innflag[0] = 'S';
        outgo_post(&post_file, currboard->filename, save_title,getSession());
    }

    if (aborted == -1) {        /* È¡ÏûPOST */
        my_unlink(filepath);
        clear();
        return FULLUPDATE;
    }
    /*
     * ÔÚboards°æ°æÖ÷·¢ÎÄ×Ô¶¯Ìí¼ÓÎÄÕÂ±ê¼Ç Bigman:2000.8.12 
     */
    if (!strcmp(currboard->filename, "BM_Apply") && !HAS_PERM(getCurrentUser(), PERM_OBOARDS) && HAS_PERM(getCurrentUser(), PERM_BOARDS)) {
        post_file.accessed[0] |= FILE_SIGN;
    }
    if(check_upload) {
        FILE *fp;
        if ((fp = fopen(filepath, "ab")) != NULL) {
            upload_post_append(fp, &post_file, getSession());
        }
        fclose(fp);
    }
#ifdef FILTER
    returnvalue =
#endif
        after_post(getCurrentUser(), &post_file, currboard->filename, re_file, !(Anony && anonyboard),getSession());
    Anony = 0;                  /*Inital For ShowOut Signature */
    
    if (!junkboard(currboard->filename)) {
        getCurrentUser()->numposts++;
    }
#ifdef FILTER
    if (returnvalue == -2) {
        clear();
        move(3, 0);
        prints("\n\n        ºÜ±§Ç¸£¬±¾ÎÄ¿ÉÄÜº¬ÓĞ²»µ±ÄÚÈİ£¬Ğè¾­ÉóºË·½¿É·¢±í¡£\n\n"
                   "        ¸ù¾İ¡¶ÕÊºÅ¹ÜÀí°ì·¨¡·£¬±»ÏµÍ³¹ıÂËµÄÎÄÕÂÊÓÍ¬¹«¿ª·¢±í¡£ÇëÄÍĞÄµÈ´ı\n"
                   "    Õ¾ÎñÈËÔ±µÄÉóºË£¬²»Òª¶à´Î³¢ÊÔ·¢±í´ËÎÄÕÂ¡£\n\n"
                   "        ÈçÓĞÒÉÎÊ£¬ÇëÖÂĞÅ SYSOP ×ÉÑ¯¡£");
        pressreturn();
        return FULLUPDATE;
    }
#endif
    switch (cmdmode) {
    case 2:
        ret=title_mode(conf,NULL,NULL);
        break;
    case 3:
        ret=marked_mode(conf,NULL,NULL);
        break;
    }
	if (ret==NEWDIRECT)
		return ret;
    return DIRCHANGED;
}

int edit_post(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
        /*
         * POST ±à¼­
         */
{
    char buf[512];
    char *t;
    long eff_size;
    long attachpos;
    bool dobmlog=false;
    struct read_arg* arg=(struct read_arg*) conf->arg;
    int ret;
    
    ret = deny_modify_article(currboard, fileinfo, arg->mode, getSession());
    if (ret) {
        switch(ret) {
        case -2:
            move(3, 0);
            clrtobot();
            prints("\n\n\t\tÄúÒÑ±»¹ÜÀíÈËÔ±È¡ÏûÔÚµ±Ç°°æÃæµÄ·¢ÎÄÈ¨ÏŞ...\n");
            pressreturn();
            clear();
            return FULLUPDATE;
            break;
        case -5:
            check_readonly(currboard->filename);
            return FULLUPDATE;
            break;
        default:
            return DONOTHING;
            break;
        }
    }

#ifdef AIX_CANCELLED_BY_LEEWARD
    if (true == check_RAM_lack())       /* Leeward 98.06.16 */
        return FULLUPDATE;
#endif

    modify_user_mode(EDIT);

    if (!HAS_PERM(getCurrentUser(), PERM_SYSOP) && chk_currBM(currBM, getCurrentUser()))
        dobmlog=true;

    clear();
    strcpy(buf, arg->direct);
    if ((t = strrchr(buf, '/')) != NULL)
        *t = '\0';
#if 0 /* #ifndef LEEWARD_X_FILTER removed by atppp */
    sprintf(genbuf, "/bin/cp -f %s/%s tmp/%d.editpost.bak", buf, fileinfo->filename, getpid()); /* Leeward 98.03.29 */
    system(genbuf);
#endif

    sprintf(genbuf, "%s/%s", buf, fileinfo->filename);
	strncpy(buf, genbuf, 512);
	buf[511]=0;
	attachpos = fileinfo->attachment;
    if (vedit_post(buf, false, &eff_size,&attachpos) != -1) {
		int changemark=0;
		if (attachpos != fileinfo->attachment){
            fileinfo->attachment=attachpos;
			changemark |= FILE_ATTACHPOS_FLAG;
		}
        if (eff_size!=fileinfo->eff_size) {
        	fileinfo->eff_size = eff_size;
			changemark |= FILE_EFFSIZE_FLAG;
		}
		if (changemark){
            struct write_dir_arg dirarg;
            init_write_dir_arg(&dirarg);
            dirarg.fd=arg->fd;
            dirarg.ent = conf->pos;
            change_post_flag(&dirarg, arg->mode, currboard,  
                fileinfo,changemark, fileinfo,dobmlog,getSession());
            free_write_dir_arg(&dirarg);
        }
        if (ADD_EDITMARK)
            add_edit_mark(buf, 0, /*NULL*/ fileinfo->title,getSession());
    }
    newbbslog(BBSLOG_USER, "edited post '%s' on %s", fileinfo->title, currboard->filename);
    return FULLUPDATE;
}

int edit_title(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
        /*
         * ±à¼­ÎÄÕÂ±êÌâ 
         */
{
    char buf[STRLEN];
    struct read_arg* arg=(struct read_arg*) conf->arg;
    int ent=conf->pos;

    /*
     * Leeward 99.07.12 added below 2 variables 
     */
    long i=0;
    struct fileheader xfh;
    int fd, ret;

    ret = deny_modify_article(currboard, fileinfo, arg->mode, getSession());
    if (ret) {
        switch(ret) {
        case -2:
            move(3, 0);
            clrtobot();
            prints("\n\n\t\tÄúÒÑ±»¹ÜÀíÈËÔ±È¡ÏûÔÚµ±Ç°°æÃæµÄ·¢ÎÄÈ¨ÏŞ...\n");
            pressreturn();
            clear();
            return FULLUPDATE;
            break;
        case -5:
            check_readonly(currboard->filename);
            return FULLUPDATE;
            break;
        default:
            return DONOTHING;
            break;
        }
    }

    strcpy(buf, fileinfo->title);
    getdata(t_lines - 1, 0, "ĞÂÎÄÕÂ±êÌâ: ", buf, 78, DOECHO, NULL, false);      /*ÊäÈë±êÌâ */
    if (buf[0] != '\0'&&strcmp(buf,fileinfo->title)) {
        char tmp[STRLEN * 2], *t;
        char tmp2[STRLEN];      /* Leeward 98.03.29 */

#ifdef FILTER
        if (check_badword_str(buf, strlen(buf), getSession())) {
            clear();
            move(3, 0);
            outs("     ºÜ±§Ç¸£¬¸Ã±êÌâ¿ÉÄÜº¬ÓĞ²»Ç¡µ±µÄÄÚÈİ£¬Çë×ĞÏ¸¼ì²é»»¸ö±êÌâ¡£");
            pressreturn();
            return PARTUPDATE;
        }
#endif
        strcpy(tmp2, fileinfo->title);  /* Do a backup */
        process_control_chars(buf);
        strnzhcpy(fileinfo->title, buf, ARTICLE_TITLE_LEN);

        strcpy(tmp, arg->direct);
        if ((t = strrchr(tmp, '/')) != NULL)
            *t = '\0';
        sprintf(genbuf, "%s/%s", tmp, fileinfo->filename);

        if(strcmp(tmp2,buf)){
			add_edit_mark(genbuf, 2, buf,getSession());
			newbbslog(BBSLOG_USER,"edit_title %s %s %s",currboard->filename, tmp2 , buf);
		}
        /*
         * Leeward 99.07.12 added below to fix a big bug
         */
		/* add by stiger */
	if(conf->pos>arg->filecount) {
            ent = get_num_records(arg->dingdirect,sizeof(struct fileheader));
            fd = open(arg->dingdirect, O_RDONLY, 0);
	} else fd=arg->fd;
	/* add end */
	if (fd!=-1) {
            for (i = ent; i > 0; i--) {//todo: dingdirect needn't read,should read from boardstatus
                if (0 == get_record_handle(fd, &xfh, sizeof(xfh), i)) {
                    if (0 == strcmp(xfh.filename, fileinfo->filename)) {
                        ent = i;
                        break;
                    }
                }
            }
	    if(conf->pos>arg->filecount) {
                close(fd);
                if (i!=0) 
                    substitute_record(arg->dingdirect, fileinfo, sizeof(*fileinfo), ent);
                board_update_toptitle(arg->bid, true);
            } else
                if (i!=0) 
                    substitute_record(arg->direct, fileinfo, sizeof(*fileinfo), ent);
        }
        if (0 == i)
            return PARTUPDATE;
        /*
         * Leeward 99.07.12 added above to fix a big bug
         */

		if(arg->mode != DIR_MODE_ORIGIN && fileinfo->id == fileinfo->groupid){
			if( setboardorigin(currboard->filename, -1) ){
				board_regenspecial(currboard->filename,DIR_MODE_ORIGIN,NULL);
			}else{
				char olddirect[PATHLEN];
	    		setbdir(DIR_MODE_ORIGIN, olddirect, currboard->filename);
				if ((fd = open(olddirect, O_RDWR, 0644)) >= 0){
					struct fileheader tmpfh;
					if (get_records_from_id(fd, fileinfo->id, &tmpfh, 1, &ent) == 0){
						close(fd);
					}else{
						close(fd);
   	                	substitute_record(olddirect, fileinfo, sizeof(*fileinfo), ent);
					}
				}
   	     //setboardorigin(currboard->filename, 1);
			}
		}
        setboardtitle(currboard->filename, 1);
    }
    return PARTUPDATE;
}

/* etnlegend, 2006.10.12, ĞŞÀíÖÃµ×ÎÊÌâ... */
int del_ding(struct _select_def *conf,struct fileheader *info,void *varg){
    struct read_arg *arg=(struct read_arg*)conf->arg;
    char ans[4];
    if(arg->mode!=DIR_MODE_NORMAL||!chk_currBM(currBM,getCurrentUser()))
        return DONOTHING;
    getdata((t_lines-1),0,"É¾³ıÖÃµ×ÎÄÕÂ, È·ÈÏ²Ù×÷ (Y/N) [N]: ",ans,2,DOECHO,NULL,true);
    if(toupper(ans[0])!='Y')
        return FULLUPDATE;
    if(do_del_ding(currboard->filename,arg->bid,(conf->pos-arg->filecount),info,getSession())==-1){
        move(t_lines-1,0);
        clrtoeol();
        prints("\033[m%s\033[0;33m<Enter>\033[m","²Ù×÷¹ı³ÌÖĞ·¢Éú´íÎó...");
        WAIT_RETURN;
        return FULLUPDATE;
    }
    return DIRCHANGED;
}

/* stiger, ÖÃ¶¥ */
int zhiding_post(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
    struct read_arg* arg=(struct read_arg*)conf->arg;
    if (fileinfo==NULL)
        return DONOTHING;
    if(conf->pos>arg->filecount)
        return del_ding(conf,fileinfo,extraarg);
    if (add_top(fileinfo, currboard->filename, 0)!=0)
        return DONOTHING;
    return DIRCHANGED;
}

int noreply_post(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
	/* add by stiger ,20030414, ÖÃ¶¥Ñ¡Ôñ*/
    char ans[4];
    int mode=0; /* 0x1°ßÖñ, 0x2:ÍÆ¼ö°æ°ßÖñ, 0x4:owner */
	int can=0; /*0x1:noreply  0x2:ding 0x4:commend */
    int ret=FULLUPDATE;
	char buf[100];

#ifdef COMMEND_ARTICLE
    int bnum;
    const struct boardheader *bp;

    bnum = getbid(COMMEND_ARTICLE,&bp);
    if( bnum && chk_currBM(bp->BM, getCurrentUser()) )
		mode |= 0x2 ;
#endif
    if (fileinfo==NULL)
        return DONOTHING;
	if(chk_currBM(currBM, getCurrentUser())) mode |= 0x1;
#if defined(OPEN_NOREPLY) || defined(COMMEND_ARTICLE)
	if(!strcmp(getCurrentUser()->userid, fileinfo->owner)) mode |= 0x4;
#endif


	if( (mode&0x1) 
#ifdef OPEN_NOREPLY
		|| (mode & 0x4)
#endif
	  ) can |= 0x1;

	if(mode & 0x1) can |= 0x2;

#ifdef COMMEND_ARTICLE
	if ( (mode & 0x1) || (mode & 0x2) || (mode & 0x4) ) can |= 0x4;
#endif

	if(can==0) return DONOTHING;

	sprintf(buf,"ÇĞ»»: 0)È¡Ïû %s%s%s[%d]", (can&0x1)?"1)²»¿Ére±ê¼Ç ":"", (can&0x2)?"2)ÖÃ¶¥±ê¼Ç ":"", (can&0x4)?"3)ÍÆ¼ö ":"", (can&0x1)?1:0);

    move(t_lines - 1, 0);
    clrtoeol();
    getdata(t_lines - 1, 0, buf, ans, 3, DOECHO, NULL, true);

    if (ans[0] == ' ') {
        ans[0] = ans[1];
        ans[1] = 0;
    }
    if (ans[0]=='0') return FULLUPDATE;
	else if(ans[0]=='2'){
		if( !(can & 0x2) )
			return FULLUPDATE;
		return zhiding_post(conf,fileinfo,extraarg);
	}
#ifdef COMMEND_ARTICLE
	else if(ans[0]=='3'){
		if(!(can & 0x4))
			return FULLUPDATE;
		return do_commend(conf,fileinfo,extraarg);
	}
#endif
	else{
		if( !(can & 0x1) )
			return FULLUPDATE;
		ret=set_article_flag(conf,fileinfo, FILE_NOREPLY_FLAG);
	}
	return ret;
}

#ifdef DENYANONY
int deny_anony(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
	char anonybuf[256];
	struct fileheader tmpfh;
	int ret=0;
	int fd;

    if(!anonymousboard(currboard->filename) || strcmp(currboard->filename, fileinfo->owner))
		return DONOTHING;
	if(!HAS_PERM(getCurrentUser(), PERM_SYSOP))
		return DONOTHING;
	setbfile(anonybuf, currboard->filename, ".ANONYDIR");
	if ((fd = open(anonybuf, O_RDWR, 0644)) >= 0){
		ret = get_records_from_id(fd, fileinfo->id, &tmpfh, 1, NULL);
		close(fd);

		if(ret != 0){
			ret = giveup_addpost(tmpfh.owner);
			if(ret){
				char title[80];
				char buff[256];
				FILE *fp;
				sprintf(buff,"tmp/%s.%d.ad", getCurrentUser()->userid, (int)getpid());
				if((fp=fopen(buff,"w"))==NULL){
					ret = 0;
				}else{
					fprintf(fp,"%s :\n\n",tmpfh.owner);
					fprintf(fp,"ÓÉÓÚÄúÔÚ\033[1;31m%s\033[m°æµÄÄäÃûÎÄÕÂ\033[1;31m%s\033[m,%s¾ö¶¨×·¼ÓÈ¡ÏûÄúµÄÈ«Õ¾postÈ¨ÏŞ1Ìì\n",currboard->filename,fileinfo->title,getCurrentUser()->userid);
					fprintf(fp,"\nÄäÃû·â½û¶ÔÓÚÄú¿´À´Äú½«ÊÇ±»Ç¿ÖÆ½ä·¢ÎÄÈ¨ÏŞ1Ìì£¬ÇëÄú·ÅĞÄ£¬Ã»ÓĞÈËÖªµÀ±»·â½ûµÄÈË¾ßÌåÊÇÄã£¬Õ¾³¤ºÍ°ßÖñÒ²¶¼²»ÖªµÀ\n");
					fclose(fp);

					sprintf(title,"%sÈ¡Ïû%s°æÄäÃû×÷ÕßÔ­id·¢ÎÄÈ¨ÏŞ1Ìì", getCurrentUser()->userid, currboard->filename);
					mail_file("SYSOP", buff, tmpfh.owner, title, BBSPOST_COPY, NULL);
					unlink(buff);
					securityreport(title, NULL,NULL, getSession());
				}
			}
		}
	}

	clear();
	if(ret)
		prints("³É¹¦\n");
	else
		prints("Ê§°Ü\n");
	pressanykey();

	return FULLUPDATE;
}
#endif

int del_post(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
    char direct[MAXPATH];
    struct read_arg* arg=conf->arg;
    int ent,ret;
    int flag=(POINTDIFF)extraarg;
    struct write_dir_arg delarg;

    if (fileinfo==NULL)
        return DONOTHING;
    ent=conf->pos;
    /* add by stiger */
    if (ent>arg->filecount)
        return del_ding(conf,fileinfo,extraarg);

    if (arg->mode== DIR_MODE_DELETED|| arg->mode== DIR_MODE_JUNK)
        return DONOTHING;

    if (deny_del_article(currboard, fileinfo, getSession())) {
        return DONOTHING;
    }

    if (!(flag&ARG_NOPROMPT_FLAG)) {
        char buf[STRLEN];
        a_prompt(-1, "É¾³ıÎÄÕÂ£¬È·ÈÏÂğ£¿(Y/N) [N] ", buf);
        if (buf[0] != 'Y' && buf[0] != 'y') {     /* if not yes quit */
            return FULLUPDATE;
        }
    }

    if (arg->writearg==NULL) {
        init_write_dir_arg(&delarg);
        if ((arg->mode!=DIR_MODE_NORMAL)&& arg->mode != DIR_MODE_DIGEST) {
            setbdir(DIR_MODE_NORMAL, direct, currboard->filename);
            delarg.filename=direct;
        } else {
            delarg.fd=arg->fd;
            delarg.ent=conf->pos;
        }
        ret=do_del_post(getCurrentUser(),&delarg,fileinfo,currboard->filename,DIR_MODE_NORMAL,flag,getSession());
        free_write_dir_arg(&delarg);
    } else
        ret=do_del_post(getCurrentUser(),arg->writearg,fileinfo,currboard->filename,DIR_MODE_NORMAL,flag,getSession());
    if (ret != 0) {
        if (!(flag&ARG_NOPROMPT_FLAG)) {
            char buf[STRLEN];
            a_prompt(-1, "É¾³ıÊ§°Ü, Çë°´ Enter ¼ÌĞø << ", buf);
            return FULLUPDATE;
        }
    }
	ret=DIRCHANGED;
    if (!(flag&ARG_BMFUNC_FLAG)&&arg->mode) {
        switch (arg->mode) {
        case DIR_MODE_THREAD:
            ret=title_mode(conf,fileinfo,extraarg);
            break;
        case DIR_MODE_MARK:
            ret=marked_mode(conf,fileinfo,extraarg);
            break;
        case DIR_MODE_ORIGIN:
        case DIR_MODE_AUTHOR:
        case DIR_MODE_TITLE:
            ret=search_mode(conf,fileinfo,arg->mode, search_data);
            break;
        default:
            break;
        }
    }
	if (ret==NEWDIRECT)
		return NEWDIRECT;
    return DIRCHANGED;
}

/* Added by netty to handle post saving into (0)Announce */
int Save_post(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
    int ret;
    char filepath[PATHLEN], ans[STRLEN], buf[STRLEN];
    bool append;

    struct read_arg* arg=(struct read_arg*)conf->arg;
    if (!HAS_PERM(getCurrentUser(), PERM_SYSOP))
        if (!chk_currBM(currBM, getCurrentUser()))
            return DONOTHING;
    sprintf(filepath, "tmp/bm.%s", getCurrentUser()->userid);
    append = false;
    if (dashf(filepath)) {
        sprintf(buf, "Òª¸½¼ÓÔÚ¾ÉÔİ´æµµÖ®ºóÂğ?(Y/N/C) [Y]: ");
        a_prompt(-1, buf, ans);
        if ((ans[0] == 'N' || ans[0] == 'n')) {
            append = false;
        } else if (ans[0] == 'C' || ans[0] == 'c')
            return DONOTHING;
        else {
            append = true;
        }
    }
    ret=a_Save(NULL, currboard->filename, fileinfo, append, arg->direct, conf->pos, getCurrentUser()->userid);
    if (ret) {
        struct write_dir_arg dirarg;
        struct fileheader data;
        init_write_dir_arg(&dirarg);
        dirarg.fd=arg->fd;
        dirarg.ent = conf->pos;
        data.accessed[0]=FILE_IMPORTED;
        change_post_flag(&dirarg, 
            arg->mode,
            currboard, 
            fileinfo, 
            FILE_IMPORT_FLAG, &data, true,getSession());
        free_write_dir_arg(&dirarg);
        sprintf(buf, " ÒÑ½«¸ÃÎÄÕÂ´æÈëÔİ´æµµ, Çë°´ ENTER ¼üÒÔ¼ÌĞø << ");
        a_prompt(-1, buf, ans);
	return DIRCHANGED;
    }
    return DONOTHING;
}

/* Semi_save ÓÃÀ´°ÑÎÄÕÂ´æµ½Ôİ´æµµ£¬Í¬Ê±É¾³ıÎÄÕÂµÄÍ·Î² Life 1997.4.6 */
int Semi_save(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
    int ret;
    char filepath[PATHLEN], ans[STRLEN], buf[STRLEN];
    bool append;
    struct read_arg* arg=(struct read_arg*)conf->arg;
    if (!HAS_PERM(getCurrentUser(), PERM_SYSOP))
        if (!chk_currBM(currBM, getCurrentUser()))
            return DONOTHING;
    sprintf(filepath, "tmp/bm.%s", getCurrentUser()->userid);
    append = false;
    if (dashf(filepath)) {
        sprintf(buf, "Òª¸½¼ÓÔÚ¾ÉÔİ´æµµÖ®ºóÂğ?(Y/N/C) [Y]: ");
        a_prompt(-1, buf, ans);
        if ((ans[0] == 'N' || ans[0] == 'n')) {
            append = false;
        } else if (ans[0] == 'C' || ans[0] == 'c')
            return DONOTHING;
        else {
            append = true;
        }
    }
    ret=a_SeSave("0Announce", currboard->filename, fileinfo, append,arg->direct,conf->pos,1, getCurrentUser()->userid);
    if (ret) {
        struct write_dir_arg dirarg;
        struct fileheader data;
        init_write_dir_arg(&dirarg);
        dirarg.fd=arg->fd;
        dirarg.ent = conf->pos;
        data.accessed[0]=FILE_IMPORTED;
        change_post_flag(&dirarg, 
            arg->mode,
            currboard, 
            fileinfo, 
            FILE_IMPORT_FLAG, &data, true,getSession());
        free_write_dir_arg(&dirarg);
        sprintf(buf, " ÒÑ½«¸ÃÎÄÕÂ´æÈëÔİ´æµµ, Çë°´ ENTER ¼üÒÔ¼ÌĞø << ");
        a_prompt(-1, buf, ans);
	return DIRCHANGED;
   }
    return DONOTHING;
}

/* Added by netty to handle post saving into (0)Announce */
int Import_post(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
    char szBuf[STRLEN],*p;
    struct read_arg* arg=(struct read_arg*)conf->arg;
    int ret=FULLUPDATE;

    if (!HAS_PERM(getCurrentUser(), PERM_SYSOP))
        if (!chk_currBM(currBM, getCurrentUser())
#ifdef FB2KPC
			&& !haspc(getCurrentUser()->userid)
#endif
						)
            return DONOTHING;

    if (fileinfo->accessed[0] & FILE_IMPORTED) {        /* Leeward 98.04.15 */
        a_prompt(-1, "±¾ÎÄÔø¾­±»ÊÕÂ¼½ø¾«»ªÇø¹ı. ÏÖÔÚÔÙ´ÎÊÕÂ¼Âğ? (Y/N) [N]: ", szBuf);
        if (szBuf[0] != 'y' && szBuf[0] != 'Y')
            return FULLUPDATE;
    }

    /* etnlegend, 2006.03.31, ÕâµØ·½ÄÇÃ´ÅªÊÇ²» nice µÄ... */
    p=NULL;
    if((arg->mode==DIR_MODE_DELETED||arg->mode==DIR_MODE_JUNK)&&(p=strrchr(fileinfo->title,'-')))
        *p=0;

    /*
     * Leeward 98.04.15 
     */
    if (a_Import(NULL, currboard->filename, fileinfo, false, arg->direct, conf->pos)==0) {
        if (!(fileinfo->accessed[0] & FILE_IMPORTED)) {
            /* etnlegend, 2006.03.31, °æÖ÷ÃÇÔ½À´Ô½ÄÑËÅºòÁË... */
            if(!((arg->mode==DIR_MODE_DELETED||arg->mode==DIR_MODE_JUNK)
                ||(arg->mode==DIR_MODE_NORMAL&&(conf->pos>arg->filecount)))){
                ret=set_article_flag(conf, fileinfo, FILE_IMPORT_FLAG);
            }
        }
    }
    if((arg->mode==DIR_MODE_DELETED||arg->mode==DIR_MODE_JUNK)&&p)
        *p='-';
    return ret;
}

int show_b_note(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
    clear();
    if (show_board_notes(currboard->filename, 1) == -1) {
        move(3, 30);
        prints("´ËÌÖÂÛÇøÉĞÎŞ¡¸±¸ÍüÂ¼¡¹¡£");
    }
    pressanykey();
    return FULLUPDATE;
}

#if 0
int show_sec_board_notes(char bname[30])
{                               /* ÏÔÊ¾°æÖ÷µÄ»° */
    char buf[256];

    sprintf(buf, "vote/%s/secnotes", bname);    /* ÏÔÊ¾±¾°æµÄ°æÖ÷µÄ»° vote/°æÃû/notes */
    if (dashf(buf)) {
        ansimore2(buf, false, 0, 23 /*19 */ );
        return 1;
    } else if (dashf("vote/secnotes")) {        /* ÏÔÊ¾ÏµÍ³µÄ»° vote/notes */
        ansimore2("vote/secnotes", false, 0, 23 /*19 */ );
        return 1;
    }
    return -1;
}

int show_sec_b_note(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
    clear();
    if (show_sec_board_notes(currboard->filename) == -1) {
        move(3, 30);
        prints("´ËÌÖÂÛÇøÉĞÎŞ¡¸ÃØÃÜ±¸ÍüÂ¼¡¹¡£");
    }
    pressanykey();
    return FULLUPDATE;
}
#endif

int into_announce(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
    if (a_menusearch("0Announce", currboard->filename, (HAS_PERM(getCurrentUser(), PERM_ANNOUNCE) || HAS_PERM(getCurrentUser(), PERM_SYSOP) || HAS_PERM(getCurrentUser(), PERM_OBOARDS)) ? PERM_BOARDS : 0))
        return FULLUPDATE;
    return DONOTHING;
}

#ifdef FB2KPC
void a_menu();

int haspc(char *userid)
{
	char buf[256];
	sprintf(buf,"%s/%c/%s", FB2KPC, toupper(userid[0]),userid);

	if(dashd(buf))
		return 1;
	return 0;
}

int Personal(char *userid)
{
   char    found[256], lookid[IDLEN];
   char buf[STRLEN];
   struct userec *lookupuser;
   
   if(!userid || userid[0]=='\0') {
      clear();
      move(1, 0);
      usercomplete( "ÄúÏë¿´Ë­µÄ¸öÈËÎÄ¼¯: " , lookid);
      if (lookid[0] == '\0') {
         clear();
         return 1;
      }
   }else 
	  strcpy(lookid, userid);

   if(lookid[0] == '*'){
      sprintf(buf,"/%c/%s", toupper(getCurrentUser()->userid[0]),getCurrentUser()->userid);
   } else {
      if (! getuser(lookid,&lookupuser) ){
         lookid[1] = toupper(lookid[0]);
	     if(lookid[1] < 'A' || lookid[1] > 'Z'){
			buf[0]='\0';
		 }else {
			sprintf(buf,"/%c",lookid[1]);
	     }
      } else {
         sprintf(buf, "/%c/%s", toupper(lookupuser->userid[0]),lookupuser->userid);
      }
   }
   if(buf[0]=='/')
	  sprintf(found,FB2KPC "%s", buf);
   else
	  sprintf(found,FB2KPC "/%s", buf);
   if(!dashd(found)) 
      strcpy(found,FB2KPC);
   a_menu("",found,((HAS_PERM(getCurrentUser(),PERM_ANNOUNCE) || HAS_PERM(getCurrentUser(),PERM_SYSOP) || HAS_PERM(getCurrentUser(),PERM_OBOARDS)) ? PERM_BOARDS : 0),0, NULL);
   return 1;
}

int into_PAnnounce(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
	Personal(NULL);
	return FULLUPDATE;
}
#endif

int sequential_read(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
    struct read_arg* arg=conf->arg;
    int findpos;
    
    findpos=find_nextnew(conf,0);
    if (findpos) {
        conf->new_pos=findpos;
        arg->readmode=READ_NEW;
        list_select_add_key(conf, 'r');
    }
    return SHOW_SELCHANGE;
}

int clear_new_flag(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
#ifdef HAVE_BRC_CONTROL
    struct read_arg* arg=conf->arg;
    /* add by stiger */
    if (conf->pos>arg->filecount||!arg->filecount)
        brc_clear(currboardent,getSession());
    else brc_clear_new_flag(fileinfo->id,currboardent,getSession());
#endif
    return PARTUPDATE;
}

int clear_all_new_flag(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
#ifdef HAVE_BRC_CONTROL
    brc_clear(currboardent,getSession());
#endif
    return FULLUPDATE;
}

int range_flag(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
    char ans[4], buf[80];
    char num1[10], num2[10];
    int inum1, inum2, total=0;
    struct stat st;
    int i,k;
    int fflag;
    struct read_arg* arg=conf->arg;
#ifdef FILTER
    int is_filter = (HAS_PERM(getCurrentUser(), PERM_SYSOP)&&(!strcmp(currboard->filename,FILTER_BOARD)));
#endif
    
    if (!chk_currBM(currBM, getCurrentUser())) return DONOTHING;
    if (arg->mode!=DIR_MODE_SUPERFITER) return DONOTHING;
    if(stat(arg->direct, &st)==-1) return DONOTHING;
    total = st.st_size/sizeof(struct fileheader);
    
    clear();
    prints("Çø¶Î±ê¼Ç, Çë½÷É÷Ê¹ÓÃ");
    getdata(2, 0, "Ê×ÆªÎÄÕÂ±àºÅ: ", num1, 10, DOECHO, NULL, true);
    inum1 = atoi(num1);
    if (inum1 <= 0) return FULLUPDATE;
    getdata(3, 0, "Ä©ÆªÎÄÕÂ±àºÅ: ", num2, 10, DOECHO, NULL, true);
    inum2 = atoi(num2);
    if (inum2 <= inum1) {
        prints("´íÎó±àºÅ\n");
        pressreturn();
        return FULLUPDATE;
    }
    sprintf(buf, "1-±£Áôm  2-±ê¼ÇÉ¾³ıt  3-ÎÄÕªg  4-²»¿ÉRe  5-±ê¼Ç#  6-±ê¼Ç%%%s:[0]",
#ifdef FILTER
        is_filter?"  7-Éó²é±ê¼Ç@":"");
#else
        "");
#endif
    getdata(4, 0, buf, ans, 4, DOECHO, NULL, true);
#ifdef FILTER
    if(ans[0]=='7'&&(!is_filter)) return FULLUPDATE;
#else
    if(ans[0]=='7') return FULLUPDATE;
#endif
    if(ans[0]<'1'||ans[0]>'7') return FULLUPDATE;
    if(askyn("ÇëÉ÷ÖØ¿¼ÂÇ, È·ÈÏ²Ù×÷Âğ?", 0)==0) return FULLUPDATE;
    k=ans[0]-'0';
    if (k==3) {
        prints("\033[1;31m±¾¹¦ÄÜ±»°µÉ±, ÈçÓĞ²»·şÕßÕÒ atppp @ newsmth.net PK. \033[36m<ENTER>\033[0m");
        WAIT_RETURN;
        return FULLUPDATE;
    }
    if(k==1) fflag=FILE_MARK_FLAG;
    else if(k==2) fflag=FILE_DELETE_FLAG;
    else if(k==3) fflag=FILE_DIGEST_FLAG;
    else if(k==4) fflag=FILE_NOREPLY_FLAG;
    else if(k==5) fflag=FILE_SIGN_FLAG;
    else if(k==6) fflag=FILE_PERCENT_FLAG;
#ifdef FILTER
    else if(k==7) fflag=FILE_CENSOR_FLAG;
#endif
    else return FULLUPDATE;
    for(i=inum1;i<=inum2;i++) {
        if(i>=1&&i<=total) {
            struct write_dir_arg dirarg;
            struct fileheader data;
            data.accessed[0]=0xff;
            data.accessed[1]=0xff; /* TODO: ÊÇ·ñ¸ù¾İÊ×ÆªÎÄÕÂµÄ±ê¼ÇÀ´¾ö¶¨¼ÓÉÏ»¹ÊÇÈ¥µô±ê¼Ç? */
            
            init_write_dir_arg(&dirarg);
            dirarg.fd=arg->fd;
            dirarg.filename=arg->direct;
            dirarg.ent = i;
            dirarg.needlock=false;
            flock(arg->fd,LOCK_EX);
            malloc_write_dir_arg(&dirarg);
            change_post_flag(&dirarg, 
                arg->mode,
                currboard, 
                dirarg.fileptr+(i-1), 
                fflag, &data, true,getSession());
            flock(arg->fd,LOCK_UN);
            free_write_dir_arg(&dirarg);
        }
    }
    prints("\nÍê³É±ê¼Ç\n");
    pressreturn();
    return DIRCHANGED;
}

int show_t_friends()
{
    if (!HAS_PERM(getCurrentUser(), PERM_BASIC))
        return PARTUPDATE;
    t_friends();
    return FULLUPDATE;
}

/* add by stiger, add template */
int b_note_edit_new(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
	char ans[4];

	if(!chk_currBM(currBM, getCurrentUser())) return DONOTHING;

    move(t_lines - 1, 0);
    clrtoeol();
    getdata(t_lines - 1, 0, "±à¼­: 0)È¡Ïû 1)±¸ÍüÂ¼ 2)±¾°æÄ£°å"
#ifdef FLOWBANNER
		" 3)µ×²¿Á÷¶¯ĞÅÏ¢"
#endif
#ifdef NEWSMTH
		" 4)ÖÎ°æ·½Õë"
#endif
		" [0]: ", ans, 3, DOECHO, NULL, true);
    if (ans[0]=='1') return b_notes_edit();
#ifdef NEWSMTH
	else if (ans[0]=='4') return b_rules_edit();
#endif
	else if(ans[0]=='2'){
		int ret;	
#ifdef NEW_HELP
		int oldhelpmode=helpmode;
		helpmode = HELP_TMPL;
#endif
		ret =  m_template();
#ifdef NEW_HELP
		helpmode = oldhelpmode;
#endif
		return ret;
	}
#ifdef FLOWBANNER
	else if(ans[0]=='3'){
		return b_banner_edit();		
	}
#endif

	return FULLUPDATE;
}

/*Add by SmallPig*/
static int catnotepad(FILE * fp, char *fname)
{
    char inbuf[256];
    FILE *sfp;
    int count;

    count = 0;
    if ((sfp = fopen(fname, "r")) == NULL) {
        fprintf(fp, "\033[31m\033[41m¡Ñ©Ø¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©Ø¡Ñ\033[m\n\n");
        return -1;
    }
    while (fgets(inbuf, sizeof(inbuf), sfp) != NULL) {
        if (count != 0)
            fputs(inbuf, fp);
        else
            count++;
    }
    fclose(sfp);
    return 0;
}

void notepad()
{
    char tmpname[STRLEN], note1[4];
    char note[3][STRLEN - 4];
    char tmp[STRLEN];
    FILE *in;
    int i, n;
    time_t thetime = time(0);

    clear();
    move(0, 0);
    prints("¿ªÊ¼ÄãµÄÁôÑÔ°É£¡´ó¼ÒÕıÊÃÄ¿ÒÔ´ı....\n");
    /* sprintf(tmpname, "etc/notepad_tmp/%s.notepad", getCurrentUser()->userid); */
    gettmpfilename( tmpname, "notepad" );
    if ((in = fopen(tmpname, "w")) != NULL) {
        for (i = 0; i < 3; i++)
            memset(note[i], 0, STRLEN - 4);
        while (1) {
            for (i = 0; i < 3; i++) {
                getdata(1 + i, 0, ": ", note[i], STRLEN - 5, DOECHO, NULL, false);
                if (note[i][0] == '\0')
                    break;
            }
            if (i == 0) {
                fclose(in);
                unlink(tmpname);
                return;
            }
            getdata(5, 0, "ÊÇ·ñ°ÑÄãµÄ´ó×÷·ÅÈëÁôÑÔ°å (Y)ÊÇµÄ (N)²»Òª (E)ÔÙ±à¼­ [Y]: ", note1, 3, DOECHO, NULL, true);
            if (note1[0] == 'e' || note1[0] == 'E')
                continue;
            else
                break;
        }
        if (note1[0] != 'N' && note1[0] != 'n') {
            sprintf(tmp, "\033[32m%s\033[37m£¨%.24s£©", getCurrentUser()->userid, getCurrentUser()->username);
            fprintf(in, "\033[m\033[31m¡Ñ©Ğ¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©È\033[37mËáÌğ¿àÀ±°å\033[31m©À¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©Ğ¡Ñ\033[m\n");
            fprintf(in, "\033[31m¡õ©È%-43s\033[33mÔÚ \033[36m%.19s\033[33m Àë¿ªÊ±ÁôÏÂµÄ»°\033[31m©À¡õ\n", tmp, Ctime(thetime));
            if (i > 2)
                i = 2;
            for (n = 0; n <= i; n++) {
#ifdef FILTER
                if (check_badword_str(note[n],strlen(note[n]), getSession())) {
                    int t;
                    for (t = n; t <= i; t++) 
                        fprintf(in, "\033[31m©¦\033[m%-74.74s\033[31m©¦\033[m\n", note[t]);
                    fclose(in);

                    post_file(getCurrentUser(), "", tmpname, FILTER_BOARD, "---ÁôÑÔ°æ¹ıÂËÆ÷---", 0, 2,getSession());

                    unlink(tmpname);
                    return;
                }
#endif
                if (note[n][0] == '\0')
                    break;
                fprintf(in, "\033[31m©¦\033[m%-74.74s\033[31m©¦\033[m\n", note[n]);
            }
            fprintf(in, "\033[31m¡õ©Ğ¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©Ğ¡õ\033[m\n");
            catnotepad(in, "etc/notepad");
            fclose(in);
            f_mv(tmpname, "etc/notepad");
        } else {
            fclose(in);
            unlink(tmpname);
        }
    }
    if (talkrequest) {
        talkreply();
    }
    clear();
    return;
}

void record_exit_time()
{                               /* ¼ÇÂ¼ÀëÏßÊ±¼ä  Luzi 1998/10/23 */
	if(uinfo.invisible == true && HAS_PERM(getCurrentUser(), PERM_SYSOP) && getCurrentUser()->exittime > getCurrentUser()->lastlogin)
		return;
    getCurrentUser()->exittime = time(NULL);
    /*
     * char path[80];
     * FILE *fp;
     * time_t now;
     * sethomefile( path, getCurrentUser()->userid , "exit");
     * fp=fopen(path, "wb");
     * if (fp!=NULL)
     * {
     * now=time(NULL);
     * fwrite(&now,sizeof(time_t),1,fp);
     * fclose(fp);
     * }
     */
}

extern int icurrchar, ibufsize;

int Goodbye()
{                               /*ÀëÕ¾ Ñ¡µ¥ */
    extern int started;
    time_t stay;
    char fname[STRLEN], notename[STRLEN];
    char sysoplist[20][STRLEN], syswork[20][STRLEN], spbuf[STRLEN], buf[STRLEN], *tmp, *pbuf;
    int i, num_sysop, choose, logouts, mylogout = false;
    FILE *sysops;
    int left = (80 - 36) / 2;
    int top = (scr_lns - 11) / 2;
    struct _select_item level_conf[] = {
        {-1, -1, -1, SIT_SELECT, (void *) ""},
        {-1, -1, -1, SIT_SELECT, (void *) ""},
        {-1, -1, -1, SIT_SELECT, (void *) ""},
        {-1, -1, -1, SIT_SELECT, (void *) ""},
        {-1, -1, -1, 0, NULL}
    };
    level_conf[0].x = left + 7;
    level_conf[1].x = left + 7;
    level_conf[2].x = left + 7;
    level_conf[3].x = left + 7;
    level_conf[0].y = top + 2;
    level_conf[1].y = top + 3;
    level_conf[2].y = top + 4;
    level_conf[3].y = top + 5;

/*---	ÏÔÊ¾±¸ÍüÂ¼µÄ¹Øµô¸ÃËÀµÄ»î¶¯¿´°å	2001-07-01	---*/
    modify_user_mode(READING);

    i = 0;
    if ((sysops = fopen("etc/sysops", "r")) != NULL) {
        while (fgets(buf, STRLEN, sysops) != NULL && i < 20) {
            if(NULL == (tmp = strtok_r(buf," \n\r\t",&pbuf) ) ) {                
                continue;
            }else{
                if(tmp[0]=='#' || tmp[0]==';'){
                        continue;
                }
                strncpy(sysoplist[i], tmp, STRLEN);
                sysoplist[i][STRLEN-1] = '\0';
            }
            if(NULL == (tmp = strtok_r(NULL," \n\r\t",&pbuf) ) ) {
                continue;
            }else{
                strncpy(syswork[i], tmp, STRLEN);
                syswork[i][STRLEN-1] = '\0';
            }
            i++;
        }
        fclose(sysops);
    }
    num_sysop = i;
    move(1, 0);
    clear();
    move(top, left);
    outs("\x1b[1;47;37m¨X¨T[*]¨T¨T¨T Message ¨T¨T¨T¨T¨T¨T¨[\x1b[m");
    move(top + 1, left);
    outs("\x1b[1;47;37m¨U\x1b[44;37m                                \x1b[47;37m¨U\x1b[m");
    move(top + 2, left);
    prints("\x1b[1;47;37m¨U\x1b[44;37m     [\x1b[33m1\x1b[37m] ¼ÄĞÅ¸ø%-10s       \x1b[47;37m¨U\x1b[m", NAME_BBS_CHINESE);
    move(top + 3, left);
    prints("\x1b[1;47;37m¨U\x1b[44;37m     [\x1b[33m2\x1b[37m] \x1b[32m·µ»Ø%-15s\x1b[37m    \x1b[47;37m¨U\x1b[m", BBS_FULL_NAME);
    move(top + 4, left);
    outs("\x1b[1;47;37m¨U\x1b[44;37m     [\x1b[33m3\x1b[37m] Ğ´Ğ´*ÁôÑÔ°å*           \x1b[47;37m¨U\x1b[m");
    move(top + 5, left);
    outs("\x1b[1;47;37m¨U\x1b[44;37m     [\x1b[33m4\x1b[37m] Àë¿ª±¾BBSÕ¾            \x1b[47;37m¨U\x1b[m");
    move(top + 6, left);
    outs("\x1b[1;47;37m¨U\x1b[0;44;34m________________________________\x1b[1;47;37m¨U\x1b[m");
    move(top + 7, left);
    outs("\x1b[1;47;37m¨U                                ¨U\x1b[m");
    move(top + 8, left);
    outs("\x1b[1;47;37m¨U          \x1b[42;33m  È¡Ïû(ESC) \x1b[0;47;30m¨z  \x1b[1;37m      ¨U\x1b[m");
    move(top + 9, left);
    outs("\x1b[1;47;37m¨U            \x1b[0;40;37m¨z¨z¨z¨z¨z¨z\x1b[1;47;37m        ¨U\x1b[m");
    move(top + 10, left);
    outs("\x1b[1;47;37m¨^¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨T¨a\x1b[m");
    outs("\x1b[1;44;37m");

    choose = simple_select_loop(level_conf, SIF_SINGLE | SIF_ESCQUIT | SIF_NUMBERKEY, t_columns, t_lines, NULL);
    if (choose == 0)
        choose = 2;
    clear();
    if (strcmp(getCurrentUser()->userid, "guest") && choose == 1) {  /* Ğ´ĞÅ¸øÕ¾³¤ */
        if (PERM_LOGINOK & getCurrentUser()->userlevel) {    /*Haohmaru.98.10.05.Ã»Í¨¹ı×¢²áµÄÖ»ÄÜ¸ø×¢²áÕ¾³¤·¢ĞÅ */
            prints("        ID        ¸ºÔğµÄÖ°Îñ\n");
            prints("   ============ =============\n");
            for (i = 1; i <= num_sysop; i++) {
                prints("[\033[33m%1d\033[m] \033[1m%-12s %s\033[m\n", i, sysoplist[i - 1], syswork[i - 1]);
            }

            prints("[\033[33m%1d\033[m] »¹ÊÇ×ßÁËÂŞ£¡\n", num_sysop + 1);      /*×îºóÒ»¸öÑ¡Ïî */

            sprintf(spbuf, "ÄãµÄÑ¡ÔñÊÇ [\033[32m%1d\033[m]£º", num_sysop + 1);
            getdata(num_sysop + 5, 0, spbuf, genbuf, 4, DOECHO, NULL, true);
            choose = genbuf[0] - '0';
            if (0 != genbuf[1])
                choose = genbuf[1] - '0' + 10;
            if (choose >= 1 && choose <= num_sysop) {
                /*
                 * do_send(sysoplist[choose-1], "Ê¹ÓÃÕß¼ÄÀ´µÄµÄ½¨ÒéĞÅ"); 
                 */
                if (choose == 1)        /*modified by Bigman : 2000.8.8 */
                    do_send(sysoplist[0], "¡¾Õ¾Îñ×Ü¹Ü¡¿Ê¹ÓÃÕß¼ÄÀ´µÄ½¨ÒéĞÅ", "");
                else if (choose == 2)
                    do_send(sysoplist[1], "¡¾ÏµÍ³Î¬»¤¡¿Ê¹ÓÃÕß¼ÄÀ´µÄ½¨ÒéĞÅ", "");
                else if (choose == 3)
                    do_send(sysoplist[2], "¡¾°æÃæ¹ÜÀí¡¿Ê¹ÓÃÕß¼ÄÀ´µÄ½¨ÒéĞÅ", "");
                else if (choose == 4)
                    do_send(sysoplist[3], "¡¾Éí·İÈ·ÈÏ¡¿Ê¹ÓÃÕß¼ÄÀ´µÄ½¨ÒéĞÅ", "");
                else if (choose == 5)
                    do_send(sysoplist[4], "¡¾ÖÙ²ÃÊÂÒË¡¿Ê¹ÓÃÕß¼ÄÀ´µÄ½¨ÒéĞÅ", "");
            }
/* added by stephen 11/13/01 */
            choose = -1;
        } else {
            /*
             * Ôö¼Ó×¢²áµÄÌáÊ¾ĞÅÏ¢ Bigman:2000.10.31 
             */
            prints("\n    Èç¹ûÄúÒ»Ö±Î´µÃµ½Éí·İÈÏÖ¤,ÇëÈ·ÈÏÄúÊÇ·ñµ½¸öÈË¹¤¾ßÏäÌîĞ´ÁË×¢²áµ¥,\n");
            prints("    Èç¹ûÄúÊÕµ½Éí·İÈ·ÈÏĞÅ,»¹Ã»ÓĞ·¢ÎÄÁÄÌìµÈÈ¨ÏŞ,ÇëÊÔ×ÅÔÙÌîĞ´Ò»±é×¢²áµ¥\n\n");
            prints("     Õ¾³¤µÄ ID   ¸ºÔğµÄÖ°Îñ\n");
            prints("   ============ =============\n");

            /*
             * added by Bigman: 2000.8.8  ĞŞ¸ÄÀëÕ¾ 
             */
            prints("[\033[33m%1d\033[m] \033[1m%-12s %s\033[m\n", 1, sysoplist[3], syswork[3]);
            prints("[\033[33m%1d\033[m] »¹ÊÇ×ßÁËÂŞ£¡\n", 2);  /*×îºóÒ»¸öÑ¡Ïî */

            sprintf(spbuf, "ÄãµÄÑ¡ÔñÊÇ %1d£º", 2);
            getdata(num_sysop + 6, 0, spbuf, genbuf, 4, DOECHO, NULL, true);
            choose = genbuf[0] - '0';
            if (choose == 1)    /*modified by Bigman : 2000.8.8 */
                do_send(sysoplist[3], "¡¾Éí·İÈ·ÈÏ¡¿Ê¹ÓÃÕß¼ÄÀ´µÄ½¨ÒéĞÅ", "");
            choose = -1;
        }
    }
    if (choose == 2)            /*·µ»ØBBS */
        return FULLUPDATE;
    if (strcmp(getCurrentUser()->userid, "guest") != 0) {
        if (choose == 3)        /*ÁôÑÔ²¾ */
            if (USE_NOTEPAD == 1 && HAS_PERM(getCurrentUser(), PERM_POST))
                notepad();
    }

    clear();
    prints("\n\n\n\n");

    if (DEFINE(getCurrentUser(), DEF_OUTNOTE /*ÍË³öÊ±ÏÔÊ¾ÓÃ»§±¸ÍüÂ¼ */ )) {
        sethomefile(notename, getCurrentUser()->userid, "notes");
        if (dashf(notename))
            ansimore(notename, true);
    }

    /*
     * Leeward 98.09.24 Use SHARE MEM and disable the old code 
     */
    if (DEFINE(getCurrentUser(), DEF_LOGOUT)) {      /* Ê¹ÓÃ×Ô¼ºµÄÀëÕ¾»­Ãæ */
        sethomefile(fname, getCurrentUser()->userid, "logout");
        if (dashf(fname))
            mylogout = true;
    }
    if (mylogout) {
        logouts = countlogouts(fname);  /* logouts Îª ÀëÕ¾»­Ãæ ×ÜÊı */
        if (logouts >= 1) {
            user_display(fname, (logouts == 1) ? 1 : (getCurrentUser()->numlogins % (logouts)) + 1, true);
        }
    } else {
        logouts = countlogouts("etc/logout");   /* logouts Îª ÀëÕ¾»­Ãæ ×ÜÊı */
        if (logouts > 0)
            user_display("etc/logout", rand() % logouts + 1, true);
    }

    //bbslog("user", "%s", "exit");

    if (started) {
        record_exit_time();     /* ¼ÇÂ¼ÓÃ»§µÄÍË³öÊ±¼ä Luzi 1998.10.23 */
        stay=time(NULL)-uinfo.logintime;
        /*---	period	2000-10-19	4 debug	---*/
        newbbslog(BBSLOG_USIES, "EXIT: Stay:%3ld (%s)[%d %d]", stay / 60, getCurrentUser()->username, getSession()->utmpent, getSession()->currentuid);
        u_exit();
        started = 0;
    }

    if (num_user_logins(getCurrentUser()->userid) == 0 || !strcmp(getCurrentUser()->userid, "guest")) {   /*¼ì²é»¹ÓĞÃ»ÓĞÈËÔÚÏßÉÏ */
        FILE *fp;
        char buf[STRLEN], *ptr;

        if (DEFINE(getCurrentUser(), DEF_MAILMSG /*ÀëÕ¾Ê±¼Ä»ØËùÓĞĞÅÏ¢ */ ) && (get_msgcount(0, getCurrentUser()->userid))) {
                mail_msg(getCurrentUser(),getSession());
        }
#if !defined(FREE) && !defined(ZIXIA)
		else
            clear_msg(getCurrentUser()->userid);
#endif
        fp = fopen("friendbook", "r");  /*ËÑË÷ÏµÍ³ Ñ°ÈËÃûµ¥ */
        while (fp != NULL && fgets(buf, sizeof(buf), fp) != NULL) {
            char uid[14];

            ptr = strstr(buf, "@");
            if (ptr == NULL) {
                del_from_file("friendbook", buf);
                continue;
            }
            ptr++;
            strcpy(uid, ptr);
            ptr = strstr(uid, "\n");
            *ptr = '\0';
            if (!strcmp(uid, getCurrentUser()->userid))      /*É¾³ı±¾ÓÃ»§µÄ Ñ°ÈËÃûµ¥ */
                del_from_file("friendbook", buf);       /*Ñ°ÈËÃûµ¥Ö»ÔÚ±¾´ÎÉÏÏßÓĞĞ§ */
        }
        if (fp)                                                                                        /*---	add by period 2000-11-11 fix null hd bug	---*/
            fclose(fp);
    }
    sleep(1);
    pressreturn();              /*Haohmaru.98.10.18 */
    output("\x1b[m",3);
    output("\x1b[H\x1b[J",6);
    oflush();

    end_mmapfile(getSession()->currentmemo, sizeof(struct usermemo), -1);

    shutdown(0, 2);
    close(0);
    exit(0);
    return -1;
}



int Info()
{                               /* ÏÔÊ¾°æ±¾ĞÅÏ¢Version.Info */
    modify_user_mode(XMENU);
    ansimore("Version.Info", true);
    clear();
    return 0;
}

int Conditions()
{                               /* ÏÔÊ¾°æÈ¨ĞÅÏ¢COPYING */
    modify_user_mode(XMENU);
    ansimore("COPYING", true);
    clear();
    return 0;
}

int ShowWeather()
{                               /* ÏÔÊ¾°æ±¾ĞÅÏ¢Version.Info */
    modify_user_mode(XMENU);
    ansimore("WEATHER", true);
    clear();
    return 0;
}

int Welcome()
{                               /* ÏÔÊ¾»¶Ó­»­Ãæ Welcome */
    modify_user_mode(XMENU);
    ansimore("Welcome", true);
    clear();
    return 0;
}

void RemoveAppendedSpace(char *ptr)
{                               /* Leeward 98.02.13 */
    int Offset;

    /*
     * Below block removing extra appended ' ' in article titles 
     */
    Offset = strlen(ptr);
    for (--Offset; Offset > 0; Offset--) {
        if (' ' != ptr[Offset])
            break;
        else
            ptr[Offset] = 0;
    }
}

int i_read_mail()
{
    if(!HAS_PERM(getCurrentUser(), PERM_BASIC)||!strcmp(getCurrentUser()->userid, "guest")) return DONOTHING;
    if (HAS_MAILBOX_PROP(&uinfo, MBP_MAILBOXSHORTCUT))
    	MailProc();
    else
    	m_read();
    setmailcheck(getCurrentUser()->userid);
    return FULLUPDATE;
}

#define ACL_MAX 10

#ifndef HAVE_IPV6_SMTH
#define IPBITS 32
#else
#define IPBITS 128
#endif

struct acl_struct {
#ifndef HAVE_IPV6_SMTH
    unsigned int ip;
    char len;
#else
    struct in6_addr ip;
    int len;
#endif
    char deny;
} * acl;
int aclt=0;

static int set_acl_list_show(struct _select_def *conf, int i)
{
    char buf[80];
#ifndef HAVE_IPV6_SMTH
    unsigned int ip,ip2;
    ip = acl[i-1].ip;
#else
    struct in6_addr ip;
    memcpy(&ip, &acl[i-1].ip, sizeof(struct in6_addr));
#endif
    if(i-1<aclt) {
#ifndef HAVE_IPV6_SMTH
        if(acl[i-1].len==0) ip2=ip+0xffffffff;
        else ip2=ip+((1<<(32-acl[i-1].len))-1);
        sprintf(buf, "%d.%d.%d.%d--%d.%d.%d.%d", ip>>24, (ip>>16)%0x100, (ip>>8)%0x100, ip%0x100, ip2>>24, (ip2>>16)%0x100, (ip2>>8)%0x100, ip2%0x100);
#else
        if (ISV4ADDR(acl[i-1].ip)) { 
            inet_ntop(AF_INET, &ip.s6_addr[12], buf, 80);
            sprintf(&buf[strlen(buf)], "/%d", acl[i-1].len+32-128);
        } else {
            inet_ntop(AF_INET6, &ip, buf, 80);
            sprintf(&buf[strlen(buf)], "/%d", acl[i-1].len);
        }
#endif
        prints("  %2d  %-50s %4s", i, buf, acl[i-1].deny?"¾Ü¾ø":"ÔÊĞí");
    }
    return SHOW_CONTINUE;
}

static int set_acl_list_prekey(struct _select_def *conf, int *key)
{
    switch (*key) {
    case 'e':
    case 'q':
        *key = KEY_LEFT;
        break;
    case 'p':
    case 'k':
        *key = KEY_UP;
        break;
    case ' ':
    case 'N':
        *key = KEY_PGDN;
        break;
    case 'n':
    case 'j':
        *key = KEY_DOWN;
        break;
    }
    return SHOW_CONTINUE;
}

static int set_acl_list_key(struct _select_def *conf, int key)
{
    int oldmode;

    switch (key) {
    case 'a':
        if (aclt<ACL_MAX) {
            char buf[IPLEN+4];
#ifndef HAVE_IPV6_SMTH
            int ip[4], i, k=0, err=0;
#else
            struct in6_addr ip, mask;
            int k=0, err=0;
#endif
            getdata(0, 0, "ÇëÊäÈëIPµØÖ·: ", buf, IPLEN+2, 1, 0, 1);
#ifndef HAVE_IPV6_SMTH
            for(i=0;i<strlen(buf);i++) if(buf[i]=='.') k++;
            if(k!=3) err=1;
            else {
                if(sscanf(buf, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3])!=4) err=1;
                else {
                    if(ip[0]==0) err=1;
                    for(i=0;i<4;i++) if(ip[i]<0||ip[i]>=256) err=1;
                }
            }
#else
            if ((!strchr(buf, ':')) && (strchr(buf, '.'))) {
                memset(&ip, 0, sizeof(ip));
                ip.s6_addr[10]=0xff;
                ip.s6_addr[11]=0xff;
                err = inet_pton(AF_INET, buf, &ip.s6_addr[12])<=0;
                k = 128-32;
			} else err = inet_pton(AF_INET6, buf, &ip)<=0;
#endif
            if(err) {
                move(0, 0);
                prints("IPÊäÈë´íÎó!");
                clrtoeol();
                refresh(); sleep(1);
            }
            else {
                getdata(0, 0, "ÇëÊäÈë³¤¶È(µ¥Î»:bit): ", buf, 4, 1, 0, 1);
                acl[aclt].len = atoi(buf);
#ifdef HAVE_IPV6_SMTH
                acl[aclt].len+= k;
#endif
                if(acl[aclt].len<0 || acl[aclt].len>IPBITS) err=1;
                if(err) {
                    move(0, 0);
                    prints("³¤¶ÈÊäÈë´íÎó!");
                    clrtoeol();
                    refresh(); sleep(1);
                }
                else {
                    getdata(0, 0, "ÔÊĞí/¾Ü¾ø(0-ÔÊĞí,1-¾Ü¾ø): ", buf, 4, 1, 0, 1);
                    if(buf[0]=='0') acl[aclt].deny=0;
                    else acl[aclt].deny=1;
#ifndef HAVE_IPV6_SMTH
                    acl[aclt].ip = (ip[0]<<24)+(ip[1]<<16)+(ip[2]<<8)+ip[3];
                    if(acl[aclt].len<32)
                        acl[aclt].ip = acl[aclt].ip&(((1<<acl[aclt].len)-1)<<(32-acl[aclt].len));
#else
                    ip_mask(&ip, ip_len2mask(acl[aclt].len, &mask), &acl[aclt].ip);
#endif
                    aclt++;
                    return SHOW_DIRCHANGE;
                }
            }
            return SHOW_REFRESH;
        }
        break;
    case 'd':
        if (aclt > 0) {
            char ans[3];

            getdata(0, 0, "È·ÊµÒªÉ¾³ıÂğ(Y/N)? [N]: ", ans, sizeof(ans), DOECHO, NULL, true);
            if (ans[0] == 'Y' || ans[0] == 'y') {
                int i;
                aclt--;
                for(i=conf->pos-1;i<aclt;i++)
                    memcpy(acl+i, acl+i+1, sizeof(struct acl_struct));
                bzero(acl+aclt, sizeof(struct acl_struct));
            }
            return SHOW_DIRCHANGE;
        }
        break;
    case 'm':
        if (aclt > 0) {
            char ans[3];
            int d;

            getdata(0, 0, "ÇëÊäÈëÒªÒÆ¶¯µ½µÄÎ»ÖÃ: ", ans, 3, DOECHO, NULL, true);
            d=atoi(ans)-1;
            if (d>=0&&d<=aclt-1&&d!=conf->pos-1) {
                struct acl_struct temp;
                int i, p;
                p = conf->pos-1;
                memcpy(&temp, acl+p, sizeof(struct acl_struct));
                if(p>d) {
                    for(i=p;i>d;i--)
                        memcpy(acl+i, acl+i-1, sizeof(struct acl_struct));
                } else {
                    for(i=p;i<d;i++)
                        memcpy(acl+i, acl+i+1, sizeof(struct acl_struct));
                }
                memcpy(acl+d, &temp, sizeof(struct acl_struct));
                return SHOW_DIRCHANGE;
            }
        }
        break;
    case 'L':
    case 'l':
        oldmode = uinfo.mode;
        show_allmsgs();
        modify_user_mode(oldmode);
        return SHOW_REFRESH;
    case 'W':
    case 'w':
        oldmode = uinfo.mode;
        if (!HAS_PERM(getCurrentUser(), PERM_PAGE))
            break;
        s_msg();
        modify_user_mode(oldmode);
        return SHOW_REFRESH;
    case 'u':
        oldmode = uinfo.mode;
        clear();
        modify_user_mode(QUERY);
        t_query(NULL);
        modify_user_mode(oldmode);
        clear();
        return SHOW_REFRESH;
	case 'U':		/* pig2532 2005.12.10 */
		board_query();
        return SHOW_REFRESH;
    }

    return SHOW_CONTINUE;
}

static int set_acl_list_refresh(struct _select_def *conf)
{
    clear();
    docmdtitle("[µÇÂ¼IP¿ØÖÆÁĞ±í]",
               "ÍË³ö[\x1b[1;32m¡û\x1b[0;37m,\x1b[1;32me\x1b[0;37m] Ñ¡Ôñ[\x1b[1;32m¡ü\x1b[0;37m,\x1b[1;32m¡ı\x1b[0;37m] Ìí¼Ó[\x1b[1;32ma\x1b[0;37m] É¾³ı[\x1b[1;32md\x1b[0;37m]\x1b[m");
    move(2, 0);
    prints("\033[0;1;37;44m  %4s  %-50s %-31s", "¼¶±ğ", "IPµØÖ··¶Î§", "ÔÊĞí/¾Ü¾ø");
    clrtoeol();
    update_endline();
    return SHOW_CONTINUE;
}

static int set_acl_list_getdata(struct _select_def *conf, int pos, int len)
{
    conf->item_count = aclt;
    if(conf->item_count==0)
        conf->item_count=1;

    return SHOW_CONTINUE;
}

int set_ip_acl()
{
    struct _select_def grouplist_conf;
    POINT *pts;
#ifndef HAVE_IPV6_SMTH
    int i,rip[4];
#else
    int i;
    struct in6_addr rip;
#endif
    int oldmode;
    FILE* fp;
    char fn[80],buf[80];

    clear();
    getdata(3, 0, "ÇëÊäÈëÄãµÄÃÜÂë: ", buf, 39, NOECHO, NULL, true);
    if (*buf == '\0' || !checkpasswd2(buf, getCurrentUser())) {
        prints("\n\nºÜ±§Ç¸, ÄúÊäÈëµÄÃÜÂë²»ÕıÈ·¡£\n");
        pressanykey();
        return 0;
    }

    acl = (struct acl_struct *) malloc(sizeof(struct acl_struct)*ACL_MAX);
    aclt=0;
    bzero(acl, sizeof(struct acl_struct)*ACL_MAX);
    sethomefile(fn, getCurrentUser()->userid, "ipacl");
    fp=fopen(fn, "r");
    if(fp){
        i=0;
        while(!feof(fp)) {
            int len,deny;
#ifndef HAVE_IPV6_SMTH
            if(fscanf(fp, "%d.%d.%d.%d %d %d", &rip[0], &rip[1], &rip[2], &rip[3], &len, &deny)<=0) break;
            acl[i].ip = (rip[0]<<24)+(rip[1]<<16)+(rip[2]<<8)+rip[3];
#else
            if(fscanf(fp, "%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX %d %d",
                &rip.s6_addr[0], &rip.s6_addr[1], &rip.s6_addr[2], &rip.s6_addr[3], &rip.s6_addr[4], &rip.s6_addr[5],
                &rip.s6_addr[6], &rip.s6_addr[7], &rip.s6_addr[8], &rip.s6_addr[9], &rip.s6_addr[10], &rip.s6_addr[11], 
                &rip.s6_addr[12], &rip.s6_addr[13], &rip.s6_addr[14], &rip.s6_addr[15], &len, &deny)<=0) break;
            memcpy(&acl[i].ip, &rip, IPBITS/8);
#endif
            acl[i].len=len;
            acl[i].deny=(char)deny;
            i++;
            if(i>=ACL_MAX) break;
        }
        aclt = i;
        fclose(fp);
    }
    clear();
    oldmode = uinfo.mode;
    modify_user_mode(SETACL);
    //TODO: ´°¿Ú´óĞ¡¶¯Ì¬¸Ä±äµÄÇé¿ö£¿ÕâÀïÓĞbug
    pts = (POINT *) malloc(sizeof(POINT) * BBS_PAGESIZE);
    for (i = 0; i < BBS_PAGESIZE; i++) {
        pts[i].x = 2;
        pts[i].y = i + 3;
    }
    bzero(&grouplist_conf, sizeof(struct _select_def));
    grouplist_conf.item_count = aclt;
    if(grouplist_conf.item_count==0)
        grouplist_conf.item_count=1;
    grouplist_conf.item_per_page = BBS_PAGESIZE;
    /*
     * ¼ÓÉÏ LF_VSCROLL ²ÅÄÜÓÃ LEFT ¼üÍË³ö 
     */
    grouplist_conf.flag = LF_VSCROLL | LF_BELL | LF_LOOP | LF_MULTIPAGE;
    grouplist_conf.prompt = "¡ô";
    grouplist_conf.item_pos = pts;
    grouplist_conf.title_pos.x = 0;
    grouplist_conf.title_pos.y = 0;
    grouplist_conf.pos = 1;     /* initialize cursor on the first mailgroup */
    grouplist_conf.page_pos = 1;        /* initialize page to the first one */

    grouplist_conf.show_data = set_acl_list_show;
    grouplist_conf.pre_key_command = set_acl_list_prekey;
    grouplist_conf.key_command = set_acl_list_key;
    grouplist_conf.show_title = set_acl_list_refresh;
    grouplist_conf.get_data = set_acl_list_getdata;

    list_select_loop(&grouplist_conf);
    free(pts);
    modify_user_mode(oldmode);
    fp=fopen(fn, "w");
    if(fp){
        for(i=0;i<aclt;i++)
#ifndef HAVE_IPV6_SMTH
            fprintf(fp, "%d.%d.%d.%d %d %d\n", acl[i].ip>>24, (acl[i].ip>>16)%0x100, (acl[i].ip>>8)%0x100, acl[i].ip%0x100, acl[i].len, acl[i].deny);
#else
            fprintf(fp, "%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX %d %d\n",
                acl[i].ip.s6_addr[0], acl[i].ip.s6_addr[1], acl[i].ip.s6_addr[2], acl[i].ip.s6_addr[3], 
                acl[i].ip.s6_addr[4], acl[i].ip.s6_addr[5], acl[i].ip.s6_addr[6], acl[i].ip.s6_addr[7],
                acl[i].ip.s6_addr[8], acl[i].ip.s6_addr[9], acl[i].ip.s6_addr[10], acl[i].ip.s6_addr[11], 
                acl[i].ip.s6_addr[12], acl[i].ip.s6_addr[13], acl[i].ip.s6_addr[14], acl[i].ip.s6_addr[15], 
                acl[i].len, acl[i].deny);
#endif
        fclose(fp);
    }

    return 0;

}


int b_results(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
    return vote_results(currboard->filename);
}

enum {
    BM_DELETE=1,
    BM_MARK,
    BM_DELMARKDEL,
    BM_IMPORT,
    BM_TMP,
    BM_MARKDEL,
    BM_NOREPLY,
    BM_TOTAL
};

const char *SR_BMitems[] = {
    "É¾³ı",
    "±£Áô",
    "É¾³ıÄâÉ¾ÎÄÕÂ",
    "·ÅÈë¾«»ªÇø",
    "·ÅÈëÔİ´æµµ",
    "Éè¶¨ÄâÉ¾±ê¼Ç",
    "Éè¶¨²»¿É»Ø¸´",
    "ÖÆ×÷ºÏ¼¯"
};
const int item_num = 8;

struct BMFunc_arg {
    bool delpostnum; /*ÊÇ·ñ¼õÎÄÕÂÊı*/
    int action;            /*°æÖ÷²Ù×÷£¬ÎªBM_DELETEµ½BM_TOTALÆäÖĞÖ®Ò»*/
    bool saveorigin;    /*ÔÚºÏ¼¯²Ù×÷µÄÊ±ºò±íÃ÷ÊÇ·ñ±£´æÔ­ÎÄ*/
    char* announce_path; /*ÊÕÂ¼¾«»ªÇøµÄÊ±ºòµÄÎ»ÖÃ*/
    bool setflag; /*ÉèÖÃ»¹ÊÇÈ¡Ïû*/
};

/*°æÖ÷Í¬Ö÷Ìâº¯Êı£¬ÓÃÓÚapply_recordµÄ»Øµ÷º¯Êı*/
static int BM_thread_func(struct _select_def* conf, struct fileheader* fh,int ent, void* extraarg)
{
    struct read_arg* arg=(struct read_arg*)conf->arg;
    struct BMFunc_arg* func_arg=(struct BMFunc_arg*)extraarg;
    int ret=APPLY_CONTINUE;

    conf->pos=ent;
    if (arg->writearg) {
        arg->writearg->ent=ent;
    }
    switch (func_arg->action) {
        case BM_DELETE:
            if (!(fh->accessed[0] & (FILE_MARKED | FILE_PERCENT))) {
                if (del_post(conf,fh,(void*)(ARG_BMFUNC_FLAG|ARG_NOPROMPT_FLAG))==DIRCHANGED)
                    ret=APPLY_REAPPLY;
            }
            break;
        case BM_MARK:
	    if (func_arg->setflag)
            fh->accessed[0] |= FILE_MARKED;
	    else
            fh->accessed[0] &= ~FILE_MARKED;
            break;
        case BM_DELMARKDEL: /* etnlegend, 2005.11.28, Í¬Ö÷Ìâ±ê¼ÇÉ¾³ı */
            if(fh->accessed[1]&FILE_DEL){
                if(!(fh->accessed[0]&(FILE_MARKED|FILE_PERCENT))){
                    if(del_post(conf,fh,(void*)(ARG_BMFUNC_FLAG|ARG_NOPROMPT_FLAG))==DIRCHANGED)
                        ret=APPLY_REAPPLY;
                }
                else
                    fh->accessed[1]&=~FILE_DEL;
            }
            break;
        case BM_MARKDEL:
	    if (func_arg->setflag) {
            if (!(fh->accessed[0] & (FILE_MARKED | FILE_PERCENT))) {
                fh->accessed[1] |= FILE_DEL;
            }
	    } else
                fh->accessed[1] &= ~FILE_DEL;
            break;
        case BM_NOREPLY:
	    if (func_arg->setflag)
                fh->accessed[1] |= FILE_READ;
	    else
                fh->accessed[1] &= ~FILE_READ;
            break;
        case BM_IMPORT:
            if (a_Import(func_arg->announce_path, 
                currboard->filename, 
                fh, 
                true, 
                arg->direct, 
                ent)==0) {
                fh->accessed[0]|=FILE_IMPORTED;
            }            
            break;
        case BM_TOTAL:
        case BM_TMP:
            a_SeSave("0Announce",
                currboard->filename,
                fh,
                true,
                arg->direct,
                ent,
                !func_arg->saveorigin,
                getCurrentUser()->userid);
            fh->accessed[0]|=FILE_IMPORTED;
            break;
    }
    return ret;
}

static int SR_BMFunc(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
    int i;
    char buf[256], ch[4], BMch;
    struct BMFunc_arg func_arg;
    bool fromfirst;
    int ent;
    struct read_arg* arg=(struct read_arg*)conf->arg;
    char linebuffer[LINELEN*3];
    char annpath[MAXPATH];
    struct write_dir_arg dirarg;

    if (fh==NULL)
        return DONOTHING;
    func_arg.delpostnum=(bool)extraarg;
    func_arg.setflag=true;
    if (!chk_currBM(currBM, getCurrentUser())) {
        return DONOTHING;
    }
    if (arg->mode != DIR_MODE_NORMAL && arg->mode != DIR_MODE_DIGEST)     /* KCN:Ôİ²»ÔÊĞí */
        return DONOTHING;
    if (conf->pos>arg->filecount) /*ÖÃ¶¥*/
        return DONOTHING;
    saveline(t_lines - 3, 0, linebuffer);
    saveline(t_lines - 2, 0, NULL);
    move(t_lines - 3, 0);
    clrtoeol();
    strcpy(buf, "Ö÷Ìâ²Ù×÷ (0)È¡Ïû ");
    for (i = 0; i < item_num; i++) {
        char t[40];

        sprintf(t, "(%d)%s ", i + 1, SR_BMitems[i]);
        strcat(buf, t);
    };
    strcat(buf, "? [0]: ");
    if (strlen(buf) > 76) {
        char savech = buf[76];

        buf[76] = 0;
        prints("%s", buf);
        buf[76] = savech;

/*        strcpy(buf,buf+76);*/
        getdata(t_lines - 2, 0, buf + 76, ch, 3, DOECHO, NULL, true);
    } else
        getdata(t_lines - 3, 0, buf, ch, 3, DOECHO, NULL, true);
    BMch = atoi(ch);
    if (BMch <= 0 || BMch > item_num) {
        saveline(t_lines - 2, 1, NULL);
        saveline(t_lines - 3, 1, linebuffer);
        return DONOTHING;
    }
    if (arg->mode == DIR_MODE_DIGEST && BMch == 3) {
        saveline(t_lines - 2, 1, NULL);
        saveline(t_lines - 3, 1, linebuffer);
        return DONOTHING;
    }
    move(t_lines - 2, 0);
    clrtoeol();
    move(t_lines - 3, 0);
    clrtoeol();

    /*
     * Leeward 98.04.16
     */
    switch (BMch) {
        case BM_MARK:
            if ((fh->accessed[0] & FILE_MARKED)!=0)
		    func_arg.setflag=false;
            break;
        case BM_MARKDEL:
            if ((fh->accessed[1] & FILE_DEL)!=0)
		    func_arg.setflag=false;
            break;
        case BM_NOREPLY:
            if ((fh->accessed[1] & FILE_READ)!=0)
		    func_arg.setflag=false;
            break;
        default:
            break;
    }
    snprintf(buf, 256, "´ÓÖ÷ÌâµÚÒ»Æª¿ªÊ¼%s%s (Y)µÚÒ»Æª (N)Ä¿Ç°ÕâÆª (C)È¡Ïû (Y/N/C)? [Y]: ",
              func_arg.setflag?"":"È¡Ïû",SR_BMitems[BMch - 1]);
    getdata(t_lines - 3, 0, buf, ch, 3, DOECHO, NULL, true);
    switch (ch[0]) {
    case 'c':
    case 'C':
        saveline(t_lines - 2, 1, NULL);
        saveline(t_lines - 3, 1, linebuffer);
        return DONOTHING;
    case 'N':
    case 'n':
        fromfirst=false;
        break;
    default:
        fromfirst=true;
        break;
    }
    bmlog(getCurrentUser()->userid, currboard->filename, 14, 1);

    if(BM_TOTAL == BMch ){ //×÷ºÏ¼¯
        sprintf(annpath,"tmp/bm.%s",getCurrentUser()->userid);
        if(dashf(annpath))unlink(annpath);
        snprintf(buf, 256, "ÊÇ·ñ±£ÁôÒıÎÄ(Y/N/C)? [Y]: ");
        getdata(t_lines - 2, 0, buf, ch, 3, DOECHO, NULL, true);
        switch (ch[0]){
        case 'n':
        case 'N':
            func_arg.saveorigin=false;
            break;
        case 'c':
        case 'C':
            saveline(t_lines - 2, 1, NULL);
            saveline(t_lines - 3, 1, linebuffer);
            return DONOTHING;
        default:
            func_arg.saveorigin=true;
        }
    } else if (BMch==BM_IMPORT) {
        if (set_import_path(annpath)!=0) {
            saveline(t_lines - 2, 1, NULL);
            saveline(t_lines - 3, 1, linebuffer);
            return DONOTHING;
        }
        func_arg.announce_path=annpath;
    }

    func_arg.action=BMch;
    ent=conf->pos;
    init_write_dir_arg(&dirarg);
    dirarg.fd=arg->fd;
    dirarg.needlock=false;
    arg->writearg=&dirarg;

    flock(arg->fd,LOCK_EX);
    if (fromfirst) {
        /*×ßµ½µÚÒ»Æª*/
        conf->new_pos=0; /* atppp 20051019 */
        apply_thread(conf,fh,fileheader_thread_read,false,false,(void*)SR_FIRST);
        if (conf->new_pos!=0)
            conf->pos=conf->new_pos;
    }
    apply_thread(conf,fh,BM_thread_func,true,true,(void*)&func_arg);
    flock(arg->fd,LOCK_UN);
    free_write_dir_arg(&dirarg);
    arg->writearg=NULL;
    conf->pos=ent; /*»Ö¸´Ô­À´µÄent*/
	if(BMch == BM_DELETE){
		/* set .ORIGIN */
	}
    if(BM_TOTAL == BMch){ //×÷ºÏ¼¯
        char title[STRLEN];
        //create new title
        strcpy(buf,"[ºÏ¼¯] ");
        if(!strncmp(fh->title,"Re: ",4))strcpy(buf+7,fh->title + 4);
        else
            strcpy(buf+7,fh->title);
        if(strlen(buf) >= STRLEN )buf[STRLEN-1] = 0;
        strcpy(title,buf);
        //post file to the board
        if(post_file(getCurrentUser(),"",annpath,currboard->filename,title,0,2,getSession()) < 0) {//fail
            sprintf(buf,"·¢±íÎÄÕÂµ½°æÃæ³ö´í!Çë°´ ENTER ¼üÍË³ö << ");
            a_prompt(-1,buf,title);
            saveline(t_lines - 2, 1, NULL);
            saveline(t_lines - 3, 1, linebuffer);
        }
        unlink(annpath);
        sprintf(annpath,"tmp/se.%s",getCurrentUser()->userid);
        unlink(annpath);
        return DIRCHANGED;
    }
    return DIRCHANGED;
}

#define SPLIT_THREAD_MAXNUM 1000
struct split_thread_arg{
       int num;
       int oldid[SPLIT_THREAD_MAXNUM];
};

static int split_thread_me(struct _select_def* conf, struct fileheader* fh,int ent, void* extraarg)
{
    struct read_arg* arg=(struct read_arg*)conf->arg;
    struct split_thread_arg* func_arg=(struct split_thread_arg*)extraarg;
    int ret=APPLY_CONTINUE;
       int i;

    conf->pos=ent;
    if (arg->writearg) {
        arg->writearg->ent=ent;
    }
       if( func_arg->num >= SPLIT_THREAD_MAXNUM ) return ret;
       for(i=0;i<func_arg->num; i++){
               if(fh->reid == func_arg->oldid[i]){
                       break;
               }
       }
       if(i>0 && i==func_arg->num) return ret;

       func_arg->oldid[func_arg->num] = fh->id;
       func_arg->num ++;

       fh->groupid = func_arg->oldid[0];
       if(func_arg->num == 1){
               char buf[256];
               fh->reid = func_arg->oldid[0];
               if(!strncmp(fh->title, "Re: ", 4)){
                       strncpy(buf, fh->title+4, 256);
                       if(*buf)
                               strcpy(fh->title, buf);
               }
       }

    return ret;
}

static int split_thread(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
    int ent;
    char buf[STRLEN];
    struct split_thread_arg func_arg;
    struct read_arg* arg=(struct read_arg*)conf->arg;
    struct write_dir_arg dirarg;

    if (fh==NULL)
        return DONOTHING;
    if (!chk_currBM(currBM, getCurrentUser())) {
        return DONOTHING;
    }
    if (arg->mode != DIR_MODE_NORMAL)     /* KCN:Ôİ²»ÔÊĞí */
        return DONOTHING;
    if (conf->pos>arg->filecount) /*ÖÃ¶¥*/
        return DONOTHING;
    if (fh->id == fh->groupid)
        return DONOTHING;

    a_prompt(-1, "ÇĞ·ÖÖ÷Ìâ£¬²Ù×÷ºóÎŞ·¨¸´Ô­£¬È·ÈÏÂğ£¿(Y/N) [N] ", buf);
    if (buf[0] != 'Y' && buf[0] != 'y') {     /* if not yes quit */
        return FULLUPDATE;
    }

    func_arg.num = 0;

    ent=conf->pos;
    init_write_dir_arg(&dirarg);
    dirarg.fd=arg->fd;
    dirarg.needlock=false;
    arg->writearg=&dirarg;

    flock(arg->fd,LOCK_EX);
    apply_thread(conf,fh,split_thread_me,true,true,(void*)&func_arg);
    flock(arg->fd,LOCK_UN);
    free_write_dir_arg(&dirarg);
    arg->writearg=NULL;
    conf->pos=ent; /*»Ö¸´Ô­À´µÄent*/

    return DIRCHANGED;
}


#ifdef FB2KPC
int read_my_pc(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{	
	Personal("*");
	return FULLUPDATE;
}

#endif

static int prompt_newkey(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg) {
    char *prompt = (char*)extraarg;

    clear();
    move(3,0);
    prints("\t\t\033[1;4;31m%s¿ì½İ¼ü»ØÊÕµÚÒ»ºÅÁî\033[0m\n\n", BBS_FULL_NAME);
    prints("\t\tÍ¬Ö¾ÃÇ£¬ÅóÓÑÃÇ£¡¿ì½İ¼ü»ØÊÕÀ²£¡\n\n\t\tÄú¸Õ²Å°´µÄ¼üÒÑ¾­±»¼¼ÊõÕ¾ÎñË½ÍÌ¡£\n\n");
    prints("\t\t\033[1;32m%s\033[0m\n",prompt);
    move(13,50); prints("\033[1;31m¨q¦ä¦ä¦ä¨r");
	move(14,50); prints("{/-¡ò¡ò-\\}");
 	move(15,50); prints(" ( (oo) )");
 	move(16,50); prints("    ¦á\033[0m");
    pressanykey();
    return FULLUPDATE;    
}

#ifdef HAPPY_BBS
/* ipv1: Let a bm modify his board's title freely. */
static int b_modify_title(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{   
    char ans[STRLEN],pmt[STRLEN];
    char *buf;
    struct boardheader newfh;
    int pos;
    if (!chk_currBM(currBM, getCurrentUser())) {
        return DONOTHING;
    }
    buf=&currboard->title[13];
    strcpy(ans, buf);
    move(t_lines - 1, 0);    
    getdata(t_lines - 1, 0, "ĞŞ¸Ä±¾°æÖĞÎÄÃû£º", ans, STRLEN - 1, DOECHO, NULL, false);
    if(!strcmp(ans,"")) return FULLUPDATE;
    if(!strcmp(ans,buf)) return FULLUPDATE;
    if (!(currboard->flag & BOARD_SUPER_CLUB) && !((currboard->title[0]=='P')&&(currboard->title[1]=='_'))) {
        sprintf(pmt, "ĞŞ¸Ä %s °æÖĞÎÄÃû", currboard->filename); 
    	securityreport(pmt, NULL,NULL, getSession());
    }
    ans[STRLEN-14]='\0';
    pos = getboardnum(currboard->filename,&newfh);
    newfh.title[13]='\0';
    strcat(newfh.title,ans);
    set_board(pos, &newfh, NULL);
    return FULLUPDATE;
}
#endif

static struct key_command read_comms[] = { /*ÔÄ¶Á×´Ì¬£¬¼ü¶¨Òå */
    {'r', (READ_KEY_FUNC)read_post,NULL},
    {'K', (READ_KEY_FUNC)skip_post,NULL},

    {'d', (READ_KEY_FUNC)del_post,(void*)0},
    {'D',(READ_KEY_FUNC)delete_range,NULL},
    {Ctrl('C'), (READ_KEY_FUNC)do_cross,NULL},
    {'Y', (READ_KEY_FUNC)UndeleteArticle,NULL},     /* Leeward 98.05.18 */
    {Ctrl('P'), (READ_KEY_FUNC)do_post,NULL},
    {'E', (READ_KEY_FUNC)edit_post,NULL},
    {'T', (READ_KEY_FUNC)edit_title,NULL},
    {':', (READ_KEY_FUNC)fhselect,NULL},
            
    {'m', (READ_KEY_FUNC)set_article_flag,(void*)FILE_MARK_FLAG},
    {';', (READ_KEY_FUNC)noreply_post,(void*)NULL},        /*Haohmaru.99.01.01,Éè¶¨²»¿ÉreÄ£Ê½ */
    {'#', (READ_KEY_FUNC)set_article_flag,(void*)FILE_SIGN_FLAG},           /* Bigman: 2000.8.12  Éè¶¨ÎÄÕÂ±ê¼ÇÄ£Ê½ */
    {'%', (READ_KEY_FUNC)set_article_flag,(void*)FILE_PERCENT_FLAG},           /* asing: 2004.4.16  Éè¶¨ÎÄÕÂ±ê¼ÇÄ£Ê½ */
#ifdef FILTER
    {'@', (READ_KEY_FUNC)set_article_flag,(void*)FILE_CENSOR_FLAG},         /* czz: 2002.9.29 ÉóºË±»¹ıÂËÎÄÕÂ */
#endif
    {'g', (READ_KEY_FUNC)set_article_flag,(void*)FILE_DIGEST_FLAG},
    {'t', (READ_KEY_FUNC)set_article_flag,(void*)FILE_DELETE_FLAG},     /*KCN 2001 */
    {'|', (READ_KEY_FUNC)split_thread,NULL},

    {'G', (READ_KEY_FUNC)range_flag,(void*)FILE_TITLE_FLAG},
        
    {'H', (READ_KEY_FUNC)read_callfunc0, (void *)read_hot_info},   /* flyriver: 2002.12.21 Ôö¼ÓÈÈÃÅĞÅÏ¢ÏÔÊ¾ */
        
    {Ctrl('G'), (READ_KEY_FUNC)change_mode,(void*)0},   /* bad : 2002.8.8 add marked mode */
    {'`', (READ_KEY_FUNC)prompt_newkey,(void*)"ÇëÊ¹ÓÃ Ctrl+G 1 ½øÈëÎÄÕªÇø"},
    {'.', (READ_KEY_FUNC)change_mode,(void*)DIR_MODE_DELETED},
    {'>', (READ_KEY_FUNC)change_mode,(void*)DIR_MODE_JUNK},
    {Ctrl('T'), (READ_KEY_FUNC)prompt_newkey,(void*)"ÇëÊ¹ÓÃ Ctrl+G 2 ½øÈëÍ¬Ö÷ÌâÔÄ¶Á"},

    {'s', (READ_KEY_FUNC)do_select,NULL},
    {'x', (READ_KEY_FUNC)into_announce,NULL},
    {'v', (READ_KEY_FUNC)read_callfunc0, (void *)i_read_mail},         /* period 2000-11-12 read mail in article list */

    {'i', (READ_KEY_FUNC)Save_post,NULL},
    {'J', (READ_KEY_FUNC)Semi_save,NULL},
    {'I', (READ_KEY_FUNC)Import_post,NULL},

    {'F', (READ_KEY_FUNC)mail_forward,NULL},
    {Ctrl('R'), (READ_KEY_FUNC)post_reply,NULL},

    {'c', (READ_KEY_FUNC)clear_new_flag,NULL},
    {'f', (READ_KEY_FUNC)clear_all_new_flag,NULL},

    {'n',  (READ_KEY_FUNC)thread_read,(void*)SR_FIRSTNEW},
    {Ctrl('N'), (READ_KEY_FUNC)prompt_newkey,(void*)"ÇëÊ¹ÓÃ n Ìø×ªµ½±¾Ö÷ÌâµÚÒ»ÆªÎ´¶Á"},
    {'\\', (READ_KEY_FUNC)thread_read,(void*)SR_LAST},
    {'=', (READ_KEY_FUNC)thread_read,(void*)SR_FIRST},

    {'a', (READ_KEY_FUNC)auth_search,(void*)false},
    {'A', (READ_KEY_FUNC)auth_search,(void*)true},
    {'/', (READ_KEY_FUNC)title_search,(void*)false},
    {'?', (READ_KEY_FUNC)title_search,(void*)true},
    {']', (READ_KEY_FUNC)thread_read,(void*)SR_NEXT},
    {'[', (READ_KEY_FUNC)thread_read,(void*)SR_PREV},

    {Ctrl('A'), (READ_KEY_FUNC)read_showauthor,NULL},
    {'~',(READ_KEY_FUNC)read_authorinfo,NULL},
    {Ctrl('W'), (READ_KEY_FUNC)read_showauthorBM,NULL}, 
    {Ctrl('O'), (READ_KEY_FUNC)read_addauthorfriend,NULL},
#ifdef DENYANONY
    {Ctrl('Y'), (READ_KEY_FUNC)deny_anony,NULL},
#else
    {Ctrl('Y'), (READ_KEY_FUNC)read_zsend,NULL},
#endif
#ifdef PERSONAL_CORP
    {'y', (READ_KEY_FUNC)read_importpc,NULL},
#endif

    {'\'',(READ_KEY_FUNC)post_search,(void*)false},
    {'\"', (READ_KEY_FUNC)post_search,(void*)true},

    {'R',  (READ_KEY_FUNC)b_results,NULL},
    {'V',  (READ_KEY_FUNC)b_vote,NULL},
    {'M',  (READ_KEY_FUNC)b_vote_maintain,NULL},
    {'W',  (READ_KEY_FUNC)b_note_edit_new,NULL},
    {'h',  (READ_KEY_FUNC)mainreadhelp,NULL},
#ifdef FB2KPC
    {'X',  (READ_KEY_FUNC)read_my_pc,NULL},     //±à¼­°æÃæµÄÖÙ²ÃÎ¯Ô±Ãûµ¥,stephen on 2001.11.1 
#else
    {'X',  (READ_KEY_FUNC)b_jury_edit,NULL},     //±à¼­°æÃæµÄÖÙ²ÃÎ¯Ô±Ãûµ¥,stephen on 2001.11.1 
#endif
    {KEY_TAB,  (READ_KEY_FUNC)show_b_note,NULL},
    {Ctrl('D'), (READ_KEY_FUNC)deny_user,NULL},
    {Ctrl('E'), (READ_KEY_FUNC)clubmember,NULL},
    {'z', (READ_KEY_FUNC)read_sendmsgtoauthor,NULL},
    {'Z', (READ_KEY_FUNC)prompt_newkey,(void*)"ÇëÊ¹ÓÃ z ¸ø×÷Õß·¢Ñ¶Ï¢"},

    {'p',  (READ_KEY_FUNC)thread_read,(void*)SR_READ},
    {Ctrl('S'), (READ_KEY_FUNC)prompt_newkey,(void*)"ÇëÊ¹ÓÃ p ½øÈëÍ¬Ö÷ÌâÔÄ¶Á"},
#ifdef FB2KPC
    {Ctrl('X'), (READ_KEY_FUNC)into_PAnnounce,NULL},
#else
    {Ctrl('X'), (READ_KEY_FUNC)thread_read,(void*)SR_READX},
#endif
    
    {Ctrl('U'), (READ_KEY_FUNC)author_read,(void*)SR_READ},
    {Ctrl('H'), (READ_KEY_FUNC)author_read,(void*)SR_READX}, 
    /*----------------------------------------*/
    {'S', (READ_KEY_FUNC)sequential_read,NULL},
    
    {'b', (READ_KEY_FUNC)SR_BMFunc,(void*)true},
    {'B', (READ_KEY_FUNC)prompt_newkey,(void*)"ÇëÊ¹ÓÃ b ½øÈëÍ¬Ö÷Ìâ²Ù×÷"},
    {',', (READ_KEY_FUNC)read_splitscreen,NULL},
    {'!', (READ_KEY_FUNC)read_callfunc0,(void *)Goodbye},
    {Ctrl('Q'), (READ_KEY_FUNC)showinfo,NULL},
    {'^', (READ_KEY_FUNC)jumpReID,NULL},
    {'&',(READ_KEY_FUNC)jump_changed_title,NULL},
    {'\0', NULL},
};

int Read()
{
    char buf[2 * STRLEN];
    char notename[STRLEN];
    time_t usetime;
    struct stat st;
    int bid;
    int returnmode=CHANGEMODE;
#ifdef NEW_HELP
    int oldhelpmode;
#endif

    if (!selboard||!currboard) {
        move(2, 0);
        prints("ÇëÏÈÑ¡ÔñÌÖÂÛÇø\n");
        pressreturn();
        move(2, 0);
        clrtoeol();
        return -1;
    }
    in_mail = false;
    bid = getbnum_safe(currboard->filename,getSession());

    currboardent=bid;
    currboard=(struct boardheader*)getboard(bid);

    if (currboard->flag&BOARD_GROUP) return -2;
#ifdef HAVE_BRC_CONTROL
    brc_initial(getCurrentUser()->userid, currboard->filename,getSession());
#endif

    setbdir(DIR_MODE_NORMAL, buf, currboard->filename);

    board_setcurrentuser(uinfo.currentboard, -1);
    uinfo.currentboard = currboardent;;
    UPDATE_UTMP(currentboard,uinfo);
    board_setcurrentuser(uinfo.currentboard, 1);
    
    setvfile(notename, currboard->filename, "notes");
    if (stat(notename, &st) != -1) {
        if (st.st_mtime < (time(NULL) - 7 * 86400)) {
/*            sprintf(genbuf,"touch %s",notename);
	    */
            f_touch(notename);
            setvfile(genbuf, currboard->filename, "noterec");
            my_unlink(genbuf);
        }
    }
    if (vote_flag(currboard->filename, '\0', 1 /*¼ì²é¶Á¹ıĞÂµÄ±¸ÍüÂ¼Ã» */ ) == 0) {
        if (dashf(notename)) {
            /*
             * period  2000-09-15  disable ActiveBoard while reading notes 
             */
            modify_user_mode(READING);
            /*-	-*/
            ansimore(notename, true);
            vote_flag(currboard->filename, 'R', 1 /*Ğ´Èë¶Á¹ıĞÂµÄ±¸ÍüÂ¼ */ );
        }
    }
    usetime = time(0);
#ifdef NEW_HELP
    oldhelpmode = helpmode;
    helpmode = HELP_ARTICLE;
#endif
    while ((returnmode==CHANGEMODE)&&!(currboard->flag&BOARD_GROUP)) {
    returnmode=new_i_read(DIR_MODE_NORMAL, buf, readtitle, (READ_ENT_FUNC) readdoent, &read_comms[0], sizeof(struct fileheader));  /*½øÈë±¾°æ */
    setbdir(DIR_MODE_NORMAL, buf, currboard->filename);
    }
#ifdef NEW_HELP
    helpmode = oldhelpmode;
#endif
    newbbslog(BBSLOG_BOARDUSAGE, "%-20s Stay: %5ld", currboard->filename, time(0) - usetime);
    bmlog(getCurrentUser()->userid, currboard->filename, 0, time(0) - usetime);
    bmlog(getCurrentUser()->userid, currboard->filename, 1, 1);

    board_setcurrentuser(uinfo.currentboard, -1);
    uinfo.currentboard = 0;
    UPDATE_UTMP(currentboard,uinfo);
    return returnmode;
}

/* etnlegend, 2006.05.28, ÔÄ¶ÁÊ®´ó ... */

#ifndef USE_PRIMORDIAL_TOP10

static void read_top_title(struct _select_def *conf){
    struct BoardStatus *bs;
    char header[PATHLEN],title[STRLEN],BM[BM_LEN],*p,*q;
    int chkmailflag,bid,online;
    if(!(bid=getbid(currboard->filename,NULL))||!(bs=getbstatus(bid)))
        return;
    if(!(currboard->BM[0]))
        sprintf(header,"³ÏÕ÷°æÖ÷ÖĞ");
    else{
        snprintf(BM,BM_LEN,"%s",currboard->BM);
        if(
#ifdef OPEN_BMONLINE
            1
#else /* OPEN_BMONLINE */
            check_board_delete_read_perm(getCurrentUser(),currboard,0)
#endif /* OPEN_BMONLINE */
            ){
            sprintf(header,"%s","°æÖ÷: ");
            q=&header[strlen(header)];
            for(online=0,p=strtok(BM," ");p;p=strtok(NULL," ")){
                if(apply_utmp(NULL,1,p,NULL)){
                    if(!online){
                        sprintf(q,"%s","\033[32m");
                        q=&header[strlen(header)];
                        online=1;
                    }
                }
                else{
                    if(online){
                        sprintf(q,"%s","\033[33m");
                        q=&header[strlen(header)];
                        online=0;
                    }
                }
                sprintf(q,"%s ",p);
                q=&header[strlen(header)];
            }
            sprintf(q,"%s","\033[m");
        }
        else
            sprintf(header,"°æÖ÷: %s",BM);
    }
    chkmailflag=chkmail();
    if(chkmailflag==2)
        sprintf(title,"%s","[ÄúµÄĞÅÏä³¬¹ıÈİÁ¿,²»ÄÜÔÙÊÕĞÅ!]");
    else if(chkmailflag)
        sprintf(title,"%s","[ÄúÓĞĞÅ¼ş]");
    else
        sprintf(title,"%s",&(currboard->title[13]));
    showtitle(header,title);
    update_endline();
    move(1, 0);
    clrtoeol();
    sprintf(genbuf,"%s\033[m",DEFINE(getCurrentUser(),DEF_HIGHCOLOR)?(
        "\033[1;31m[Ê®´óÄ£Ê½] \033[1;37mÀë¿ª[\033[1;32m¡û\033[1;37m,\033[1;32me\033[1;37m] "
        "Ñ¡Ôñ[\033[1;32m¡ü\033[1;37m,\033[1;32m¡ı\033[1;37m] ÔÄ¶Á[\033[1;32m¡ú\033[1;37m,\033[1;32mr\033[1;37m] "
        "°ïÖú[\033[1;32mh\033[1;37m] Í¬Ö÷Ìâ[\033[1;32m^X\033[1;37m,\033[1;32mp\033[1;37m] "
        "Í¬×÷Õß[\033[1;32m^U\033[1;37m,\033[1;32m^H\033[1;37m]"):(
        "\033[31m[Ê®´óÄ£Ê½] \033[37mÀë¿ª[\033[1;32m¡û\033[37m,\033[32me\033[37m] "
        "Ñ¡Ôñ[\033[32m¡ü\033[37m,\033[32m¡ı\033[37m] ÔÄ¶Á[\033[32m¡ú\033[37m,\033[32mr\033[37m] "
        "°ïÖú[\033[32mh\033[37m] Í¬Ö÷Ìâ[\033[32m^X\033[37m,\033[32mp\033[37m] "
        "Í¬×÷Õß[\033[32m^U\033[37m,\033[32m^H\033[37m]"));
    prints("%s",genbuf);
    move(2, 0);
    setfcolor(WHITE,1);
    setbcolor(BLUE);
    clrtoeol();
    sprintf(genbuf,"%-80.80s","  ±àºÅ   ¿¯ µÇ Õß     ÈÕ  ÆÚ  ÎÄÕÂ±êÌâ");
    prints("%s",genbuf);
    sprintf(title,"ÔÚÏß: %-4d [Ê®´óÄ£Ê½]",bs->currentusers);
    move(2,-(strlen(title)+1));
    prints("%s",title);
    resetcolor();
    return;
}

static char* read_top_ent(char *buf,int num,struct fileheader *fh,struct fileheader *read_fh,struct _select_def *conf){
    char date[8],title[48],threadprefix[2][16],threadsuffix[16],*highstr;
    int type,isreply,isthread,attachch;
    time_t ftime;
    type=get_article_flag(fh,getCurrentUser(),currboard->filename,0,NULL,getSession());
    if((ftime=get_posttime(fh))>740000000)
        snprintf(date,7,"%s",ctime(&ftime)+4);
    else
        date[0]=0;
    attachch=((fh->attachment)?'@':' ');
	strnzhcpy(title,fh->title,45);
    isthread=(read_fh&&read_fh->groupid==fh->groupid);
    isreply=((fh->groupid!=fh->id)&&(!strncasecmp(title,"Re: ",4)||!strncmp(title,"»Ø¸´: ",6)));
    highstr=DEFINE(getCurrentUser(),DEF_HIGHCOLOR)?"1;":"";
	if(isthread){
        sprintf(threadprefix[0],"\033[%s%dm",highstr,(isreply?36:33));
        sprintf(threadprefix[1],"\033[%s%dm.",highstr,(isreply?36:33));
        sprintf(threadsuffix,"%s","\033[m");
	}else{
		threadprefix[0][0]=0;
        threadprefix[1][0]=' ';
        threadprefix[1][1]=0;
		threadsuffix[0]=0;
	}
    sprintf(buf," %s%4d%s %c %-12.12s %s%s%c%s%s%s ",threadprefix[0],num,threadsuffix,type,fh->owner,date,
        threadprefix[1],attachch,(isreply?"":FIRSTARTICLE_SIGN" "),title,threadsuffix);
    return buf;
}

static int read_top_post(struct _select_def *conf,struct fileheader *fh,void *varg){
    struct read_arg *arg;
    char buf[PATHLEN],*p;
    int key,repeat,ret;
    if(!fh||(fh->owner[0]=='-'))
        return DONOTHING;
    snprintf(buf,PATHLEN,"%s",read_getcurrdirect(conf));
    if(!(p=strrchr(buf,'/')))
        return DONOTHING;
    *++p=0;
    clear();
    snprintf(p,(PATHLEN-(p-buf)),"%s",fh->filename);
    snprintf(quote_board,BOARDNAMELEN,"%s",currboard->filename);
    snprintf(quote_title,ARTICLE_TITLE_LEN,"%s",fh->title);
    snprintf(quote_user,OWNER_LEN,"%s",fh->owner);
    register_attach_link(board_attach_link,fh);
    key=ansimore_withzmodem(buf,false,fh->title);
    register_attach_link(NULL,NULL);
#ifdef HAVE_BRC_CONTROL
    brc_add_read(fh->id,currboardent,getSession());
#endif /* HAVE_BRC_CONTROL */
    arg=(struct read_arg*)conf->arg;
    move(t_lines-1,0);
    clrtoeol();
    switch(arg->readmode){
        case READ_THREAD:
            sprintf(genbuf,"%s","\033[36;44m[Ê®´óÄ£Ê½] \033[31;44m[Í¬Ö÷ÌâÔÄ¶Á]\033[33;44m "
                "½áÊø Q,¡û | ÉÏÒ»Æª ¡ü | ÏÂÒ»Æª <Space>,¡ı ");
            break;
        case READ_AUTHOR:
            sprintf(genbuf,"%s","\033[36;44m[Ê®´óÄ£Ê½] \033[31;44m[Í¬×÷ÕßÔÄ¶Á]\033[33;44m "
                "½áÊø Q,¡û | ÉÏÒ»Æª ¡ü | ÏÂÒ»Æª <Space>,¡ı ");
            break;
        default:
            sprintf(genbuf,"%s","\033[36;44m[Ê®´óÄ£Ê½] \033[31;44m[ÔÄ¶ÁÎÄÕÂ]\033[33;44m "
                "½áÊø Q,¡û | ÉÏÒ»Æª ¡ü | ÏÂÒ»Æª <Space>,¡ı | Í¬Ö÷Ìâ ^X,p ");
            break;
    }
    prints(DEFINE(getCurrentUser(),DEF_HIGHCOLOR)?"\033[1m%s\033[m":"%s\033[m",genbuf);
    resetcolor();
    setreadpost(conf,fh);
    ret=FULLUPDATE;
    if(!(key==KEY_RIGHT||key==KEY_PGUP||key==KEY_UP||key==KEY_DOWN)&&(!(key>0)||!strchr("RrEexp",key)))
        key=igetkey();
    repeat=0;
    do{
        if(repeat)
            key=igetkey();
        repeat=0;
        switch(key){
            case KEY_LEFT:
            case 'Q':
            case 'q':
            case KEY_REFRESH:
                break;
            case KEY_DOWN:
            case KEY_PGDN:
            case ' ':
            case 'j':
            case 'n':
                return READ_NEXT;
            case KEY_UP:
            case KEY_PGUP:
            case 'k':
            case 'l':
                return READ_PREV;
            case KEY_RIGHT:
            case 'p':
            case Ctrl('X'):
            case Ctrl('S'):
            case Ctrl('U'):
            case Ctrl('H'):
                if(arg->readmode!=READ_NORMAL)
                    return READ_NEXT;
                move(t_lines-1,0);
                clrtoeol();
                sprintf(genbuf,"%s",
                    (key==Ctrl('U')||key==Ctrl('H'))?
                    "\033[36;44m[Ê®´óÄ£Ê½] \033[31;44m[Í¬×÷ÕßÔÄ¶Á]\033[33;44m "
                    "½áÊø Q,¡û | ÉÏÒ»Æª ¡ü | ÏÂÒ»Æª <Space>,¡ı ":
                    "\033[36;44m[Ê®´óÄ£Ê½] \033[31;44m[Í¬Ö÷ÌâÔÄ¶Á]\033[33;44m "
                    "½áÊø Q,¡û | ÉÏÒ»Æª ¡ü | ÏÂÒ»Æª <Space>,¡ı "
                    );
                prints(DEFINE(getCurrentUser(),DEF_HIGHCOLOR)?"\033[1m%s\033[m":"%s\033[m",genbuf);
                arg->readmode=((key==Ctrl('U')||key==Ctrl('H'))?READ_AUTHOR:READ_THREAD);
                arg->oldpos=((key==KEY_RIGHT||key==Ctrl('X')||key==Ctrl('H'))?conf->pos:0);
                repeat=1;
                break;
            case 'R':
            case 'r':
            case 'Y':
            case 'y':
                clear();
                move(5,0);
                if(currboard->flag&BOARD_NOREPLY)
                    prints("\t\t\033[1;33m%s\033[0;33m<Enter>\033[m","¸Ã°æÃæÒÑÉèÖÃÎª²»¿É»Ø¸´ÎÄÕÂ...");
                else if(fh->accessed[1]&FILE_READ)
                    prints("\t\t\033[1;33m%s\033[0;33m<Enter>\033[m","±¾ÎÄÒÑÉèÖÃÎª²»¿É»Ø¸´, ÇëÎğÊÔÍ¼ÌÖÂÛ...");
                else{
                    do_reply(conf,fh);
                    return DIRCHANGED;
                }
                WAIT_RETURN;
                break;
            case Ctrl('R'):
                post_reply(conf,fh,NULL);
                break;
            case Ctrl('A'):
                clear();
                read_showauthor(conf,fh,NULL);
                return READ_NEXT;
            case Ctrl('Z'):
            case 'H':
                r_lastmsg();
                break;
            case 'Z':
            case 'z':
                if(HAS_PERM(getCurrentUser(),PERM_PAGE)){
                    read_sendmsgtoauthor(conf,fh,NULL);
                    return READ_NEXT;
                }
                break;
            case 'u':
                clear();
                modify_user_mode(QUERY);
                t_query(NULL);
                break;
            case 'L':
                if(uinfo.mode==LOOKMSGS)
                    return DONOTHING;
                show_allmsgs();
                break;
            case 'O':
            case 'o':
                if(HAS_PERM(getCurrentUser(),PERM_BASIC)){
                    t_friends();
                }
                break;
            case 'U':
                return board_query();
            case Ctrl('O'):
                clear();
                read_addauthorfriend(conf,fh,NULL);
                return READ_NEXT;
            case '~':
                ret=read_authorinfo(conf,fh,NULL);
                break;
            case Ctrl('W'):
                ret=read_showauthorBM(conf,fh,NULL);
                break;
            case Ctrl('D'):
                zsend_attach(conf->pos,fh,read_getcurrdirect(conf));
                break;
            case Ctrl('Y'):
                read_zsend(conf,fh,NULL);
                break;
            case '!':
                Goodbye();
                break;
        }
    }
    while(repeat);
    if(ret==FULLUPDATE&&arg->oldpos!=0){
        conf->new_pos=arg->oldpos;
	    arg->oldpos=0;
	    list_select_add_key(conf,KEY_REFRESH);
        arg->readmode=READ_NORMAL;
        return SELCHANGE;
    }
    return ret;
}

static int read_top_edit_title(struct _select_def *conf,struct fileheader *fh,void *varg){
    struct read_arg *arg=(struct read_arg*)(conf->arg);
    char buf[STRLEN],path[PATHLEN];
    int change,index;
    switch(deny_modify_article(currboard,fh,arg->mode,getSession())){
        case 0:
            break;
        case -2:
            move(3,0);
            clrtobot();
            move(5,0);
            prints("\t\t\033[1;33m%s\033[0;33m<Enter>\033[m","ÄúÒÑ±»¹ÜÀíÈËÔ±È¡ÏûÔÚµ±Ç°°æÃæµÄ·¢ÎÄÈ¨ÏŞ...");
            WAIT_RETURN;
            return FULLUPDATE;
        case -5:
            check_readonly(currboard->filename);
            return FULLUPDATE;
        default:
            return DONOTHING;
    }
    snprintf(buf,STRLEN,"%s",fh->title);
    getdata(t_lines-1,0,"ĞÂÎÄÕÂ±êÌâ: ",buf,78,DOECHO,NULL,false);
    if(!(change=(buf[0]&&strcmp(buf,fh->title))))
        return PARTUPDATE;
#ifdef FILTER
    if(check_badword_str(buf,strlen(buf),getSession())){
        move(3,0);
        clrtobot();
        prints("\t\t\033[1;33m%s\033[0;33m<Enter>\033[m","¸Ã±êÌâÖĞ¿ÉÄÜº¬ÓĞ²»µ±ÄÚÈİ, Çë¼ì²é¸ü»»...");
        WAIT_RETURN;
        return PARTUPDATE;
    }
#endif /* FILTER */
    process_control_chars(buf);
    setbfile(path,currboard->filename,fh->filename);
    add_edit_mark(path,2,buf,getSession());
    newbbslog(BBSLOG_USER,"read_top_edit_title %s %s %s",currboard->filename,fh->title,buf);
    strnzhcpy(fh->title,buf,ARTICLE_TITLE_LEN);
    setbdir(DIR_MODE_NORMAL,path,currboard->filename);
    if((index=get_ent_from_id(DIR_MODE_NORMAL,fh->id,currboard->filename))!=0)
        substitute_record(path,fh,sizeof(struct fileheader),index);
    if(fh->id==fh->groupid){
        if(setboardorigin(currboard->filename,-1))
            board_regenspecial(currboard->filename,DIR_MODE_ORIGIN,NULL);
        else{
            setbdir(DIR_MODE_ORIGIN,path,currboard->filename);
            if((index=get_ent_from_id(DIR_MODE_ORIGIN,fh->id,currboard->filename))!=0)
                substitute_record(path,fh,sizeof(struct fileheader),index);
        }
    }
    if(fh->accessed[0]&FILE_MARKED){
        if(setboardmark(currboard->filename,-1))
            board_regenspecial(currboard->filename,DIR_MODE_MARK,NULL);
        else{
            setbdir(DIR_MODE_MARK,path,currboard->filename);
            if((index=get_ent_from_id(DIR_MODE_MARK,fh->id,currboard->filename))!=0)
                substitute_record(path,fh,sizeof(struct fileheader),index);
        }
    }
    setboardtitle(currboard->filename,1);
    return PARTUPDATE;
}

static struct key_command read_top_comms[]={
    {'r',(READ_KEY_FUNC)read_top_post,NULL},
    {Ctrl('A'),(READ_KEY_FUNC)read_showauthor,NULL},
    {'K',(READ_KEY_FUNC)skip_post,NULL},
    {Ctrl('C'),(READ_KEY_FUNC)do_cross,NULL},
    {'v',(READ_KEY_FUNC)read_callfunc0,(void*)i_read_mail},
    {'=',(READ_KEY_FUNC)thread_read,(void*)SR_FIRST},
    {'^',(READ_KEY_FUNC)jumpReID,NULL},
    {'&',(READ_KEY_FUNC)jump_changed_title,NULL},
    {'z',(READ_KEY_FUNC)read_sendmsgtoauthor,NULL},
    {'p',(READ_KEY_FUNC)thread_read,(void*)SR_READ},
    {'F',(READ_KEY_FUNC)mail_forward,NULL},
    {'f',(READ_KEY_FUNC)clear_all_new_flag,NULL},
    {'c',(READ_KEY_FUNC)clear_new_flag,NULL},
    {'E',(READ_KEY_FUNC)edit_post,NULL},
    {'T',(READ_KEY_FUNC)read_top_edit_title,NULL},
    {Ctrl('R'),(READ_KEY_FUNC)post_reply,NULL},
    {']',(READ_KEY_FUNC)thread_read,(void*)SR_NEXT},
    {'[',(READ_KEY_FUNC)thread_read,(void*)SR_PREV},
    {'n',(READ_KEY_FUNC)thread_read,(void*)SR_FIRSTNEW},
    {'\\',(READ_KEY_FUNC)thread_read,(void*)SR_LAST},
    {'a',(READ_KEY_FUNC)auth_search,(void*)false},
    {'A',(READ_KEY_FUNC)auth_search,(void*)true},
    {'/',(READ_KEY_FUNC)title_search,(void*)false},
    {'?',(READ_KEY_FUNC)title_search,(void*)true},
    {'\'',(READ_KEY_FUNC)post_search,(void*)false},
    {'\"',(READ_KEY_FUNC)post_search,(void*)true},
    {'h',(READ_KEY_FUNC)mainreadhelp,NULL},
    {Ctrl('X'),(READ_KEY_FUNC)thread_read,(void*)SR_READX},
    {Ctrl('U'),(READ_KEY_FUNC)author_read,(void*)SR_READ},
    {Ctrl('H'),(READ_KEY_FUNC)author_read,(void*)SR_READX},
    {',',(READ_KEY_FUNC)read_splitscreen,NULL},
    {Ctrl('Q'),(READ_KEY_FUNC)showinfo,NULL},
    {Ctrl('O'),(READ_KEY_FUNC)read_addauthorfriend,NULL},
#ifdef PERSONAL_CORP
    {'y',(READ_KEY_FUNC)read_importpc,NULL},
#endif
    {'~',(READ_KEY_FUNC)read_authorinfo,NULL},
    {Ctrl('W'),(READ_KEY_FUNC)read_showauthorBM,NULL},
    {Ctrl('Y'),(READ_KEY_FUNC)read_zsend,NULL},
    {'!',(READ_KEY_FUNC)read_callfunc0,(void*)Goodbye},
    {0,NULL}
};

static int read_top(int index,int force){
/*
 *  ¹ØÓÚ RT_INTERVAL ºÍ RT_INTERVAL_FORCE ºêµÄ×÷ÓÃ ----
 *
 *  ÕâÁ½¸öºê¶¨ÒåµÄÊ±¼ä¼ä¸ô(µ¥Î»ÎªÃë)¿ØÖÆ¸üĞÂÊ®´óÖ÷ÌâËùÓÃµÄË÷Òı(.TOP.<gid>)µÄËÉ³Ú²ßÂÔ, Èô°æÃæ
 *  ÎÄÕÂË÷Òı(.DIR)µÄĞŞ¸ÄÊ±¼ä±ÈÊ®´óË÷Òı(.TOP.<gid>)µÄĞŞ¸ÄÊ±¼äĞÂ RT_INTERVAL ÃëÒÔÉÏ, »òÕß .DIR
 *  ±È .TOP.<gid> ĞÂ²¢ÇÒºóÕßÒÑ¾­³¬¹ı RT_INTERVAL_FORCE ÃëÎ´¸üĞÂ, Ôò´Ó .DIR ÖØĞÂÉú³É .TOP.<gid>,
 *  ·ñÔòÖ±½ÓÊ¹ÓÃÔ­ÓĞµÄ .TOP.<gid>, µ±È», Èç¹û .TOP.<gid> ²»´æÔÚ»òÕß²ÎÊı `force` ÉèÖÃÎªÕæÒ²»á
 *  µ¼ÖÂÖØĞÂÉú³É .TOP.<gid>, ¾­¹ı³õ²½¹ÀËã, ÔÄ¶ÁÊ®´óµÄÏµÍ³ÏûºÄ²¢²»±ÈÍ¬×÷ÕßËÑË÷/±êÌâ¹Ø¼ü×ÖËÑË÷
 *  ¸ü´ó, Òò´ËÔÚ¹æÄ£²»ÊÇºÜ´óµÄÕ¾µãÉÏÕâÁ½¸öºê¿ÉÒÔ·ÅĞÄµÄ¶¨ÒåÎª 0, µ«Ä¿Ç°´Ë²¿·Ö´úÂëÉĞÎ´ÔÚË®Ä¾Éç
 *  ÇøÉÏÍê³ÉÈ«ÃæµÄÑ¹Á¦²âÊÔ, Òò´Ë cvs ÉÏÈÔÈ»±£Áô RT_INTERVAL/RT_INTERVAL_FORCE Îª 15/30 Ãë...
 *
 *  ×¢ÊÍÓÚ 2006 Äê 10 ÔÂ 7 ÈÕ, etnlegend ...
 *
*/
#define RT_INTERVAL         15
#define RT_INTERVAL_FORCE   30
    static const struct flock lck_set={.l_type=F_WRLCK,.l_whence=SEEK_SET,.l_start=0,.l_len=0,.l_pid=0};
    static const struct flock lck_clr={.l_type=F_UNLCK,.l_whence=SEEK_SET,.l_start=0,.l_len=0,.l_pid=0};
    struct stat st_dir,st_top;
    const struct fileheader *ptr;
    char top[PATHLEN],dir[PATHLEN];
    int bid,ret,fd,count,i,u_mode,save_currboardent,save_uinfo_currentboard,missing;
#ifdef NEW_HELP
    int save_helpmode;
#endif /* NEW_HELP */
    unsigned int gid;
    ssize_t length,writen;
    time_t read_begin,read_end;
    void *vp;
    const void *data;
    save_currboardent=currboardent;
    save_uinfo_currentboard=uinfo.currentboard;
    bid=publicshm->top[index].bid;
    gid=publicshm->top[index].gid;
    currboardent=bid;
    if(!(currboard=(struct boardheader*)getboard(bid)))
        return -1;
    if(currboard->flag&BOARD_GROUP)
        return -2;
    in_mail=false;
#ifdef HAVE_BRC_CONTROL
    brc_initial(getCurrentUser()->userid,currboard->filename,getSession());
#endif /* HAVE_BRC_CONTROL */
    board_setcurrentuser(uinfo.currentboard,-1);
    uinfo.currentboard=currboardent;
    UPDATE_UTMP(currentboard,uinfo);
    board_setcurrentuser(uinfo.currentboard,1);
#ifdef NEW_HELP
    save_helpmode=helpmode;
    helpmode=HELP_ARTICLE;
#endif /* NEW_HELP */
    snprintf(dir,PATHLEN,"boards/%s/.DIR",currboard->filename);
    if(stat(dir,&st_dir)==-1||!S_ISREG(st_dir.st_mode))
        return -3;
    snprintf(top,PATHLEN,"boards/%s/.TOP.%u",currboard->filename,gid);
    ret=stat(top,&st_top);
    if(!((missing=(ret==-1&&errno==ENOENT))||(!ret&&S_ISREG(st_top.st_mode))))
        return -4;
#define RT_UPDATE (\
    ((st_top.st_mtime+RT_INTERVAL)<st_dir.st_mtime)||\
    ((st_top.st_mtime<st_dir.st_mtime)&&((st_top.st_mtime+RT_INTERVAL_FORCE)<time(NULL)))\
    )
    do{
        if(force||missing||RT_UPDATE){
            if((fd=open(dir,O_RDONLY,0644))==-1)
                return -5;
            vp=mmap(NULL,st_dir.st_size,PROT_READ,MAP_SHARED,fd,0);
            close(fd);
            if(vp==MAP_FAILED)
                return -6;
            if((fd=open(top,
                ((!force&&!missing)?(O_WRONLY|O_CREAT):(O_WRONLY|O_CREAT|O_TRUNC)),
                0644))==-1){
                munmap(vp,st_dir.st_size);
                return -7;
            }
            if(fcntl(fd,F_SETLKW,&lck_set)==-1){
                close(fd);
                munmap(vp,st_dir.st_size);
                return -8;
            }
            if(!force&&!missing){
                if(fstat(fd,&st_top)==-1){
                    fcntl(fd,F_SETLKW,&lck_clr);
                    close(fd);
                    munmap(vp,st_dir.st_size);
                    return -9;
                }
                if(!RT_UPDATE){
                    fcntl(fd,F_SETLKW,&lck_clr);
                    close(fd);
                    munmap(vp,st_dir.st_size);
                    continue;
                }
                if(ftruncate(fd,0)==-1){
                    fcntl(fd,F_SETLKW,&lck_clr);
                    close(fd);
                    munmap(vp,st_dir.st_size);
                    unlink(top);
                    return -10;
                }
            }
            for(ptr=((const struct fileheader*)vp),
                count=(st_dir.st_size/sizeof(struct fileheader)),
                i=0;i<count;i++){
                if(ptr[i].groupid==gid){
                    for(data=&ptr[i],length=sizeof(struct fileheader),writen=0;
                        writen!=-1&&length>0;vpm(data,writen),length-=writen){
                        writen=write(fd,data,length);
                    }
                    if(writen==-1){
                        fcntl(fd,F_SETLKW,&lck_clr);
                        close(fd);
                        munmap(vp,st_dir.st_size);
                        unlink(top);
                        return -11;
                    }
                }
            }
            fcntl(fd,F_SETLKW,&lck_clr);
            close(fd);
            munmap(vp,st_dir.st_size);
        }
    }
    while(0);
#undef RT_UPDATE
    u_mode=uinfo.mode;
    read_begin=time(NULL);
    ret=new_i_read(DIR_MODE_TOP10,top,read_top_title,(READ_ENT_FUNC)read_top_ent,read_top_comms,sizeof(struct fileheader));
    read_end=time(NULL);
    modify_user_mode(u_mode);
#ifdef NEW_HELP
    helpmode=save_helpmode;
#endif /* NEW_HELP */
    newbbslog(BBSLOG_BOARDUSAGE,"%-20s Stay: %5ld",currboard->filename,(read_end-read_begin));
    bmlog(getCurrentUser()->userid,currboard->filename,0,(read_end-read_begin));
    bmlog(getCurrentUser()->userid,currboard->filename,1,1);
    board_setcurrentuser(uinfo.currentboard,-1);
    uinfo.currentboard=save_uinfo_currentboard;
    UPDATE_UTMP(currentboard,uinfo);
    board_setcurrentuser(uinfo.currentboard,1);
    currboardent=save_currboardent;
    currboard=((struct boardheader*)getboard(save_currboardent));
#ifdef HAVE_BRC_CONTROL
    brc_initial(getCurrentUser()->userid,currboard->filename,getSession());
#endif /* HAVE_BRC_CONTROL */
    return ret;
#undef RT_INTERVAL
#undef RT_INTERVAL_FORCE
}

static int select_top(void){
#define ST_UPDATE_TOPINFO()                                                             \
    do{                                                                                 \
        version=publicshm->top_version;                                                 \
        for(total=0;total<10;total++){                                                  \
            if(!(publicshm->top[total].bid)||!(publicshm->top[total].gid)){             \
                break;                                                                  \
            }                                                                           \
        }                                                                               \
        if(!total||(stat("etc/posts/day",&st)==-1||!S_ISREG(st.st_mode))){              \
            move(t_lines-1,0);                                                          \
            clrtoeol();                                                                 \
            prints("\033[1;31m%s\033[0;33m<Enter>\033[m","Ä¿Ç°ÉĞÎŞÊ®´óÈÈÃÅ»°Ìâ!");      \
            WAIT_RETURN;                                                                \
            return -1;                                                                  \
        }                                                                               \
    }while(0)
    struct stat st;
    int total,index,key,valid_key,old_index,update;
    unsigned int version;
    index=0;
    update=1;
    ST_UPDATE_TOPINFO();
    do{
        if(update){
            ansimore("etc/posts/day",0);
            move(t_lines-1,6);
            prints("%s","\033[1;33m¶¨Î»\033[1;37m[\033[1;32mUP\033[1;37m,\033[1;32mDOWN\033[1;37m,\033[1;32mNUM\033[1;37m,"
                "\033[1;32mHOME\033[1;37m,\033[1;32mEND\033[1;37m]/\033[1;33mÍË³ö\033[1;37m[\033[1;32mESC\033[1;37m,"
                "\033[1;32mLEFT\033[1;37m,\033[1;32mQ\033[1;37m]/\033[1;33mÑ¡Ôñ\033[1;37m[\033[1;32mENTER\033[1;37m,"
                "\033[1;32mRIGHT\033[1;37m,\033[1;32mSPACE\033[1;37m]\033[m");
            update=0;
        }
        move((2+2*index),3);
        prints("\033[1;31m%2d\033[m",(index+1));
        move((3+2*index),2);
        prints("\033[1;33m%s\033[m","¡ô");
        do{
            valid_key=1;
            old_index=-1;
            if(version!=publicshm->top_version){
                ST_UPDATE_TOPINFO();
                update=1;
            }
            else{
                switch(toupper(key=igetkey())){
                    case KEY_DOWN:
                        old_index=index++;
                        if(index==total)
                            index=0;
                        break;
                    case KEY_UP:
                        old_index=index--;
                        if(index==-1)
                            index=(total-1);
                        break;
                    case KEY_LEFT:
                    case KEY_ESC:
                    case 'Q':
                        return 0;
                    case KEY_HOME:
                        if(index!=0){
                            old_index=index;
                            index=0;
                        }
                        else
                            valid_key=0;
                        break;
                    case KEY_END:
                        if(index!=(total-1)){
                            old_index=index;
                            index=(total-1);
                        }
                        else
                            valid_key=0;
                        break;
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        key-='1';
                        if(index!=key&&total>key){
                            old_index=index;
                            index=key;
                        }
                        else
                            valid_key=0;
                        break;
                    case '0':
                        if(index!=9&&total==10){
                            old_index=index;
                            index=9;
                        }
                        else
                            valid_key=0;
                        break;
                    case KEY_RIGHT:
                    case '\r':
                    case '\n':
                    case ' ':
                        if(read_top(index,0)<0){
                            move(t_lines-1,6);
                            clrtoeol();
                            prints("\033[1;31m%s\033[0;33m<Enter>\033[m","¼ìË÷Ê®´óĞÅÏ¢Ê±·¢Éú´íÎó!");
                            WAIT_RETURN;
                        }
                        update=1;
                        break;
                    default:
                        valid_key=0;
                        break;
                }
            }
        }
        while(!valid_key);
        if(old_index!=-1){
            move((2+2*old_index),3);
            prints("\033[31m%2d\033[m",(old_index+1));
            move((3+2*old_index),2);
            prints("%s","  ");
        }
    }
    while(1);
}

#endif /* USE_PRIMORDIAL_TOP10 */

/* END -- etnlegend, 2006.05.30, ÔÄ¶ÁÊ®´ó ... */

