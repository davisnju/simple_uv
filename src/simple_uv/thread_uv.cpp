#include "thread_uv.h"
#include "UVThreadMng.h"

void CUVThread::PushBackMsg(NodeMsg *msg)
{
	m_lock.WriteLock();
	if (!m_pMsgTail)
	{
		msg->next = msg;
		m_pMsgTail = msg;
	}
	else
	{
		msg->next = m_pMsgTail->next;
		m_pMsgTail->next = msg;
		m_pMsgTail = msg;
	}
	m_lock.WriteUnLock();
}

void CUVThread::ThreadFun(void* arg)
{
	CUVThread *pThread = (CUVThread *)arg;

	if (!pThread->OnInit())
	{
		pThread->Run();
	}	

	uv_thread_join(&pThread->m_thread);
}

void CUVThread::Run()
{
	NodeMsg* req;
	NodeMsg* first;
	NodeMsg* next;

	while (true)
	{
		if (m_pMsgTail == nullptr)
		{
			uv_thread_sleep(100);
			continue;
		}

		m_lock.WriteLock();
		first = m_pMsgTail->next;
		next = first;
		m_pMsgTail = nullptr;
		m_lock.WriteUnLock();

		while (next != nullptr) {
			req = next;
			next = req->next != first ? req->next : nullptr;

			this->OnDispatchMsg(req->m_nMsgType, req->m_pData, req->m_nSrcAddr);

			delete req;
			req = nullptr;
		}
	}
}

void CUVThread::OnUvThreadMessage(CRegistMsg msg, unsigned int nSrcAddr)
{
	// m_mapThread[msg.m_nType] = (CUVThread *)(msg.m_pData);
	m_mapThread->insert(map<unsigned int, CUVThread*>::value_type(msg.m_nType, (CUVThread *)(msg.m_pData)));
}

void CUVThread::OnUvThreadMessage( unsigned int msg, unsigned int nSrcAddr )
{
	map<unsigned int, CUVThread *>::iterator it = m_mapThread->find(msg);

	if (it != m_mapThread->end())
	{
		m_mapThread->erase(it);
	}
}
int CUVThread::OnInit()
{
	CUVThreadMng::GetInstance()->RegistThread(m_nThreadType, this);

	return 0;
}

void CUVThread::OnExit()
{
	CUVThreadMng::GetInstance()->UnRegistThread(m_nThreadType);

	for (map<unsigned int, CUVThread *>::iterator it = m_mapThread->begin();
		it != m_mapThread->end(); )
	{
		m_mapThread->erase(it++);
	}

}

void CUVThread::Start()
{
	if (m_bIsRunning) {
		return;
	}
	uv_thread_create(&m_thread, ThreadFun, this);
	m_bIsRunning = true;
}

