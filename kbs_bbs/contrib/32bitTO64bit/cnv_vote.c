/* add userdefine1 to userec */

#include "bbs.h"
#include "stdio.h"

struct votebal1 {
    char userid[IDLEN + 1];
    char title[STRLEN];
    char type;
    char items[32][38];
    int maxdays;
    int maxtkt;
    int totalitems;
    int opendate;
};

struct votebal2 {
    char userid[IDLEN + 1];
    char title[STRLEN];
    char type;
    char items[32][38];
    int maxdays;
    int maxtkt;
    int totalitems;
    time_t opendate;
};

int main(int argc , char* argv[])
{
        FILE * fp,*fp2;
        struct votebal1 bh;
        struct votebal2 bhnew;

        int i;

	if(argc !=3 )
	{
		printf("usage: %s Old_VOTE_FILE New_VOTE_FILE\n",argv[0]);
		exit(0);
	}
	if((fp=fopen(argv[2],"r"))!=NULL){
		printf("NEW FILE exist!");
		fclose(fp);
		exit(0);
	}
        if((fp = fopen(argv[1],"r")) == NULL)
        {
                printf("open %s file failed!",argv[1]);
                exit(0);
        }

        if((fp2 = fopen(argv[2],"w")) == NULL)
        {
                printf("cant create newboards file!");
                exit(0);
        }

        while( fread(&bh,sizeof(struct votebal1),1,fp) ){
//        	memset(&bhnew,0,sizeof(struct userec2));
		memcpy(&bhnew,&bh,sizeof(struct votebal1));
                bhnew.opendate=bh.opendate;
        	fwrite(&bhnew,sizeof(struct votebal2),1,fp2);
		}

        fclose(fp2);
        fclose(fp);
        return 0;
}


