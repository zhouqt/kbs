
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
    char unused[114];
    unsigned char accessed[4];
};

int cnv_board_index(char* fname) {
	FILE *fpi, *fpo;
	struct fileheader_old fhi;
	struct fileheader_new fho;
	char fnamenew[256], cmd[1024];
	
	fpi = fopen(fname, "r");
	if(!fpi) {
		//printf("cannot open %s\n", fname);
		return 0;
	}
	sprintf(fnamenew, "%s.new", fname);
	fpo = fopen(fnamenew, "w");
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
	
	sprintf(cmd, "cp %s %s.old", fname, fname);
	system(cmd);
	sprintf(cmd, "cp %s %s", fnamenew, fname);
	system(cmd);
	
	//printf("index file %s done\n", fname);
	return 0;
}

int undo_board_index(char* fname) {
	char cmd[1024];
	
	sprintf(cmd, "cp %s.old %s", fname, fname);
	system(cmd);
	
	return 0;
}

static int cnv_board(struct boardheader* bh, void* arg) {
	char fname[256];
	setbdir(DIR_MODE_NORMAL, fname, bh->filename);
	cnv_board_index(fname);
	setbdir(DIR_MODE_DIGEST, fname, bh->filename);
	cnv_board_index(fname);
	setbdir(DIR_MODE_THREAD, fname, bh->filename);
	cnv_board_index(fname);
	setbdir(DIR_MODE_MARK, fname, bh->filename);
	cnv_board_index(fname);
	setbdir(DIR_MODE_DELETED, fname, bh->filename);
	cnv_board_index(fname);
	setbdir(DIR_MODE_JUNK, fname, bh->filename);
	cnv_board_index(fname);
	setbdir(DIR_MODE_ORIGIN, fname, bh->filename);
	cnv_board_index(fname);
	setbdir(DIR_MODE_WEB_THREAD, fname, bh->filename);
	cnv_board_index(fname);
	setbdir(DIR_MODE_ZHIDING, fname, bh->filename);
	cnv_board_index(fname);
	board_update_toptitle(getbid(bh->filename, NULL), false);
	printf("board %s is done.\n", bh->filename);
	return 0;
}

static int undo_board(struct boardheader* bh, void* arg) {
	char fname[256];
	setbdir(DIR_MODE_NORMAL, fname, bh->filename);
	undo_board_index(fname);
	setbdir(DIR_MODE_DIGEST, fname, bh->filename);
	undo_board_index(fname);
	setbdir(DIR_MODE_THREAD, fname, bh->filename);
	undo_board_index(fname);
	setbdir(DIR_MODE_MARK, fname, bh->filename);
	undo_board_index(fname);
	setbdir(DIR_MODE_DELETED, fname, bh->filename);
	undo_board_index(fname);
	setbdir(DIR_MODE_JUNK, fname, bh->filename);
	undo_board_index(fname);
	setbdir(DIR_MODE_ORIGIN, fname, bh->filename);
	undo_board_index(fname);
	setbdir(DIR_MODE_WEB_THREAD, fname, bh->filename);
	undo_board_index(fname);
	setbdir(DIR_MODE_ZHIDING, fname, bh->filename);
	undo_board_index(fname);
	board_update_toptitle(getbid(bh->filename, NULL), false);
	printf("board %s is undone.\n", bh->filename);
	return 0;
}

int main(int argc, char* argv[]) {
	
	int mode;
	
	chdir(BBSHOME);
	resolve_boards();
	load_ucache();
	
	mode = 0;
	if(argc > 1) {
		if(strcmp(argv[1], "-u") == 0)
			mode = 1;
	}
	
	if(mode == 0)
		apply_boards(cnv_board, NULL);
	else
		apply_boards(undo_board, NULL);

	return 0;
}


