#define BBSMAIN
#include "bbs.h"
#include "/usr/include/mysql/mysql.h"

void main()
{
    MYSQL s;
    MYSQL_RES* res;
    MYSQL_ROW row;
    int i;
    char sql[600], word[300];
    clear();
    prints("Hello");
    refresh();
    i=igetkey();
    mysql_init(&s);
    if(!mysql_real_connect(&s, "166.111.8.235", "smth", "3s4m5t9h", "ciba2k", 9527, 0, 0))
    {
        prints("%s\n", mysql_error(&s));
        return;
    }
    getdata(2, 0, "«Î ‰»Îµ•¥ :  ", word, 70, true, NULL, true);
    sprintf(sql, "SELECT * FROM ecdict WHERE word='%s'", word);
    if(mysql_real_query(&s, sql, strlen(sql))) {
        prints("%s\n", mysql_error(&s));
        return;
    }
    res = mysql_store_result(&s);
    row = mysql_fetch_row(res);
    while(row!=NULL) {
        prints("%s\n", row[2]);
        row = mysql_fetch_row(res);
    }
    i = igetkey();

    mysql_close(&s);
}
