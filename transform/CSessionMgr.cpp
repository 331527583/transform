#include "CSessionMgr.h"

CSessionMgr* CSessionMgr::m_pInstance = NULL;

CSessionMgr::CSessionMgr()
{

}

bool CSessionMgr::init(uint32_t dwMaxSize /* = 50000 */)
{
	vecSessions.resize(dwMaxSize,0);
	return true;
}