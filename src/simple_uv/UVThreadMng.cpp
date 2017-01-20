// #include "stdafx.h"
#include "UVThreadMng.h"
#include "BaseMsgDefine.h"

CUVThreadMng* CUVThreadMng::m_pMng = nullptr;
CUVMutex CUVThreadMng::m_Mutex;

CUVThreadMng* CUVThreadMng::GetInstance()
{
	if (!m_pMng)
	{
		m_Mutex.Lock();

		if (!m_pMng)
		{
			m_pMng = new CUVThreadMng;
		}
		
		m_Mutex.UnLock();
	}

	return m_pMng;
}

void CUVThreadMng::RegistThread(unsigned int nType, CUVThread *pThread)
{
	m_lock.WriteLock();
	

	m_mapThread[nType] = pThread;

	for (map<unsigned int, CUVThread *>::iterator it = m_mapThread.begin();
	it != m_mapThread.end(); ++it)
	{
		CRegistMsg msg;
		msg.m_nType = nType;
		msg.m_pData = pThread;
		it->second->PushBackMsg(REGIST_THREAD_MSG, msg);
	}


	m_lock.WriteUnLock();
}

void CUVThreadMng::UnRegistThread(unsigned int nType)
{
	m_lock.WriteLock();
	map<unsigned int, CUVThread *>::iterator it = m_mapThread.find(nType);

	if (it != m_mapThread.end())
	{
		m_mapThread.erase(it);
	}

	for (it = m_mapThread.begin();
		it != m_mapThread.end(); ++it)
	{
		it->second->PushBackMsg(UN_REGIST_THREAD_MSG, nType);
	}

	m_lock.WriteUnLock();
}

CUVThreadMng::CUVThreadMng()
	: m_pHandleAsync(nullptr)
{
}


CUVThreadMng::~CUVThreadMng()
{
}

void CUVThreadMng::RegistHandle( uv_async_t *handle )
{
	m_pHandleAsync = handle;
}

void CUVThreadMng::UnRegistHandle()
{
	m_pHandleAsync = nullptr;
}


