#include "bbs.h"
#ifdef lint
#include <sys/uio.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
int a;

typedef struct node GOPHER;
struct node {
    char file[81], title[71], server[41];
    int port;
    int position;
    GOPHER *next;
};

GOPHER *g_main[100];            /*100 directories to move in to */
GOPHER *tmpitem;
GOPHER *topitem;
int gopher_position = 0;


GOPHER *next(node)
    GOPHER *node;
{
    if (node == NULL)
        return NULL;
    else
        return node->next;
}


GOPHER *find_kth(node, n)
    GOPHER *node;
    int n;
{
    int i;
    GOPHER *tmpnode;

    tmpnode = node;
    for (i = 0; i <= n; i++) {
        tmpnode = next(tmpnode);
    }
    return tmpnode;
}

int readfield(fd, ptr, maxlen)
    int fd;
    char *ptr;
    int maxlen;
{
    int n;
    int rc;
    char c;

    for (n = 1; n < maxlen; n++) {
        if ((rc = read(fd, &c, 1)) == 1) {
            *ptr++ = c;
            if (c == '\t') {
                *(ptr - 1) = '\0';
                break;
            }
        } else if (rc == 0) {
            if (n == 1)
                return (0);     /* EOF, no data read */
            else
                break;          /* EOF, some data was read */
        } else
            return (-1);        /* error */
    }

    *ptr = 0;                   /* Tack a NULL on the end */
    return (n);
}

