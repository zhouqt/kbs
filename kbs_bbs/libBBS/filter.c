#include "bbs.h"
extern int prepf(int fp,void** patternbuf,size_t* patt_image_len);
extern int mgrep(int fp,void* patternbuf);
extern int mgrep_str(char* data,int len,void* patternbuf);
extern void releasepf(void* patternbuf);
//³ÌÐòÃû

int WORDBOUND, WHOLELINE, NOUPPER, INVERSE, FILENAMEONLY, SILENT, FNAME;
int ONLYCOUNT, num_of_matched, total_line;
char *CurrentFileName;

int build_badwordimage()
{
    int fp;
    void* pattern_buf;
    size_t pattern_imagesize;
    fp = open("etc/badwords", O_RDONLY);
    if (fp==-1)
    	return -1;
    flock(fp,LOCK_EX);
    if (dashf("etc/badwords.img")) {
    	flock(fp,LOCK_UN);
    	close(fp);
    	return 0;
    }
    prepf(fp,&pattern_buf,&pattern_imagesize);
   	flock(fp,LOCK_UN);
    close(fp);
    fp = open("etc/badwords.img", O_WRONLY|O_TRUNC|O_CREAT);
    if (fp==-1)
    	return -1;
    write(fp,pattern_buf,pattern_imagesize);
    close(fp);
    releasepf(pattern_buf);
    return 0;
}

int check_badword(char *checkfile)
{
    char *ptr;
    int size,retv;
    void* pattern_img_ptr;
    int pattern_img_size;
    BBS_TRY {
        if (safe_mmapfile(checkfile, O_RDONLY, PROT_READ, MAP_SHARED, (void **) &ptr, &size, NULL) == 0)
            BBS_RETURN(0);
        retry:
        if (safe_mmapfile("etc/badwords.img", O_RDONLY, PROT_READ, MAP_SHARED, (void **) &pattern_img_ptr, &pattern_img_size, NULL) == 0)
        {
            if (!dashf("etc/badwords.img")) {
            	if (build_badwordimage()==0)
            		goto retry;
            }
            BBS_RETURN(-1);
        }
        retv = mgrep_str(ptr, size,pattern_img_ptr);
    }
    BBS_CATCH {
    	retv=-2;
    }
    BBS_END end_mmapfile((void *) ptr, size, -1);
    return retv;
}

int check_filter(char *patternfile, char *checkfile,int defaultval)
{
    int fp;
    char *ptr;
    int size, retv;
    void* pattern_buf;
    size_t pattern_imagesize;

    WHOLELINE = 0;
    NOUPPER = 0;
    INVERSE = 0;
    FILENAMEONLY = 1;
    WORDBOUND = 0;
    SILENT = 1;
    FNAME = 1;
    ONLYCOUNT = 0;

    CurrentFileName = checkfile;
    num_of_matched = 0;
    fp = open(patternfile, O_RDONLY);
    prepf(fp,&pattern_buf,&pattern_imagesize);
    BBS_TRY {
        if (safe_mmapfile(checkfile, O_RDONLY, PROT_READ, MAP_SHARED, (void **) &ptr, &size, NULL) == 0)
            BBS_RETURN(0);
        retv = mgrep_str(ptr, size,pattern_buf);
    }
    BBS_CATCH {
    	retv=defaultval;
    }
    BBS_END end_mmapfile((void *) ptr, size, -1);
    close(fp);
    releasepf(pattern_buf);
    return retv;
}
