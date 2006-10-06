#include "bbs.h"

static int simple_digest(char *str, int maxlen)
{
    char x[sizeof(int)];
    char *p;
    bzero(x, sizeof(int));
    for (p = str; *p && ((p - str) < maxlen); p++)
        x[(p - str) % sizeof(int)] += *p;
    return (*(int *) &x[0]);
}

int generate_board_title(struct boardheader *bh, void *arg)
{
    int fd2, size = sizeof(fileheader), total, i;
    struct boardheader btmp;
    char olddirect[PATHLEN];
    char *ptr, *t, *t2;
    FILE *fp;
    char opath[512];
    struct fileheader *ptr1;
    struct search_temp {
        bool has_pre;
        int digest;
        int thread_id;
        int id;
        int next;
    } *index;
    struct stat buf;
    int gen_threadid;
	struct BoardStatus* bs;

    setbdir(DIR_MODE_NORMAL, olddirect, bh->filename);
	setbdir(DIR_MODE_ORIGIN, opath, bh->filename);

	gen_threadid = 1;
    if ((fd2 = open(olddirect, O_RDWR, 0664)) == -1) {
        return 0;
    }

    BBS_TRY {
        if (safe_mmapfile_handle(fd2, PROT_READ | PROT_WRITE, MAP_SHARED, (void **) &ptr, &buf.st_size) == 0) {
            close(fd2);
            return 0;
        }
	fp = fopen(opath, "wb");
	
        total = buf.st_size / size;
        index = (struct search_temp *) malloc(sizeof(*index) * total);
        ptr1 = (struct fileheader *) ptr;
        for (i = 0; i < total; i++, ptr1++) {
            int j;

            t2 = ptr1->title;
            if (!strncmp(t2, "Re:", 3)) {
                index[i].has_pre = true;
                t2 += 4;
            } else
                index[i].has_pre = false;
            index[i].thread_id = 0;
            index[i].digest = simple_digest(t2, ARTICLE_TITLE_LEN);
            for (j = i - 1; j >= 0; j--) {
                struct fileheader *tmppost;

                if (index[j].digest != index[i].digest)
                    continue;
                tmppost = ((struct fileheader *) (ptr + j * size));
                t = tmppost->title;
                if (index[j].has_pre)
                    t += 4;
                if (!strcmp(t, t2)) {
                    index[j].next = i;
                    index[i].thread_id = index[j].thread_id;
                    index[i].next = 0;
                    break;
                }
            }
            if (index[i].thread_id == 0) {
                index[i].thread_id = gen_threadid;
                index[i].id = gen_threadid;
                index[i].next = 0;
                gen_threadid++;
            } else {
                index[i].id = gen_threadid;
                gen_threadid++;
            }
        }
        ptr1 = (struct fileheader *) ptr;
        for (i = 0; i < total; i++, ptr1++) {
            ptr1->id = index[i].id;
            ptr1->groupid = index[i].thread_id;
            ptr1->reid = index[i].thread_id;
	    if (index[i].has_pre ==  false)
                fwrite(ptr1, 1, sizeof(struct fileheader), fp);
        }
    }
    BBS_CATCH {
    }
    BBS_END;
    free(index);

    end_mmapfile((void *) ptr, buf.st_size, -1);
    fclose(fp);
    close(fd2);
    memcpy(&btmp, getbcache(bh->filename), sizeof(btmp));
	bs=getbstatus(getbnum(bh->filename));
	bs->nowid=gen_threadid + 1;
    set_board(getbnum(bh->filename), &btmp, NULL);
    return 0;
}

int generate_all_title()
{
    apply_boards(generate_board_title, NULL);
    return 0;
}

int main(int argc, char **argv)
{
    int allflag = 0;
    struct boardheader bh;
    char *name;

    while (1) {
        int c;

        c = getopt(argc, argv, "a");
        if (c == -1)
            break;
        switch (c) {
        case 'a':
            allflag = 1;
            break;
        case 'h':
            printf("%s [-a|boardname]\n  generatate board thread index", argv[0]);
            return 0;
        case '?':
            printf("%s:Unknown argument.\nTry `%s -h' for more information.\n", argv[0], argv[0]);
            return 0;
        }
    }
    chdir(BBSHOME);
    resolve_boards();
    if (optind < argc) {
        name = argv[optind++];
        if (optind < argc) {
            printf("%s:Too many arguments.\nTry `%s -h' for more information.\n", argv[0], argv[0]);
            return 0;
        }
        strncpy(bh.filename, name, STRLEN);
        generate_board_title(&bh, NULL);
    }
    if (allflag) {
        generate_all_title();
    }
    return 0;
}
