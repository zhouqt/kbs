#include "bbs.h"

struct key_struct *keymem=NULL;
int keymem_total;

int release_key()
{
    if(keymem) {
        free(keymem);
        keymem=NULL;
    }
    keymem_total=0;
    return 0;
}

int load_key()
{
    FILE* fp;
    char fname[STRLEN];
    struct stat st;
    release_key();
    sethomefile(fname, currentuser->userid, "definekey");
    fp=fopen(fname, "rb");
    if(fp==NULL) return -1;
    fstat(fp, &st);
    keymem_total = st.st_size/sizeof(struct key_struct);
    keymem = malloc(100*sizeof(struct key_struct));
    fread(keymem, st.st_size, 1, fp);
    fclose(fp);
    return 0;
}

int save_key()
{
    FILE* fp;
    char fname[STRLEN];
    struct stat st;
    sethomefile(fname, currentuser->userid, "definekey");
    fp=fopen(fname, "wb");
    if(fp==NULL) return -1;
    fwrite(keymem, keymem_total*sizeof(struct key_struct), 1, fp);
    ftruncate(fp, keymem_total*sizeof(struct key_struct));
    fclose(fp);
    return 0;
}

int define_key()
{
}

