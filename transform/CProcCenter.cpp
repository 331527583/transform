#include "CProcCenter.h"
#include "CPlayerMgr.h"
#include "CRoomMgr.h"
#include "CReqMgr.h"
#include "CSessionMgr.h"


CProcCenter*  CProcCenter::m_pInstance = NULL;

CProcCenter::CProcCenter(void)
{

}

CProcCenter::~CProcCenter(void)
{

}


void CProcCenter::getRoomsFromXml(TiXmlElement *root,MAP_ROOM_INFO &mapRoomInfos)
{
	TiXmlElement *childNode = root->FirstChildElement();

	while( childNode )
	{
		string sType = childNode->Value();

		TiXmlElement *infoNode = childNode->FirstChildElement();
		while(infoNode)
		{

			SRoomInfo stInfo;
			stInfo.wMaxNum = 0;

			infoNode->QueryStringAttribute("id",&stInfo.sRoomId);
			infoNode->QueryIntAttribute("max_num",(int*)&stInfo.wMaxNum);

			if(!stInfo.sRoomId.empty())
			{
				mapRoomInfos[sType].push_back(stInfo);
			}

			infoNode = infoNode->NextSiblingElement();
		}

		childNode =childNode->NextSiblingElement();
	}
}


int CProcCenter::init()
{
    m_stConfig = CConfigMgr::getInstance().getConfig();

    if(CTask::init(m_stConfig.wThreadNum,m_stConfig.dwTaskQueueSize) < 0)
    {
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"task init error");
        return -1;
    }

	if(CRequestMgr::getInstance().init() < 0)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"request init  error");
		return -2;
	}

	if(CSessionMgr::getInstance().init(50000) < 0)
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"session init error");
		return -3;
	}

	TiXmlDocument doc(m_stConfig.sRoomsXmlPath);

	if(!doc.LoadFile())
	{
		snprintf(m_szErrMsg,sizeof(m_szErrMsg),"load xml data error");
		return -4;
	}

	//从XML获取房间信息，并初始化系统房间

	TiXmlElement *roomNode = doc.RootElement();
	getRoomsFromXml(roomNode,m_mapRoomsInfo);

	for(MAP_ROOM_INFO::iterator it = m_mapRoomsInfo.begin();it!=m_mapRoomsInfo.end();++it)
	{

		for(VEC_ROOM_INFO::iterator it2=it->second.begin();it2 != it->second.end();++it2)
		{
			SRoom *pstRoom = CRoomMgr::getInstance().add(it->first,it2->sRoomId);
			if(pstRoom)
			{
				pstRoom->wMaxNum = it2->wMaxNum;
			}
		}
	}

	CCommMgr::getInstance().addTimer(TIMER_SESSION_TIMEOUT,TIME_OUT_CHECK,this);

    return 0;
}


void CProcCenter::onRead(SSession &stSession,const char * pszData, const int iSize)
{
	updateSession(stSession);

	SHead * pstHead =(SHead*)pszData;
	m_pstSession = &stSession;

	uint16_t wCmd = pstHead->getCmd();

	LOG_INFO("Ip:%s,SessionId:%d,Cmd:%d",stSession.getStrIp().c_str(),stSession.iFd,wCmd);

	switch(wCmd)
	{
	case CMD_LOGIN:
		{
			praseLoginPkg(pszData,iSize);
			break;
		}
	case CMD_JOIN_ROOM:
		{
			praseJoinRoomPkg(pszData,iSize);
			break;
		}
	case CMD_EXIT_ROOM:
		{
			praseExitRoomPkg(pszData,iSize);
			break;
		}
	case CMD_KICK_ROOM:
		{
			praseKickRoomPkg(pszData,iSize);
			break;
		}
	case CMD_BROADCAST_ROOM:
		{
			praseBroadcastRoomPkg(pszData,iSize);
			break;
		}
	case CMD_BROADCAST_SERVER:
		{
			praseBroadcastServerPkg(pszData,iSize);
			break;
		}
	case CMD_BROADCAST_SOMEBODY:
		{
			praseBroadcastSomebodyPkg(pszData,iSize);
			break;
		}
	case CMD_HEART_BEAT:
		{
			praseHeartBeatPkg(pszData,iSize);
			break;
		}
	case CMD_QUICK_START:
		{
			praseQuickStartPkg(pszData,iSize);
			break;
		}
	case CMD_GET_ROOMS_INFO:
		{
			praseGetRoomsInfo(pszData,iSize);
			break;
		}
	case CMD_GET_ROOM_INFO:
		{
			praseGetRoomsInfo(pszData,iSize);
			break;
		}
	default:
		{
			LOG_ERROR("Ip:%s,SessionId:%d,Invalid CMD:%d",stSession.getStrIp().c_str(),stSession.iFd,wCmd);
		}

	}
}

