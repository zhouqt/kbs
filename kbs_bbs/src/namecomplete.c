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
#include "bbs.h"
struct word *toplev = NULL, *current = NULL;
struct word* GetNameListHead(void){
    return toplev;
}
int GetNameListCount(void){
    struct word *p;
    int ret;
    for(ret=0,p=toplev;p;p=p->next)
        ret++;
    return ret;
}
static void FreeNameList(void){
    /*
     * 加上 static, 显式释放 NameList 应该使用 CreateNameList 函数, 
     * 不应该在本文件外有对 FreeNameList 的调用... 
    */
    struct word *p,*temp;
    for(p=toplev;p;p=temp){
        temp=p->next;
        free(p->word);
        free(p);
    }
}
void CreateNameList(void){
    if(toplev)
        FreeNameList();
    toplev=NULL;
    current=NULL;
}
void AddNameList(const char *name){
    struct word *node;
    if(!name)
        return;
    if(!(node=(struct word*)malloc(sizeof(struct word))))
        return;
    if(!(node->word=strdup(name))){
        free(node);
        return;
    }
    node->next=NULL;
    if(!current)
        toplev=node;
    else{
        while(current->next)
            current=current->next;
        current->next=node;
    }
    current=node;
    return;
}
static int CompareName(const void *v1,const void *v2){
    return strcmp((*((const char**)v1)),(*((const char**)v2)));
}
static int CompareNameCase(const void *v1,const void *v2){
    return strcasecmp((*((const char**)v1)),(*((const char**)v2)));
}
void SortNameList(int case_sensitive){
    struct word *p;
    const char **array,**t;
    int count;
    if(!(count=GetNameListCount()))
        return;
    if(!(array=(const char**)malloc(count*sizeof(const char*))))
        return;
    for(p=toplev,t=array;p;p=p->next,t++)
        (*t)=p->word;
    qsort(array,count,sizeof(const char*),(case_sensitive?CompareName:CompareNameCase));
    for(p=toplev,t=array;p;p=p->next,t++)
        p->word=(char*)(*t);
    free(array);
    return;
}

int NumInList(list)
register struct word *list;
{
    register int i;

    for (i = 0; list != NULL; i++, list = list->next)
        /*
         * Null Statement 
         */ ;
    return i;
}

/* etnlegend, 2005.12.26, 遍历 NameList 时可传递附加参数 */
void ApplyToNameList(int (*fptr)(char*,void*),void *arg){
    struct word *p;
    if(fptr){
        for(p=toplev;p;p=p->next)
            (*fptr)(p->word,arg);
    }
    return;
}

int chkstr(otag, tag, name)
char *otag, *tag, *name;
{
    char ch, *oname = name;

    while (*tag != '\0') {
        ch = *name++;
        if (*tag != chartoupper(ch))
            return 0;
        tag++;
    }
    if (*tag != '\0' && *name == '\0')
        strcpy(otag, oname);
    return 1;
}
struct word *GetSubList(register char *tag, register struct word *list)
{
    struct word *wlist, *wcurr;
    char tagbuf[STRLEN];
    int n;

    wlist = NULL;
    wcurr = NULL;
    for (n = 0; tag[n] != '\0'; n++) {
        tagbuf[n] = chartoupper(tag[n]);
    }
    tagbuf[n] = '\0';
    while (list != NULL) {
        if (chkstr(tag, tagbuf, list->word)) {
            register struct word *node;
            node = (struct word *) malloc(sizeof(struct word));
            node->word = list->word;
            node->next = NULL;
            if (wlist)
                wcurr->next = node;
            else
                wlist = node;
            wcurr = node;
        }
        list = list->next;
    }
    return wlist;
}

void ClearSubList(list)
struct word *list;
{
    struct word *tmp_list;

    while (list) {
        tmp_list = list->next;
        free(list);
        list = tmp_list;
    }
}
int MaxLen(list, count)
struct word *list;
int count;
{
    int len = strlen(list->word);

    while (list != NULL && count) {
        int t = strlen(list->word);

        if (t > len)
            len = t;
        list = list->next;
        count--;
    }
    return len;
}

