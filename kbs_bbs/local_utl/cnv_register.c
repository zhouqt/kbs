/* 将pre_register中满了等待时间的用户注册单放到new_register中去. */

#include "bbs.h"

int main(int argc, char **argv)
{
    FILE* fin,*fout1,*fout2;
	char genbuf[8][256];
	int i;
	struct userec * uc;
	int now;
    int exit = 0;
	char userid[IDLEN+2];

	chdir(BBSHOME);
	
	resolve_ucache();
	resolve_utmp();

    now = time(NULL);
	if((fin = fopen("pre_register","r")) == NULL)
	{
	    printf("open pre_register file failed.\n");
	    return 0;
	}
	if((fout1 = fopen("pre_register1","w")) == NULL)
	{
	    fclose(fin);
	    printf("open pre_register1 file failed.\n");
	    return 0;
	}
	if((fout2 = fopen("new_register","a")) == NULL)
	{
	    fclose(fin);
		fclose(fout1);
	    printf("open pre_register file failed.\n");
	    return 0;
	}

	while(!exit){
	    for(i = 0; i < 8;i++)
		{
            if(fgets(genbuf[i], STRLEN, fin) == NULL){
			    exit = 1;
			    break;
			}
		}
		if(exit == 1)break;
		strncpy(userid,genbuf[1]+8,strlen(genbuf[1])-9);
		userid[IDLEN]=0;
		if(getuser(userid,&uc) == 0)
		{
		    printf("%s not found\n",userid);
		    continue;  //get userid's userec
		}
		if((now - uc->firstlogin) > REGISTER_WAIT_TIME)
		{
            for(i = 0; i < 8;i++)fputs(genbuf[i],fout2);
		}
		else
            for(i = 0; i < 8;i++)fputs(genbuf[i],fout1);
    }
    fclose(fin);
	fclose(fout1);
	fclose(fout2);
	
	f_mv("pre_register1","pre_register");
}