void CProcCenter::onClose(SSession &stSession)
{
	LOG_INFO("Ip:%s,SessionId:%d close",stSession.getStrIp().c_str(),stSession.iFd);

	SSession *pstSession = CSessionMgr::getInstance().get(stSession.iFd);
	if(pstSession->pData != NULL)
	{
		SPlayer *pstPlayer = (SPlayer *)pstSession->pData;

		if(pstPlayer->sRoomId.size() > 0)
		{
			LOG_INFO("player:%s,exit from roomId:%s",pstPlayer->sName.c_str(),pstPlayer->sRoomId.c_str());

			CRoomMgr::getInstance().exit(pstPlayer->sRoomType,pstPlayer->sRoomId,pstPlayer->sName);
			CRoomMgr::getInstance().changeRoomer(pstPlayer->sRoomType,pstPlayer->sRoomId,pstPlayer->sName);

			pushChangeRoomData(pstPlayer->sRoomType,pstPlayer->sRoomId);
		}
		CPlayerMgr::getInstance().del(pstPlayer->sName);
		CSessionMgr::getInstance().del(stSession.iFd);
	}

}


void CProcCenter::onConnect(SSession &stSession,bool bOk)
{

	if(bOk)
	{

		lce::setNODelay(stSession.iFd);

		SSession *pstSession = CSessionMgr::getInstance().add(stSession.iFd);

		if(!pstSession)
		{
			LOG_INFO("onConnect Ip:%s,SessionId:%d add fail",stSession.getStrIp().c_str(),stSession.iFd);
			return;
		}

		(*pstSession) = stSession;

		LOG_INFO("Ip:%s,SessionId:%d connected",stSession.getStrIp().c_str(),stSession.iFd);
	}
	else
	{
		LOG_INFO("Ip:%s,SessionId:%d connect fail",stSession.getStrIp().c_str(),stSession.iFd);
	}

}

void CProcCenter::onError(SSession &stSession,const char * szErrMsg,int iError)
{
	LOG_ERROR("onError %s",szErrMsg);
}

void CProcCenter::onTimer(uint32_t dwTimerId,void *pData)
{

	if(dwTimerId == TIMER_SESSION_TIMEOUT)
	{
		vector<int> *pvecSessionIds = new vector<int>;
		dispatch(MSG_SESSION_TIMEOUT,pvecSessionIds);
	}
}


void CProcCenter::onMessage(int dwMsgType,void *pData)
{
	
	if(dwMsgType == MSG_SESSION_TIMEOUT)
	{
		
		vector<int> vecSessionIds = *(vector<int>*)pData;

		for(size_t i = 0;i<vecSessionIds.size();i++)
		{
			SSession *pstSession = CSessionMgr::getInstance().get(vecSessionIds[i]);
			if(pstSession)
			{
				onClose(*pstSession);
				CCommMgr::getInstance().close(*pstSession);
			}
			
		}
		delete (vector<int>*)pData;
	}

}

//线程函数 对于共享资源需加锁
void CProcCenter::onWork(int iTaskType,void *pData,int iIndex)
{

	if(iTaskType == MSG_SESSION_TIMEOUT)
	{
		vector<int> *pvecSessionIds = (vector<int>*)pData;

		CSessionMgr::VEC_SESSION &vecSession = CSessionMgr::getInstance().all();

		time_t dwTimeNow = time(0);
		for(size_t i = 0;i<vecSession.size();i++)
		{
			SSession *pstSession = vecSession[i];

			if(pstSession)
			{
				if(dwTimeNow - pstSession->dwBeginTime > m_stConfig.dwSessionTimeOut)
				{
					pvecSessionIds->push_back(i);
				}
			}
		}
		CCommMgr::getInstance().sendMessage(MSG_SESSION_TIMEOUT,this,pvecSessionIds);
	}
}

int CProcCenter::praseLoginPkg(const char * pszData, const int iSize)
{
	try
	{
		SRequest *pstRequest = CRequestMgr::getInstance().add();
		pstRequest->dwSessionId = m_pstSession->iFd;

		pstRequest->oPkgReq.decodeJSON(pszData,iSize);
		
		string sName = pstRequest->oPkgReq["name"];
		string sPwd = pstRequest->oPkgReq["pwd"];
	
		onLogin(sName,sPwd,pstRequest->dwId);
	}
	catch(const CAnyValue::Error e)
	{
		LOG_ERROR("package decode error: %s",e.what());
	}

	return 0;
}


int CProcCenter::praseJoinRoomPkg(const char * pszData, const int iSize)
{

	try
	{
		SRequest *pstRequest = CRequestMgr::getInstance().add();
		pstRequest->dwSessionId = m_pstSession->iFd;

		pstRequest->oPkgReq.decodeJSON(pszData,iSize);

		string sName = pstRequest->oPkgReq["player"];
		string sType = pstRequest->oPkgReq["type"];
		string sRoomId = pstRequest->oPkgReq["id"];

		onJoinRoom(sName,sType,sRoomId,pstRequest->dwId);
	}
	catch(const CAnyValue::Error e)
	{
		LOG_ERROR("package decode error: %s",e.what());
	}
	return 0;
}

int CProcCenter::praseExitRoomPkg(const char * pszData, const int iSize)
{
	try
	{
		SRequest *pstRequest = CRequestMgr::getInstance().add();
		pstRequest->dwSessionId = m_pstSession->iFd;

		pstRequest->oPkgReq.decodeJSON(pszData,iSize);

		string sName = pstRequest->oPkgReq["player"];
		string sType = pstRequest->oPkgReq["type"];
		string sRoomId = pstRequest->oPkgReq["id"];

		onExitRoom(sName,sType,sRoomId,pstRequest->dwId);
	}
	catch(const CAnyValue::Error e)
	{
		LOG_ERROR("package decode error: %s",e.what());
	}
	return 0;
}

