#ifndef _MUTEX_H_432532532_
#define _MUTEX_H_432532532_
#include <pthread.h>

class CMutex{
private:
public:
	CMutex(){};
	virtual ~CMutex(){};

	virtual int Lock(){return 1;};
	virtual int Unlock(){return 1;};
	virtual int TestLock(){return 1;};
};

class CThreadMutex:public CMutex {
private:
	pthread_mutex_t	m_nMutex;
public:
	CThreadMutex();
	~CThreadMutex();

	int Lock();
	int Unlock();
	int TestLock();
	
	pthread_mutex_t* GetHandle() {return &m_nMutex;};
	operator pthread_mutex_t () {return m_nMutex;};
};

class CThreadCondVar
{
private:
  CThreadMutex* m_objMutex;
  pthread_cond_t m_nCond;
public:
  CThreadCondVar(CThreadMutex* mutex);
  ~CThreadCondVar();

  int BroadCast();
  int Signal();
  int Wait();
//  TimedWait();
};

#endif

