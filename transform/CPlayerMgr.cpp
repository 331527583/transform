#include "CPlayerMgr.h"

CPlayerMgr* CPlayerMgr::m_pInstance = NULL;

CPlayerMgr::CPlayerMgr()
{

}


SPlayer * CPlayerMgr::add(const string &sName)
{
	MAP_PLAYER::iterator it =m_mapPlayer.find(sName);
	if (it != m_mapPlayer.end())
	{
		return NULL;
	}
	else
	{
		SPlayer *pstPlayer = new SPlayer;
		pstPlayer->sName = sName;
		m_mapPlayer[sName]=pstPlayer;
		return pstPlayer;
	}
}


bool CPlayerMgr::exist(const string &sName)
{
	MAP_PLAYER::iterator it =m_mapPlayer.find(sName);
	if (it!=m_mapPlayer.end())
	{
		return true;
	}
	else
	{
		return false;
	} 
	return false;
}


bool CPlayerMgr::del(const string &sName)
{
	MAP_PLAYER::iterator it =m_mapPlayer.find(sName);
	if (it != m_mapPlayer.end())
	{
		delete it->second;
		it->second = NULL;
		m_mapPlayer.erase(it);
	}
	return true;
}

SPlayer * CPlayerMgr::get(const string &sName)
{
	MAP_PLAYER::iterator it =m_mapPlayer.find(sName);
	if (it != m_mapPlayer.end())
	{
		return it->second;
	}
	else
	{
		return NULL;
	}
}