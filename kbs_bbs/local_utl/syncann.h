#include "app.h"
#include "stream.h"
#include "queue.h"

enum {
	ACT_ADD,
	ACT_DEL,
	ACT_MOD
};

class CQueue {
private:
	typedef struct _node{
		void*	data;
		struct _node*	next;
	}CNode,*PCNode;
	int num;

	PCNode	m_pHead,m_pTail;
public:
	CQueue();
	~CQueue();
	
	BOOL	IsEmpty();
	
	void RemoveAll();
	
	void Add(void* data);
	void* Remove();
	
	POS GetHeadPosition();
	int GetNum() { return num;};
	void* GetNext(POS& pos);
};

class CSendApp:public CApplication
{
public:
	CSendApp();
	virtual int Run();
	virtual int OnSignalTerm();
	virtual int PreRun();
	int LoadNames(FILE* file,char* dir,struct _AnnounceEntry_ * &pEntry);
	int SaveNames(FILE* file,char* dir,struct _AnnounceEntry_ * &pEntry,int count);
	
	int LogUpdate(int action,struct _AnnounceEntry_* entry,char* lastfilename=NULL);
	int DoDirectory(char* dir,int ignoreold=0);
	int UpdateFile(char* file);
	int MakeDir(char* dir);
	
	FILE* m_updateLogfile;
	CQueue m_queueDirectory;
};