#define NUMLINES (t_lines - 4)
bool in_do_sendmsg=0;

int namecomplete(prompt, data)
char *prompt, *data;
{
    char *temp;
    int ch;
    int count = 0;
    int clearbot = false;
    extern int ingetdata;

    ingetdata = true;
    if (scrint) {
        struct word *cwlist, *morelist;
        int x, y;
        int origx, origy;

        if (prompt != NULL) {
            prints("%s", prompt);
            clrtoeol();
        }
        temp = data;
        if (toplev == NULL)
            AddNameList("");
        cwlist = GetSubList("", toplev);
        morelist = NULL;
        getyx(&y, &x);
        getyx(&origy, &origx);
        while ((ch = igetkey()) != EOF) {
        	/* TODO: add KEY_REFRESH support */
        	if (ch>255&&ch<0)
        		continue;
            if(ch==KEY_ESC){ /* etnlegend, 2006.04.07, 处理 ESC 键... */
                *data=0;
                ClearSubList(cwlist);
                ingetdata=false;
                return KEY_ESC;
            }
            if (ch == '\n' || ch == '\r') {
                *temp = '\0';
                prints("\n");
                if (NumInList(cwlist) == 1)
                    strcpy(data, cwlist->word);
                /*---	Modified by period	2000-09-13		---*
                 *---	when more results found, compare one by one	---*/
                /*
                 * if(!strcasecmp(data,cwlist->word))
                 * strcpy(data,cwlist->word) ;
                 */
                else {
                    struct word *list;

                    for (list = cwlist; list != NULL; list = list->next)
                        if (!strcasecmp(data, list->word)) {
                            strcpy(data, list->word);
                            break;
                        }
                }
                ClearSubList(cwlist);
                break;
            }
            if ((ch == ' ')||(ch == '\t')) {	/* add TAB key by pig2532 on 2005.12.10 */
                int col, len;

                if (NumInList(cwlist) == 1) {
                    /*
                     * added for * boards. cityhunter on 2k.5.21 
                     */
                    if (cwlist->word[0] != '_') {
                        strcpy(data, cwlist->word);
                        move(y, x);
                        prints("%s", data + count);
                        count = strlen(data);
                        temp = data + count;
                        getyx(&y, &x);
                        continue;
                    }
                }
                if (NumInList(cwlist)) {
                    int j;
                    struct word *list;
                    int len = strlen(cwlist->word);
                    j = temp-data;
                    while(j <= len) {
                        int ok=1;
                        j++;
                        for (list = cwlist; list != NULL; list = list->next)
                            if (strncasecmp(cwlist->word, list->word, j) != 0) {
                                ok=0;
                                break;
                            }
                        if(!ok) break;
                    }
                    j--;
                    while(temp-data<j) {
                        move(y, x);
                        outc(cwlist->word[temp-data]);
                        x++;
                        *temp = cwlist->word[temp-data];
                        temp++;
                        count++;
                    }
                    *temp = 0;
                }
                clearbot = true;
                col = 0;
                if (!morelist)
                    morelist = cwlist;
                len = MaxLen(morelist, NUMLINES);
                move(2, 0);
                clrtobot();
                prints("\x1b[7m");
                printdash(" 列表 ");
                prints("\x1b[m");
                while (len + col < scr_cols) {
                    int i;

                    for (i = NUMLINES; (morelist) && (i > 0); i--) {
                        move(3 + (NUMLINES - i), col);
                        /*
                         * add for * boards 
                         */
                        if (morelist->word[0] != '_')
                            prints("%s", morelist->word);
                        morelist = morelist->next;
                    }
                    col += len + 2;
                    if (!morelist)
                        break;
                    len = MaxLen(morelist, NUMLINES);
                }
                if (morelist) {
                    move(t_lines - 1, 0);
                    prints("\033[44m-- 还有 --                                                                     \033[m");
                }
                move(y, x);
                continue;
            }
            if (ch == '\177' || ch == '\010') {
                if (temp == data)
                    continue;
                temp--;
                count--;
                *temp = '\0';
                ClearSubList(cwlist);
                cwlist = GetSubList(data, toplev);
                morelist = NULL;
                x--;
                move(y, x);
                outc(' ');
                move(y, x);
                continue;
            }
            if (isprint2(ch)) {
				if( ch == '#' && count==0 && in_do_sendmsg ){
                	*temp = '\0';
                	ClearSubList(cwlist);
        			ingetdata = false;
					return ( ch ) ;
				}
                if (count < STRLEN) {
                    struct word *node;

                    *temp++ = ch;
                    count++;
                    *temp = '\0';
                    node = GetSubList(data, cwlist);
                    if (node == NULL) {
                        temp--;
                        *temp = '\0';
                        count--;
                        continue;
                    }
                    ClearSubList(cwlist);
                    cwlist = node;
                    morelist = NULL;
                    move(y, x);
                    outc(ch);
                    x++;
                } else
                    bell();
            }
        }
        if (ch == EOF)
            longjmp(byebye, -1);
        prints("\n");
        if (clearbot) {
            move(2, 0);
            clrtobot();
        }
        if (*data) {
            move(origy, origx);
            prints("%s\n", data);
            /*
             * for (x=1; x<500; x++);  delay 
             */
        }
        ingetdata = false;
        return 0;
    }
    if (prompt != NULL) {
        prints("%s", prompt);
        oflush();
    }
    if (fgets(data, STRLEN, stdin) == NULL)
        longjmp(byebye, -1);
    if ((temp = strchr(data, '\n')) != NULL)
        *temp = '\0';
    ingetdata = false;
    return 0;
}

