#include <stdio.h>
#include <time.h>
#include "bbs.h"
#ifdef COMMEND_ARTICLE

#define	MAX_COMMEND 15

void gen_commend_xml()
{
	int fd,dirfd;
	FILE *fp;
	struct fileheader dirfh;
	struct fileheader fh;
	char dirpath[STRLEN];
	char dirfile[STRLEN];
	int show=0;
	char xml_buf[256];

	setbfile(dirpath, COMMEND_ARTICLE, DIGEST_DIR);

    if ((fp = fopen("xml/commend.xml", "w")) == NULL) 
		return -1;

	fprintf(fp, "<?xml version=\"1.0\" encoding=\"GBK\"?>\n");
	fprintf(fp, "<hotsubjects>\n");

	dirfd = open(dirpath, O_RDONLY);
	if( dirfd >= 0 ){
		while(read(dirfd, &dirfh, sizeof(dirfh)) >= sizeof(dirfh) ){

			if(show > MAX_COMMEND)
				break;

			setbdir(DIR_MODE_NORMAL, dirfile, dirfh.o_board);

			if ((fd = open(dirfile, O_RDWR, 0644)) < 0)
				continue;

    		if(get_records_from_id(fd, dirfh.o_id, &fh, 1, NULL) == 0 )
			{
				close(fd);
				continue;
			}
			close(fd);

			fprintf(fp, "<hotsubject>\n");
			fprintf(fp, "<title>%s</title>\n", encode_xml(xml_buf, fh.title, 
						sizeof(xml_buf)));
			fprintf(fp, "<author>%s</author>\n", fh.owner);
			fprintf(fp, "<board>%s</board>\n", dirfh.o_board);
			//fprintf(fp, "<time>%d</time>\n", top[i].date);
			//fprintf(fp, "<number>%d</number>\n", top[i].number);
			fprintf(fp, "<groupid>%d</groupid>\n", fh.id);
			fprintf(fp, "</hotsubject>\n");
        }
		close(dirfd);
    }

	fprintf(fp, "</hotsubjects>\n");
    fclose(fp);
}

int main(int argc, char **argv)
{
    time_t now;
    struct tm ptime;
    int i;

    chdir(BBSHOME);

	gen_commend_xml();

    return 0;
}
#endif
