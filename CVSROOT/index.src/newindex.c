/******************************************************
令狐冲搜索 index 2003, 作者: bad@smth.org  Qian Wenjie
在水木清华bbs系统上运行

本软件是自由软件，请随意复制移植
请在修改后的文件头部保留版权信息
******************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#define MAX_WORD 2513257
#define MAX_FILE 15132573
#define POS_LIMIT 100000

#define SPLIT_NUM 300*1024*1024

struct data_head {
    int id;
    char * content;
} ** hash_word, ** hash_file, * word, * file, *titled;

struct index_file {
    int fileid;
    char post;
    short int * pos;
    struct index_file * next;
};

struct index_lib {
    int total;
    struct index_file * head;
} * lib;

int wordt, filet, memt=0, indext = 0;

void init_database()
{
    wordt = 0;
    filet = 0;
    word = (struct data_head *) malloc(sizeof(struct data_head)*MAX_WORD);
    file = (struct data_head *) malloc(sizeof(struct data_head)*MAX_FILE);
    titled = (struct data_head *) malloc(sizeof(struct data_head)*MAX_FILE);
    hash_word = (struct data_head **) malloc(sizeof(struct data_head *)*MAX_WORD);
//    hash_file = (struct data_head **) malloc(sizeof(struct data_head *)*MAX_FILE);
    memset(hash_word, 0, sizeof(struct data_head *)*MAX_WORD);
//    memset(hash_file, 0, sizeof(struct data_head *)*MAX_FILE);
    lib = (struct index_lib *) malloc(sizeof(struct index_lib)*MAX_WORD);
    memset(lib, 0, sizeof(struct index_lib)*MAX_WORD);
}

void output_database()
{
    int i,j;
    char c;
    short int cc;
    FILE* fp;
    char buf[20];
    char * b;
    struct index_file * p, * q;
    
    fp = fopen("_word_.index", "wb");
    fwrite(&wordt, 1, 4, fp);
    for(i=0;i<wordt;i++) {
        c = strlen(word[i].content);
        fwrite(&c, 1, 1, fp);
        fwrite(word[i].content, 1, c, fp);
    }
    fclose(fp);

    fp = fopen("_file_.index", "wb");
    fwrite(&filet, 1, 4, fp);
    for(i=0;i<filet;i++) {
        cc = strlen(file[i].content);
        fwrite(&cc, 1, 2, fp);
        fwrite(file[i].content, 1, cc, fp);
    }
    fclose(fp);
    
    fp = fopen("_title_.index", "wb");
    fwrite(&filet, 1, 4, fp);
    for(i=0;i<filet;i++) {
        c = strlen(titled[i].content);
        fwrite(&c, 1, 1, fp);
        fwrite(titled[i].content, 1, c, fp);
    }
    fclose(fp);
    
    sprintf(buf, "%d.index", indext);
    fp = fopen(buf, "wb");
    fwrite(&wordt, 1, 4, fp);
    b = (char *)malloc(21000000);
    for(i=0;i<wordt;i++) {
        int t = 0;
    	fwrite(&lib[i].total, 1, 4, fp);
    	p = lib[i].head;
    	while(p) {
    	    memcpy(b+t, &p->fileid, 4);
    	    t+=4;
    	    memcpy(b+t, &p->post, 1);
    	    t++;
    	    memcpy(b+t, p->pos, p->post*2);
    	    t+=p->post*2;
    	    free(p->pos);
    	    q = p->next;
    	    free(p);
    	    p = q;
    	}
    	fwrite(b, 1, t, fp);
    }
    free(b);
    fclose(fp);
    memset(lib, 0, sizeof(struct index_lib)*MAX_WORD);
}

char filename[250], wordname[250];

int hash_t(char * s, int t)
{
    unsigned int i,j=0;
    for(i=0;i<strlen(s);i++)
        j=(j*256+s[i])%t;
    return j;
}

char *get_filename(int id)
{
    if (id>=filet)
        return NULL;
    else {
        strcpy(filename, file[id].content);
        return filename;
    }
}

/*int exist_file(char * fn)
{
    int i;
    i = hash_t(fn, MAX_FILE);
    while(hash_file[i]) {
        if(!strcmp(hash_file[i]->content, fn))
            return 1;
        i++;
        if(i>=MAX_FILE) i=0;
    }
    return 0;
}*/

int get_fileid(char * fn)
{
    int i;
/*    i = hash_t(fn, MAX_FILE);
    while(hash_file[i]) {
        if(!strcmp(hash_file[i]->content, fn))
            return hash_file[i]->id;
        i++;
        if(i>=MAX_FILE) i=0;
    }*/
    file[filet].id = filet;
    file[filet].content = (char*)malloc(strlen(fn)+1);
    strcpy(file[filet].content, fn);
//    hash_file[i] = &(file[filet]);
    return (filet++);
}

