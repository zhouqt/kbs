/******************************************************
令狐冲搜索 queryd 2003, 作者: bad@smth.org  Qian Wenjie
在水木清华bbs系统上运行

本游戏是自由软件，请随意复制移植
请在修改后的文件头部保留版权信息
******************************************************/

#ifndef LIB_NUMBER
#define LIB_NUMBER 16
#endif

#ifdef USE_BACKUP
#define BACKUP_SUCC "_bk"
#else
#define BACKUP_SUCC ""
#endif

#define INDEX_DIR "index/"

#define MAX_BIG_KEEP 100000
#define MAX_KEEP 100

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <db.h>
#include <math.h>

DB *db_file, *sdb_word, *db_title;
DB *db_index[LIB_NUMBER];
DB_ENV *dbenv;

int init_index(int i)
{
    char f1[100];
    int ret;
    sprintf(f1, INDEX_DIR"index_%d.db", i);
	if ((ret = db_create(&(db_index[i]), dbenv, 0)) != 0) {
	    return -1;
	}
	db_index[i]->set_flags(db_index[i], DB_DUP|DB_DUPSORT);
//	db_index[i]->set_cachesize(db_index[i], 0, 100*1024*1024, 0);
	if ((ret = db_index[i]->open(db_index[i], NULL, f1, NULL, DB_HASH, DB_RDONLY, 0664)) != 0) {
	    return -1;
	}
}

void close_index(int i)
{
    db_index[i]->close(db_index[i], 0);
}

int init_database()
{
    int ret,i;
    
    db_env_create(&dbenv, 0);
    dbenv->set_errfile(dbenv, stdout);
    dbenv->set_errpfx(dbenv, "queryd");
    dbenv->set_cachesize(dbenv, 1, 500*1024*1024, 0);
    dbenv->set_data_dir(dbenv, "/home/bbs");
    dbenv->open(dbenv, "/home/bbs", DB_CREATE|DB_INIT_MPOOL, 0);
    
	if ((ret = db_create(&db_file, dbenv, 0)) != 0) {
		return -1;
	}
//	db_file->set_cachesize(db_file, 0, 200*1024*1024, 0);
	if ((ret = db_file->open(db_file, NULL, INDEX_DIR"file"BACKUP_SUCC".db", NULL, DB_BTREE, DB_RDONLY, 0664)) != 0) {
		return -1;
	}
	
	if ((ret = db_create(&sdb_word, dbenv, 0)) != 0) {
		db_file->close(db_file, 0);
		return -1;
	}
//	sdb_word->set_cachesize(sdb_word, 0, 20*1024*1024, 0);
	if ((ret = sdb_word->open(sdb_word, NULL, INDEX_DIR"aword"BACKUP_SUCC".db", NULL, DB_HASH, DB_RDONLY, 0664)) != 0) {
		db_file->close(db_file, 0);
		return -1;
	}

	if ((ret = db_create(&db_title, dbenv, 0)) != 0) {
		db_file->close(db_file, 0);
		sdb_word->close(sdb_word, 0);
		return -1;
	}
//	sdb_word->set_cachesize(sdb_word, 0, 150*1024*1024, 0);
	if ((ret = db_title->open(db_title, NULL, INDEX_DIR"title"BACKUP_SUCC".db", NULL, DB_BTREE, DB_RDONLY, 0664)) != 0) {
		db_file->close(db_file, 0);
		sdb_word->close(sdb_word, 0);
		return -1;
	}
	for(i=0;i<LIB_NUMBER;i++)
	    init_index(i);
	return 0;
}