int CProcCenter::praseQuickStartPkg(const char * pszData, const int iSize)
{
	try
	{
		SRequest *pstRequest = CRequestMgr::getInstance().add();
		pstRequest->dwSessionId = m_pstSession->iFd;

		pstRequest->oPkgReq.decodeJSON(pszData,iSize);

		string sName = pstRequest->oPkgReq["player"];
		string sType = pstRequest->oPkgReq["type"];
		onQuickStart(sName,sType,pstRequest->dwId);
	}
	catch(const CAnyValue::Error e)
	{
		LOG_ERROR("package decode error: %s",e.what());
	}
	return 0;
}

int CProcCenter::praseKickRoomPkg(const char * pszData, const int iSize)
{
	try
	{
		SRequest *pstRequest = CRequestMgr::getInstance().add();
		pstRequest->dwSessionId = m_pstSession->iFd;

		pstRequest->oPkgReq.decodeJSON(pszData,iSize);

		string sName = pstRequest->oPkgReq["player"];
		string sRoomer = pstRequest->oPkgReq["roomer"];
		string sType = pstRequest->oPkgReq["type"];
		string sRoomId = pstRequest->oPkgReq["id"];

		onKickRoom(sRoomer,sName,sType,sRoomId,pstRequest->dwId);
	}
	catch(const CAnyValue::Error e)
	{
		LOG_ERROR("package decode error: %s",e.what());
	}
	return 0;
}



int CProcCenter::praseBroadcastRoomPkg(const char * pszData, const int iSize)
{
	try
	{
		SRequest *pstRequest = CRequestMgr::getInstance().add();
		pstRequest->dwSessionId = m_pstSession->iFd;

		pstRequest->oPkgReq.decodeJSON(pszData,iSize);

		string sName = pstRequest->oPkgReq["player"];
		string sType = pstRequest->oPkgReq["type"];
		string sRoomId = pstRequest->oPkgReq["id"];
		string sCmd = pstRequest->oPkgReq["cmd"];

		onBroadcastRoom(sName,sType,sRoomId,sCmd,pstRequest->dwId);
	}
	catch(const CAnyValue::Error e)
	{
		LOG_ERROR("package decode error: %s",e.what());
	}
	return 0;
}

int CProcCenter::praseBroadcastServerPkg(const char * pszData, const int iSize)
{
	try
	{
		SRequest *pstRequest = CRequestMgr::getInstance().add();
		pstRequest->dwSessionId = m_pstSession->iFd;

		pstRequest->oPkgReq.decodeJSON(pszData,iSize);

		string sName = pstRequest->oPkgReq["player"];
		string sCmd = pstRequest->oPkgReq["cmd"];
		onBroadcastServer(sName,sCmd,pstRequest->dwId);
	}
	catch(const CAnyValue::Error e)
	{
		LOG_ERROR("package decode error: %s",e.what());
	}
	return 0;
}


int CProcCenter::praseBroadcastSomebodyPkg(const char * pszData, const int iSize)
{

	try
	{
		SRequest *pstRequest = CRequestMgr::getInstance().add();
		pstRequest->dwSessionId = m_pstSession->iFd;

		pstRequest->oPkgReq.decodeJSON(pszData,iSize);

		string sName = pstRequest->oPkgReq["player"];
		string sCmd = pstRequest->oPkgReq["cmd"];

		CAnyValue oReceivers(pstRequest->oPkgReq["receivers"]);
		set<string> setReceivers;

		for(size_t i =0;i<oReceivers.size();i++)
		{
			setReceivers.insert((string)oReceivers[i]);
		}

		onBroadcastSomebody(sName,setReceivers,sCmd,pstRequest->dwId);
	}
	catch(const CAnyValue::Error e)
	{
		LOG_ERROR("package decode error: %s",e.what());
	}

	return 0;
}

int CProcCenter::praseHeartBeatPkg(const char * pszData, const int iSize)
{
	SRequest *pstRequest = CRequestMgr::getInstance().add();
	pstRequest->dwSessionId = m_pstSession->iFd;
	pstRequest->oPkgResp["ret"] = 0;
	pstRequest->oPkgResp["msg"] = "ok";

	reResult(pstRequest->dwId);

	return 0;
}

int CProcCenter::praseGetRoomsInfo(const char * pszData, const int iSize)
{
	try
	{
		SRequest *pstRequest = CRequestMgr::getInstance().add();
		pstRequest->dwSessionId = m_pstSession->iFd;

		pstRequest->oPkgReq.decodeJSON(pszData,iSize);

		string sType = pstRequest->oPkgReq["type"];

		onGetRoomsInfo(sType,pstRequest->dwId);
	}
	catch(const CAnyValue::Error e)
	{
		LOG_ERROR("package decode error: %s",e.what());
	}
	return 0;
}

int CProcCenter::praseGetRoomInfo(const char * pszData, const int iSize)
{
	try
	{
		SRequest *pstRequest = CRequestMgr::getInstance().add();
		pstRequest->dwSessionId = m_pstSession->iFd;

		pstRequest->oPkgReq.decodeJSON(pszData,iSize);

		string sRoomId = pstRequest->oPkgReq["id"];
		string sType = pstRequest->oPkgReq["type"];

		onGetRoomInfo(sType,sRoomId,pstRequest->dwId);
	}
	catch(const CAnyValue::Error e)
	{
		LOG_ERROR("package decode error: %s",e.what());
	}
	return 0;
}


