#ifndef __CONFIG_MGR_H__
#define __CONFIG_MGR_H__

#include <iostream>
#include <vector>
#include <set>
#include "lce.h"

using namespace std;
using namespace lce;

const uint32_t MAX_OPEN_FILE = 100000;
const uint64_t MAX_CORE_FILE = 10*1024*1024;

struct SConfig{
	SConfig()
		:sLogPath("/tmp/")
	{
		bDaemon = true;
		bUsingCore = false;
		dwMaxOpenFile = MAX_OPEN_FILE;
		dwMaxCoreFile = MAX_CORE_FILE;
		dwLogFileSize = 5*1024*1024;
		dwLogFileCount = 10;
		bShowCmd = false;

	}
	//系统信息
	bool bUsingCore;				//使用core file
	uint32_t dwMaxOpenFile;		//可打开最大的文件描述符
	uint32_t dwMaxCoreFile;		//core文件的大小

	//日志信息
	string sLogPath;				//日志目录
	uint32_t dwLogFileSize;	//一个日志文件的最大值
	uint32_t dwLogFileCount;	//日志文件的个数
	bool bShowCmd;					//是否在命令行显示日志信息
	uint8_t cLogLevel;
	bool bShowLine;

	//inner udp server info
	string sTcpIp;
	uint16_t wTcpPort;
	uint32_t dwTcpRecvBufSize;	//服务输入缓冲大小
	uint32_t dwTcpSendBufSize;	//服务输出缓冲大小
	uint32_t dwMaxTcpRecvBufSize;	//服务输入缓冲大小
	uint32_t dwMaxTcpSendBufSize;	//服务输出缓冲大小

	//inner udp server info
	string sHttpIp;
	uint16_t wHttpPort;
	uint32_t dwHttpRecvBufSize;	//服务输入缓冲大小
	uint32_t dwHttpSendBufSize;	//服务输出缓冲大小
	uint32_t dwMaxHttpRecvBufSize;	//服务输入缓冲大小
	uint32_t dwMaxHttpSendBufSize;	//服务输出缓冲大小

	uint16_t wThreadNum;				//proc_center处理线程数
	uint32_t dwTaskQueueSize;				//任务队列长度

    uint32_t dwMaxRequests;//当前最大请求个数
	uint32_t dwMaxClients; //最大连接数
	
	string sRoomsXmlPath;//房间配置文件 

	uint32_t dwSessionTimeOut;//会话空闲时间

	int iTcpSrvId;
	int iHttpSrvId;
	//其他
	string sProgramName;					//程序名
	string sCfgFile;							//配置文件名
	bool bDaemon;							//是否daemon执行程序


	string SVR_VERSION;
};



class CConfigMgr
{
	public:

	~CConfigMgr(void);

	static CConfigMgr& getInstance()
    {
        if (NULL == m_pInstance)
        {
            m_pInstance = new CConfigMgr;
        }
        return *m_pInstance;
	}


    public:

	bool init( const string &sProgramName );

	SConfig & getConfig() { return m_stConfig ;}
    char * getErrMsg() { return m_szErrMsg; }
private:
	CConfigMgr(void);

private:
	static CConfigMgr* m_pInstance;
	SConfig  m_stConfig;
	char m_szErrMsg[1024];
};


#endif

