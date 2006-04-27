#include "bbs.h"

int generate_board_title(struct boardheader *bh,void* arg)
{
    int fd2, total, i;
    char olddirect[PATHLEN];
    char *ptr;
    struct fileheader * ptr1;
    struct BoardStatus* bs;
    struct stat buf;
	unsigned int iid;

    setbdir(DIR_MODE_NORMAL, olddirect, bh->filename);
    
    if ((fd2 = open(olddirect, O_RDWR, 0664)) == -1) {
		perror(olddirect);
        return 0;
    }

        if (safe_mmapfile_handle(fd2, PROT_READ | PROT_WRITE, MAP_SHARED, &ptr, &buf.st_size) == 0) {
            close(fd2);
            return 0;
        }
        total = buf.st_size / (sizeof(struct fileheader));
        ptr1 = (struct fileheader *) ptr;

		iid = ptr1[total-1].id;

		if(iid <=0) iid = 1;

    end_mmapfile((void *) ptr, buf.st_size, -1);
    close(fd2);

    bs=getbstatus(getbid(bh->filename,NULL));
	if(bs)
    	bs->nowid= iid + 1;

    return 0;
}

int generate_all_title()
{
	apply_boards(generate_board_title,NULL);
    return 0;
}

int main(int argc,char** argv)
{
	int allflag=0;
	struct boardheader bh;
	char* name;
    while (1) {
    	int c;
    	c = getopt(argc, argv, "ah");
    	if (c == -1)
            break;
        switch (c) {
        case 'a':
        	allflag = 1;
        	break;
        case 'h':
        	printf("%s [-a|boardname]\n  generatate board nowid.\n",argv[0]);
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
        if(strchr(name,'/') || name[0] == '.') {
            printf("arguments must be boardname\n");
            return -1;
        }
    	strncpy(bh.filename,name,STRLEN);
    	generate_board_title(&bh,NULL);
    }
    if (allflag) {
    	generate_all_title();
    }
    return 0;
}
