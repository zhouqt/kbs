#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <utime.h>

#define SMTH_ROOT "/home/bbs/smthAnnounce"
#define SYNC_ROOT "/home/bbs/0Announce"
#include "syncann.h"

struct _AnnounceEntry_ {
	char filepath[1024];
	char title[256];
	char owner[32];
	int  issysop;
	int  isbm;
	int  updatetime;
	int isdir;
	
	int flags;
};

struct queuenode {
	char* path;
	int ignoreold;
};

char* tags[] = {
	"Name=",
	"Path=",
	"Numb=",
	"UpdateTime=",
	NULL
};

void rotatespace(char* str)
{
	char* p;
	if (!*str) return;
	p = str+strlen(str)-1;
	while ((*p==' ')&&(p!=str)) p--;
	if (*p==' ') *p=0;
	else
		*(p+1)=0;
}

CQueue::CQueue()
{
	m_pHead = NULL;
	m_pTail = NULL;
	num = 0;
}

CQueue::~CQueue()
{
	RemoveAll();
}

void CQueue::RemoveAll()
{
	CNode	*p;
	p=m_pHead;
	while(p) {
		CNode *p1;
		p1=p;
		p=p->next;
		delete p1;
	}
	num = 0;
	m_pHead = NULL;
	m_pTail = NULL;
}

int	CQueue::IsEmpty()
{
	return (m_pHead==NULL);
}

void CQueue::Add(void* data)
{
	CNode *p=new CNode;
	p->data=data;
	p->next=NULL;
	
	if (m_pTail) {
		m_pTail->next = p;
		m_pTail = p;
	} else {
		m_pTail = p;
		m_pHead = p;
	}
	num++;
}


void* CQueue::Remove()
{
	CNode* p=m_pHead;
	void* ret;
	
	if (!p) return NULL;
	
	m_pHead = p->next;
	ret = p->data;
	
	if (m_pTail==p) m_pTail = NULL;
	delete p;
	num--;
	return ret;
}

POS CQueue::GetHeadPosition()
{
	return (void*)m_pHead;
}

void* CQueue::GetNext(POS& pos)
{
	void* data;
	if (!pos) return NULL;
	data = ((CNode*)pos)->data;
	pos = ((CNode*)pos)->next;
	
	return data;
}

int CSendApp::UpdateFile(char* filepath,time_t modtime)
{
  
	const int buflen=40960;
	char buf[buflen];
	int fd,fout;
	struct stat st;
	int len,pos,readlen;
	struct utimbuf utm;

	utm.actime=modtime;
	utm.modtime=modtime;
	
	fd = open(filepath,O_RDONLY);
	if (fd==-1) {
		printf("Can't Open File %s:%s\n",filepath,strerror(errno));
		return -1;
	}
	
	if (fstat(fd,&st)) return -1;
	len = st.st_size;
	pos=0;
	
	sprintf(buf,"%s/%s",SYNC_ROOT,filepath);
	fout = open(buf,O_CREAT|O_TRUNC|O_WRONLY,0640);
	if (fout==-1) {
		printf("Can't Open File %s:%s",buf,strerror(errno));
		return -1;
	}
	
	while ((readlen = read(fd,buf+pos,buflen-pos))>0) {
		pos+=readlen;
		if (pos==buflen) {
		  write(fout,buf,pos);
			pos = 0;
		}
	}
	if (pos) {
	  write(fout,buf,pos);
	}
	close(fd);
	close(fout);
	sprintf(buf,"%s/%s",SYNC_ROOT,filepath);
	utime(buf,&utm);
//	printf("Update File %s\n",filepath);
	return 0;
}

int CSendApp::SaveNames(FILE* file,char* dir,struct _AnnounceEntry_ * &pEntry,int count)
{
	fprintf(file,"#Directory=%s\n",dir);
	fprintf(file,"#\n%s\n#\n",m_strDirectoryTitle);
	for (int i=0;i<count;i++) {
		fprintf(file,"Name=%s\n",pEntry[i].title);
		char* p;
		for (p = pEntry[i].filepath+strlen(pEntry[i].filepath);*p!='/';p--);
		fprintf(file,"Path=~/%s\n",p+1);
		fprintf(file,"Numb=%d\n",i+1);
		fprintf(file,"UpdateTime=%d\n#\n",pEntry[i].updatetime);
	}
	return 0;
}

