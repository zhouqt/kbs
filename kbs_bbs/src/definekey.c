#include "bbs.h"

extern struct key_struct *keymem;
extern int keymem_total;

#define MAX_KEY_DEFINE 100

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
    keymem = malloc(MAX_KEY_DEFINE*sizeof(struct key_struct));
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

int add_key(struct key_struct *s)
{
    if(keymem_total>=MAX_KEY_DEFINE) return -1;
    if(!keymem)
        keymem = malloc(MAX_KEY_DEFINE*sizeof(struct key_struct));
    memcpy(keymem+keymem_total*sizeof(struct key_struct), s, sizeof(struct key_struct));
    keymem_total++;
    return 0;
}

int remove_key(int i)
{
    int j;
    if(keymem_total<=0) return -1;
    for(j=i;j<keymem_total-1;j++)
        memcpy(keymem+j*sizeof(struct key_struct), keymem+(j+1)*sizeof(struct key_struct), sizeof(struct key_struct));
    keymem_total--;
    return 0;
}

int define_key()
{

}

