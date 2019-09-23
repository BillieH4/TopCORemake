#ifndef _GATESERVER_H_
#define _GATESERVER_H_

#include <iostream>
#include <map>
#include <time.h>
#include <dstring.h>
#include <datasocket.h>
#include <threadpool.h>
#include <commrpc.h>
#include <point.h>
#include <inifile.h>
#include <gamecommon.h>
#include <prealloc.h>
#include <ntservice.h>
#include <LogStream.h>
#include <algo.h>

#include "i18n.h"		//Add by lark.li 20080130

// Add by lark.li 20080731 begin
#include "pi_Memory.h"
#include "pi_Alloc.h"
// End

using namespace std;
using namespace dbc;

// ��ʱ�˳����ƺ��л�
//#define CHAEXIT_ONTIME

// ���� Client ���Ӳ��֣�������������������
struct Player;
class ToClient : public TcpServerApp, public RPCMGR
{
	friend class TransmitCall;
public:
    ToClient(char const* fname, ThreadPool* proc, ThreadPool* comm);
    ~ToClient();

    void post_mapcrash_msg(Player* ply);
	dstring			GetDisconnectErrText(int reason);
	void SetMaxCon(uShort maxcon){m_maxcon	=maxcon;}
	uShort GetMaxCon(){return m_maxcon;}
    void CM_LOGIN(DataSocket* datasock, RPacket& recvbuf);
	WPacket CM_LOGOUT(DataSocket* datasock, RPacket& recvbuf);
	WPacket CM_DISCONN(DataSocket* datasock, RPacket& recvbuf);
	void CM_BGNPLAY(DataSocket* datasock, RPacket& recvbuf);
	void CM_ENDPLAY(DataSocket* datasock, RPacket& recvbuf);
	void CM_NEWCHA(DataSocket* datasock, RPacket& recvbuf);
	void CM_REGISTER(DataSocket* datasock, RPacket& recvbuf);
	void CP_CHANGEPASS(DataSocket* datasock, RPacket& recvbuf);
	void CM_DELCHA(DataSocket* datasock, RPacket& recvbuf);
	void CM_CREATE_PASSWORD2(DataSocket* datasock, RPacket& recvbuf);
	void CM_UPDATE_PASSWORD2(DataSocket* datasock, RPacket& recvbuf);
	void CM_OPERGUILDBANK(DataSocket* datasock, RPacket& recvbuf);
	uShort GetVersion() { return m_version; }
	int GetCallTotal() { return m_calltotal; }

	uShort GetCheckSpan(){return m_checkSpan;}
	uShort GetCheckWaring(){return m_checkWaring;}
	uShort GetCheckError(){return m_checkError;}

	void	SetCheckSpan(uShort checkSpan);
	void	SetCheckWaring(uShort checkWaring);
	void	SetCheckError(uShort checkError);

private:
	bool		 DoCommand(DataSocket* datasock, cChar *cmdline);
    virtual bool OnConnect(DataSocket* datasock); // ����ֵ:true-��������,false-����������
    virtual void OnConnected(DataSocket* datasock);
    virtual void OnDisconnect(DataSocket* datasock, int reason);
    virtual void OnProcessData(DataSocket* datasock, RPacket &recvbuf);
    virtual WPacket OnServeCall(DataSocket* datasock, RPacket &in_para);
	virtual bool OnSendBlock(DataSocket	*datasock){return false;}

    // communication encryption
    virtual void OnEncrypt(DataSocket *datasock,char *ciphertext,const char *text,uLong &len);
    virtual void OnDecrypt(DataSocket *datasock,char *ciphertext,uLong &len);

	InterLockedLong	m_atexit,m_calltotal;
	volatile uShort m_maxcon;
	uShort m_version;
    int _comm_enc; // �����㷨����

	volatile uShort	m_checkSpan;
	volatile uShort	m_checkWaring;
	volatile uShort	m_checkError;

	IMPLEMENT_CDELETE(ToClient)
};

// ���� GameServer ���Ӳ��֣�����������������������������
class ToGameServer;

#define MAX_MAP 100
#define MAX_GAM 100
struct GameServer : public PreAllocStru
{
    friend class PreAllocHeap<GameServer>;

private:
    GameServer(uLong Size) : PreAllocStru(Size),
        m_datasock(NULL), next(NULL) {}
    ~GameServer()
        {if (m_datasock != NULL) {m_datasock->SetPointer(NULL); m_datasock = NULL;}}
    GameServer(GameServer const&) : PreAllocStru(1) {}
    GameServer& operator =(GameServer const&) {}
    void Initially();
    void Finally();
public:
	void EnterMap(Player *ply,uLong actid, uLong dbid,uLong worldid,cChar *map, Long lMapCpyNO,uLong x,uLong y,char entertype,short swiner);	//��ɫchaid���뱾�������ĵ�ͼmap�е�λ��(x,y) winerָ����ɫ�Ƿ����Ҷ�֮����
public:
	InterLockedLong m_plynum;
    string gamename;
    string ip;
    uShort port;
    DataSocket* m_datasock;
    GameServer* next;
    string maplist[MAX_MAP];
    uShort mapcnt;
};