int CSendApp::LoadNames(FILE* file,char* dir,struct _AnnounceEntry_ * &pEntry)
{
	int count;
	char buf[1024];
	int read;
	int buflen;
	char* p;
	
	pEntry = new struct _AnnounceEntry_[1024];
	buflen = 1024;
	count = 0;
	read  = 0;
	
	memset(pEntry,0,sizeof(struct _AnnounceEntry_));
	while (fgets(buf,1024,file)) {
		int i;
		if (buf[0]=='#') {
			if (!strncmp(buf,"# Title=",8)) {
				strcpy(m_strDirectoryTitle,buf);
			};
			if (!read) continue; /*没有Entry */
			else {
				struct stat st;
				if (stat(pEntry[count].filepath,&st)) continue; /* 错误的文件或目录 */
				if (!pEntry[count].title[0]) continue; /*没有标题*/
				if (!pEntry[count].filepath[0]) continue; /*没有文件*/
				if (pEntry[count].issysop||pEntry[count].isbm) continue;
				if (!pEntry[count].updatetime)  /* 取文件更新时间 */
					pEntry[count].updatetime = st.st_mtime;
				pEntry[count].isdir = S_ISDIR(st.st_mode);
				count++;
				if (count >= buflen) {
					buflen+=1024;
					struct _AnnounceEntry_* pnewEntry = new struct _AnnounceEntry_[buflen];
					memcpy(pnewEntry,pEntry,(buflen-1024)*sizeof(struct _AnnounceEntry_));
					delete pEntry;
					pEntry = pnewEntry;
				}
				memset(&pEntry[count],0,sizeof(struct _AnnounceEntry_));
				read = 0;
				continue;
			}
		}
		if (buf[strlen(buf)-1]=='\n') buf[strlen(buf)-1] = 0;
		for (i=0;tags[i];i++)
			if (!strncmp(buf,tags[i],strlen(tags[i]))) break;
		if (!tags[i]) continue; /* 无效的tags */
		switch (i) {
		case 0:/* Names */
			strcpy(pEntry[count].title,buf+strlen(tags[i]));
			if (strlen(pEntry[count].title) > 38) {
				strcpy(pEntry[count].owner,&pEntry[count].title[38]);
/*				pEntry[count].title[39]=0;*/
			} else {
				pEntry[count].owner[0]=0;
			}
			
			rotatespace(pEntry[count].title);
			rotatespace(pEntry[count].owner);
			if (!strcmp(pEntry[count].owner,"(BM: SYSOPS)"))
				pEntry[count].issysop=1;
			if (!strcmp(pEntry[count].owner,"(BM: BMS)"))
				pEntry[count].isbm=1;
			break;
		case 1:/* path */
			p=buf+strlen(tags[i]);
			if ((*p=='~')&&(*(p+1)=='/')) p+=2;
			sprintf(pEntry[count].filepath,"%s/%s",dir,p);
			rotatespace(pEntry[count].filepath);
			break;
		case 2:/* Number */
			read = 1;
			break;
		case 3:/* UpdateTime */
			char* p;
			p=buf+strlen(tags[i]);
			pEntry[count].updatetime = atoi(p);
			break;
		}
	}
	return count;
}

int CSendApp::LogUpdate(int action,struct _AnnounceEntry_* entry,char* lastfilename)
{
	char act;
	char* p;

	if (lastfilename) {
		p=strlen(lastfilename)+lastfilename;
		while ((*p!='/')&&(p!=lastfilename)) p--;
		if (*p=='/')
			p++;
	} else
		p="";
	switch (action) {
	case ACT_ADD:
		act = '+';
		break;
	case ACT_DEL:
		act = '-';
		break;
	case ACT_MOD:
		act = '*';
		break;
	}
	fprintf(m_updateLogfile,"%c \"%s\" \"%s\" \"%s\" %d %d %d %d %d \"%s\"\n",
		act,
		entry->filepath,
		entry->title,
		entry->owner,
		0,
		entry->issysop,
		entry->isbm,
		entry->isdir,
		entry->updatetime,
		p);
	return 0;
}

int CSendApp::MakeDir(char* dir,time_t modtime)
{
	char buf[1024];
	struct utimbuf utm;
	int ret;

	utm.actime = modtime;
	utm.modtime = modtime;
	sprintf(buf,"%s/%s",SYNC_ROOT,dir);
	
	ret = mkdir(buf,0750);
	utime(buf,&utm);
	return ret;
}

