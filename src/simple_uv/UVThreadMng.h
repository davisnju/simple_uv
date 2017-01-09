#ifndef UV_THREAD_MNG_H_34525634543
#define UV_THREAD_MNG_H_34525634543

#include <map>
#include "thread_uv.h"
#include "simple_uv_export.h"
using namespace std;

class CUVThreadMng
{
public:
	
	static SUV_EXPORT CUVThreadMng* GetInstance();
	void RegistThread(unsigned int nType, CUVThread *pThread);
	void UnRegistThread(unsigned int nType);

	template<class TYPE>
	int  SendUvMessage(const TYPE& msg, size_t nMsgType, unsigned int nDstAddr, unsigned int nSrcAddr = 0)
	{
		m_lock.ReadLock();
		map<unsigned int, CUVThread *>::iterator it = m_mapThread.find(nDstAddr);

		if (it == m_mapThread.end())
		{
			m_lock.ReadUnLock();
			return -1;
		}

		it->second->PushBackMsg(nMsgType, msg, nSrcAddr);
		m_lock.ReadUnLock();

		return 0;
	}

private:
	CUVThreadMng();
	~CUVThreadMng();
	map<unsigned int, CUVThread *> m_mapThread;
	CUVRWLock            m_lock;
	static CUVThreadMng* m_pMng;
	static CUVMutex      m_Mutex;
};

#endif

