/*
 * $Id$
 */
#include "bbslib.h"

char genbuf[1024];

int show_user_plan(userid)
    char userid[IDLEN];
{
    char pfile[STRLEN];
	int fd;

    sethomefile(pfile, userid, "plans");
    if ((fd = open(pfile, O_RDONLY, 0644)) < 0) {
        hprintf("[36m√ª”–∏ˆ»ÀÀµ√˜µµ[m\n");
    	printf("</pre>\n");
        return 0;
    } else {
		size_t filesize;
		char *ptr;
		const int outbuf_len = 4096;
		buffered_output_t *out;

        hprintf("[36m∏ˆ»ÀÀµ√˜µµ»Áœ¬£∫[m\n");
    	printf("</pre>\n");
		if (flock(fd, LOCK_EX) == -1){
			close(fd);
			return 0;
		}
		BBS_TRY
		{
			if (safe_mmapfile_handle(fd, O_RDONLY, PROT_READ, MAP_SHARED,
						(void **)&ptr, &filesize) == 0)
			{
				flock(fd, LOCK_UN);
				close(fd);
				BBS_RETURN(0);
			}
			alloc_output(outbuf_len);
			output_ansi_html(ptr, filesize, &out,NULL);
			free_output(out);
		}
		BBS_CATCH
		{
		}
		BBS_END end_mmapfile((void *)ptr, filesize, -1);
		flock(fd, LOCK_UN);
		close(fd);
        return 1;
    }
}

int t_printstatus(struct user_info *uentp, int *arg, int pos)
{
    if (uentp->invisible == 1) {
        if (!HAS_PERM(currentuser, PERM_SEECLOAK))
            return COUNT;
    }
    (*arg)++;
    if (*arg == 1)
        strcpy(genbuf, "ƒø«∞‘⁄’æ…œ£¨◊¥Ã¨»Áœ¬£∫\n");
    if (uentp->invisible)
        strcat(genbuf, "[32m“˛…Ì÷–   [m");
    else {
        char buf[80];

        sprintf(buf, "[1m%s[m ", modestring(uentp->mode, uentp->destuid, 0,   /* 1->0 ≤ªœ‘ æ¡ƒÃÏ∂‘œÛµ» modified by dong 1996.10.26 */
                                              (uentp->in_chat ? uentp->chatid : NULL)));
        strcat(genbuf, buf);
    }
    if ((*arg) % 8 == 0)
        strcat(genbuf, "\n");
    UNUSED_ARG(pos);
    return COUNT;
}

void display_user(char *userid)
{
    char uident[STRLEN], *newline;
    int tuid = 0;
    int exp, perf;
    struct user_info uin;
    char qry_mail_dir[STRLEN];
    char planid[IDLEN + 2];
    char permstr[USER_TITLE_LEN];
    char exittime[40];
    time_t exit_time, temp;
    int logincount, seecount;
    struct userec *lookupuser;
    uinfo_t *ui;
    uinfo_t guestui;

    printf("</center><pre>\n");
    strcpy(uident, strtok(userid, " "));
    if (!(tuid = getuser(uident, &lookupuser))) {
        printf("”√ªß [%s] ≤ª¥Ê‘⁄.", userid);
        http_quit();
    }
    ui = getcurruinfo();
    if (ui == NULL) {
        ui = &guestui;
        ui->in_chat = 0;
    }
    ui->destuid = tuid;

    setmailfile(qry_mail_dir, lookupuser->userid, DOT_DIR);

    exp = countexp(lookupuser);
    perf = countperf(lookupuser);
    /*---	modified by period	2000-11-02	hide posts/logins	---*/
    hprintf("%s (%s) π≤…œ’æ %d ¥Œ£¨∑¢±Ìπ˝ %d ∆™Œƒ’¬", lookupuser->userid, lookupuser->username, lookupuser->numlogins, lookupuser->numposts);
    strcpy(planid, lookupuser->userid);
    if ((newline = strchr(genbuf, '\n')) != NULL)
        *newline = '\0';
    seecount = 0;
    logincount = apply_utmp(t_printstatus, 10, lookupuser->userid, &seecount);
    /* ªÒµ√¿Îœﬂ ±º‰ Luzi 1998/10/23 */
    exit_time = get_exit_time(lookupuser->userid, exittime);
    if ((newline = strchr(exittime, '\n')) != NULL)
        *newline = '\0';

    if (exit_time <= lookupuser->lastlogin) {
        if (logincount != seecount) {
            temp = lookupuser->lastlogin + ((lookupuser->numlogins + lookupuser->numposts) % 100) + 60;
            strcpy(exittime, ctime(&temp));     /*Haohmaru.98.12.04.»√“˛…Ì”√ªßø¥…œ»•¿Îœﬂ ±º‰±»…œœﬂ ±º‰ÕÌ60µΩ160√Î÷” */
            if ((newline = strchr(exittime, '\n')) != NULL)
                *newline = '\0';
        } else
            strcpy(exittime, "“Ú‘⁄œﬂ…œªÚ∑«≥£∂œœﬂ≤ªœÍ");
    }
    hprintf("\n…œ¥Œ‘⁄  [%s] ¥” [%s] µΩ±æ’æ“ª”Œ°£\n¿Îœﬂ ±º‰[%s] ", wwwCTime(lookupuser->lastlogin), ((lookupuser->lasthost[0] == '\0') ? "(≤ªœÍ)" : lookupuser->lasthost), exittime);
    uleveltochar(&permstr, lookupuser);
    hprintf("–≈œ‰£∫[[5m%2s[m] …˙√¸¡¶£∫[%d] …Ì∑›: [%s]%s\n",
            (check_query_mail(qry_mail_dir) == 1) ? "–≈" : "  ", compute_user_value(lookupuser), permstr, (lookupuser->userlevel & PERM_SUICIDE) ? " (◊‘…±÷–)" : "°£");

    if ((genbuf[0]) && seecount) {
        hprintf("%s", genbuf);
        printf("\n");
    }
    show_user_plan(planid);
    printf("<br><br><a href=\"/bbspstmail.php?userid=%s&title=√ª÷˜Ã‚\">[–¥–≈Œ ∫Ú]</a> ", lookupuser->userid);
    printf("<a href=\"/bbssendmsg.php?destid=%s\">[∑¢ÀÕ—∂œ¢]</a> ", lookupuser->userid);
    printf("<a href=\"bbsfadd?userid=%s\">[º”»Î∫√”—]</a> ", lookupuser->userid);
    printf("<a href=\"bbsfdel?userid=%s\">[…æ≥˝∫√”—]</a>", lookupuser->userid);
    printf("<hr>");
    printf("</center>\n");

    ui->destuid = 0;
}

int main()
{
    char userid[14];

    init_all();
    strsncpy(userid, getparm("userid"), 13);
    printf("<center>");
    printf("%s -- ≤È—ØÕ¯”—<hr color=green>\n", BBSNAME);
    if (userid[0] == 0) {
        printf("<form action=bbsqry>\n");
        printf("«Î ‰»Î”√ªß√˚: <input name=userid maxlength=12 size=12>\n");
        printf("<input type=submit value=≤È—Ø”√ªß>\n");
        printf("</form><hr>\n");
        http_quit();
    }
    display_user(userid);
    http_quit();
}
