#include "bbs.h"
static void* badword_img=NULL;
static int badword_img_size;

extern int prepf(int fp,void** patternbuf,size_t* patt_image_len);
extern int mgrep(int fp,void* patternbuf);
extern int mgrep_str(char* data,int len,void* patternbuf);
extern void releasepf(void* patternbuf);
//³ÌÐòÃû
#ifdef BBSMAIN
extern void abort_bbs(int sign);
#endif

int WORDBOUND, WHOLELINE, NOUPPER, INVERSE, FILENAMEONLY, SILENT, FNAME;
int ONLYCOUNT, num_of_matched, total_line;
char *CurrentFileName;

int build_badwordimage()
{
    int fp;
    void* pattern_buf;
    size_t pattern_imagesize;
    fp = open("etc/badword", O_RDONLY);
    if (fp==-1)
    	return -1;
    flock(fp,LOCK_EX);
    if (dashf("etc/badword.img")) {
    	flock(fp,LOCK_UN);
    	close(fp);
    	return 0;
    }
    prepf(fp,&pattern_buf,&pattern_imagesize);
   	flock(fp,LOCK_UN);
    close(fp);
    fp = open("etc/badword.img", O_WRONLY|O_TRUNC|O_CREAT,0600);
    if (fp==-1) {
        releasepf(pattern_buf);
    	return -1;
    }
    write(fp,pattern_buf,pattern_imagesize);
    close(fp);
    releasepf(pattern_buf);
    return 0;
}

static int check_badwordimg(int checkreload)
{
retry2:
    if (badword_img==NULL) {
      if (!dashf("etc/badword"))
      	return -1;
      retry:
      if (safe_mmapfile("etc/badword.img", O_RDONLY, PROT_READ, MAP_SHARED, (void **) &badword_img, &badword_img_size, NULL) == 0)
      {
        if (!dashf("etc/badword.img")) {
        	if (build_badwordimage()==0)
        		goto retry;
        }
        return -1;
      }
    }
    if (checkreload) {
    	end_mmapfile(badword_img,badword_img_size,-1);
    	badword_img=NULL;
    	checkreload=0;
    	goto retry2;
    }
    return 0;
}

static void default_setting()
{
    WHOLELINE = 0;
    NOUPPER = 1;
    INVERSE = 0;
    FILENAMEONLY = 1;
    WORDBOUND = 0;
    SILENT = 1;
    FNAME = 1;
    ONLYCOUNT = 0;

    num_of_matched = 0;
}

int check_badword(char *checkfile)
{
    char *ptr;
    int size,retv;
    void* pattern_img_ptr;
    int pattern_img_size;
    int retrycount=0;

    alarm(2);
#ifdef BBSMAIN
    signal(SIGALRM,abort_bbs);
#endif
retry:
    default_setting();
    CurrentFileName = checkfile;
    BBS_TRY {
        if (safe_mmapfile(checkfile, O_RDONLY, PROT_READ, MAP_SHARED, (void **) &ptr, &size, NULL) == 0)
	{
    alarm(0);
#ifdef BBSMAIN
    signal(SIGALRM,SIG_IGN);
#endif
            BBS_RETURN(0);
	}
        if (check_badwordimg(0)!=0)
	{
    alarm(0);
#ifdef BBSMAIN
    signal(SIGALRM,SIG_IGN);
#endif
            BBS_RETURN(0);
	}
        retv = mgrep_str(ptr, size,badword_img);
    }
    BBS_CATCH {
        if (check_badwordimg(1)!=0)
	{
    alarm(0);
#ifdef BBSMAIN
    signal(SIGALRM,SIG_IGN);
#endif
            BBS_RETURN(0);
	}
	retrycount++;
	if (retrycount==0)
	  goto retry;
    	retv=-2;
    }
    BBS_END end_mmapfile((void *) ptr, size, -1);
    alarm(0);
#ifdef BBSMAIN
    signal(SIGALRM,SIG_IGN);
#endif
    return retv;
}

int check_badword_str(char *string,int str_len)
{
    int retv;
    int retrycount=0;

    alarm(2);
#ifdef BBSMAIN
    signal(SIGALRM,abort_bbs);
#endif
    default_setting();
    CurrentFileName = "";
retry:
    BBS_TRY {
        if (check_badwordimg(0)!=0) {
    alarm(0);
#ifdef BBSMAIN
    signal(SIGALRM,SIG_IGN);
#endif
            BBS_RETURN(0);
	}
        retv = mgrep_str(string, str_len,badword_img);
    }
    BBS_CATCH {
        if (check_badwordimg(1)!=0) {
    alarm(0);
#ifdef BBSMAIN
    signal(SIGALRM,SIG_IGN);
#endif
            BBS_RETURN(0);
	}
	retrycount++;
	if (retrycount==0)
	  goto retry;
    	retv=-2;
    }
    BBS_END
    alarm(0);
#ifdef BBSMAIN
    signal(SIGALRM,SIG_IGN);
#endif
    return retv;
}

int check_filter(char *patternfile, char *checkfile,int defaultval)
{
    int fp;
    char *ptr;
    int size, retv;
    void* pattern_buf;
    size_t pattern_imagesize;

    default_setting();
    CurrentFileName = checkfile;
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
