#ifndef UV_THREAD_MNG_H_34525634543
#define UV_THREAD_MNG_H_34525634543

#include <map>
#include "thread_uv.h"
using namespace std;

class CUVThreadMng
{
public:
	
	static CUVThreadMng* GetInstance();
	void RegistThread(unsigned int nType, CUVThread *pThread);
	void UnRegistThread(unsigned int nType);
private:
	CUVThreadMng();
	~CUVThreadMng();
	map<unsigned int, CUVThread *> m_mapThread;
	CUVRWLock            m_lock;
	static CUVThreadMng* m_pMng;
	static CUVMutex      m_Mutex;
};

#endif

