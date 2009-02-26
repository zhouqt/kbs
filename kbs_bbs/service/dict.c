#include "service.h"
#include "bbs.h"
#if defined(BBS_SERVICE_DICT) && HAVE_MYSQL_SMTH == 1
#include <mysql.h>

SMTH_API struct user_info uinfo;
SMTH_API int t_lines;

#define WORDLEN 100
char save_scr[LINEHEIGHT][LINELEN*3];
int save_y, save_x;

void print_res(char * r)
{
    char * p;
    int insound=0;
    for (p=r;*p!=0;p++) {
        if (insound) {
            if (*p==25) insound=0;
//  else prints("%c",*p);
        } else
            switch (*p) {
                case 1: prints("n.");break;
                case 2: prints("v.");break;
                case 3: prints("a.");break;
                case 4: prints("ad.");break;
                case 5: prints("vi.");break;
                case 6: prints("vt.");break;
                case 8: prints("vbl.");break;
                case 9: prints("int.");break;
                case 10: prints("art.");break;
                case 11: prints("aux.");break;
                case 12: prints("num.");break;
                case 13: prints("conj.");break;
                case 14: prints("prep.");break;
                case 15: prints("pron.");break;
                case 16: prints("st.");break;
                case 17: prints("adj.");break;
                case 18: prints("adv.");break;
                case 19: prints("pr.");break;
                case 20: prints("abbr.");break;
                case 21: break;
                case 22: break;
                case 25: insound=1; break;
                case 26: break;
                case 30: break;
                case 31: p++; p++; break;
                case 59: prints("\n"); break;
                default: prints("%c",*p);
            }
    }
}

void prinf_res(FILE* fp, char * r)
{
    char * p;
    int insound=0;
    for (p=r;*p!=0;p++) {
        if (insound) {
            if (*p==25) insound=0;
        } else
            switch (*p) {
                case 1: fprintf(fp, "n.");break;
                case 2: fprintf(fp, "v.");break;
                case 3: fprintf(fp, "a.");break;
                case 4: fprintf(fp, "ad.");break;
                case 5: fprintf(fp, "vi.");break;
                case 6: fprintf(fp, "vt.");break;
                case 8: fprintf(fp, "vbl.");break;
                case 9: fprintf(fp, "int.");break;
                case 10: fprintf(fp, "art.");break;
                case 11: fprintf(fp, "aux.");break;
                case 12: fprintf(fp, "num.");break;
                case 13: fprintf(fp, "conj.");break;
                case 14: fprintf(fp, "prep.");break;
                case 15: fprintf(fp, "pron.");break;
                case 16: fprintf(fp, "st.");break;
                case 17: fprintf(fp, "adj.");break;
                case 18: fprintf(fp, "adv.");break;
                case 19: fprintf(fp, "pr.");break;
                case 20: fprintf(fp, "abbr.");break;
                case 21: break;
                case 22: break;
                case 25: insound=1; break;
                case 26: break;
                case 30: break;
                case 31: p++; p++; break;
                case 59: fprintf(fp, "\n"); break;
                default: fprintf(fp, "%c",*p);
            }
    }
}