int CProcCenter::onLogin(const string &sName,const string &sPwd,uint32_t dwReqId)
{

	LOG_INFO("onLogin player:%s",sName.c_str());

	SRequest *pstRequest = CRequestMgr::getInstance().get(dwReqId);
	if(pstRequest == NULL)
		return 0;

	pstRequest->oPkgResp["ret"] = 0;
	pstRequest->oPkgResp["msg"] = "ok";


	bool bFlag1 = CPlayerMgr::getInstance().exist(sName);

	if(bFlag1)
	{
		pstRequest->oPkgResp["ret"] = 1;
		pstRequest->oPkgResp["msg"] = "user have login";
		reResult(dwReqId);
		return 0;
	}


	SPlayer *pstPlayer = CPlayerMgr::getInstance().add(sName);

	if(!pstPlayer)
	{
		pstRequest->oPkgResp["ret"] = 2;
		pstRequest->oPkgResp["msg"] = "user have login";
		reResult(dwReqId);
		return 0;
	}

	pstPlayer->dwLoginTime = time(NULL);
	pstPlayer->dwSessionId = pstRequest->dwSessionId;

	CSessionMgr::getInstance().get(pstRequest->dwSessionId)->pData = pstPlayer;

	reResult(dwReqId);

	return 0;
}

int CProcCenter::onJoinRoom(const string &sName,const string &sType,const string &sRoomId,uint32_t dwReqId)
{
	LOG_INFO("onJoinRoom player:%s,roomId:%s",sName.c_str(),sRoomId.c_str());

	bool bFlag1 = CPlayerMgr::getInstance().exist(sName);
	bool bFlag2 = CRoomMgr::getInstance().exist(sType,sRoomId);

	SRequest *pstRequest = CRequestMgr::getInstance().get(dwReqId);

	pstRequest->oPkgResp["ret"] = 0;
	pstRequest->oPkgResp["msg"] = "ok";


	if(!bFlag1)
	{
		pstRequest->oPkgResp["ret"] = 1;
		pstRequest->oPkgResp["msg"] = "invalid player";
		reResult(dwReqId);
		LOG_ERROR("invalid Player %s",sName.c_str());
		return 0;
	}

	if(!bFlag2)
	{
		pstRequest->oPkgResp["ret"] = 2;
		pstRequest->oPkgResp["msg"] = "invalid RoomId";
		reResult(dwReqId);
		LOG_ERROR("invalid RoomId %s",sRoomId.c_str());
		return 0;
	}

	SRoom *pstRoom = CRoomMgr::getInstance().get(sType,sRoomId);

	if(pstRoom->setPlayers.size() >= pstRoom->wMaxNum)
	{
		pstRequest->oPkgResp["ret"] = 3;
		pstRequest->oPkgResp["msg"] = "room is full";
		reResult(dwReqId);
		return 0;
	}

	SPlayer *pstPlayer= CPlayerMgr::getInstance().get(sName);
	pstPlayer->sRoomId = pstRoom->sRoomId;
	pstPlayer->sRoomType = sType;

	//如果房间没人，第一个进入的人就是房主


	if(pstRoom->setPlayers.empty())
	{
		pstRoom->sRoomer = sName;
		LOG_INFO("roomer is %s",pstRoom->sRoomer.c_str());
	}

	pstRoom->setPlayers.insert(sName);

	reResult(dwReqId);
	pushChangeRoomData(sType,sRoomId);

	return 0;
}

int CProcCenter::onExitRoom(const string &sName,const string &sType,const string &sRoomId,uint32_t dwReqId)
{

	LOG_INFO("onExitRoom player:%s,roomId:%s",sName.c_str(),sRoomId.c_str());

	bool bFlag1 = CPlayerMgr::getInstance().exist(sName);
	bool bFlag2 = CRoomMgr::getInstance().exist(sType,sRoomId);

	SRequest *pstRequest = CRequestMgr::getInstance().get(dwReqId);

	pstRequest->oPkgResp["ret"] = 0;
	pstRequest->oPkgResp["msg"] = "ok";


	if(!bFlag1)
	{
		pstRequest->oPkgResp["ret"] = 1;
		pstRequest->oPkgResp["msg"] = "invalid player";
		reResult(dwReqId);
		LOG_ERROR("invalid Player %s",sName.c_str());
		return 0;
	}

	if(!bFlag2)
	{
		pstRequest->oPkgResp["ret"] = 2;
		pstRequest->oPkgResp["msg"] = "invalid RoomId";
		reResult(dwReqId);
		LOG_ERROR("invalid RoomId %s",sRoomId.c_str());
		return 0;
	}

	bool bFlag3 = CRoomMgr::getInstance().isInRoom(sType,sRoomId,sName);

	if(!bFlag3)
	{
		pstRequest->oPkgResp["ret"] =3;
		pstRequest->oPkgResp["msg"] = "player not in room";
		reResult(dwReqId);
		LOG_ERROR("player:%s not in room roomId %s",sName.c_str(),sRoomId.c_str());
		return 0;
	}

	CRoomMgr::getInstance().exit(sType,sRoomId,sName);
	CRoomMgr::getInstance().changeRoomer(sType,sRoomId,sName);

	reResult(dwReqId);
	pushChangeRoomData(sType,sRoomId);

	return 0;
}