void close_database()
{
    int i;
	for(i=0;i<LIB_NUMBER;i++)
	    close_index(i);
    db_file->close(db_file, 0);
    sdb_word->close(sdb_word, 0);
    db_title->close(db_title, 0);
    
    dbenv->close(dbenv, 0);
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

char *get_filetitle(int id)
{
    DBT data, key;

    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));
    key.data = &id;
    key.size = 3;
    if(db_title->get(db_title, NULL, &key, &data, 0)!=0)
        return NULL;
    else {
        memcpy(filename, data.data, data.size);
        filename[data.size] = 0;
        return filename;
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
        return -1;
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

int res_id[MAX_BIG_KEEP];
short int weight[MAX_BIG_KEEP][101], res_pos[MAX_BIG_KEEP][101];
char res_title[MAX_KEEP][80],res_filename[MAX_KEEP][200];
int res_total=-1,startt=0,toomany=0;
int keys_id[100][3], keys_total=0, dbi, minus=0;

int lower(unsigned int a, unsigned int b)
{
    return ((a&0xff)<(b&0xff))||
            ((a&0xff)==(b&0xff))&&(((a>>8)&0xff)<((b>>8)&0xff))||
            ((a&0xff)==(b&0xff))&&(((a>>8)&0xff)==((b>>8)&0xff))&&(((a>>16)&0xff)<((b>>16)&0xff));
}

void process_word(int fileid, int pos, char * word)
{
    int i,j,k,t,wordid;
    unsigned short int pt[100];
    int ret,pp=0,nfirst=0;
    DBT key, data;
    DBC *dbcp;
    void* p;
    size_t retklen, retdlen;
	char *retkey, *retdata;
	db_recno_t countp;

    for(i=0;i<strlen(word);i++)
        word[i] = tolower(word[i]);
    wordid = get_wordid(word);
    if(wordid==-1) {
        if(strlen(word)>4&&!minus)
            res_total = startt;
        return;
    }
    
    if(keys_total<100&&!minus) {
        keys_id[keys_total][0] = wordid;
        keys_id[keys_total][1] = pos;
        keys_id[keys_total][2] = strlen(word);
        keys_total++;
    }
    else if(!minus) return;
    
    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));
    key.data = &wordid;
    key.size = 3;
/*    data.data = tmp;
    data.ulen = 3*1024*1024;
    data.flags = DB_DBT_USERMEM;
    ret = db_index[dbi]->get(db_index[dbi], NULL, &key, &data, DB_MULTIPLE);*/
    db_index[dbi]->cursor(db_index[dbi],NULL,&dbcp,0);
    ret=dbcp->c_get(dbcp,&key,&data,DB_SET);
//    dbcp->c_count(dbcp,&countp,0);
//    printf("%d ", countp);
    
    if(ret!=0) {
        if(!minus)
            res_total=startt;
        dbcp->c_close(dbcp);
        return;
    }
    if(res_total==-1) {
        pp=-1;
        res_total=startt;
    }
    else pp=startt;
	while(1) {
	    if(nfirst)
	        ret=dbcp->c_get(dbcp,&key,&data,DB_NEXT_DUP);
	    nfirst++;
		if (ret != 0 || nfirst>=MAX_BIG_KEEP) {
		    if(pp!=-1&&!minus) {
		        for(;pp<res_total;pp++)
		            res_id[pp] = -1;
		    }
			break;
		}
		j = 0;
		memcpy(&j, data.data, 3);
		t = (data.size-3)/2;
		memcpy(pt, data.data+3, t*2);
		if(pp==-1) {
		    if(res_total<MAX_BIG_KEEP) {
    		    res_id[res_total] = j;
    		    memcpy(&(res_pos[res_total][1]), pt, t*2);
    		    res_pos[res_total][0] = t;
    		    for(j=1;j<=t;j++)
    		    weight[res_total][j] = sqrt(res_pos[res_total][j]);
	    	    res_total++;
	    	}
	    	else break;
		}
		else {
		    while((lower(res_id[pp],j)||res_id[pp]==-1)&&pp<res_total) {
		        if(!minus)
    		        res_id[pp] = -1;
		        pp++;
		    }
		    if(pp>=res_total) break;
		    if(j==res_id[pp]&&pp<res_total) 
    		    if(minus) {
    		        res_id[pp] = -1;
    		        pp++;
    		    }
    		    else
    		    {
    		        int tt=(keys_id[keys_total-2][1]+keys_id[keys_total-2][2]==keys_id[keys_total-1][1]),tp=1,
    		            lk=keys_id[keys_total-2][2];
    		        j = 0;
    		        for(i=1;i<=res_pos[pp][0];i++)
    		        if(res_pos[pp][i]!=-1)
    		        {
    		            while(j+1<t&&abs(res_pos[pp][i]-pt[j]+lk)>abs(res_pos[pp][i]-pt[j+1]+lk)) j++;
    		            if(tt&&(res_pos[pp][i]-pt[j]+lk))
    		                res_pos[pp][i]=-1;
    		            else {
    		                weight[pp][i]+=sqrt(abs(res_pos[pp][i]-pt[j]+lk));
    		                res_pos[pp][i]=pt[j];
    		                tp = 0;
    		            }
    		        }
    		        if(tp)
    		            res_id[pp] = -1;
    		        pp++;
    		    }
		}
	}
    dbcp->c_close(dbcp);
	if(pp!=-1) {
        j = res_total;
        res_total = startt;
        for(i=startt;i<j;i++) 
        if(res_id[i]!=-1) {
            res_id[res_total] = res_id[i];
            memcpy(res_pos[res_total], res_pos[i], sizeof(res_pos[i]));
            memcpy(weight[res_total], weight[i], sizeof(weight[i]));
            res_total++;
        }
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
        process_word(fileid, pos+i, tmp);
        i+=j;
    }
}

