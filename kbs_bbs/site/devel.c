#include "bbs.h"

//×Ô¶¯Í¨¹ı×¢²áµÄº¯Êı  binxun
int auto_register(char *userid, char *email, int msize)
{
    struct userdata ud;
    struct userec *uc;
    char *item, *temp;
    char fdata[7][STRLEN];
    char genbuf[STRLEN];
    char buf[STRLEN];
    char fname[STRLEN];
    int unum;
    FILE *fout;
    int n;
    struct userec deliveruser;
    static const char *finfo[] = { "ÕÊºÅÎ»ÖÃ", "ÉêÇë´úºÅ", "ÕæÊµĞÕÃû", "·şÎñµ¥Î»",
        "Ä¿Ç°×¡Ö·", "Á¬Âçµç»°", "Éú    ÈÕ", NULL
    };
    static const char *field[] = { "usernum", "userid", "realname", "career",
        "addr", "phone", "birth", NULL
    };

    bzero(&deliveruser, sizeof(struct userec));
    strcpy(deliveruser.userid, "deliver");
    deliveruser.userlevel = -1;
    strcpy(deliveruser.username, "×Ô¶¯·¢ĞÅÏµÍ³");



    bzero(fdata, 7 * STRLEN);

    if ((unum = getuser(userid, &uc)) == 0)
        return -1;              //faild
    if (read_userdata(userid, &ud) < 0)
        return -1;

    strncpy(genbuf, email, STRLEN - 16);
    item = strtok(genbuf, "#");
    if (item)
        strncpy(ud.realname, item, NAMELEN);
    item = strtok(NULL, "#");   //Ñ§ºÅ
    item = strtok(NULL, "#");
    if (item)
        strncpy(ud.address, item, STRLEN);

    email[strlen(email) - 3] = '@';
    strncpy(ud.realemail, email, STRLEN - 16);  //email length must be less STRLEN-16


    sprintf(fdata[0], "%d", unum);
    strncpy(fdata[2], ud.realname, NAMELEN);
    strncpy(fdata[4], ud.address, STRLEN);
    strncpy(fdata[5], ud.email, STRLEN);
    strncpy(fdata[1], userid, IDLEN);

    sprintf(buf, "tmp/email/%s", userid);
    if ((fout = fopen(buf, "w")) != NULL) {
        fprintf(fout, "%s\n", email);
        fclose(fout);
    }

    if (write_userdata(userid, &ud) < 0)
        return -1;
    mail_file("deliver", "etc/s_fill", userid, "¹§Ï²Äã,ÄãÒÑ¾­Íê³É×¢²á.", 0, 0);
    //sprintf(genbuf,"deliver ÈÃ %s ×Ô¶¯Í¨¹ıÉí·İÈ·ÈÏ.",uinfo.userid);

    sprintf(fname, "tmp/security.%d", getpid());
    if ((fout = fopen(fname, "w")) != NULL) {
        fprintf(fout, "ÏµÍ³°²È«¼ÇÂ¼ÏµÍ³\n[32mÔ­Òò£º%s×Ô¶¯Í¨¹ı×¢²á[m\n", userid);
        fprintf(fout, "ÒÔÏÂÊÇÍ¨¹ıÕß¸öÈË×ÊÁÏ");
        fprintf(fout, "\n\nÄúµÄ´úºÅ     : %s\n", ud.userid);
        fprintf(fout, "ÄúµÄêÇ³Æ     : %s\n", uc->username);
        fprintf(fout, "ÕæÊµĞÕÃû     : %s\n", ud.realname);
        fprintf(fout, "µç×ÓÓÊ¼şĞÅÏä : %s\n", ud.email);
        fprintf(fout, "ÕæÊµ E-mail  : %s\n", ud.realemail);
        fprintf(fout, "·şÎñµ¥Î»     : %s\n", "");
        fprintf(fout, "Ä¿Ç°×¡Ö·     : %s\n", ud.address);
        fprintf(fout, "Á¬Âçµç»°     : %s\n", "");
        fprintf(fout, "×¢²áÈÕÆÚ     : %s", ctime(&uc->firstlogin));
        fprintf(fout, "×î½ü¹âÁÙÈÕÆÚ : %s", ctime(&uc->lastlogin));
        fprintf(fout, "×î½ü¹âÁÙ»úÆ÷ : %s\n", uc->lasthost);
        fprintf(fout, "ÉÏÕ¾´ÎÊı     : %d ´Î\n", uc->numlogins);
        fprintf(fout, "ÎÄÕÂÊıÄ¿     : %d(Board)\n", uc->numposts);
        fprintf(fout, "Éú    ÈÕ     : %s\n", "");

        fclose(fout);
        //post_file(currentuser, "", fname, "Registry", str, 0, 2);

        sprintf(genbuf, "%s ×Ô¶¯Í¨¹ı×¢²á", ud.userid);
        post_file(&deliveruser, "", fname, "Registry", genbuf, 0, 1);
        /*
         * if (( fout = fopen(logfile,"a")) != NULL)
         * {
         * fclose(fout);
         * }
         */
    }

    sethomefile(buf, userid, "/register");
    if ((fout = fopen(buf, "w")) != NULL) {
        for (n = 0; field[n] != NULL; n++)
            fprintf(fout, "%s     : %s\n", finfo[n], fdata[n]);
        fprintf(fout, "ÄúµÄêÇ³Æ     : %s\n", uc->username);
        fprintf(fout, "µç×ÓÓÊ¼şĞÅÏä : %s\n", ud.email);
        fprintf(fout, "ÕæÊµ E-mail  : %s\n", ud.realemail);
        fprintf(fout, "×¢²áÈÕÆÚ     : %s\n", ctime(&uc->firstlogin));
        fprintf(fout, "×¢²áÊ±µÄ»úÆ÷ : %s\n", uc->lasthost);
        fprintf(fout, "Approved: %s\n", userid);
        fclose(fout);
    }

    return 0;
}