int CProcCenter::onKickRoom(const string &sKicker,const string &sName,const string &sType,const string &sRoomId,uint32_t dwReqId)
{
	LOG_INFO("onExitRoom player:%s,roomId:%s",sName.c_str(),sRoomId.c_str());

	bool bFlag1 = CPlayerMgr::getInstance().exist(sName);
	bool bFlag2 = CRoomMgr::getInstance().exist(sType,sRoomId);

	SRequest *pstRequest = CRequestMgr::getInstance().get(dwReqId);

	pstRequest->oPkgResp["ret"] = 0;
	pstRequest->oPkgResp["msg"] = "ok";


	if(!bFlag1)
	{
		pstRequest->oPkgResp["ret"] = 1;
		pstRequest->oPkgResp["msg"] = "invalid player";
		reResult(dwReqId);
		LOG_ERROR("invalid Player %s",sName.c_str());
		return 0;
	}

	if(!bFlag2)
	{
		pstRequest->oPkgResp["ret"] = 2;
		pstRequest->oPkgResp["msg"] = "invalid RoomId";
		reResult(dwReqId);
		LOG_ERROR("invalid RoomId %s",sRoomId.c_str());
		return 0;
	}

	bool bFlag3 = CRoomMgr::getInstance().isInRoom(sType,sRoomId,sName);

	if(!bFlag3)
	{
		pstRequest->oPkgResp["ret"] =3;
		pstRequest->oPkgResp["msg"] = "player not in room";
		reResult(dwReqId);
		LOG_ERROR("player:%s not in room roomId %s",sName.c_str(),sRoomId.c_str());
		return 0;
	}

	bool bFlag4 = CRoomMgr::getInstance().isInRoom(sType,sRoomId,sKicker);

	if(!bFlag4)
	{
		pstRequest->oPkgResp["ret"] =4;
		pstRequest->oPkgResp["msg"] = "roomer not in room";
		reResult(dwReqId);
		LOG_ERROR("player:%s not in room roomId %s",sName.c_str(),sRoomId.c_str());
		return 0;
	}

	SPlayer *pstPlayer = CPlayerMgr::getInstance().get(sKicker);

	if(pstPlayer->cPrivilege < CPlayerMgr::PRIV_VIP)
	{
		pstRequest->oPkgResp["ret"] = 5;
		pstRequest->oPkgResp["msg"] = "you are not a vip";
		reResult(dwReqId);
		LOG_ERROR("player:%s not a vip roomId %s",sName.c_str(),sRoomId.c_str());
		return 0;
	}

	CRoomMgr::getInstance().exit(sType,sRoomId,sName);
	CRoomMgr::getInstance().changeRoomer(sType,sRoomId,sName);

	reResult(dwReqId);
	pushKickRoomData(sKicker,sName,sType,sRoomId);
	pushChangeRoomData(sType,sRoomId);
	return 0;
}

int CProcCenter::onGetRoomsInfo(const string &sType,uint32_t dwReqId)
{
	CRoomMgr::MAP_ROOM *pmapRooms = CRoomMgr::getInstance().rooms(sType);

	SRequest *pstRequest = CRequestMgr::getInstance().get(dwReqId);

	pstRequest->oPkgResp["ret"] = 0;
	pstRequest->oPkgResp["msg"] = "ok";


	if(!pmapRooms)
	{
		pstRequest->oPkgResp["ret"] = 1;
		pstRequest->oPkgResp["msg"] = "not found rooms type";
		reResult(dwReqId);
		return 0;
	}

	pstRequest->oPkgResp["type"]=sType;

	for(CRoomMgr::MAP_ROOM::iterator it=pmapRooms->begin();it!=pmapRooms->end();++it)
	{
		CAnyValue oValue;
		oValue["id"]=it->second->sRoomId;
		oValue["max_num"]=it->second->wMaxNum;
		oValue["ctype"]=it->second->cType;
		oValue["curr_num"]=it->second->setPlayers.size();
		oValue["roomer"]=it->second->sRoomer;
		pstRequest->oPkgResp["infos"].push_back(oValue);
	}

	reResult(dwReqId);

	return 0;

}

int CProcCenter::onGetRoomInfo(const string &sType,const string &sRoomId,uint32_t dwReqId)
{
	SRequest *pstRequest = CRequestMgr::getInstance().get(dwReqId);
	pstRequest->oPkgResp["ret"] = 0;
	pstRequest->oPkgResp["msg"] = "ok";

	bool bFlag1 = CRoomMgr::getInstance().exist(sType,sRoomId);

	if(!bFlag1)
	{
		pstRequest->oPkgResp["ret"] = 2;
		pstRequest->oPkgResp["msg"] = "invalid RoomId";
		reResult(dwReqId);
		LOG_ERROR("invalid RoomId %s",sRoomId.c_str());
		return 0;
	}

	SRoom *pstRoom = CRoomMgr::getInstance().get(sType,sRoomId);

	pstRequest->oPkgResp["info"]["id"] = pstRoom->sRoomId;
	pstRequest->oPkgResp["info"]["max_num"] = pstRoom->wMaxNum;
	pstRequest->oPkgResp["info"]["ctype"] = pstRoom->cType;
	pstRequest->oPkgResp["info"]["roomer"]= pstRoom->sRoomer;
	pstRequest->oPkgResp["info"]["curr_num"] = pstRoom->setPlayers.size();

	reResult(dwReqId);

	return 0;

}

