/********
stiger: 计算文章有效字节数
新发的文章bbsd都会计算了，这个程序主要用于计算以前的文章的有效字节数
并会改写该版面的.DIR里的effsize
*/


#include "bbs.h"

int calcboard(struct boardheader *bh, void *arg)
{
    char fn[80];
    int fd, total, i;
    struct stat buf;
    struct flock ldata;
    struct fileheader *ptr1;
    char *ptr;
    int size = sizeof(struct fileheader);

    setbdir(0, fn, bh->filename);
    printf("\r%s:\n", bh->filename);
    if ((fd = open(fn, O_RDWR, 0664)) == -1) {
        bbslog("user", "%s", "recopen err");
        return 0;               /* 创建文件发生错误 */
    }
    fstat(fd, &buf);
    ldata.l_type = F_RDLCK;
    ldata.l_whence = 0;
    ldata.l_len = 0;
    ldata.l_start = 0;
    fcntl(fd, F_SETLKW, &ldata);
    total = buf.st_size / size;

    if ((i = safe_mmapfile_handle(fd, PROT_READ | PROT_WRITE, MAP_SHARED, (void **) &ptr, &buf.st_size)) != 1) {
        if (i == 2)
            end_mmapfile((void *) ptr, buf.st_size, -1);
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        return 0;
    }
    ptr1 = (struct fileheader *) ptr;
    for (i = 0; i < total; i++) {
        struct stat st;
        char *p, *op;
        char ffn[80];
        int j;
        off_t fsize;

        printf("\r%d", i);
        setbfile(ffn, bh->filename, ptr1->filename);
        {
            int k, abssize = 0, entercount = 0, ignoreline = 0;
            char *attach;
            long attach_len;

            j = safe_mmapfile(ffn, O_RDONLY, PROT_READ, MAP_SHARED, (void **) &p, &fsize, NULL);
            op = p;
            if (j) {
                k = fsize;
                while (k) {
			/*
                    if (NULL != (checkattach(p, k, &attach_len, &attach))) {
                        k -= (attach - p) + attach_len;
                        p = attach + attach_len;
                        continue;
                    }  */
                    if (k >= 3 && *p == '\n' && *(p + 1) == '-' && *(p + 2) == '-' && *(p + 3) == '\n')
                        break;
                    if (*p == '\n') {
                        entercount++;
                        ignoreline = 0;
                    }
                    if (k >= 5 && *p == '\n' && *(p + 1) == '\xa1' && *(p + 2) == '\xbe' && *(p + 3) == ' ' && *(p + 4) == '\xd4' && *(p + 5) == '\xda')
                        ignoreline = 1;
                    if (k >= 2 && *p == '\n' && *(p + 1) == ':' && *(p + 2) == ' ')
                        ignoreline = 2;
                    if (k >= 2 && *p == KEY_ESC && *(p + 1) == '[' && *(p + 2) == 'm')
                        ignoreline = 3;

                    k--;
                    p++;
                    if (entercount >= 4 && !ignoreline)
                        abssize++;
                }
                ptr1->eff_size = abssize;
                end_mmapfile((void *) op, fsize, -1);
            }
        }
        //ptr1->posttime = get_posttime(ptr1);
        set_posttime(ptr1);
        ptr1++;
    }
    end_mmapfile((void *) ptr, buf.st_size, -1);
    ldata.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &ldata);
    close(fd);
    return 0;
}



int main(int argc, char **argv)
{
    chdir(BBSHOME);
    resolve_boards();
    resolve_ucache();

    apply_boards(calcboard, NULL);
    return 0;
}
