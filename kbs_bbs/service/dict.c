#define BBSMAIN
#include "bbs.h"
#include "/usr/include/mysql/mysql.h"
int dict_main()
{
    MYSQL s;
    MYSQL_RES *res;
    MYSQL_ROW row;
    int i;
    char sql[600], word[300];

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
        return;
    }
    while (1) {
    char* table;
    getdata(2, 0, "请输入单词:  ", word, 70, true, NULL, true);
    clear();
    move(3,0);
    if (word[0]==0) break;
    if (word[0]&0x80)
        table="cedict";
    else
        table="ecdict";
#ifdef SMTH
    if (!HAS_PERM(currentuser,PERM_SYSOP))
        sprintf(sql, "SELECT * FROM %s WHERE word='%s' and dictid=0", table, word);
    else
#endif
    sprintf(sql, "SELECT * FROM %s WHERE word='%s'", table, word);
    if (mysql_real_query(&s, sql, strlen(sql))) {
        prints("%s\n", mysql_error(&s));
        pressanykey();
        return;
    }
    res = mysql_store_result(&s);
    row = mysql_fetch_row(res);
    if (row==NULL)
        prints("没有这个词！");
    else{
        prints("%s\n", row[1]);
        prints("----------------------------\n");
    while (row != NULL) {
	char* p;
	int insound=0;
	for (p=row[2];*p!=0;p++) {
            if (insound) {
                if (*p==25) insound=0;
//		else prints("%c",*p);
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
            case 31: p++; if (*p!=0) p++; break;
            case 59: prints("\n"); break;
            default: prints("%c",*p);
            }
	}
	prints("\n");
        row = mysql_fetch_row(res);
    }
    }
    //i = igetkey();
    }
    mysql_close(&s);
}
