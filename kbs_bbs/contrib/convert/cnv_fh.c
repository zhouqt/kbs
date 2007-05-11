
#include "bbs.h"

struct fileheader_old {
    char filename[FILENAME_LEN];
    unsigned int id, groupid, reid;

    int o_bid;
    unsigned int o_id;
    unsigned int o_groupid;
    unsigned int o_reid;

    char innflag[2];
    char owner[OWNER_LEN];
    unsigned int eff_size;
    int posttime;
    unsigned int attachment;
    char title[ARTICLE_TITLE_LEN];
    unsigned char accessed[4];
};

struct fileheader_new {
    char filename[FILENAME_LEN];
    unsigned int id, groupid, reid;

    int o_bid;
    unsigned int o_id;
    unsigned int o_groupid;
    unsigned int o_reid;

    char innflag[2];
    char owner[OWNER_LEN];
    unsigned int eff_size;
    int posttime;
    unsigned int attachment;
    char title[ARTICLE_TITLE_LEN];
    unsigned short replycount;
    unsigned char accessed[4];
};

int main(int argc, char* argv[]) {
	char fname[256];
	FILE *fpi, *fpo;
	struct fileheader_old fhi;
	struct fileheader_new fho;
	
	if(argc != 2) {
		printf("usage:  %s filename\n", argv[0]);
		return 0;
	}
	
	strcpy(fname, argv[1]);
	fpi = fopen(fname, "r");
	if(!fpi) {
		printf("cannot open %s\n", fname);
		return 0;
	}
	strcat(fname, ".new");
	fpo = fopen(fname, "w");
	while(fread(&fhi, sizeof(struct fileheader_old), 1, fpi) > 0) {
		strcpy(fho.filename, fhi.filename);
		fho.id = fhi.id;
		fho.groupid = fhi.groupid;
		fho.reid = fhi.reid;
		fho.o_bid = fhi.o_bid;
		fho.o_id = fhi.o_id;
		fho.o_groupid = fhi.o_groupid;
		fho.o_reid = fhi.o_reid;
		fho.innflag[0] = fhi.innflag[0];
		fho.innflag[1] = fhi.innflag[1];
		strcpy(fho.owner, fhi.owner);
		fho.eff_size = fhi.eff_size;
		fho.posttime = fhi.posttime;
		fho.attachment = fhi.attachment;
		strcpy(fho.title, fhi.title);
		fho.replycount = -1;
		fho.accessed[0] = fhi.accessed[0];
		fho.accessed[1] = fhi.accessed[1];
		fho.accessed[2] = fhi.accessed[2];
		fho.accessed[3] = fhi.accessed[3];
		fwrite(&fho, sizeof(struct fileheader_new), 1, fpo);
	}
	fclose(fpi);
	fclose(fpo);
	printf("index file %s done\n", fname);

	return 0;
}


