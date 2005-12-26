#include "bbs.h"

int generate_board(struct boardheader *bh,void* arg)
{
	char buf[256];

	if(bh==NULL || bh->filename[0]=='\0')
		return 0;

	setboardorigin(bh->filename, 1);
	setboardmark(bh->filename, 1);

	return 0;
}

int generate_all()
{
	apply_boards(generate_board,NULL);
    return 0;
}

int main(int argc,char** argv)
{
	struct boardheader bh;

	chdir(BBSHOME);
	resolve_boards();

	if(argc < 2){
		printf("%s [-a|boardname]\n", argv[0]);
		exit(0);
	}
	if(argv[1][0]!='-'){
		bzero(&bh, sizeof(bh));
    	strncpy(bh.filename,argv[1],STRLEN);
    	generate_board(&bh,NULL);
	}else{
		if(!strcmp(argv[1],"-a")){
    		generate_all();
		}else{
			printf("%s [-a|boardname]\n", argv[0]);
		}
	}

    return 0;
}
