#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

extern int prepf(int fp);
extern int mgrep(int fp);
//³ÌÐòÃû

int WORDBOUND,WHOLELINE,NOUPPER,INVERSE,FILENAMEONLY,SILENT,FNAME;
int COUNT,num_of_matched,total_line;
char CurrentFileName[255];
char* Progname="";

int check_filter(char* patternfile,char* checkfile)
{
	int fp,fd;
	WHOLELINE=0;
	NOUPPER=0;
	INVERSE=0;
	FILENAMEONLY=1;
	WORDBOUND=0;
	SILENT=1;
	FNAME=1;
	COUNT=0;

	num_of_matched=0;
	fp=open(patternfile,O_RDONLY);
	if (fp==-1) return 0;
	fd=open(checkfile,O_RDONLY);
	if (fd==-1) {
            close(fp);
	    return 0;
	};
	prepf(fp);
	mgrep(fd);
	close(fp);
	close(fd);
	return num_of_matched;
}
