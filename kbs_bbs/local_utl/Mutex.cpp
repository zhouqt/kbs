#include <unistd.h>
#include <pthread.h>

#include "Mutex.h"

pthread_mutex_t g_ThreadMutexInit = PTHREAD_MUTEX_INITIALIZER;

CThreadMutex::CThreadMutex()
{
	m_nMutex = g_ThreadMutexInit;
}


CThreadMutex::~CThreadMutex()
{
	pthread_mutex_destroy(&m_nMutex);
}


int CThreadMutex::Lock()
{
	return pthread_mutex_lock(&m_nMutex);
}

int CThreadMutex::Unlock()
{
	return pthread_mutex_unlock(&m_nMutex);
}

int CThreadMutex::TestLock()
{
	return pthread_mutex_trylock(&m_nMutex);
}

CThreadCondVar::CThreadCondVar(CThreadMutex* mutex)
{
  m_objMutex = mutex;
  pthread_cond_init(&m_nCond,NULL);
}

CThreadCondVar::~CThreadCondVar()
{
	pthread_cond_destroy(&m_nCond);
}

int CThreadCondVar::BroadCast()
{
  return pthread_cond_broadcast(&m_nCond);
}

int CThreadCondVar::Signal()
{
  return pthread_cond_signal(&m_nCond);
}

int CThreadCondVar::Wait()
{
  return pthread_cond_wait(&m_nCond,m_objMutex->GetHandle());
}

