/******************************************************
令狐冲搜索 queryd 2003, 作者: bad@smth.org  Qian Wenjie
在水木清华bbs系统上运行

本游戏是自由软件，请随意复制移植
请在修改后的文件头部保留版权信息
******************************************************/

#ifndef LIB_NUMBER
#define LIB_NUMBER 32
#endif

#define MAX_BIG_KEEP 100000
#define MAX_KEEP 100

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#define MAX_WORD 2513257
#define MAX_FILE 15132573
#define POS_LIMIT 100000

struct data_head {
    int id;
    char * content;
} ** hash_word, * word, * file, *titled;

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
unsigned int * libs;
FILE * fps[LIB_NUMBER];
int limits[LIB_NUMBER][2];
char boards[1000][30];
int boardsl[1000][2];
int boardt=0;

int hash_t(char * s, int t)
{
    unsigned int i,j=0;
    for(i=0;i<strlen(s);i++)
        j=(j*256+s[i])%t;
    return j;
}

int init_database()
{
    FILE * fp, * outfp;
    int i,j,k,t,p,l,o,lc;
    unsigned char c, buf[20];
    char last[100]="";
    short int cc;
    
    wordt = 0;
    filet = 0;
    word = (struct data_head *) malloc(sizeof(struct data_head)*MAX_WORD);
    file = (struct data_head *) malloc(sizeof(struct data_head)*MAX_FILE);
    titled = (struct data_head *) malloc(sizeof(struct data_head)*MAX_FILE);
    hash_word = (struct data_head **) malloc(sizeof(struct data_head *)*MAX_WORD);
    memset(hash_word, 0, sizeof(struct data_head *)*MAX_WORD);
    libs = (int *) malloc(4*LIB_NUMBER*MAX_WORD);
    memset(libs, 0, 4*LIB_NUMBER*MAX_WORD);
    
    fp = fopen("_word_.index", "rb");
    fread(&wordt, 1, 4, fp);
    for(i=0;i<wordt;i++) {
        fread(&c, 1, 1, fp);
        word[i].id = i;
        word[i].content = (char*) malloc(c+1);
        word[i].content[c] = 0;
        fread(word[i].content, 1, c, fp);
        j = hash_t(word[i].content, MAX_WORD);
        while(hash_word[j]) {j++; if(j>=MAX_WORD) j=0;}
        hash_word[j] = &word[i];
    }
    fclose(fp);

    fp = fopen("_file_.index", "rb");
    fread(&filet, 1, 4, fp);
    for(i=0;i<filet;i++) {
        fread(&cc, 1, 2, fp);
        file[i].id = i;
        file[i].content = (char*) malloc(cc+1);
        file[i].content[cc] = 0;
        fread(file[i].content, 1, cc, fp);
        if(!strncmp("0Announce/groups/",file[i].content,17)) {
            int j,k;
            char now[300];
            j=17;
            while(file[i].content[j]!='/') j++;
            j++; k=j;
            while(file[i].content[k]!='/') k++;
            strcpy(now, file[i].content+j);
            now[k-j] = 0;
            if(strcmp(now,last)) {
                if(last[0]) {
                    boardsl[boardt-1][1] = i;
                    strcpy(boards[boardt], now);
                    boardsl[boardt][0] = i;
                }
                else {
                    strcpy(boards[boardt], now);
                    boardsl[boardt][0] = i;
                }
                boardt++;
            }
            strcpy(last, now);
        }
        else {
            if(last[0]) {
                last[0] = 0;
                boardsl[boardt][1] = i;
            }
        }
    }
    if(last[0]) boardsl[boardt][1] = i;
    fclose(fp);
    
    fp = fopen("_title_.index", "rb");
    fread(&filet, 1, 4, fp);
    for(i=0;i<filet;i++) {
        fread(&c, 1, 1, fp);
        titled[i].id = i;
        titled[i].content = (char*) malloc(c+1);
        titled[i].content[c] = 0;
        fread(titled[i].content, 1, c, fp);
    }
    fclose(fp);
    
    fp=fopen("_all_.index","rb");
    if(fp==NULL) {
        for(i=0;i<LIB_NUMBER;i++) {
            limits[i][0] = filet;
            limits[i][1] = 0;
            sprintf(buf, "%d.index", i);
            fp = fopen(buf, "rb");
            fread(&t, 1, 4, fp);
            for(j=0;j<t;j++) {
                libs[i*MAX_WORD+j] = ftell(fp);
                fread(&k, 1, 4, fp);
                while(k) {
                    k--;
                    fread(&o, 1, 4, fp);
                    if(o<limits[i][0]) limits[i][0] = o;
                    if(o>limits[i][1]) limits[i][1] = o;
                    fread(&c, 1, 1, fp);
                    for(o=0;o<(unsigned char)c;o++) fread(&l, 1, 2, fp);
                }
            }
            fps[i] = fp;
            printf("%ld\n",ftell(fp));
        }
        fp = fopen("_all_.index", "wb");
        fwrite(libs,1,4*LIB_NUMBER*MAX_WORD,fp);
        fwrite(limits,1,sizeof(limits),fp);
        fclose(fp);
    }
    else {
        fread(libs,1,4*LIB_NUMBER*MAX_WORD,fp);
        fread(limits,1,sizeof(limits),fp);
        fclose(fp);
        for(i=0;i<LIB_NUMBER;i++) {
            sprintf(buf, "%d.index", i);
            fps[i] = fopen(buf, "rb");
        }
    }
}

void close_database()
{
}

char filename[250], wordname[250];

