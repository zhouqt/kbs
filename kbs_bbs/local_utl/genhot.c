#include <stdio.h>
#include <time.h>
#include "bbs.h"
#include "urlencode.c"
#ifdef COMMEND_ARTICLE

#define	MAX_COMMEND 5

int gen_commend_xml()
{
	int dirfd;
	FILE *fp;
	FILE *fp1;
	struct fileheader dirfh;
	struct fileheader fh;
	char dirpath[STRLEN];
	char dirfile[STRLEN];
	char xml_buf[1024]; //changed from 256 to 1024, for <brief> - atppp
	char buf[256];
	char url_buf[1024]; //changed from 256 to 1024, for <brief> - atppp
	struct stat st;
	int numrecords;
	int i;
	char *c;
	struct boardheader *bh;

	setbfile(dirpath, COMMEND_ARTICLE, DIGEST_DIR);
    if (stat(dirpath, &st) < 0)
        return -1;
	numrecords = st.st_size / sizeof(struct fileheader) ;
	if (numrecords <= 0)
		return -1;

    if ((fp = fopen("xml/commend.xml", "w")) == NULL) 
		return -1;

	fprintf(fp, "<?xml version=\"1.0\" encoding=\"GBK\"?>\n");
	fprintf(fp, "<hotsubjects>\n");

	dirfd = open(dirpath, O_RDONLY);
	if( dirfd >= 0 ){
		if( numrecords > MAX_COMMEND )
			lseek(dirfd, sizeof(struct fileheader)*(numrecords - MAX_COMMEND), SEEK_SET);

		numrecords -= MAX_COMMEND;

		while(read(dirfd, &dirfh, sizeof(dirfh)) >= sizeof(dirfh) ){

			setbfile(dirfile, COMMEND_ARTICLE, dirfh.filename);

			if(( fp1=fopen(dirfile, "r"))==NULL )
				continue;

			numrecords ++;

			fprintf(fp, "<hotsubject>\n");
			fprintf(fp, "<title>%s</title>\n", encode_url(url_buf,encode_xml(xml_buf, dirfh.title, sizeof(xml_buf)),sizeof(url_buf)));
			fprintf(fp, "<author>%s</author>\n", encode_url(url_buf,dirfh.owner,sizeof(url_buf)));
			fprintf(fp, "<time>%d</time>\n", get_posttime(&dirfh));
			fprintf(fp, "<board>%s</board>\n", encode_url(url_buf,COMMEND_ARTICLE,sizeof(url_buf)));
			fprintf(fp, "<id>%d</id>\n", dirfh.id);
			bh = getboard(dirfh.o_bid);
			fprintf(fp, "<o_board>%s</o_board>\n", encode_url(url_buf,(bh ? bh->filename : ""),sizeof(url_buf)));
			fprintf(fp, "<o_id>%d</o_id>\n",dirfh.o_id);
			if( fgets(buf, 255, fp1) ){
				if( ! strncmp(buf, "∑¢–≈»À: ", 8) ){
					if( (c=strchr(buf+8, ' ')) != NULL )
						*c = 0;
					fprintf(fp, "<owner>%s</owner>\n", buf+8);
				}
			}
			fprintf(fp, "<o_groupid>%d</o_groupid>\n<brief>", dirfh.o_groupid);
			for(i=0;i<3;i++) fgets(buf, 255, fp1);
			for(i=0;i<4;){
				if(fgets(buf, 255, fp1) ){
					if( buf[0] == '\n' || buf[0] == '\r' || buf[0]=='\0' )
						continue;
					buf[255]=0;
					while( (c=strchr(buf, '\x1b')) != NULL )
						*c='*';
					fprintf(fp, "%s", encode_url(url_buf,encode_xml(xml_buf, buf, sizeof(xml_buf)),sizeof(url_buf)) );
					i++;
				}else
					break;
			}
			fprintf(fp, "</brief>\n</hotsubject>\n");

			fclose(fp1);
        }
		close(dirfd);
    }

	fprintf(fp, "</hotsubjects>\n");
    fclose(fp);
}

#endif
int main(int argc, char **argv)
{
    time_t now;
    struct tm ptime;
    int i;

    chdir(BBSHOME);

#ifdef COMMEND_ARTICLE
	resolve_boards();
	gen_commend_xml();
#endif

    return 0;
}
