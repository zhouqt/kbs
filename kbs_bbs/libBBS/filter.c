#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

extern int prepf(int fp);
extern int mgrep(int fp);

//³ÌÐòÃû

int WORDBOUND, WHOLELINE, NOUPPER, INVERSE, FILENAMEONLY, SILENT, FNAME;
int COUNT, num_of_matched, total_line;
char *CurrentFileName;

int check_filter(char *patternfile, char *checkfile,int defaultval)
{
    int fp;
    char *ptr;
    int size, retv;

    WHOLELINE = 0;
    NOUPPER = 0;
    INVERSE = 0;
    FILENAMEONLY = 1;
    WORDBOUND = 0;
    SILENT = 1;
    FNAME = 1;
    COUNT = 0;

    CurrentFileName = checkfile;
    num_of_matched = 0;
    fp = open(patternfile, O_RDONLY);
    prepf(fp);
    BBS_TRY {
        if (safe_mmapfile(checkfile, O_RDONLY, PROT_READ, MAP_SHARED, (void **) &ptr, &size, NULL) == 0)
            BBS_RETURN(0);
        retv = mgrep_str(ptr, size);
    }
    BBS_CATCH {
    	retv=defaultval;
    }
    BBS_END end_mmapfile((void *) ptr, size, -1);
    close(fp);
    return retv;
}
