#ifndef _RECORD_H_432_
#define _RECORD_H_432_
#define COMP_FUNC	(int(*)(void*,void*))

typedef int(*APPLY_FUNC)(void*);

class CRecordFile{
private:
	int	m_nPos;
	char*	m_strFileName;
public:
	CRecordFile(char* filename);
	~CRecordFile();
	int SearchRecord(void* data,int len,int (*func)(void*,void*),void* param);
	int GetRecord(void* data,int len,int pos);
	int SubstituteRecord(void* data,int len,int pos);
	int AppendRecord(void* data,int len);
	int ApplyRecord(int len,APPLY_FUNC fn);
};
#endif
