#include "CReqMgr.h"

CRequestMgr* CRequestMgr::m_pInstance = NULL;

CRequestMgr::CRequestMgr()
{
	m_dwMaxReqSize = 5000;
}

CRequestMgr::~CRequestMgr()
{

}

bool CRequestMgr::init(uint32_t dwMaxReqSize)
{

	m_dwMaxReqSize = dwMaxReqSize;

	m_vecRequests.resize(m_dwMaxReqSize, NULL);

	for ( uint32_t i=1; i<m_dwMaxReqSize; ++i )
	{
		m_deqIdleReqnos.push_back(i);
	}

	return true;
}

SRequest* CRequestMgr::get(const uint32_t dwReqno)
{
	SRequest* pstRequest = NULL;
	if ( 0 < dwReqno && dwReqno < m_dwMaxReqSize )
	{
		pstRequest = m_vecRequests[dwReqno];
	}

	return pstRequest;
}

void CRequestMgr::eraseTimeout(uint32_t dwTimeOut)
{
	time_t dwCurTime = time(NULL);
	SRequest* pstRequest = NULL;
	for ( uint32_t i=0; i<m_dwMaxReqSize; ++i )
	{
		pstRequest = m_vecRequests[i];
		if ( NULL != pstRequest && pstRequest->dwCreateTime + dwTimeOut <= dwCurTime )
		{
            del(i);
		}
	}
}