char *get_filename(int id)
{
    if (id>=filet) {
        filename[0] = 0;
        return filename;
    }
    else {
        strcpy(filename, file[id].content);
        return filename;
    }
}

char *get_filetitle(int id)
{
    if (id>=filet) {
        filename[0] = 0;
        return filename;
    }
    else {
        strcpy(filename, titled[id].content);
        return filename;
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
    return -1;
}

int res_id[MAX_BIG_KEEP];
short int weight[MAX_BIG_KEEP][101], res_pos[MAX_BIG_KEEP][101];
char res_title[MAX_KEEP][80],res_filename[MAX_KEEP][200];
int res_total=-1,startt=0,toomany=0,boa=-1;
int keys_id[100][3], keys_total=0, dbi, minus=0;

void process_word(int fileid, int pos, char * word)
{
    int i,j,k,t,li,lt,wordid;
    unsigned short int pt[100];
    int ret,pp=0,nfirst=0;
    struct index_file * ltt;
    char buf[20];
    FILE * fp;

    for(i=0;i<strlen(word);i++)
        word[i] = tolower(word[i]);
    wordid = get_wordid(word);
    if(wordid==-1||libs[dbi*MAX_WORD+wordid]==0) {
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
    
    fp = fps[dbi];
    fseek(fp, libs[dbi*MAX_WORD+wordid], SEEK_SET);
    fread(&lt, 1, 4, fp);
    ltt = (struct index_file *) malloc(sizeof(struct index_file)*lt);
    for(i=0;i<lt;i++) {
        fread(&ltt[i].fileid, 1, 4, fp);
        fread(&ltt[i].post, 1, 1, fp);
        j = ltt[i].post;
        if(j<0) j+=256;
        ltt[i].pos = (short int *) malloc(j*2);
        fread(ltt[i].pos, 1, j*2, fp);
    }
    
    if(lt==0) {
        if(!minus)
            res_total=startt;
        return;
    }
    if(res_total==-1) {
        pp=-1;
        res_total=startt;
    }
    else pp=startt;
    for(li=0;li<lt;li++) {
	    nfirst++;
		if (nfirst>=MAX_BIG_KEEP) {
		    if(pp!=-1&&!minus) {
		        for(;pp<res_total;pp++)
		            res_id[pp] = -1;
		    }
			break;
		}
		j = ltt[li].fileid;
		t = ltt[li].post;
                if(t<0) t+=256;
                if(t>100) t=100;
		memcpy(pt, ltt[li].pos, t*2);
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
		    while((res_id[pp]>j||res_id[pp]==-1)&&pp<res_total) {
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
    for(li=0;li<lt;li++)
        free(ltt[li].pos);
	free(ltt);
    if(pp!=-1&&!minus) {
        for(;pp<res_total;pp++)
            res_id[pp] = -1;
    }
	if(pp!=-1||boa!=-1) {
        j = res_total;
        res_total = startt;
        for(i=startt;i<j;i++) 
        if(res_id[i]!=-1&&(boa==-1||(res_id[i]>=boardsl[boa][0]&&res_id[i]<boardsl[boa][1]))) {
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

time_t t1,t2,t3,t4;
int di,dj;

int intersect(int l1,int l2,int l3,int l4)
{
    return (l1<=l3&&l3<=l2)||(l1<=l4&&l4<=l2)||(l3<=l1&&l2<=l4);
}

void do_query_all(char * inp)
{
    FILE* fp;
    int c=0, i, j, k, t;
    char buf[1024];
    int wt[MAX_KEEP*10];
    int buft,inpt=0;
    int fileid, pos=0;

    res_total = -1; startt = 0;
    toomany = 0; boa = -1;
    t1 = time(0);
    if(inp[0]=='=') {
        for(i=0;i<boardt;i++)
            if(!strncasecmp(boards[i], inp+1, strlen(boards[i]))&&inp[strlen(boards[i])+1]==' ') {
                strcpy(buf, inp);
                strcpy(inp, buf+2+strlen(boards[i]));
                boa = i;
                break;
            }
    }
    
    for(dbi=0; dbi<LIB_NUMBER; dbi++) 
    if(boa==-1||intersect(limits[dbi][0],limits[dbi][1],boardsl[boa][0],boardsl[boa][1]-1)){
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
                    if(buft>=12) break;
                    buf[buft++] = c;
                }
                buf[buft] = 0;
                process_word(fileid, p, buf);
                minus = 0;
                buft = 0;
                if(!c) break;
            }
            else if(c>='0'&&c<='9') {
                int p=pos-1;
                buf[buft++] = c;
                while(inpt<strlen(inp)) {
                    c = inp[inpt++];
                    pos++;
                    if(!c) break;
                    if(!(c>='0'&&c<='9')) break;
                    if(buft>=5) break;
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
    FILE* fp, *qfp;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(4875);
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if ((bind(sockfd, (struct sockaddr *) &sin, sizeof(sin)) < 0) || (listen(sockfd, 50) < 0)) {
        return;
    }
    qfp = fopen("queryall.txt", "a");
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
        fprintf(qfp, "%s", cmd);
        fflush(qfp);
        do_query_all(cmd);
        fprintf(fp, "%d %d %d\n", res_total, di*MAX_KEEP, dj);
        for(i=0;i<dj;i++)
        fprintf(fp, "%s\n%s\n", res_title[i], res_filename[i]);
        fflush(fp);
        fclose(fp);
        close(csock);
        
        printf("key:%sresult:%d\ntime:%d\n\n", cmd, res_total, t4-t1);
    }
    fclose(qfp);
}

int main()
{
    init_database();
    
    start_listen();

    close_database();
}