/*void get_weight()
{
    int i,j,k,l,ret;
    char tmp[5*1024*1024];
    char buf[8];
    int pos[2][1000],wt[1000],post[2];
    DBT key,data;
    void* p;
    size_t retklen, retdlen;
	char *retkey, *retdata;

    j = res_total;
    res_total = startt;
    for(i=startt;i<j;i++) 
    if(res_id[i]!=-1) {
        res_id[res_total] = res_id[i];
        res_total++;
    }

    for(i=startt;i<res_total;i++) {
        post[0] = 0;
        for(j=0;j<keys_total;j++) {
            memset(&key, 0, sizeof(DBT));
            memset(&data, 0, sizeof(DBT));
            memcpy(buf+4, &(res_id[i]), 4);
            memcpy(buf, &(keys_id[j][0]), 4);
            key.data = buf;
            key.size = 8;
            data.data = tmp;
            data.ulen = 5*1024*1024;
            data.flags = DB_DBT_USERMEM;
            ret = db_pos->get(db_pos, NULL, &key, &data, DB_MULTIPLE);
            if(ret!=0) {
//                prints(db_strerror(ret));
//                igetkey();
                post[1] = 0;
            }
            else {
                post[1] = 0;
            	for (DB_MULTIPLE_INIT(p, &data);;) {
            		DB_MULTIPLE_NEXT(p, &data, retdata, retdlen);
            		if (p == NULL) {
            		    break;
            		}
            		memcpy(&k, retdata, 4);
            		if(post[1]<100) { //(res_total-startt)*post[1]*post[1]<1000) {
            		    pos[1][post[1]]=k;
            		    post[1]++;
            		}
            		else
            		    break;
            	}
            }
            if(j==0) {
                post[0] = post[1];
                memcpy(pos[0], pos[1], sizeof(pos[0]));
                for(k=0;k<post[0];k++)
                    wt[k] = sqrt(pos[0][k]);
            }
            else {
                for(k=0;k<post[0];k++)
                if(pos[0][k]!=-1) {
                    int l0=-1,t=pos[0][k]+keys_id[j-1][2];
                    for(l=0;l<post[1];l++)
                    if(l0==-1||abs(pos[1][l]-t)<abs(pos[1][l0]-t))
                        l0=l;
                    if(l0==-1||keys_id[j-1][1]+keys_id[j-1][2]==keys_id[j][1]&&pos[1][l0]!=t)
                        pos[0][k]=-1;
                    else {
                        pos[0][k]=pos[1][l0];
                        wt[k]+= sqrt(abs(pos[1][l0]-t));
                    }
                }
            }
        }
        k=-1;
        for(j=0;j<post[0];j++)
        if(pos[0][j]!=-1&&(k==-1||wt[j]<wt[k]))
            k=j;
        if(k==-1) res_id[i]=-1;
        else weight[i]=wt[k];
    }

    j = res_total;
    res_total = startt;
    for(i=startt;i<j;i++) 
    if(res_id[i]!=-1) {
        res_id[res_total] = res_id[i];
        res_total++;
    }
}*/

time_t t1,t2,t3,t4;
int di,dj;

