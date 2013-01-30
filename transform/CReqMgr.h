#ifndef __REQUEST_MGR_H__
#define __REQUEST_MGR_H__
#include "lce.h"
#include <vector>
#include <deque>
#include "package.h"

using namespace std;
using namespace lce;

struct SRequest
{
	SRequest()
		:dwId(0)
		,dwCreateTime(time(NULL))
		,dwSessionId(0)
	{}
	uint32_t dwId;
	time_t dwCreateTime;
	uint32_t dwSessionId;
	CPkgTransform oPkgReq;
	CPkgTransform oPkgResp;
};

class CRequestMgr
{
public:

	typedef std::vector<SRequest*> VEC_REQUEST;
	typedef std::deque<uint32_t> DEQ_REQNO;


public:
	~CRequestMgr(void);

	static CRequestMgr& getInstance()
	{
		if (NULL == m_pInstance){
			m_pInstance = new CRequestMgr;
		}
		return *m_pInstance;
	}

	bool init(uint32_t dwMaxReqSize = 5000);

	void eraseTimeout(uint32_t dwTimeOut = 2);

	SRequest* get(const uint32_t dwReqno);

	inline SRequest* add()
	{
		SRequest* pstRequest = new SRequest;
		if ( NULL != pstRequest )
		{

			if ( m_deqIdleReqnos.size() > 0  )
			{
				pstRequest->dwId = m_deqIdleReqnos.front();
				m_deqIdleReqnos.pop_front();
				m_vecRequests[pstRequest->dwId] = pstRequest;
			}
			else
			{
				delete pstRequest;
				pstRequest = NULL;
			}
		}
		return pstRequest;
	}


	uint32_t getReqSize() const 
	{	
		return (uint32_t)( m_dwMaxReqSize - m_deqIdleReqnos.size()-1 );	
	}

	inline void del(const uint32_t dwReqno)
	{
		if ( 0 < dwReqno && dwReqno < m_dwMaxReqSize )
		{
			if ( m_vecRequests[dwReqno] != NULL )
			{
				delete m_vecRequests[dwReqno];
				m_vecRequests[dwReqno] = NULL;
				m_deqIdleReqnos.push_back(dwReqno);
			}
		}
	}

private:
	CRequestMgr(void);
	CRequestMgr& operator=(const CRequestMgr& rhs);
	CRequestMgr(const CRequestMgr& rhs);
private:
	static CRequestMgr* m_pInstance;
	VEC_REQUEST m_vecRequests;
	DEQ_REQNO	m_deqIdleReqnos;
	uint32_t m_dwMaxReqSize;

};


#endif



