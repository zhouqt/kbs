/*
 * $Id$
 */
#include "bbslib.h"

char genbuf[1024];

static void flush_buffer(buffered_output_t *output)
{
	*(output->outp) = '\0'; 
	printf("%s", output->buf);
	output->outp = output->buf;
}

static int buffered_output(char *buf, size_t buflen, void *arg)
{
	buffered_output_t *output = (buffered_output_t *)arg;
	if (output->buflen <= buflen)
	{
		output->flush(output);
		printf("%s", buf);
		return 0;
	}
	if ((output->buflen - (output->outp - output->buf) - 1) <= buflen) 
		output->flush(output);
	strncpy(output->outp, buf, buflen); 
	output->outp += buflen;

	return 0;
}

int show_user_plan(userid)
    char userid[IDLEN];
{
    char pfile[STRLEN];
	int fd;

    sethomefile(pfile, userid, "plans");
    if ((fd = open(pfile, O_RDONLY, 0644)) < 0) {
        hprintf("[36mÃ»ÓĞ¸öÈËËµÃ÷µµ[m\n");
    	printf("</pre>\n");
        return 0;
    } else {
		size_t filesize;
		char *ptr;
		const int outbuf_len = 4096;
		buffered_output_t out;

        hprintf("[36m¸öÈËËµÃ÷µµÈçÏÂ£º[m\n");
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
			if ((out.buf = (char *)malloc(outbuf_len)) == NULL)
			{
				end_mmapfile((void *)ptr, filesize, -1);
				flock(fd, LOCK_UN);
				close(fd);
				BBS_RETURN(0);
			}
			out.outp = out.buf;
			out.buflen = outbuf_len;
			out.output = buffered_output;
			out.flush = flush_buffer;
			output_ansi_html(ptr, filesize, &out,NULL);
			free(out.buf);
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
        strcpy(genbuf, "Ä¿Ç°ÔÚÕ¾ÉÏ£¬×´Ì¬ÈçÏÂ£º\n");
    if (uentp->invisible)
        strcat(genbuf, "[32mÒşÉíÖĞ   [m");
    else {
        char buf[80];

        sprintf(buf, "[1m%s[m ", modestring(uentp->mode, uentp->destuid, 0,   /* 1->0 ²»ÏÔÊ¾ÁÄÌì¶ÔÏóµÈ modified by dong 1996.10.26 */
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
    char permstr[10];
    char exittime[40];
    time_t exit_time, temp;
    int logincount, seecount;
    struct userec *lookupuser;
    uinfo_t *ui;
    uinfo_t guestui;

    printf("</center><pre>\n");
    strcpy(uident, strtok(userid, " "));
    if (!(tuid = getuser(uident, &lookupuser))) {
        printf("ÓÃ»§ [%s] ²»´æÔÚ.", userid);
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
    hprintf("%s (%s) ¹²ÉÏÕ¾ %d ´Î£¬·¢±í¹ı %d ÆªÎÄÕÂ", lookupuser->userid, lookupuser->username, lookupuser->numlogins, lookupuser->numposts);
    strcpy(planid, lookupuser->userid);
    if ((newline = strchr(genbuf, '\n')) != NULL)
        *newline = '\0';
    seecount = 0;
    logincount = apply_utmp(t_printstatus, 10, lookupuser->userid, &seecount);
    /* »ñµÃÀëÏßÊ±¼ä Luzi 1998/10/23 */
    exit_time = get_exit_time(lookupuser->userid, exittime);
    if ((newline = strchr(exittime, '\n')) != NULL)
        *newline = '\0';

    if (exit_time <= lookupuser->lastlogin) {
        if (logincount != seecount) {
            temp = lookupuser->lastlogin + ((lookupuser->numlogins + lookupuser->numposts) % 100) + 60;
            strcpy(exittime, ctime(&temp));     /*Haohmaru.98.12.04.ÈÃÒşÉíÓÃ»§¿´ÉÏÈ¥ÀëÏßÊ±¼ä±ÈÉÏÏßÊ±¼äÍí60µ½160ÃëÖÓ */
            if ((newline = strchr(exittime, '\n')) != NULL)
                *newline = '\0';
        } else
            strcpy(exittime, "ÒòÔÚÏßÉÏ»ò·Ç³£¶ÏÏß²»Ïê");
    }
    hprintf("\nÉÏ´ÎÔÚ  [%s] ´Ó [%s] µ½±¾Õ¾Ò»ÓÎ¡£\nÀëÏßÊ±¼ä[%s] ", wwwCTime(lookupuser->lastlogin), ((lookupuser->lasthost[0] == '\0') ? "(²»Ïê)" : lookupuser->lasthost), exittime);
    uleveltochar(&permstr, lookupuser);
    hprintf("ĞÅÏä£º[[5m%2s[m] ÉúÃüÁ¦£º[%d] Éí·İ: [%s]%s\n",
            (check_query_mail(qry_mail_dir) == 1) ? "ĞÅ" : "  ", compute_user_value(lookupuser), permstr, (lookupuser->userlevel & PERM_SUICIDE) ? " (×ÔÉ±ÖĞ)" : "¡£");

    if ((genbuf[0]) && seecount) {
        hprintf("%s", genbuf);
        printf("\n");
    }
    show_user_plan(planid);
    printf("<br><br><a href=\"/bbspstmail.php?userid=%s&title=Ã»Ö÷Ìâ\">[Ğ´ĞÅÎÊºò]</a> ", lookupuser->userid);
    printf("<a href=\"/bbssendmsg.php?destid=%s\">[·¢ËÍÑ¶Ï¢]</a> ", lookupuser->userid);
    printf("<a href=\"bbsfadd?userid=%s\">[¼ÓÈëºÃÓÑ]</a> ", lookupuser->userid);
    printf("<a href=\"bbsfdel?userid=%s\">[É¾³ıºÃÓÑ]</a>", lookupuser->userid);
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
    printf("%s -- ²éÑ¯ÍøÓÑ<hr color=green>\n", BBSNAME);
    if (userid[0] == 0) {
        printf("<form action=bbsqry>\n");
        printf("ÇëÊäÈëÓÃ»§Ãû: <input name=userid maxlength=12 size=12>\n");
        printf("<input type=submit value=²éÑ¯ÓÃ»§>\n");
        printf("</form><hr>\n");
        http_quit();
    }
    display_user(userid);
    http_quit();
}