int UserMaxLen(cwlist, cwnum, morenum, count)
char cwlist[][IDLEN + 1];
int cwnum, morenum, count;
{
    int len, max = 0;

    while (count-- > 0 && morenum < cwnum) {
        len = strlen(cwlist[morenum++]);
        if (len > max)
            max = len;
    }
    return max;
}

int UserSubArray(cwbuf, cwlist, cwnum, key, pos)
char cwbuf[][IDLEN + 1];
char cwlist[][IDLEN + 1];
int cwnum, key, pos;
{
    int key2, num = 0;
    int n, ch;

    key = chartoupper(key);
    if (key >= 'A' && key <= 'Z') {
        key2 = key - 'A' + 'a';
    } else {
        key2 = key;
    }
    for (n = 0; n < cwnum; n++) {
        ch = cwlist[n][pos];
        if (ch == key || ch == key2) {
            strcpy(cwbuf[num++], cwlist[n]);
        }
    }
    return num;
}

int usercomplete(prompt, data)
char *prompt, *data;
{
    extern int ingetdata;
    ingetdata = true;
    if (scrint) {
        char *cwbuf, *cwlist, *temp;
        int cwnum, x, y, origx, origy;
        int clearbot = false, count = 0, morenum = 0;
        int ch;
        struct userec *lookupuser;

/* 大量内存临时占用 KCN,TODO*/
        cwbuf = malloc(MAXUSERS * (IDLEN + 1));
        if (prompt != NULL) {
            prints("%s", prompt);
            clrtoeol();
        }
        temp = data;
        cwlist = NULL;
        cwnum = 0;
        getyx(&y, &x);
        getyx(&origy, &origx);
        while ((ch = igetkey()) != EOF) {
        	/* TODO: add KEY_REFRESH support */
        	if (ch>255&&ch<0)
        		continue;
            if (ch == '\n' || ch == '\r') {
                int i;
                char *ptr;

                *temp = '\0';
                prints("\n");
                ptr = cwlist;
                if (cwnum) {
                    for (i = 0; i < cwnum; i++) {
                        if (strncasecmp(data, ptr, IDLEN + 1) == 0) {
                            strcpy(data, ptr);
                            break;
                        }
                        ptr += IDLEN + 1;
                    }
                } else {
                    i = getuser(data, &lookupuser);
                    if (i != 0)
                        memcpy(data, lookupuser->userid, IDLEN);
                    else if (!id_invalid(data))
                        data[0] = 0;
                    data[IDLEN] = 0;
                }
                break;
            } else if ((ch == ' ')||(ch == '\t')) {		/* add TAB key by pig2532 on 2005.12.10 */
                int col, len;

                if (cwnum == 1) {
                    strcpy(data, cwlist);
                    move(y, origx);
                    prints("%s", data);
                    count = strlen(data);
                    temp = data + count;
                    getyx(&y, &x);
                    continue;
                }
                if (cwnum) {
                    int i, j;
                    char * ptr;
                    j = temp-data;
                    while(1) {
                        int ok=1;
                        j++;
                        ptr = cwlist;
                        for (i = 0; i < cwnum; i++) {
                            if (strncasecmp(cwlist, ptr, j) != 0) {
                                ok=0;
                                break;
                            }
                            ptr += IDLEN + 1;
                        }
                        if(!ok) break;
                    }
                    j--;
                    while(temp-data<j) {
                        move(y, x);
                        outc(cwlist[temp-data]);
                        x++;
                        *temp = cwlist[temp-data];
                        temp++;
                        count++;
                    }
                    *temp = 0;
                }
                if (count < 2)
                    continue;
                cwlist = u_namearray((char (*)[13]) cwbuf, &cwnum, data);
                clearbot = true;
                col = 0;
                len = UserMaxLen((char (*)[13]) cwlist, cwnum, morenum, NUMLINES);
                move(2, 0);
                clrtobot();
                printdash(" 所有使用者列表 ");
                while (len + col < scr_cols-1) {
                    int i;

                    for (i = 0; morenum < cwnum && i < NUMLINES; i++) {
                        move(3 + i, col);
                        prints("%s ", cwlist + (IDLEN + 1) * morenum++);
                    }
                    col += len + 2;
                    if (morenum >= cwnum)
                        break;
                    len = UserMaxLen((char (*)[13]) cwlist, cwnum, morenum, NUMLINES);
                }
                if (morenum < cwnum) {
                    move(t_lines - 1, 0);
                    prints("\033[44m-- 还有使用者 --                                                               \033[m");
                } else {
                    morenum = 0;
                }
                move(y, x);
                continue;
            } else if (ch == '\177' || ch == '\010') {
                if (temp == data)
                    continue;
                temp--;
                count--;
                *temp = '\0';
                cwlist = NULL;
                morenum = 0;
                cwnum = 0;
                x--;
                move(y, x);
                outc(' ');
                move(y, x);
                continue;
            } else if (isprint2(ch)) {
                if (count <= IDLEN) {
                    int n;

                    *temp++ = ch;
                    *temp = '\0';
                    if ((count > 1) && cwnum) {
                        n = UserSubArray((char (*)[13]) cwbuf, (char (*)[13]) cwlist, cwnum, ch, count);
                        if (n == 0) {
                            temp--;
                            *temp = '\0';
                            continue;
                        }
                        cwlist = cwbuf;
                        cwnum = n;
                    }
                    count++;
                    morenum = 0;
                    move(y, x);
                    outc(ch);
                    x++;
                } else
                    bell();
            }
        }
        free(cwbuf);
        if (ch == EOF)
            longjmp(byebye, -1);
        prints("\n");
        if (clearbot) {
            move(2, 0);
            clrtobot();
        }
        if (*data) {
            move(origy, origx);
            prints("%s\n", data);
        }
        ingetdata = false;
        return 0;
    } else {
        char *temp;

        if (prompt != NULL) {
            prints("%s", prompt);
            oflush();
        }
        if (fgets(data, STRLEN, stdin) == NULL)
            longjmp(byebye, -1);
        if ((temp = strchr(data, '\n')) != NULL)
            *temp = '\0';
        ingetdata = false;
        return 0;
    }
}
