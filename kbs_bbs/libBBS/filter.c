#include "bbs.h"

#ifdef FILTER
extern int prepf(int fp,void** patternbuf,size_t* patt_image_len);
extern int mgrep(int fp,void* patternbuf,session_t* session);
extern int mgrep_str(char* data,int len,void* patternbuf,session_t* session);
extern void releasepf(void* patternbuf);

static char* badword_img=NULL;
static time_t badimg_time;
static off_t badword_img_size;
static int build_badwordimage()
{
    int fp,imgfp;
    void* pattern_buf;
    size_t pattern_imagesize;
    struct stat st;
    fp = open("etc/badword", O_RDWR);
    if (fp==-1)
        return -1;
    writew_lock(fp, 0, SEEK_SET, 0);
    if (dashf(BADWORD_IMG_FILE)) {
        un_lock(fp, 0, SEEK_SET, 0);
        close(fp);
        return 0;
    }
    prepf(fp,&pattern_buf,&pattern_imagesize);

    imgfp = open(BADWORD_IMG_FILE, O_WRONLY|O_TRUNC|O_CREAT,0600);
    if (imgfp==-1) {
        releasepf(pattern_buf);
        return -1;
    }
    write(imgfp,pattern_buf,pattern_imagesize);
    stat(BADWORD_IMG_FILE,&st);
    badimg_time=st.st_mtime;
    close(imgfp);
    un_lock(fp, 0, SEEK_SET, 0);
    close(fp);
    releasepf(pattern_buf);
    return 0;
}

static int check_badwordimg(int checkreload)
{
    struct stat st;
    stat(BADWORD_IMG_FILE,&st);
    if ((badword_img!=NULL)&&(badimg_time!=st.st_mtime))
        checkreload=1;
    if (checkreload) {
        if (badword_img)
            end_mmapfile(badword_img,badword_img_size,-1);
        badword_img=NULL;
        checkreload=0;
    }
    if (badword_img==NULL) {
        badimg_time=st.st_mtime;
        if (!dashf("etc/badword"))
            return -1;
retry:
        if (safe_mmapfile(BADWORD_IMG_FILE, O_RDONLY, PROT_READ, MAP_SHARED, &badword_img, &badword_img_size, NULL) == 0) {
            if (!dashf(BADWORD_IMG_FILE)) {
                if (build_badwordimage()==0)
                    goto retry;
            }
            return -1;
        }
    }
    return 0;
}

static void default_setting(session_t* session)
{
    WHOLELINE = 0;
    NOUPPER = 1;
    INVERSE = 0;
    FILENAMEONLY = 1;
    WORDBOUND = 0;
    SILENT = 1;
    FNAME = 1;
    ONLYCOUNT = 0;

    session->num_of_matched = 0;
}

int check_badword(char *checkfile, int len, session_t* session)
{
    char *ptr;
    off_t size;
    int check_size;
    int retv;
    int retrycount=0;

retry:
    default_setting(session);
    session->CurrentFileName = checkfile;
    BBS_TRY {
        if (safe_mmapfile(checkfile, O_RDONLY, PROT_READ, MAP_SHARED, &ptr, &size, NULL) == 0) {
            BBS_RETURN(0);
        }
        if (check_badwordimg(0)!=0) {
            end_mmapfile((void *) ptr, size, -1);
            BBS_RETURN(0);
        }
        check_size = len;
        if (check_size <= 0 || check_size > size) check_size = size;
        retv = mgrep_str(ptr, check_size,badword_img, session);
    }
    BBS_CATCH {
        if (check_badwordimg(1)!=0) {
            end_mmapfile((void *) ptr, size, -1);
            BBS_RETURN(0);
        }
        retrycount++;
        if (retrycount==0)
            goto retry;
        retv=-2;
    }
    BBS_END;
    end_mmapfile((void *) ptr, size, -1);
    return retv;
}

int check_badword_str(char *string,int str_len, session_t* session)
{
    int retv;
    int retrycount=0;

    default_setting(session);
    session->CurrentFileName = "";
retry:
    BBS_TRY {
        if (check_badwordimg(0)!=0) {
            BBS_RETURN(0);
        }
        retv = mgrep_str(string, str_len,badword_img,session);
    }
    BBS_CATCH {
        if (check_badwordimg(1)!=0) {
            BBS_RETURN(0);
        }
        retrycount++;
        if (retrycount==0)
            goto retry;
        retv=-2;
    }
    BBS_END;
    return retv;
}
int check_filter(char *patternfile, char *checkfile,int defaultval, session_t* session)
{
    int fp;
    char *ptr;
    off_t size;
    int retv;
    void* pattern_buf;
    size_t pattern_imagesize;

    default_setting(session);
    session->CurrentFileName = checkfile;
    fp = open(patternfile, O_RDONLY);
    prepf(fp,&pattern_buf,&pattern_imagesize);
    BBS_TRY {
        if (safe_mmapfile(checkfile, O_RDONLY, PROT_READ, MAP_SHARED, &ptr, &size, NULL) == 0) {
            close(fp);
            BBS_RETURN(0);
        }
        retv = mgrep_str(ptr, size,pattern_buf, session);
    }
    BBS_CATCH {
        retv=defaultval;
    }
    BBS_END;
    end_mmapfile((void *) ptr, size, -1);
    close(fp);
    releasepf(pattern_buf);
    return retv;
}
#endif