int dict_main()
{
    MYSQL s;
    MYSQL_RES *res;
    MYSQL_ROW row;
    int oldmode;
    int i;
    char sql[600], word[WORDLEN];
    FILE* fp;
    char fn[80];

    noscroll();
    oldmode = uinfo.mode;
    modify_user_mode(DICT);
    getyx(&save_y, &save_x);
    for (i=0;i<t_lines;i++)
        saveline(i, 0, save_scr[i]);
    clear();
    mysql_init(&s);
    if (!mysql_real_connect(&s,
                            sysconf_str("MYSQLHOST"),
                            sysconf_str("MYSQLUSER"),
                            sysconf_str("MYSQLPASSWORD"),
                            sysconf_str("MYSQLDATABASE"),
                            sysconf_eval("MYSQLPORT",1521), sysconf_str("MYSQLSOCKET"), 0)) {
        prints("%s\n", mysql_error(&s));
        pressanykey();
        goto exit_dict;
    }
    while (1) {
        char table[8];
        int len=0;
        move(2,0);
        word[0]=0;
        while (1) {
            int ch;
            move(2,0);
            prints("请输入单词:  %s", word);
            clrtoeol();
            ch = igetkey();
            clear();
            if (ch==13||ch==10) break;
            else if (((ch==KEY_DEL)||(ch==8)||(ch=='\177')||(ch==Ctrl('H')))&&len) {
                len--; word[len]=0;
#ifdef CHINESE_CHARACTER
                if (DEFINE(getCurrentUser(), DEF_CHCHAR)) {
                    if (word[len-1] < 0) {
                        len--;
                        word[len] = '\0';
                    }
                }
#endif /* CHINESE_CHARACTER */
            } else if (ch==KEY_TAB&&len) {
            } else if (isprint2(ch)&&!strchr("';\\\"/",ch)) {
                if (len==WORDLEN-1) continue;
                word[len]=ch;
                len++;
                word[len]=0;
            }

            if (len>=4) {
                int i=0;
                if (word[0]&0x80)
                    strcpy(table, "cedict");
                else
                    strcpy(table, "ecdict");
                sprintf(sql, "SELECT * FROM %s WHERE word like '%s%%' and dictid=0", table, word);
                if (mysql_real_query(&s, sql, strlen(sql))) {
                    prints("%s\n", mysql_error(&s));
                    pressanykey();
                    goto exit_dict;
                }
                res = mysql_store_result(&s);
                row = mysql_fetch_row(res);
                while (row!=NULL) {
                    move(3+i, 0);
                    prints("%s ", row[1]);
                    print_res(row[2]);
                    i++;
                    if (i>=20) break;
                    row = mysql_fetch_row(res);
                }
                mysql_free_result(res);
            }
        }
        if (word[0]==0) break;
        if (strchr(word,'\'')) break;
        if (word[0]&0x80)
            strcpy(table, "cedict");
        else
            strcpy(table, "ecdict");
#ifdef SMTH
        if (!HAS_PERM(getCurrentUser(),PERM_SYSOP))
            sprintf(sql, "SELECT * FROM %s WHERE word='%s' and dictid=0", table, word);
        else
#endif
            sprintf(sql, "SELECT * FROM %s WHERE word='%s'", table, word);
        if (mysql_real_query(&s, sql, strlen(sql))) {
            prints("%s\n", mysql_error(&s));
            pressanykey();
            goto exit_dict;
        }
        res = mysql_store_result(&s);
        row = mysql_fetch_row(res);
        if (row==NULL) {
            clear();
            move(3, 0);
            prints("没有这个词！");
        } else {
            char title[80];
            sprintf(title, "单词%s的含义", word);
            sprintf(fn, "tmp/%s.%d.dict", getCurrentUser()->userid, rand());
            fp = fopen(fn, "w");
            fprintf(fp, "\x1b[1;32;4m%s\x1b[m\n", row[1]);
            strcpy(word,row[1]);
            while (row != NULL) {
                if (strcmp(word,row[1]))
                    fprintf(fp, "\x1b[1;32;4m%s\x1b[m\n", row[1]);
                prinf_res(fp, row[2]);
                fprintf(fp, "\n");
                row = mysql_fetch_row(res);
            }
            fclose(fp);
            ansimore_withzmodem(fn, true, title);
            clear();
            unlink(fn);
        }
        mysql_free_result(res);
        //i = igetkey();
    }
exit_dict:
    mysql_close(&s);
    modify_user_mode(oldmode);
    for (i=0;i<t_lines;i++)
        saveline(i, 1, save_scr[i]);
    move(save_y, save_x);
    return 0;
}
#else
int dict_main()
{
    return 0;
}
#endif