class ToGameServer : public TcpServerApp, public RPCMGR
{
	friend class ToGroupServer;
public:
    ToGameServer(char const* fname, ThreadPool* proc, ThreadPool* comm);
    ~ToGameServer();

    GameServer* find(cChar* mapname);
	GameServer* GetGameList() { return _game_list; }

private:
    virtual bool OnConnect(DataSocket* datasock); //����ֵ:true-��������,false-����������
    virtual void OnDisconnect(DataSocket* datasock, int reason);
    virtual	WPacket OnServeCall(DataSocket* datasock, RPacket &in_para);
    virtual void OnProcessData(DataSocket* datasock, RPacket &recvbuf);

    PreAllocHeap<GameServer> _game_heap; // GameServer ���������
    void MT_LOGIN(DataSocket* datasock, RPacket& recvbuf); // GameServer ��¼ GateServer

    GameServer* _game_list; // �洢 GameServer �������������
    short _game_num;
    void _add_game(GameServer* game);
    bool _exist_game(char const* game);
    void _del_game(GameServer* game);
    map<string, GameServer*> _map_game; // �ӵ�ͼ����Ӧ GameServer ��������
    Mutex _mut_game;

	IMPLEMENT_CDELETE(ToGameServer)
};

// ���� GroupServer ���Ӳ��֣�������������������
class ToGroupServer;
class ConnectGroupServer : public Task
{
public:
    ConnectGroupServer(ToGroupServer* tgts) {_tgps = tgts; _timeout = 3000;}
private:
    virtual long Process();
	virtual Task*Lastly();

    ToGroupServer* _tgps;
    DWORD _timeout;
};

struct GroupServer
{
    GroupServer() : datasock(NULL), next(NULL) {}
    string ip; uShort port;
    DataSocket* datasock;
    GroupServer* next;
};

class ToGroupServer : public TcpClientApp, public RPCMGR
{
    friend class ConnectGroupServer;
	friend void ToGameServer::MT_LOGIN(DataSocket* datasock, RPacket& rpk);
public:
    ToGroupServer(char const* fname, ThreadPool* proc, ThreadPool* comm);
    ~ToGroupServer();

    DataSocket* get_datasock() const {return _gs.datasock;}
    RPacket get_playerlist();
	
	int GetCallTotal() { return m_calltotal; }

	// Add by lark.li 20081119 begin
	bool	IsSync() { return m_bSync; }
	void	SetSync(bool sync=true) { m_bSync = sync; }
	// End

	// ׼����
	bool	IsReady() {  return (!m_bSync && _connected); }

private:
    virtual bool OnConnect(DataSocket* datasock); // ����ֵ:true-��������,false-����������
    virtual void OnDisconnect(DataSocket* datasock, int reason);
    virtual void OnProcessData(DataSocket* datasock, RPacket &recvbuf);
    virtual WPacket OnServeCall(DataSocket* datasock, RPacket &in_para);
    
	InterLockedLong	m_atexit,m_calltotal;

	string _myself; // GateServer�Լ�������
    GroupServer _gs;
    bool volatile _connected;

	// Add by lark.li 20081119 begin
    bool volatile m_bSync;
	// End

	IMPLEMENT_CDELETE(ToGroupServer)
};
// GateServer �����֣�����������������������������������������
struct Player : public PreAllocStru , public RunBiDirectItem<Player>
{
    friend class PreAllocHeap<Player>;
	friend class DelayLogout;
public:
    bool InitReference(DataSocket* datasock);
	void Free(){PreAllocStru::Free();}

	// Add by lark.li 20081119 begin
	bool BeginRun();
	bool EndRun();
	//End


	//add by Billy
	void SetSpectating(Player* ply, bool hijack = false, bool lock = false);
	void AddSpectator(Player* ply);
	void RemoveSpectator(Player* ply);
	void UpdateSpectated();
	void StopSpectating();
	void StopSpectators();
	void SendPacketToClient(WPacket pkt);
	void SendPacketToGameServer(WPacket pkt);
	void SendPacketToGroupServer(WPacket pkt);

	vector<Player*> spectators;
	Player* spectating;
	bool isSpectated;
	bool hijack;
	bool islocked;
	bool haslocked;

	// End

	char	m_chapstr[20];
	char	m_password[ROLE_MAXSIZE_PASSWORD2+1];
	char	m_szSendKey[4];
	char	m_szRecvKey[4];




	uLong	volatile	m_actid;
    uLong   volatile    m_loginID;
    uLong	volatile	m_dbid;		// ��ǰ��ɫ�����ݿ�ID
	uLong	volatile	m_worldid;	// ��ǰ��ɫ���ڴ�ΨһID
	uLong	volatile	m_pingtime;
    uInt	volatile	comm_key_len; // ͨѶ��Կ����
    char	comm_textkey[12]; // GateServer �� Client ֮�����ͨѶ����Կ
    InterLockedLong gm_addr; // GameServer �� Player �����ָ��
    InterLockedLong gp_addr; // GroupServer �� Player �����ָ��
    DataSocket* volatile m_datasock; // �� Player �� GateServer <-> Client ����
    GameServer* volatile game; // �� Player ��ǰ���ڵ� GameServer ��������
    volatile bool enc; // �Ƿ����ͨ������

	// �Ƿ����
	uLong	volatile m_lestoptick;
	bool	volatile m_estop;
	short	volatile m_sGarnerWiner;

	struct
	{
		Mutex				m_mtxstat;					//0:����m_status;
		volatile char		m_status;					//0:��Ч;1.ѡ��ɫ�ڼ�;2.����Ϸ��.
		volatile char		m_exit;						//0:��Ч��1.��ɫ���������ʱ�˳��У�׼������ѡ�˽��棻2.��ɫ���������ʱ�˳��У�Ҫ�����˳���Ϸ��
	};

	dbc::dstring m_chamap;
	void		SetMapName(dbc::cChar *cszName) {m_chamap = cszName;}
	const char*	GetMapName(void) {return m_chamap.c_str();}

private:
    Player(uLong Size) : PreAllocStru(Size),
        m_datasock(NULL), game(NULL), gm_addr(0), gp_addr(0), m_dbid(0),m_worldid(0),m_status(0),m_sGarnerWiner(0)
	{m_mtxstat.Create(false);}
    ~Player()
        {if (m_datasock != NULL) {m_datasock->SetPointer(NULL); m_datasock = NULL;}}
    Player(Player const&) : PreAllocStru(1) {}
    Player& operator =(Player const&) {}
    void Initially(); void Finally();
};
class TransmitCall :public PreAllocTask
{
public:
	TransmitCall(uLong size):PreAllocTask(size){};
	void Init(DataSocket* datasock, RPacket &recvbuf){m_datasock	=datasock;m_recvbuf	=recvbuf;}
	long Process();

	DataSocket* m_datasock;
	RPacket		m_recvbuf;
};
class GateServer
{
public:
    GateServer(char const* fname);
    ~GateServer();
	void RunLoop(); // ��ѭ��
	ThreadPool	*m_clproc,*m_clcomm,*m_gpcomm,*m_gpproc,*m_gmcomm;
    ToGroupServer* gp_conn; // ͬGroupServer�����Ӷ��������������ƣ�
    ToGameServer* gm_conn; // ͬGameServer�����Ӷ��󣨱�����
    ToClient* cli_conn; // ͬClient�����Ӷ��󣨱�����
	Mutex	_mtxother;

    PreAllocHeap<Player>		player_heap; // ��Ҷ����

	// Add by lark.li 20081119 begin
	RunBiDirectChain<Player> 	m_plylst;	// �������
	// End

	PreAllocHeap<TransmitCall>	m_tch;

	IMPLEMENT_CDELETE(GateServer)
};
extern GateServer* g_gtsvr;
extern bool volatile g_appexit;

class CUdpManage;
class CUdpServer;
class GateServerApp:public NTService
{
public:
	GateServerApp();

	void ServiceStart();
	void ServiceStop();
	virtual cChar *SetSvcName()const	{return "GateServer";	}
	virtual cChar *SetDispName()const	{return "[Kop Online]Gate Server";	}//��ʾ��ȱʡ���ڷ�����
	virtual bool CanPaused()const		{return true;};	//ȱʡ֧����ͣ�ͼ�������

	int		GetShowMin()				{ return _nShowMin;		}
	int		GetShowMax()				{ return _nShowMax;		}
	void	SetShowRange( int min, int max )	{ _nShowMin=min; _nShowMax=max;		}

	
private:	// ������δ����ǰ���ͻ���ͨ��udp�����Ϸͳ����ϢJerry 2006-1-10
	CUdpManage*	_pUdpManage;
	static void _NotifySocketNumEvent( CUdpManage* pManage, CUdpServer* pUdpServer, const char* szClientIP, unsigned int nClientPort, const char* pData, int len );
	static int	_nShowMin;	
	static int	_nShowMax;	
};

extern LogStream g_gateerr;
extern LogStream g_gatelog;
extern LogStream g_chkattack;
extern LogStream g_gateconnect;
//extern LogStream g_gatepacket;
extern GateServerApp* g_app;
extern BYTE g_wpe;
extern BYTE g_ddos;
extern uShort g_wpeversion;
extern HANDLE hConsole;
extern InterLockedLong g_exit;

#define C_PRINT(s, ...) \
	SetConsoleTextAttribute(hConsole, 14); \
	printf(s, __VA_ARGS__); \
	SetConsoleTextAttribute(hConsole, 10);

#define C_TITLE(s) \
	char szPID[32]; \
	_snprintf_s(szPID,sizeof(szPID),_TRUNCATE, "%d", GetCurrentProcessId()); \
	std::string strConsoleT; \
	strConsoleT += "[PID:"; \
	strConsoleT += szPID; \
	strConsoleT += "]"; \
	strConsoleT += s; \
	SetConsoleTitle(strConsoleT.c_str());

extern CResourceBundleManage g_ResourceBundleManage; //Add by lark.li 20080130

#endif // _GATESERVER_H_