void do_query_all(char * inp)
{
    FILE* fp;
    int c=0, i, j, k, t;
    char buf[1024];
    int wt[MAX_KEEP*10];
    int buft,inpt=0;
    int fileid, pos=0;

    res_total = -1; startt = 0;
    toomany = 0;
    t1 = time(0);
    
    for(dbi=0; dbi<LIB_NUMBER; dbi++) {
        buft = 0;
        inpt = 0; pos = 0; c = 0;
        keys_total = 0;
        while(inpt<strlen(inp)) {
            if(!c) {
                c = (unsigned char)inp[inpt++];
                pos++;
            }
            if(!c) break;
            
            if(c>='A'&&c<='Z'||c>='a'&&c<='z') {
                int p=pos-1;
                buf[buft++] = c;
                while(inpt<strlen(inp)) {
                    c = inp[inpt++];
                    pos++;
                    if(!c) break;
                    if(!(c>='A'&&c<='Z'||c>='a'&&c<='z'||c>='0'&&c<='9')) break;
                    if(buft>=10) break;
                    buf[buft++] = c;
                }
                buf[buft] = 0;
                process_word(fileid, p, buf);
                minus = 0;
                buft = 0;
                if(!c) break;
            }
            else if(c>=0xb0&&c<=0xf7) {
                int ch = 1, space = 0, p = pos-1;
                buf[buft++] = c;
                while(inpt<strlen(inp)) {
                    c = (unsigned char)inp[inpt++];
                    pos++;
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
                            if(space) break;
                            if(buft>=100) break;
                            ch = 1;
                            buf[buft++] = c;
                            space = 0;
                        }
                        else if(c==' ') {
                            space = 1;
                        }
                        else if(c==10||c==13) {
                            space = 0;
                        }
                        else break;
                    }
                }
                buf[buft/2*2] = 0;
                if(buft/2)
                    process_chinese_word(fileid, p, buf);
                minus = 0;
                buft = 0;
                if(!c) break;
            }
            else {
                if(c>=0x80&&inpt<strlen(inp)) inpt++; 
                if(c=='-'&&res_total!=-1) minus = 1;
                c = 0;
            }
        }
        
        startt = res_total;
        res_total = -1;
        if(startt>=MAX_BIG_KEEP) break;
    }
    t2 = time(0);

    res_total = startt;
//    if(res_total>MAX_KEEP) {
//        toomany = res_total;
//        res_total = MAX_KEEP;
//    }
    
    for(i=0;i<res_total&&i<MAX_KEEP*10;i++) {
        k = -1;
        for(j=1;j<=res_pos[i][0];j++)
        if(res_pos[i][j]!=-1&&(k==-1||weight[i][j]<weight[i][k]))
            k = j;
        if(k==-1) wt[i] = 10000000;
        else wt[i] = weight[i][k];
    }
    for(i=0;i<res_total&&i<MAX_KEEP*10;i++)
        for(j=i+1;j<res_total&&j<MAX_KEEP*10;j++)
        if(wt[i]>wt[j]) {
            k=wt[i];
            wt[i]=wt[j];
            wt[j]=k;
            k=res_id[i];
            res_id[i]=res_id[j];
            res_id[j]=k;
        }
    t3 = time(0);
    dj = 0;
    for(i=di*MAX_KEEP;i<(di+1)*MAX_KEEP&&i<res_total;i++) {
        dj=i-di*MAX_KEEP+1;
        strncpy(res_title[i-di*MAX_KEEP], get_filetitle(res_id[i]), 80);
        res_title[i-di*MAX_KEEP][79]=0;
        strncpy(res_filename[i-di*MAX_KEEP], get_filename(res_id[i]), 200);
        res_filename[i-di*MAX_KEEP][199]=0;
    }
    t4 = time(0);
}

void start_listen()
{
    struct sockaddr_in sin;
    int sockfd, csock, value;
    FILE* fp;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(4875);
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if ((bind(sockfd, (struct sockaddr *) &sin, sizeof(sin)) < 0) || (listen(sockfd, 50) < 0)) {
        return;
    }
    while(1) {
        char cmd[512];
        int i,ret;
        value = sizeof(sin);
        csock = accept(sockfd, (struct sockaddr *) &sin, (socklen_t *) &value);
        if(csock<0) continue;
        fp = fdopen(csock, "r+");
        fgets(cmd, 512, fp);
        if(cmd[0]=='\n') fgets(cmd, 512, fp);
        if(cmd[0]>='0'&&cmd[0]<='9') {
            cmd[strlen(cmd)-1] = 0;
            di=atoi(cmd);
        }
        else
            di=0;
        fgets(cmd, 512, fp);
        do_query_all(cmd);
        fprintf(fp, "%d %d %d\n", res_total, di*MAX_KEEP, dj);
        for(i=0;i<dj;i++)
        fprintf(fp, "%s\n%s\n", res_title[i], res_filename[i]);
        fflush(fp);
        fclose(fp);
        close(csock);
        
        printf("key:%sresult:%d\ntime:%d %d\n\n", cmd, res_total, t2-t1, t4-t2);
    }
}

int main()
{
    if(init_database()) return -1;
    
    start_listen();

    close_database();
}