char *get_wordname(int id)
{
    if (id>=wordt)
        return NULL;
    else {
        strcpy(wordname, word[id].content);
        return wordname;
    }
}

int exists_word(char * fn)
{
    int i;
    i = hash_t(fn, MAX_WORD);
    while(hash_word[i]) {
        if(!strcmp(hash_word[i]->content, fn))
            return 1;
        i++;
        if(i>=MAX_WORD) i=0;
    }
    return 0;
}

int get_wordid(char * fn)
{
    int i;
    i = hash_t(fn, MAX_WORD);
    while(hash_word[i]) {
        if(!strcmp(hash_word[i]->content, fn))
            return hash_word[i]->id;
        i++;
        if(i>=MAX_WORD) i=0;
    }
    word[wordt].id = wordt;
    word[wordt].content = (char*)malloc(strlen(fn)+1);
    strcpy(word[wordt].content, fn);
    hash_word[i] = &(word[wordt]);
    return (wordt++);
}

void init_wordlib()
{
    FILE* fp;
    int i;
    char word[100];
    fp=fopen("word.txt","r");
    for(i=0;i<130198;i++) {
        fscanf(fp,"%s",word);
        get_wordid(word);
    }
    fclose(fp);
}

int poss[8000][2],posst;

void put_word(int wordid, int pos)
{
    poss[posst][0] = wordid;
    poss[posst][1] = pos;
    posst++;
}

void finalize_index(int fileid)
{
    int i,j,k,u;
    struct index_file * t, * p;
    
    for(i=0;i<posst;i++) 
    if(poss[i][0]!=-1&&lib[poss[i][0]].total<POS_LIMIT) {
        u = poss[i][0];
        k = 0;
        for(j=i;j<posst;j++)
        if(u==poss[j][0]) k++;
        t = (struct index_file *)malloc(sizeof(struct index_file));
        t->fileid = fileid;
        if(k>100) k = 100;
        t->post = k;
        t->pos = (short int *)malloc(2*k);
        memt += sizeof(struct index_file)+2*k;
        k = 0;
        for(j=i;j<posst;j++)
        if(u==poss[j][0]) {
            if (k<100)
                t->pos[k++] = poss[j][1];
            poss[j][0] = -1;
        }
        lib[u].total++;
        p = lib[u].head;
        t->next = p;
        lib[u].head = t;
    }
}

void process_word(int fileid, int pos, char * word)
{
    int i,j,wordid;
    for(i=0;i<strlen(word);i++)
        word[i] = tolower(word[i]);
    wordid = get_wordid(word);
    put_word(wordid, pos);
}

void process_chinese_word(int fileid, int pos, char * word)
{
    int i,j,len=strlen(word),wordid;
    char tmp[1024];
    i=0;
    while(i<len) {
        j=len-i;
        if(j>10) j=10;
        strncpy(tmp, word+i, j);
        tmp[j] = 0;
        while(!exists_word(tmp)&&j>2) {
            j-=2;
            strncpy(tmp, word+i, j);
            tmp[j] = 0;
        }
        wordid = get_wordid(tmp);
        put_word(wordid, pos+i);
        i+=j;
    }
}