int readline(fd, ptr, maxlen)
    int fd;
    char *ptr;
    int maxlen;
{
    int n;
    int rc;
    char c;


    for (n = 1; n < maxlen; n++) {
        if ((rc = read(fd, &c, 1)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;
        } else if (rc == 0) {
            if (n == 1)
                return (0);     /* EOF, no data read */
            else
                break;          /* EOF, some data was read */
        } else
            return (-1);        /* error */
    }

    *ptr = 0;                   /* Tack a NULL on the end */
    return (n);
}

int savetmpfile(tmpname)
    char tmpname[];
{
    char ch;
    char buf[256];
    FILE *fp;
    int cc;

    if ((fp = fopen(tmpname, "w")) == NULL)
        return -1;
    show_message("\033[5m转换文件资料为暂存档");
    fprintf(fp, "来  源: %s\n", tmpitem->server);
    fprintf(fp, "档  名: %s(使用 %d 埠)\n", tmpitem->file, tmpitem->port);
    fprintf(fp, "标  题: %s\n\n", tmpitem->title + 1);
    while (1) {
        if ((cc = read(a, buf, 255)) > 0) {
            buf[cc] = '\0';
            fprintf(fp, "%s", buf);
        } else {
            fclose(fp);
            break;
        }
    }
    show_message(NULL);
    fclose(fp);
    return 1;

}

int print_gophertitle()
{
    char buf[256];
    char title[256];

    sprintf(title, "%s", g_main[gopher_position]->title + 1);
    title[70] = '\0';
    move(0, 0);
    clrtobot();
    refresh();
    sprintf(buf, "%*s", (80 - strlen(title)) / 2, " ");
    prints("\033[1;44m%s%s%s\033[m\n", buf, title, buf);
    prints("             \033[1;32mF \033[37m寄回自己的信箱 \033[32m ↑↓\033[37m 移动  \033[32m→ <Enter>\033[37m 读取 \033[32m ←\033[37m 离开");
}

printgopher_title()
{

    move(2, 0);
    clrtoeol();

    prints("\033[1;37;44m 编号 [类别] 标    题                                                          \033[m\n");
}

int g_refresh()
{
    print_gophertitle();
    show_gopher();
    update_endline();
}

int deal_gopherkey(ch, allnum, pagenum)
    char ch;
    int allnum, pagenum;
{
    int rc;
    char fname[STRLEN], chr, fpath[STRLEN];
    FILE *fp;
    struct boardheader fhdr;


    switch (ch) {
    case 'h':
    case 'H':
        show_help("help/announcereadhelp");
        g_refresh();
        break;
    case 'E':
    case 'e':
        tmpitem = find_kth(topitem, allnum);
        sethomefile(fname, currentuser->userid, "gopher.tmp");
        if (tmpitem->title[0] != '0') {
            return 1;
        }
        if (get_con(tmpitem->server, tmpitem->port) == -1)
            return 1;
        enterdir(tmpitem->file);
        savetmpfile(fname);
        if (dashf(fname)) {
            vedit(fname, NA,NULL, NULL);
            unlink(fname);
            g_refresh();
        }
        show_message(NULL);
        break;
    case '=':
        {
            tmpitem = find_kth(topitem, allnum);
            move(2, 0);
            clrtobot();
            prints("\033[1;44;37m");
            printdash("BBS Gopher 物件基本资料");
            prints("\033[m");
            prints("类型：%s\n", (tmpitem->title[0] == '0') ? "文件" : "目录");
            prints("标题：%s\n", tmpitem->title + 1);
            prints("位置：%s\n", tmpitem->server);
            prints("使用：%d埠\n", tmpitem->port);
            prints("路径：%s\n", tmpitem->file);
            pressanykey();
            g_refresh();
        }
        break;
    case Ctrl('P'):
        tmpitem = find_kth(topitem, allnum);
        if (!HAS_PERM(currentuser, PERM_POST))
            break;
        sethomefile(fname, currentuser->userid, "gopher.tmp");
        if (tmpitem->title[0] != '0') {
            return 1;
        }
        if (get_con(tmpitem->server, tmpitem->port) == -1)
            return 1;
        enterdir(tmpitem->file);
        savetmpfile(fname);
        if (dashf(fname)) {
            char bname[30];

            clear();
            if (get_a_boardname(bname, "请输入要转贴的讨论区名称: ")) {
                move(1, 0);
                clrtoeol();
                sprintf(fpath, "你确定要转贴到 %s 板吗", bname);
                if (askyn(fpath, 0) == 1) {
                    move(2, 0);
                    postfile(fname, bname, tmpitem->title + 1, 2);
                    sprintf(fpath, "\033[1m已经帮你到 %s 板了...\033[m", bname);
                    prints(fpath);
                    refresh();
                    sleep(1);
                }
            }
        }
        unlink(fname);
        g_refresh();
        return 1;
    case 'U':
    case 'F':
    case 'u':
    case 'f':
    case 'z':
    case 'Z':
        tmpitem = find_kth(topitem, allnum);
        sethomefile(fname, currentuser->userid, "gopher.tmp");
        if (tmpitem->title[0] != '0') {
            return 1;
        }
        if (get_con(tmpitem->server, tmpitem->port) == -1)
            return 1;
        enterdir(tmpitem->file);
        savetmpfile(fname);
        if (dashf(fname)) {
            strncpy(fhdr.title, tmpitem->title + 1, 70);
            strncpy(fhdr.filename, "gopher.tmp", STRLEN);
            sethomepath(fpath, currentuser->userid);
            if (ch == 'Z' || ch == 'z')
                a_download(fname);
            else {
                switch (doforward(fpath, &fhdr, (ch == 'u' || ch == 'U') ? 1 : 0)) {
                case 0:
                    show_message("文章转寄完成!");
                    break;
                case -1:
                    show_message("system error!!.");
                    break;
                case -2:
                    show_message("invalid address.");
                    break;
                default:
                    show_message("取消转寄动作.");
                }
            }
            refresh();
            pressanykey();
            g_refresh();
            unlink(fname);
            return 1;
        } else
            return 1;
        break;
    default:
        return 0;
    }
    return 1;
}

int enterdir(path)
    char path[];
{
    char buf[256];

    sprintf(buf, "%s\r\n", path == NULL ? "" : path);
    write(a, buf, sizeof(buf));
}

int get_con(servername, port)
    char *servername;
    int port;
{
    struct hostent /**h,*/ *h2;
    char buf[1024];
    char hostname[81];
    struct sockaddr_in sin;
    char ch[1];
    int get;

    show_message("建立连线中...");
    /*    gethostname(hostname,80);
       if(!(h = gethostbyname(hostname))) {
       perror("gethostbyname") ;
       return -1 ;
       }
     */
    if ((h2 = gethostbyname(servername)) == NULL)
        sin.sin_addr.s_addr = inet_addr(servername);
    else
        memcpy(&sin.sin_addr.s_addr, h2->h_addr, h2->h_length);
    sin.sin_family = AF_INET;
    if (!(a = socket(AF_INET, SOCK_STREAM, 0))) {
        perror("Socket:");
        return -1;
    }
    sin.sin_port = htons(port);
    /*    bind(a,h->h_addr,h->h_length);
       bind(a,(struct sockaddr *)h,sizeof h); */
    if ((connect(a, (struct sockaddr *) &sin, sizeof sin))) {
        perror("connect fail");
        return -1;
    }
    show_message(NULL);
    return 1;
}

int do_gopher(page, num)
    int page, num;
{
    return -1;
}


int show_gopher()
{
    extern int page, range;
    int i;
    GOPHER *tmpnode;

    printgopher_title();
    move(3, 0);
    clrtobot();
    tmpnode = find_kth(topitem, page);
    for (i = page; i < page + 19 && i < range; i++) {
        move(i - page + 3, 0);
        prints(" %4d [\033[1m%9s\033[m] %-65s\n", i + 1, ((tmpnode->title[0] == '0') ? "\033[36m文件" : "\033[37m目录"), tmpnode->title + 1);
        tmpnode = next(tmpnode);
    }
    clrtobot();
}


int freeitems(num)
    int num;
{
    GOPHER *tmpnode, *tmpnode2;
    int i = 0;

    tmpnode2 = topitem->next;
    for (i = 0; i < num; i++) {
        tmpnode = next(tmpnode2);
        free(tmpnode2);
        tmpnode2 = tmpnode;
    }
    topitem->next == NULL;
}


int showout()
{
    int i, i2 = 0, i3, len;
    int item;
    struct hostent *h, *h2;
    char foo[1024], cch[5], tmpbar[71];
    char hostname[81], tmpfile[STRLEN];
    struct sockaddr_in sin;
    char ch, buf[20];
    int get, notreload = 0;
    FILE *fp;
    GOPHER *newitem, *tmpnode;

    while (1) {
        if (gopher_position < 0) {
            return;
        }
        print_gophertitle();
        printgopher_title();
        update_endline();
        if (!notreload) {
            i = 0;
            if (get_con(g_main[gopher_position]->server, g_main[gopher_position]->port) == -1) {
                show_message(NULL);
                free(g_main[gopher_position]);
                gopher_position--;
                notreload = 0;
                continue;
            }
            enterdir(g_main[gopher_position]->file);
            show_message("读取准备中");
            for (i = 0; i < MAXGOPHERITEMS; i++) {
                newitem = (GOPHER *) malloc(sizeof(GOPHER));
                if (readfield(a, foo, 1024) <= 0) {
                    free(newitem);
                    break;
                }
                if (foo[0] == '.' && foo[1] == '\r' && foo[2] == '\n') {
                    free(newitem);
                    break;
                }
                strncpy(newitem->title, foo, 70);
                if (readfield(a, foo, 1024) == 0) {
                    free(newitem);
                    break;
                }
                strncpy(newitem->file, foo, 80);
                if (readfield(a, foo, 1024) == 0) {
                    free(newitem);
                    break;
                }
                strncpy(newitem->server, foo, 40);
                if (readline(a, foo, 1024) == 0) {
                    free(newitem);
                    break;
                }
                newitem->port = atoi(foo);
                if (newitem->title[0] != '0' && newitem->title[0] != '1') {
                    free(newitem);
                    i--;
                    continue;
                }
                refresh();
                newitem->next == NULL;
                if (i == 0) {
                    topitem->next = newitem;
                    tmpnode = newitem;
                } else {
                    tmpnode->next = newitem;
                    tmpnode = newitem;
                }
                sprintf(buf, "\033[1;3%dm转\033[3%dm换\033[3%dm资\033[3%dm料\033[3%dm中\033[m", (i % 7) + 1, ((i + 1) % 7) + 1, ((i + 2) % 7) + 1, ((i + 3) % 7) + 1, ((i + 4) % 7) + 1);
                show_message(buf);
            }
            show_message(NULL);
        } else
            notreload = 0;
        if (i <= 0) {
            move(3, 0);
            clrtobot();
            move(10, 0);
            clrtoeol();
            prints("                             \033[1;31m没有任何的资料...\033[m");
            pressanykey();
            free(g_main[gopher_position]);
            gopher_position--;
            continue;
        }
        close(a);
        move(0, 0);
        clrtobot();
        setlistrange(i);
        i2 = choose(NA, g_main[gopher_position]->position, print_gophertitle, deal_gopherkey, show_gopher, do_gopher);
        if (i2 == -1) {
            free(g_main[gopher_position]);
            gopher_position--;
            freeitems(i);
            continue;
        }
        g_main[gopher_position]->position = i2;
        tmpitem = find_kth(topitem, i2);
        if (tmpitem->title[0] == '0') {
            if (get_con(tmpitem->server, tmpitem->port) == -1)
                return -1;
            enterdir(tmpitem->file);
            sethomefile(tmpfile, currentuser->userid, "gopher.tmp");
            savetmpfile(tmpfile);
            close(a);
            ansimore(tmpfile, true);
            notreload = 1;
            unlink(tmpfile);
            continue;
        } else {
            gopher_position++;
            newitem = (GOPHER *) malloc(sizeof(GOPHER));
            strncpy(newitem->server, tmpitem->server, 40);
            strncpy(newitem->file, tmpitem->file, 80);
            strncpy(newitem->title, tmpitem->title, 70);
            newitem->port = tmpitem->port;
            newitem->position = 0;
            g_main[gopher_position] = newitem;
            freeitems(i);
            continue;
        }
    }
}


gopher(serv, dire, port, title)
    char serv[], dire[], title[];
    int port;
{
    GOPHER *newitem;
    char buf[80];

    modify_user_mode(CCUGOPHER);
    gopher_position = 0;
    newitem = (GOPHER *) malloc(sizeof(GOPHER));
    strncpy(newitem->server, serv, 40);
    strncpy(newitem->file, dire, 80);
    sprintf(buf, " %s", title);
    strncpy(newitem->title, buf, 70);
    newitem->port = port;
    newitem->position = 0;
    g_main[gopher_position] = newitem;
    topitem = (GOPHER *) malloc(sizeof(GOPHER));
    topitem->next = NULL;
    showout();
    modify_user_mode(CSIE_ANNOUNCE);
}
