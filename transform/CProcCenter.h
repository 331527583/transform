#include "lce.h"
#include "CConfigMgr.h"
#include "CReqMgr.h"
#include "package.h"
#include "tinyxml.h"

using namespace lce;
using namespace std;

const int TIME_OUT_CHECK = 1000;

class CProcCenter :public CTask, public CProcessor
{
public:
	
	enum{

		TIMER_SESSION_TIMEOUT = 1,
	};
	
	enum{

		MSG_SESSION_TIMEOUT = 1,
	};

	struct SRoomInfo
	{
		string sRoomId;
		uint16_t wMaxNum;
	};


	typedef vector<SRoomInfo> VEC_ROOM_INFO;

	typedef map<string,VEC_ROOM_INFO> MAP_ROOM_INFO;

	void getRoomsFromXml(TiXmlElement *root,MAP_ROOM_INFO &mapRoomInfos);

    int init();
    void onRead(SSession &stSession,const char * pszData, const int iSize);
    void onClose(SSession &stSession);
    void onConnect(SSession &stSession,bool bOk);
    void onError(SSession &stSession,const char * szErrMsg,int iError);
    void onWork(int iTaskType,void *pData,int iIndex);
    void onTimer(uint32_t dwTimerId,void *pData);
    void onMessage(int dwMsgType,void *pData);


	static CProcCenter& getInstance()
	{
		if ( NULL == m_pInstance )
		{
			m_pInstance = new CProcCenter;
		}

		return *m_pInstance;
	}
    char *getErrMsg() { return m_szErrMsg; }
	~CProcCenter();
private:
	int praseLoginPkg(const char * pszData, const int iSize);
	int praseJoinRoomPkg(const char * pszData, const int iSize);
	int praseExitRoomPkg(const char * pszData, const int iSize);
	int praseKickRoomPkg(const char * pszData, const int iSize);
	int praseBroadcastRoomPkg(const char * pszData, const int iSize);
	int praseBroadcastServerPkg(const char * pszData, const int iSize);
	int praseBroadcastSomebodyPkg(const char * pszData, const int iSize);
	int praseQuickStartPkg(const char * pszData, const int iSize);
	int praseHeartBeatPkg(const char * pszData, const int iSize);
	int praseGetRoomsInfo(const char * pszData, const int iSize);
	int praseGetRoomInfo(const char * pszData, const int iSize);

	int onLogin(const string &sName,const string &sPwd,uint32_t dwReqId);
	
	int onJoinRoom(const string &sName,const string &sType,const string &sRoomId,uint32_t dwReqId);
	int onExitRoom(const string &sName,const string &sType,const string &sRoomId,uint32_t dwReqId);

	int onKickRoom(const string &sRoomer,const string &sName,const string &sType,const string &sRoomId,uint32_t dwReqId);

	int onBroadcastRoom(const string &sName,const string &sType,const string &sRoomId ,const string & sCmd,uint32_t dwReqId);
	int onBroadcastServer(const string &sName,const string & sCmd,uint32_t dwReqId);
	int onBroadcastSomebody(const string &sName,const set<string> &setReceivers,const string & sCmd,uint32_t dwReqId);

	int onQuickStart(const string &sName,const string &sType,uint32_t dwReqId);

	int onGetRoomsInfo(const string &sType,uint32_t dwReqId);
	int onGetRoomInfo(const string &sType,const string &sRoomId,uint32_t dwReqId);

	int pushDataToRoom(const string &sName,const string &sType,const string &sRoomId,const string & sCmd,const CAnyValue &oData);
	int pushDataToServer(const string &sName,const string & sCmd,const CAnyValue &oData);
	int pushDataToSomebody(const string &sName,const set<string> &setReceivers,const string & sCmd,const CAnyValue &oData);

	int pushKickRoomData(const string &sRoomer,const string &sName,const string &sType,const string &sRoomId);
	int pushChangeRoomData(const string &sType,const string &sRoomId);
	

	int reResult(uint32_t dwReqId,bool bClose=false);

	int updateSession(const SSession &stSession);
	
private:

	
    CProcCenter();
	CProcCenter& operator=(const CProcCenter& rhs);
	CProcCenter(const CProcCenter& rhs);

    static CProcCenter *m_pInstance;
    char m_szErrMsg[1024];
	SSession *m_pstSession;
	SConfig m_stConfig;
	MAP_ROOM_INFO m_mapRoomsInfo;

};