int CProcCenter::pushChangeRoomData(const string &sType,const string &sRoomId)
{

	//广播数据到客户端

	SRoom *pstRoom = CRoomMgr::getInstance().get(sType,sRoomId);

	CPkgTransform oPkg;
	oPkg["id"]=sRoomId;
	oPkg["max_num"]=pstRoom->wMaxNum;
	oPkg["ctype"]=pstRoom->cType;
	oPkg["type"]=sType;
	oPkg["roomer"]=pstRoom->sRoomer;


	for(set<string>::iterator it = pstRoom->setPlayers.begin();it!=pstRoom->setPlayers.end();++it)
	{
		SPlayer *pstPlayer = CPlayerMgr::getInstance().get(*it);
		CAnyValue oValue;
		oValue["name"]=pstPlayer->sName;
		oValue["login_time"]=pstPlayer->dwLoginTime;
		oValue["privilege"] = pstPlayer->cPrivilege;
		oPkg["players"].push_back(oValue);
	}

	oPkg.encodeJSON();
	oPkg.head().setStx();
	oPkg.head().setCmd(CMD_CHANGE_ROOM_DATA);
	oPkg.head().setLen(oPkg.size()+1);
	oPkg.setEtx();

	for(set<string>::iterator it = pstRoom->setPlayers.begin();it!=pstRoom->setPlayers.end();++it)
	{

		SPlayer *pstPlayer = CPlayerMgr::getInstance().get(*it);

		if(!pstPlayer)
		{
			LOG_ERROR("pushChangeRoomData not found player:%s",it->c_str());
			continue;
		}

		SSession *pstSession = CSessionMgr::getInstance().get(pstPlayer->dwSessionId);

		if(!pstSession)
		{
			LOG_ERROR("pushChangeRoomData not found session:%ul",pstPlayer->dwSessionId);
			continue;
		}

		CCommMgr::getInstance().write(*pstSession,oPkg.data(),oPkg.size(),false);
		LOG_INFO("push change room data to %s",it->c_str());

	}
	return 0;
}
int CProcCenter::pushKickRoomData(const string &sRoomer,const string &sName,const string & sType,const string &sRoomId)
{
	//广播数据到客户端

	CPkgTransform oPkg;
	oPkg["id"]=sRoomId;
	oPkg["type"]=sType;
	oPkg["player"]=sRoomer;
	oPkg.encodeJSON();
	oPkg.head().setStx();
	oPkg.head().setCmd(CMD_KICK_ROOM_DATA);
	oPkg.head().setLen(oPkg.size()+1);
	oPkg.setEtx();

	SPlayer *pstPlayer = CPlayerMgr::getInstance().get(sName);

	if(!pstPlayer)
	{
		LOG_ERROR("pushKickRoomData not found player:%s",sName.c_str());
		return 0;
	}

	SSession *pstSession = CSessionMgr::getInstance().get(pstPlayer->dwSessionId);

	if(!pstSession)
	{
		LOG_ERROR("pushKickRoomData not found session:%ul",pstPlayer->dwSessionId);
		return 0;
	}

	CCommMgr::getInstance().write(*pstSession,oPkg.data(),oPkg.size(),false);

	LOG_INFO("push data kick to %s",sName.c_str());

	return 0;
}

int CProcCenter::onBroadcastRoom(const string &sName,const string & sType,const string &sRoomId,const string &sCmd,uint32_t dwReqId)
{
	LOG_INFO("onBroadcastRoom player:%s,roomId:%s",sName.c_str(),sRoomId.c_str());

	bool bFlag1 = CPlayerMgr::getInstance().exist(sName);
	bool bFlag2 = CRoomMgr::getInstance().exist(sType,sRoomId);

	SRequest *pstRequest = CRequestMgr::getInstance().get(dwReqId);

	CAnyValue oValue(pstRequest->oPkgReq["params"]);


	pstRequest->oPkgResp["ret"] = 0;
	pstRequest->oPkgResp["msg"] = "ok";


	if(!bFlag1)
	{
		pstRequest->oPkgResp["ret"] = 1;
		pstRequest->oPkgResp["msg"] = "invalid player";
		reResult(dwReqId);
		LOG_ERROR("invalid Player %s",sName.c_str());
		return 0;
	}

	if(!bFlag2)
	{
		pstRequest->oPkgResp["ret"] = 2;
		pstRequest->oPkgResp["msg"] = "invalid RoomId";
		reResult(dwReqId);
		LOG_ERROR("invalid RoomId %s",sRoomId.c_str());
		return 0;
	}


	bool bFlag3 = CRoomMgr::getInstance().isInRoom(sType,sRoomId,sName);

	if(!bFlag3)
	{
		pstRequest->oPkgResp["ret"] =3;
		pstRequest->oPkgResp["msg"] = "player not in room";
		reResult(dwReqId);
		LOG_ERROR("player:%s not in room roomId %s",sName.c_str(),sRoomId.c_str());
		return 0;
	}
	
	pushDataToRoom(sName,sType,sRoomId,sCmd,oValue);

	reResult(dwReqId);
	return 0;

}

