/******************************************************
令狐冲搜索 index 2003, 作者: bad@smth.org  Qian Wenjie
在水木清华bbs系统上运行

本游戏是自由软件，请随意复制移植
请在修改后的文件头部保留版权信息
******************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <db.h>

DB *db_file, *sdb_file, *db_word, *sdb_word, *db_index, *db_title;
DBT key, data;
int ret, t_ret;

void init_database()
{
	if ((ret = db_create(&db_file, NULL, 0)) != 0) {
		fprintf(stderr, "db_create: %s\n", db_strerror(ret));
		exit (1);
	}
	db_file->set_cachesize(db_file, 0, 5*1024*1024, 0);
	if ((ret = db_file->open(db_file, NULL, "index/file.db", NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
		db_file->close(db_file, 0);
		exit (1);
	}
	
	if ((ret = db_create(&sdb_file, NULL, 0)) != 0) {
		fprintf(stderr, "db_create: %s\n", db_strerror(ret));
		exit (1);
	}
	sdb_file->set_cachesize(sdb_file, 0, 5*1024*1024, 0);
	if ((ret = sdb_file->open(sdb_file, NULL, "index/afile.db", NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
		db_file->close(db_file, 0);
		exit (1);
	}
	
	if ((ret = db_create(&db_word, NULL, 0)) != 0) {
		fprintf(stderr, "db_create: %s\n", db_strerror(ret));
		exit (1);
	}
	db_word->set_cachesize(db_word, 0, 32*1024*1024, 0);
	if ((ret = db_word->open(db_word, NULL, "index/word.db", NULL, DB_HASH, DB_CREATE, 0664)) != 0) {
		db_file->close(db_file, 0);
		db_word->close(db_word, 0);
		exit (1);
	}
	
	if ((ret = db_create(&sdb_word, NULL, 0)) != 0) {
		fprintf(stderr, "db_create: %s\n", db_strerror(ret));
		exit (1);
	}
	sdb_word->set_cachesize(sdb_word, 0, 32*1024*1024, 0);
	if ((ret = sdb_word->open(sdb_word, NULL, "index/aword.db", NULL, DB_HASH, DB_CREATE, 0664)) != 0) {
		db_file->close(db_file, 0);
		db_word->close(db_word, 0);
		exit (1);
	}
	
	if ((ret = db_create(&db_title, NULL, 0)) != 0) {
		fprintf(stderr, "db_create: %s\n", db_strerror(ret));
		db_file->close(db_file, 0);
		db_word->close(db_word, 0);
		exit (1);
	}
	if ((ret = db_title->open(db_title, NULL, "index/title.db", NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
		db_file->close(db_file, 0);
		db_word->close(db_word, 0);
		exit (1);
	}
}

void init_index(int i)
{
    char f1[100];
    sprintf(f1, "index/index_%d.db", i);
	if ((ret = db_create(&db_index, NULL, 0)) != 0) {
		fprintf(stderr, "db_create: %s\n", db_strerror(ret));
		db_file->close(db_file, 0);
		db_word->close(db_word, 0);
		exit (1);
	}
	db_index->set_flags(db_index, DB_DUP|DB_DUPSORT);
	db_index->set_cachesize(db_index, 0, 1000*1024*1024, 0);
	if ((ret = db_index->open(db_index, NULL, f1, NULL, DB_HASH, DB_CREATE, 0664)) != 0) {
		db_file->close(db_file, 0);
		db_word->close(db_word, 0);
		db_index->close(db_index, 0);
		exit (1);
	}
}

void close_index()
{
    db_index->close(db_index, 0);
}

void close_database()
{
    db_file->close(db_file, 0);
    db_word->close(db_word, 0);
    sdb_file->close(sdb_file, 0);
    sdb_word->close(sdb_word, 0);
    db_title->close(db_title, 0);
}

char filename[250], wordname[250];

char *get_filename(int id)
{
    DBT data, key;

    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));
    key.data = &id;
    key.size = 3;
    if(db_file->get(db_file, NULL, &key, &data, 0)!=0)
        return NULL;
    else {
        memcpy(filename, data.data, data.size);
        filename[data.size] = 0;
        return filename;
    }
}

int exist_file(char * fn)
{
    DBT data, key, pkey;
    int i;
    
    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));
    
    key.data = fn;
    key.size = strlen(fn);
    if(sdb_file->get(sdb_file, NULL, &key, &data, 0)==0)
        return 1;
    else
        return 0;
}

int get_fileid(char * fn)
{
    DBT data, key, pkey;
    int i;
    
    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));
    
    key.data = fn;
    key.size = strlen(fn);
    if(sdb_file->get(sdb_file, NULL, &key, &data, 0)==0) {
        i = 0;
        memcpy(&i, data.data, 3);
        return i;
    }
    else {
        do{
            i = rand();
        }while(get_filename(i));
        key.data = &i;
        key.size = 3;
        data.data = fn;
        data.size = strlen(fn);
        db_file->put(db_file, NULL, &key, &data, 0);
        sdb_file->put(sdb_file, NULL, &data, &key, 0);
        return i;
    }
}

char *get_wordname(int id)
{
    DBT data, key;

    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));
    key.data = &id;
    key.size = 3;
    if(db_word->get(db_word, NULL, &key, &data, 0)!=0)
        return NULL;
    else {
        memcpy(wordname, data.data, data.size);
        wordname[data.size] = 0;
        return wordname;
    }
}

int get_wordid(char * fn)
{
    DBT data, key, pkey;
    int i, ret;
    
    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));
    
    key.data = fn;
    key.size = strlen(fn);
    if((ret=sdb_word->get(sdb_word, NULL, &key, &data, 0))==0) {
        i = 0;
        memcpy(&i, data.data, 3);
        return i;
    }
    else {
        do{
            i = rand();
        }while(get_wordname(i));
        key.data = &i;
        key.size = 3;
        data.data = fn;
        data.size = strlen(fn);
        db_word->put(db_word, NULL, &key, &data, 0);
        sdb_word->put(sdb_word, NULL, &data, &key, 0);
        return i;
    }
}

int exists_word(char * fn)
{
    DBT data, key, pkey;
    int i, ret;
    
    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));
    
    key.data = fn;
    key.size = strlen(fn);
    if((ret=sdb_word->get(sdb_word, NULL, &key, &data, 0))==0) {
        return 1;
    }
    else 
        return 0;
}

void init_wordlib()
{
    FILE* fp;
    int i;
    char word[100];
    fp=fopen("word.txt","r");
    for(i=0;i<130198;i++) {
//        printf("%d\r", i);
        fflush(stdout);
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
    DBT key, data;
    char buf[5000];
    int i,j,k;
    
    for(i=0;i<posst;i++) 
    if(poss[i][0]!=-1) {
        k = 0;
        memcpy(buf+k, &fileid, 3);
        k+=3;
        memcpy(buf+k, &poss[i][1], 2);
        k+=2;
        for(j=i+1;j<posst;j++)
        if(poss[i][0]==poss[j][0]) {
            memcpy(buf+k, &poss[j][1], 2);
            k+=2;
            poss[j][0] = -1;
            if(k>=100*2+3) break;
        }
        memset(&key, 0, sizeof(DBT));
        memset(&data, 0, sizeof(DBT));
        
        key.data = &poss[i][0];
        key.size = 3;
        data.data = buf;
        data.size = k;
        db_index->put(db_index, NULL, &key, &data, DB_NODUPDATA);
    }
}

void process_word(int fileid, int pos, char * word)
{
    int i,j,wordid;
    if(!word[0]||!word[1]) return;
    for(i=0;i<strlen(word);i++)
        word[i] = tolower(word[i]);
//    printf("%s\n", word);
    if(strcmp(word, "the")&&strcmp(word, "to")&&strcmp(word, "an")
        &&strcmp(word, "bbs")&&strcmp(word, "smth")&&strcmp(word, "org")
        &&strcmp(word, "as")&&strcmp(word, "at")&&strcmp(word, "do")
        &&strcmp(word, "is")) {
        wordid = get_wordid(word);
        put_word(wordid, pos);
    }
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
        if(strcmp(tmp,"了")&&strcmp(tmp,"的")&&strcmp(tmp,"地")&&
            strcmp(tmp,"得")&&strcmp(tmp,"啊")&&strcmp(tmp,"哦")&&
            strcmp(tmp,"哎")&&strcmp(tmp,"是")) {
            wordid = get_wordid(tmp);
//        printf("%s\n", tmp);
            put_word(wordid, pos+i);
        }
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
    DBT key, data;
    int fd;
    unsigned char * ptr, * pp;
    struct stat st;

    if (exist_file(fn))
        return 0;
    fileid = get_fileid(fn);
    posst = 0;
    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));
    key.data = &fileid;
    key.size = 3;
    data.data = title;
    data.size = strlen(title);
    fd = open(fn, O_RDONLY);
    if(fd==-1) return;
    if(fstat(fd, &st)<0||st.st_size<=0) {
        close(fd);
        return 0;
    }
    ptr = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    db_title->put(db_title, NULL, &key, &data, 0);
    pp = ptr;
    buft = 0;
    while(pos<st.st_size) {
        if(!c) {
            if(pos+10<st.st_size&&!strncmp(pp,"发信站: ",8)) {
                while(pp[pos++]!='\n'&&pos<st.st_size) ;
            }
            c = pp[pos++];
        }
        if(!c) break;
        
        if(c>='A'&&c<='Z'||c>='a'&&c<='z') {
            int p=pos-1;
            buf[buft++] = c;
            while(pos<st.st_size) {
                c = pp[pos++];
                if(!c) break;
                if(!(c>='A'&&c<='Z'||c>='a'&&c<='z'||c>='0'&&c<='9')) break;
                if(buft>=10) break;
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
    if(l_size >= 500*1024*1024) {
        l_size = 0;
        close_index();
        close_database();
        sprintf(buf, "cp -f index/file.db index/file_bk.db");
        system(buf);
        sprintf(buf, "cp -f index/afile.db index/afile_bk.db");
        system(buf);
        sprintf(buf, "cp -f index/word.db index/word_bk.db");
        system(buf);
        sprintf(buf, "cp -f index/aword.db index/aword_bk.db");
        system(buf);
        sprintf(buf, "cp -f index/title.db index/title_bk.db");
        system(buf);
        l_now++;
        init_database();
        init_wordlib();
        init_index(l_now);
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
    l_size = 0;
    l_now = 0;
    init_index(l_now);
    analyze_announce("0Announce/");
    close_index();
    
    close_database();
}
