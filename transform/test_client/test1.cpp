#include <iostream>
#include "lce.h"
#include "../package.h"

using namespace std;
using namespace lce;

int iSrv1;
string sName;

struct SRequest
{
    uint32_t dwReqId;
    SSession stSession;
    CHttpParser oParser;
    CHttpResponse oResponse;

};


class CProCenter : public CTask ,public CProcessor
{
private:
    CProCenter(){ m_pstSession = new SSession; }
    static CProCenter *m_pInstance;
	SSession *m_pstSession;
public:
	

    void onRead(SSession &stSession,const char * pszData, const int iSize)
    {

		SHead * pstHead =(SHead*)pszData;
		*m_pstSession = stSession;

		uint16_t wCmd = pstHead->getCmd();

		LOG_INFO("cmd=%d",wCmd);

		switch(wCmd)
		{
		case CMD_LOGIN:
			{
				praseLoginPkg(pszData,iSize);
				break;
			}
		case CMD_LOGOUT:
			{
				praseLogoutPkg(pszData,iSize);
				break;
			}
		case CMD_JOIN_ROOM:
			{
				praseJoinRoomPkg(pszData,iSize);
				break;
			}
		case CMD_BROADCAST_ROOM_DATA:
			{
				praseBroadcastRoomDataPkg(pszData,iSize);
				break;
			}
		case CMD_CHANGE_ROOM_DATA:
			{
				praseChangeRoomData(pszData,iSize);
				break;
			}
		default:
			{
				LOG_ERROR("invalid CMD:%d",wCmd);
			}

		}
    }

	int praseLoginPkg(const char * pszData,int iSize)
	{
		CAnyValuePackage<SHead> oPkg;
		oPkg.decodeJSON(pszData,iSize);
		int iRet=oPkg["ret"];
		if(iRet == 0)
		{
			cout<<"login ok"<<endl;
			CAnyValuePackage<SHead> oPkg;
			oPkg["roomId"]=100;
			oPkg["player"]=sName;
			oPkg["params"]["xxx"];

			oPkg.head().setStx();
			oPkg.head().setCmd(CMD_JOIN_ROOM);
			oPkg.encodeJSON();
			oPkg.head().setLen(oPkg.size()+1);
			oPkg.setEtx();


			CCommMgr::getInstance().write(*m_pstSession,oPkg.data(),oPkg.size(),false);
		}
		else
		{
			cout<<"login fail " <<(string)oPkg["msg"]<<endl;
		}
	}

	int praseChangeRoomData(const char * pszData,int iSize)
	{
		cout<<"praseChangeRoomData"<<endl;
		CAnyValuePackage<SHead> oPkg;
		oPkg.decodeJSON(pszData,iSize);

		CAnyValue oValue(oPkg.root());
		string sOut;
		oValue.encodeJSON(sOut);
		cout<<sOut<<endl;
	}

	int praseLogoutPkg(const char * pszData,int iSize)
	{
		CAnyValuePackage<SHead> oPkg;
		oPkg.decodeJSON(pszData,iSize);

		CAnyValue oValue(oPkg.root());
		string sOut;
		oValue.encodeJSON(sOut);
		cout<<sOut<<endl;
	}

	int praseJoinRoomPkg(const char * pszData,int iSize)
	{
		CAnyValuePackage<SHead> oPkg;
		oPkg.decodeJSON(pszData,iSize);

		CAnyValue oValue(oPkg.root());
		string sOut;
		oValue.encodeJSON(sOut);
		cout<<sOut<<endl;
	}

	int praseBroadcastRoomDataPkg(const char * pszData,int iSize)
	{
		
		CAnyValuePackage<SHead> oPkg;
		oPkg.decodeJSON(pszData,iSize);

		CAnyValue oValue(oPkg.root());
		string sOut;
		oValue.encodeJSON(sOut);
		cout<<"data="<<sOut<<endl;
		return 0;
	}
    void onWork(int iTaskType,void *pData)
    {

    }

    void onMessage(uint32_t dwMsgType,void *pData)
    {

    }


	void onClose(SSession &stSession)
	{
		printf("onclose id=%d\n",stSession.iFd);

	}

	void onConnect(SSession &stSession,bool bOk)
	{
		if(bOk)
		{

			printf("onconnect id=%d ok\n",stSession.iFd);

			CAnyValuePackage<SHead> oPkg;
			oPkg["name"]=sName;
			oPkg["pwd"]="840206";
			oPkg["params"]["xxx"];

			oPkg.head().setStx();
			oPkg.head().setCmd(CMD_LOGIN);
			oPkg.encodeJSON();
			oPkg.head().setLen(oPkg.size()+1);
			oPkg.setEtx();


			CCommMgr::getInstance().write(stSession,oPkg.data(),oPkg.size(),false);

		}
		else
		{
			printf("onconnect id=%d fail\n",stSession.iFd);
		}

	}

	void onError(char * szErrMsg)
	{
		cout<<szErrMsg<<endl;
	}

	void onTimer(uint32_t dwTimerId,void *pData)
	{
		if(dwTimerId == 0)
		{
			cout<<"push"<<endl;
			CAnyValuePackage<SHead> oPkg;
			oPkg["roomId"]=100;
			oPkg["player"]=sName;
			oPkg["params"]["xxx"]=sName+"xxxx";
			oPkg["params"]["yyy"]=sName+"yyyy";

			oPkg.head().setStx();
			oPkg.head().setCmd(CMD_BROADCAST_ROOM);
			oPkg.encodeJSON();
			oPkg.head().setLen(oPkg.size()+1);
			oPkg.setEtx();
			CCommMgr::getInstance().write(*m_pstSession,oPkg.data(),oPkg.size(),false);

			CCommMgr::getInstance().addTimer(dwTimerId,2000,this,pData);
		}
		else if(dwTimerId == 1)
		{
			CCommMgr::getInstance().connect(iSrv1,"127.0.0.1",8080);
		}

	}

	
	void onSignal(int iSignal)
	{
		switch(iSignal)
		{
			case SIGINT:
			{
				cout<<"stopping..."<<endl;
				CCommMgr::getInstance().stop();
			}
			break;
			case SIGHUP:
			{
				cout<<"sighup"<<endl;
				exit(0);
			}
			break;
		}
	}
	
    static CProCenter &getInstance()
    {
        if (NULL == m_pInstance)
        {
			m_pInstance = new CProCenter;
		}
		return *m_pInstance;
    }


};


CProCenter *CProCenter::m_pInstance = NULL;



int main(int argc,char *argv[])
{
    
	if(argc < 2)
	{
		printf("params less than 2\n");
		return 0;
	}

	sName.assign(argv[1]);

    CProCenter::getInstance().init(8,50000);
    CProCenter::getInstance().run();

    if(CCommMgr::getInstance().init() < 0)
    {
        printf("%s\n",CCommMgr::getInstance().getErrMsg());
        return 0;
    }

	iSrv1=CCommMgr::getInstance().createAsyncConn();
	
    if(iSrv1 < 0 )
    {
        cout<<CCommMgr::getInstance().getErrMsg()<<endl;
    }

    CCommMgr::getInstance().setProcessor(iSrv1,&CProCenter::getInstance(),CCommMgr::PKG_H2LT3);

    CCommMgr::getInstance().addTimer(1,3000,&CProCenter::getInstance(),NULL);
    CCommMgr::getInstance().addTimer(0,2000,&CProCenter::getInstance(),NULL);
    CCommMgr::getInstance().addSigHandler(SIGINT,&CProCenter::getInstance());

    CCommMgr::getInstance().start();
    return 0;
}