int CProcCenter::pushDataToRoom(const string &sName,const string & sType,const string &sRoomId,const string &sCmd,const CAnyValue &oData)
{

	//广播数据到客户端

	SRoom *pstRoom = CRoomMgr::getInstance().get(sType,sRoomId);

	CPkgTransform oPkg;
	oPkg["id"]=sRoomId;
	oPkg["type"]=sType;
	oPkg["player"]=sName;
	oPkg["cmd"]=sCmd;
	oPkg["params"]=oData;
	oPkg.encodeJSON();
	oPkg.head().setStx();
	oPkg.head().setCmd(CMD_BROADCAST_ROOM_DATA);
	oPkg.head().setLen(oPkg.size()+1);
	oPkg.setEtx();

	for(set<string>::iterator it = pstRoom->setPlayers.begin();it!=pstRoom->setPlayers.end();++it)
	{
		//不发送给自己
		if((*it) == sName)
		{
			continue;
		}

		SPlayer *pstPlayer = CPlayerMgr::getInstance().get(*it);

		if(!pstPlayer)
		{
			LOG_ERROR("pushDataToRoom not found player:%s",it->c_str());
			continue;
		}

		SSession *pstSession = CSessionMgr::getInstance().get(pstPlayer->dwSessionId);

		if(!pstSession)
		{
			LOG_ERROR("pushDataToRoom not found session:%ul",pstPlayer->dwSessionId);
			continue;
		}

		CCommMgr::getInstance().write(*pstSession,oPkg.data(),oPkg.size(),false);
		LOG_INFO("push broadcast data to %s",it->c_str());

	}
	return 0;

}

int CProcCenter::onBroadcastServer(const string &sName,const string &sCmd,uint32_t dwReqId)
{
	LOG_INFO("onBroadcastServer player:%s",sName.c_str());

	bool bFlag1 = CPlayerMgr::getInstance().exist(sName);

	SRequest *pstRequest = CRequestMgr::getInstance().get(dwReqId);
	CAnyValue oValue(pstRequest->oPkgReq["params"]);


	pstRequest->oPkgResp["ret"] = 0;
	pstRequest->oPkgResp["msg"] = "ok";


	if(!bFlag1)
	{
		pstRequest->oPkgResp["ret"] = 1;
		pstRequest->oPkgResp["msg"] = "invalid player";
		reResult(dwReqId);
		LOG_ERROR("invalid Player %s",sName.c_str());
		return 0;
	}

	SPlayer *pstPlayer = CPlayerMgr::getInstance().get(sName);
	if(pstPlayer->cPrivilege < CPlayerMgr::PRIV_ADMIN)
	{
		pstRequest->oPkgResp["ret"] = 2;
		pstRequest->oPkgResp["msg"] = "you are not a admin";
		reResult(dwReqId);
		LOG_ERROR("%s are not a admin",sName.c_str());
		return 0;
	}


	//全服推送数据
	pushDataToServer(sName,sCmd,oValue);

	reResult(dwReqId);
	return 0;
}


int CProcCenter::pushDataToServer(const string &sName,const string &sCmd,const CAnyValue &oData)
{
	CPlayerMgr::MAP_PLAYER &mapPlayers = CPlayerMgr::getInstance().all();

	CPkgTransform oPkg;
	oPkg["player"]=sName;
	oPkg["cmd"]=sCmd;
	oPkg["params"]=oData;
	oPkg.encodeJSON();
	oPkg.head().setStx();
	oPkg.head().setCmd(CMD_BROADCAST_SERVER_DATA);
	oPkg.head().setLen(oPkg.size()+1);
	oPkg.setEtx();


	for(CPlayerMgr::MAP_PLAYER::iterator it = mapPlayers.begin();it != mapPlayers.end();++it)
	{
		
		SSession *pstSession = CSessionMgr::getInstance().get(it->second->dwSessionId);

		if(!pstSession)
		{
			LOG_ERROR("pushDataToServer not found session:%ul",it->second->dwSessionId);
			continue;
		}

		CCommMgr::getInstance().write(*pstSession,oPkg.data(),oPkg.size(),false);
		LOG_INFO("push broadcast data to %s",it->first.c_str());
	}

	return 0;
}

int CProcCenter::onBroadcastSomebody(const string &sName,const set<string> &setReceivers,const string &sCmd,uint32_t dwReqId)
{
	LOG_INFO("onBroadcastSomebody player:%s",sName.c_str());

	bool bFlag1 = CPlayerMgr::getInstance().exist(sName);

	SRequest *pstRequest = CRequestMgr::getInstance().get(dwReqId);
	CAnyValue oValue(pstRequest->oPkgReq["params"]);


	pstRequest->oPkgResp["ret"] = 0;
	pstRequest->oPkgResp["msg"] = "ok";


	if(!bFlag1)
	{
		pstRequest->oPkgResp["ret"] = 1;
		pstRequest->oPkgResp["msg"] = "invalid player";
		reResult(dwReqId);
		LOG_ERROR("invalid Player %s",sName.c_str());
		return 0;
	}

	if(setReceivers.size() == 0)
	{
		pstRequest->oPkgResp["ret"] = 3;
		pstRequest->oPkgResp["msg"] = "receivers is empty";
		reResult(dwReqId);
		LOG_ERROR("%s are not a admin",sName.c_str());
		return 0;
	}

	pushDataToSomebody(sName,setReceivers,sCmd,oValue);

	reResult(dwReqId);
	return 0;
}