int CSendApp::DoDirectory(char* dir,int ignoreold)
{
	FILE* indexfile,*logfile;
	char filepath[1024],logfilepath[1024];

//	printf("Do Directory:%s\n",dir);
	
	sprintf(filepath,"%s/.Names",dir);
	sprintf(logfilepath,"%s/%s/.Names",SYNC_ROOT,dir);
	
	if ((indexfile = fopen(filepath,"r"))==NULL) {
		printf("Can't open %s for read:%s\n",dir,strerror(errno));
		return 0;
	}
	if (!ignoreold)
		logfile = fopen(logfilepath,"rt");
	else
		logfile = NULL;

	struct _AnnounceEntry_* pEntry,*pLogEntry;
	int i,j,count,logcount;
	pLogEntry = NULL;

	count = LoadNames(indexfile,dir,pEntry);
	if (logfile) {
		logcount = LoadNames(logfile,dir,pLogEntry);
/*		printf("load save log:\n");
		for (i=0;i<logcount;i++)
			printf("%d:path:%s title:%s owner:%s issysop:%d isbm:%d udpatetime:%d\n",
			i,
			pLogEntry[i].filepath,
			pLogEntry[i].title,
			pLogEntry[i].owner,
			pLogEntry[i].issysop,
			pLogEntry[i].isbm,
			pLogEntry[i].updatetime);
*/
	}
	else {
//		printf("Not log file\n");
		logcount = 0;
	};
	
	/* 先把被删除的处理掉 */
	for (i=0;i<logcount;i++) {
		for (j=0;j<count;j++)
			if (!strcmp(pEntry[j].filepath,pLogEntry[i].filepath))
				break;
		if (j==count) { /* 被删除了 */
			pLogEntry[i].flags=1;
			LogUpdate(ACT_DEL,&pLogEntry[i]);
		}
	}
	/* 查找增加的和变化的 */
	for (i=0;i<count;i++) {
		int found;
		found = 0;
/*		printf("%d:path:%s title:%s owner:%s issysop:%d isbm:%d udpatetime:%d\n",
			i,
			pEntry[i].filepath,
			pEntry[i].title,
			pEntry[i].owner,
			pEntry[i].issysop,
			pEntry[i].isbm,
			pEntry[i].updatetime);
*/
		for (j=0;j<logcount;j++)
			if (!strcmp(pEntry[i].filepath,pLogEntry[j].filepath)) {
				found = 1;
				break;
			}
		if (found) {
			int changed = 1 , orderchange = 0;
			int k;
			if (!strcmp(pEntry[i].title,pLogEntry[j].title)&&
			    !strcmp(pEntry[i].owner,pLogEntry[j].owner)&&
			    (pEntry[i].updatetime == pLogEntry[j].updatetime))
			    	changed = 0; /* 描述没有变化 */

			for (k=i-1;k>=0;k--)
				if (!pEntry[k].flags) break; /* 跳过新建目录项 */
			if (j==0) {
				if (k!=-1) 
					orderchange=1; /* 是第一个目录，位置有变化 */
			} else
			if (k>=0) {
				if (strcmp(pEntry[k].filepath,pLogEntry[j-1].filepath)) /* 目录的顺序有变化 */
					orderchange = 1;
			}
			else /* k<0表示新的位置是第一个，但是j!=0，原来的位置不是第一个*/
				orderchange=1;

			if ((pLogEntry[j].updatetime<pEntry[i].updatetime)&&!pEntry[i].isdir)
				/* 老的目录项，但内容已经变化 */
				UpdateFile(pEntry[i].filepath,pEntry[i].updatetime);

			if (changed||orderchange)
				if (i==0)
					LogUpdate(ACT_MOD,&pEntry[i],".");
				else 
					LogUpdate(ACT_MOD,&pEntry[i],pEntry[i-1].filepath);
		} else { /* add a entry */
			if (!pEntry[i].isdir)
				UpdateFile(pEntry[i].filepath,pEntry[i].updatetime);
			else
				MakeDir(pEntry[i].filepath,pEntry[i].updatetime);
			if (i)
				LogUpdate(ACT_ADD,&pEntry[i],pEntry[i-1].filepath);
			else
				LogUpdate(ACT_ADD,&pEntry[i],".");
		}
		if (pEntry[i].isdir) { /*把需要处理的目录加入队列 */
			struct queuenode *pnode = new struct queuenode;
			char* p = new char[strlen(pEntry[i].filepath)+1];
			strcpy(p,pEntry[i].filepath);
			pnode->path = p;
			pnode->ignoreold = ignoreold?1:(!found);
			m_queueDirectory.Add(pnode);
		}
	}

	if (logfile)
		fclose(logfile);
	if ((logfile = fopen(logfilepath,"w+"))==NULL)
		printf("Can't open %s for write:%s.\n",logfilepath,strerror(errno));
	else {
		SaveNames(logfile,dir,pEntry,count);
		fclose(logfile);
	}
	fclose(indexfile);
	if (pLogEntry)
		delete pLogEntry;
	delete pEntry;
	return 0;
}

int CSendApp::Run()
{
	mkdir(SYNC_ROOT,0750);
	chdir(SMTH_ROOT);
	if ((m_updateLogfile = fopen("/tmp/AnnounceUpdate","w+")) ==NULL) {
		printf("can't open /tmp/AnnounceUpdate for write\n");
		return -1;
	};
	
	char* rootdir = new char[2];
	strcpy(rootdir,".");
	struct queuenode *pnode=new struct queuenode;
	pnode->path=rootdir;
	pnode->ignoreold = 0;
	m_queueDirectory.Add(pnode);
	while (!m_queueDirectory.IsEmpty()) {
		struct queuenode* dir = (struct queuenode*)m_queueDirectory.Remove();
		DoDirectory(dir->path,dir->ignoreold);
		delete dir->path;
		delete dir;
	}
	
	fclose(m_updateLogfile);
	
	return 0;
}

int CSendApp::OnSignalTerm()
{
	return 1;
}

int CSendApp::PreRun()
{
	return 0;
}

CSendApp::CSendApp()
{
}

main()
{
  CSendApp* app=new CSendApp();
  app->Run();
}
