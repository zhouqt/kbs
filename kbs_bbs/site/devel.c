#include "bbs.h"

//自动通过注册的函数  binxun
int auto_register(char *userid, char *email, int msize, session_t* session)
{
    struct userdata ud;
    struct userec *uc;
    char *item;
    char fdata[7][STRLEN];
    char genbuf[STRLEN];
    char buf[STRLEN];
    char fname[STRLEN];
    int unum;
    FILE *fout;
    int n;
    struct userec deliveruser;
    static const char * const finfo[] = { "帐号位置", "申请代号", "真实姓名", "服务单位",
        "目前住址", "连络电话", "生    日", NULL
    };
    static const char *const field[] = { "usernum", "userid", "realname", "career",
        "addr", "phone", "birth", NULL
    };

    bzero(&deliveruser, sizeof(struct userec));
    strcpy(deliveruser.userid, "deliver");
    deliveruser.userlevel = -1;
    strcpy(deliveruser.username, "自动发信系统");



    bzero(fdata, 7 * STRLEN);

    if ((unum = getuser(userid, &uc)) == 0)
        return -1;              //faild
    if (read_userdata(userid, &ud) < 0)
        return -1;

    strncpy(genbuf, email, STRLEN - 16);
    item = strtok(genbuf, "#");
    if (item)
        strncpy(ud.realname, item, NAMELEN);
    item = strtok(NULL, "#");   //学号
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
    mail_file("deliver", "etc/s_fill", userid, "恭喜你,你已经完成注册.", 0, 0);
    //sprintf(genbuf,"deliver 让 %s 自动通过身份确认.",uinfo.userid);

    sprintf(fname, "tmp/security.%d", getpid());
    if ((fout = fopen(fname, "w")) != NULL) {
        fprintf(fout, "系统安全记录系统\n\033[32m原因：%s自动通过注册\033[m\n", userid);
        fprintf(fout, "以下是通过者个人资料");
        fprintf(fout, "\n\n您的代号     : %s\n", ud.userid);
        fprintf(fout, "您的昵称     : %s\n", uc->username);
        fprintf(fout, "真实姓名     : %s\n", ud.realname);
        fprintf(fout, "电子邮件信箱 : %s\n", ud.email);
        fprintf(fout, "真实 E-mail  : %s\n", ud.realemail);
        fprintf(fout, "服务单位     : %s\n", "");
        fprintf(fout, "目前住址     : %s\n", ud.address);
        fprintf(fout, "连络电话     : %s\n", "");
        fprintf(fout, "注册日期     : %s", ctime(&uc->firstlogin));
        fprintf(fout, "最近光临日期 : %s", ctime(&uc->lastlogin));
        fprintf(fout, "最近光临机器 : %s\n", uc->lasthost);
        fprintf(fout, "上站次数     : %d 次\n", uc->numlogins);
        fprintf(fout, "文章数目     : %d(Board)\n", uc->numposts);
        fprintf(fout, "生    日     : %s\n", "");

        fclose(fout);
        //post_file(currentuser, "", fname, "Registry", str, 0, 2);

        sprintf(genbuf, "%s 自动通过注册", ud.userid);
        post_file(&deliveruser, "", fname, "Registry", genbuf, 0, 1, session);
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
        fprintf(fout, "您的昵称     : %s\n", uc->username);
        fprintf(fout, "电子邮件信箱 : %s\n", ud.email);
        fprintf(fout, "真实 E-mail  : %s\n", ud.realemail);
        fprintf(fout, "注册日期     : %s\n", ctime(&uc->firstlogin));
        fprintf(fout, "注册时的机器 : %s\n", uc->lasthost);
        fprintf(fout, "Approved: %s\n", userid);
        fclose(fout);
    }

    return 0;
}