int CProcCenter::pushDataToSomebody(const string &sName,const set<string> &setReceivers,const string &sCmd,const CAnyValue &oData)
{

	//广播数据到客户端

	CPkgTransform oPkg;

	oPkg["player"]=sName;
	oPkg["params"]=oData;
	oPkg["cmd"]=sCmd;
	oPkg.encodeJSON();
	oPkg.head().setStx();
	oPkg.head().setCmd(CMD_BROADCAST_SOMEBODY_DATA);
	oPkg.head().setLen(oPkg.size()+1);
	oPkg.setEtx();

	for(set<string>::iterator it = setReceivers.begin();it!=setReceivers.end();++it)
	{
		
		SPlayer *pstPlayer = CPlayerMgr::getInstance().get(*it);

		if(!pstPlayer)
		{
			LOG_ERROR("pushDataToSomebody not found player:%s",it->c_str());
			continue;
		}

		SSession *pstSession = CSessionMgr::getInstance().get(pstPlayer->dwSessionId);

		if(!pstPlayer)
		{
			LOG_ERROR("pushDataToSomebody not found session:%ul",pstPlayer->dwSessionId);
			continue;
		}

		CCommMgr::getInstance().write(*pstSession,oPkg.data(),oPkg.size(),false);
		LOG_INFO("push data to %s",it->c_str());

	}
	return 0;

}

int CProcCenter::updateSession(const SSession &stSession)
{
	SSession *pstSession = CSessionMgr::getInstance().get(stSession.iFd);

	if(!pstSession)
	{
		LOG_ERROR("updateSession not found seesion:%ul",stSession.iFd);
		return 0;
	}
	
	pstSession->dwBeginTime = stSession.dwBeginTime;

	return 0;
}

int CProcCenter::reResult(uint32_t dwReqId,bool bClose)
{

	SRequest *pstRequest = CRequestMgr::getInstance().get(dwReqId);
	if(pstRequest == NULL)
		return 0;

	pstRequest->oPkgResp.encodeJSON();
	pstRequest->oPkgResp.sethead(pstRequest->oPkgReq.head());
	pstRequest->oPkgResp.head().setLen(pstRequest->oPkgResp.size()+1);
	pstRequest->oPkgResp.setEtx();

	SSession *pstSession = CSessionMgr::getInstance().get(pstRequest->dwSessionId);

	CCommMgr::getInstance().write(*pstSession,pstRequest->oPkgResp.data(),pstRequest->oPkgResp.size(),bClose);
	
	CRequestMgr::getInstance().del(dwReqId);

	return 0;

}

int CProcCenter::onQuickStart(const string &sName,const string &sType,uint32_t dwReqId)
{


	LOG_INFO("onQuickStart player:%s",sName.c_str());

	CRoomMgr::MAP_ROOM *pmapRooms = CRoomMgr::getInstance().rooms(sType);

	SRequest *pstRequest = CRequestMgr::getInstance().get(dwReqId);

	pstRequest->oPkgResp["ret"] = 0;
	pstRequest->oPkgResp["msg"] = "ok";


	if(!pmapRooms)
	{
		pstRequest->oPkgResp["ret"] = 1;
		pstRequest->oPkgResp["msg"] = "not found rooms type";
		reResult(dwReqId);
		return 0;
	}

	string sEmptyRoomId;
	string sRoomId;

	for(CRoomMgr::MAP_ROOM::iterator it=pmapRooms->begin();it!=pmapRooms->end();++it)
	{
		if(it->second->setPlayers.size() >0 && it->second->setPlayers.size() < it->second->wMaxNum)
		{
			sRoomId = it->second->sRoomId;
			break;
		}
		else if(it->second->setPlayers.size() == 0 && sEmptyRoomId.empty())
		{
			sEmptyRoomId = it->second->sRoomId;
		}
	}
	
	if(sRoomId.empty()) sRoomId = sEmptyRoomId;

	if(sRoomId.empty())
	{
		pstRequest->oPkgResp["ret"] = 2;
		pstRequest->oPkgResp["msg"] = "all room is full";
		reResult(dwReqId);
		return 0;
	}

	SRoom *pstRoom = CRoomMgr::getInstance().get(sType,sRoomId);

	SPlayer *pstPlayer= CPlayerMgr::getInstance().get(sName);
	pstPlayer->sRoomId = pstRoom->sRoomId;
	pstPlayer->sRoomType = sType;

	if(pstRoom->setPlayers.empty())
	{
		pstRoom->sRoomer = sName;
		LOG_INFO("roomer is %s",pstRoom->sRoomer.c_str());
	}

	pstRoom->setPlayers.insert(sName);

	pstRequest->oPkgResp["id"]=sRoomId;
	pstRequest->oPkgResp["type"]=sType;


	reResult(dwReqId);
	pushChangeRoomData(sType,sRoomId);

	return 0;
}