int analyze(char * title, char * fn)
{
    FILE* fp;
    int c=0;
    char buf[1024*64];
    int buft;
    int fileid, pos=0;
    int fd;
    unsigned char * ptr, * pp;
    struct stat st;

//    if (exist_file(fn))
//        return 0;
    fileid = get_fileid(fn);
    posst = 0;
    titled[fileid].content = (char*) malloc(strlen(title)+1);
    strcpy(titled[fileid].content, title);
    fd = open(fn, O_RDONLY);
    if(fd==-1) return;
    if(fstat(fd, &st)<0||st.st_size<=0) {
        close(fd);
        return 0;
    }
    ptr = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    pp = ptr;
    buft = 0;
    while(pos<st.st_size) {
        if(!c)
            c = pp[pos++];
        if(!c) break;
        
        if(c>='A'&&c<='Z'||c>='a'&&c<='z') {
            int p=pos-1;
            buf[buft++] = c;
            while(pos<st.st_size) {
                c = pp[pos++];
                if(!c) break;
                if(!(c>='A'&&c<='Z'||c>='a'&&c<='z'||c>='0'&&c<='9')) break;
                if(buft>=12) break;
                buf[buft++] = c;
            }
            buf[buft] = 0;
            process_word(fileid, p, buf);
            buft = 0;
            if(!c) break;
        }
        else if(c>='0'&&c<='9') {
            int p=pos-1;
            buf[buft++] = c;
            while(pos<st.st_size) {
                c = pp[pos++];
                if(!c) break;
                if(!(c>='0'&&c<='9')) break;
                if(buft>=5) break;
                buf[buft++] = c;
            }
            buf[buft] = 0;
            process_word(fileid, p, buf);
            buft = 0;
            if(!c) break;
        }
        else if(c>=0xb0&&c<=0xf7) {
            int ch = 1, space = 0, p = pos-1;
            buf[buft++] = c;
            while(pos<st.st_size) {
                c = pp[pos++];
                if(!c) break;
                if(ch==1) {
                    ch = 0;
                    if(c>=0xa1&&c<=0xfe)
                        buf[buft++] = c;
                    else
                        break;
                }
                else {
                    if(c>=0xb0&&c<=0xf7) {
                        if(buft>=100) break;
                        ch = 1;
                        buf[buft++] = c;
                        space = 0;
                    }
                    else if(c==' ') {
                    }
                    else if(c==10||c==13) {
                        space++;
                        if(space>=3) break;
                    }
                    else break;
                }
            }
            buf[buft/2*2] = 0;
            if(buft/2)
                process_chinese_word(fileid, p, buf);
            buft = 0;
            if(!c) break;
        }
        else {
            if(c>=0x80&&pos<st.st_size)
                pos++;
            if(c==0x1b) {
                while(pos<st.st_size) {
                    c = pp[pos++];
                    if(c>='a'&&c<='z'||c>='A'&&c<='Z') break;
                }
            }
            c = 0;
        }
        if(pos>10*1024) break;
    }
    munmap(ptr, st.st_size);
    close(fd);
    finalize_index(fileid);
    return 1;
}

void analyze_board(char * bn)
{
#define STRLEN          80
#define BM_LEN 60
#define FILENAME_LEN 20
#define OWNER_LEN 30
typedef struct fileheader {     /* This structure is used to hold data in */
    char filename[FILENAME_LEN];      /* the DIR files */
    unsigned int id, groupid, reid;
    char unused1[46];
    char innflag[2];
    char owner[OWNER_LEN];
    char unused2[38];
    unsigned int eff_size;
    time_t posttime;
    long attachment;
    char title[STRLEN];
    unsigned level;
    unsigned char accessed[12]; /* struct size = 256 bytes */
};
    struct fileheader o;

    char dfn[160];
    FILE * fp;
    int i=0;
    sprintf(dfn, "boards/%s/.DIR", bn);
    fp = fopen(dfn, "rb");
    while(!feof(fp)) {
        i++;
        fread(&o, sizeof(o), 1, fp);
        sprintf(dfn, "boards/%s/%s", bn, o.filename);
        printf("%d %s\n", i, dfn);
        fflush(stdout);
        analyze(o.title, dfn);
    }
    
    fclose(fp);
}

int l_size = 0, l_now = 0;

void analyze_announce(char * path)
{
    char dfn[320],buf[320],title[320];
    struct stat st;
    FILE * fp;
    int i=0;
    sprintf(dfn, "%s.Names", path);
    fp = fopen(dfn, "r");
    if(!fp) return;
    while(!feof(fp)) {
        if(!fgets(buf,320,fp)) break;
        if(buf[0]&&buf[strlen(buf)-1]=='\n') {
            int i=strlen(buf);
            buf[i-1]=0;
        }
        if(!strncmp(buf,"Name=",5))
            strcpy(title,buf+5);
        if(!strncmp(buf,"Path=~/",7)&&!strstr(title,"(BM: SYSOPS)")&&!strstr(title,"(BM: BMS)")) {
            if(buf[7]==0) continue;
            sprintf(dfn,"%s%s",path,buf+7);
            stat(dfn, &st);
            if(st.st_mode&S_IFDIR) {
                sprintf(dfn,"%s%s/",path,buf+7);
                analyze_announce(dfn);
            }
            else {
//                printf("%s\n", dfn);
//                fflush(stdout);
                if(analyze(title, dfn))
                    l_size += st.st_size;
            }
        }
    }
    fclose(fp);
    if(memt >= SPLIT_NUM) {
        memt = 0;
        output_database();
        indext++;
    }
}

int main(int argc, char ** argv)
{
    char fn[160];
/*    char div[9][20]={"GROUP_0","GROUP_1","GROUP_2","GROUP_3","GROUP_4","GROUP_5","GROUP_6","GROUP_7","GROUP_8"};

    char div[13][30]={"system.faq","thu.faq","sci.faq","rec.faq","literal.faq","social.faq","game.faq","sport.faq","talk.faq","info.faq","develop.faq","os.faq","comp.faq"};
*/
    int i;
    srand(time(0));
    init_database();
    init_wordlib();
    
//    analyze_board(argv[1]);
    analyze_announce("0Announce/");
    
    output_database();
}
