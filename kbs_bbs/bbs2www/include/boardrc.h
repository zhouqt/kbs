#if !defined BOARDRC_H
#define BOARDRC_H

/*#define BRC_MAXSIZE     50000*/
#define BRC_MAXNUM      60
#define BRC_STRLEN      15
#define BRC_ITEMSIZE    (BRC_STRLEN + 1 + BRC_MAXNUM * sizeof( int ))
extern char    brc_buf[BRC_MAXSIZE];
extern int     brc_size;
extern char    brc_name[BRC_STRLEN];
extern int     brc_list[BRC_MAXNUM], brc_num;

char *brc_getrecord(char *ptr, char *name, int *pnum, int *list);

char *brc_putrecord(char *ptr, char *name, int num, int *list);

void brc_update(char *userid, char *board);

int brc_init(char *userid, char *boardname);

int brc_add_read(char *filename);

int brc_un_read(char *file);

int brc_clear();

int brc_has_read(char *file);
#endif /* BOARDRC_H */
