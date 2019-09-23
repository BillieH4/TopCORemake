#pragma once

#include <string>

#define MAX_GATE 5
#define MAX_MAP  100
#define MAX_MAPNAME_LENGTH	32

class CGameCommand
{
public:
	CGameCommand();
	bool	Load(char *pszFileName);
	void	SetDefault();

	// commands
	char	m_cMove[32];
	char	m_cMake[32];
	char	m_cNotice[32];
	char	m_cHide[32];
	char	m_cUnhide[32];
	char	m_cGoto[32];
	char	m_cKick[32];
	char	m_cReload[32];
	char	m_cRelive[32];
	char	m_cQcha[32];
	char	m_cQitem[32];
	char	m_cCall[32];
	char	m_cGamesvrstop[32];
	char	m_cUpdateall[32];
	char	m_cMisreload[32];
	char	m_cSummon[32];
	char	m_cSummonex[32];
	char	m_cKill[32];
	char	m_cAddmoney[32];
	char	m_cAddexp[32];
	char	m_cAttr[32];
	char	m_cItemattr[32];
	char	m_cSkill[32];
	char	m_cDelitem[32];
	char	m_cLuaall[32];
	char	m_cAddkb[32];
	char	m_cLua[32];
	char	m_cAddImp[32];
};

class CGameConfig
{

public:	
	
    CGameConfig();
	
	bool	Load(char *pszFileName);
	void	SetDefault();
	bool	Reload(char *pszFileName);
	bool	LoadCmd(char *pszFileName);
	
public:

	// ��ע��˴���Ҫ�����κηǹ̶��ڴ�ߴ�ı���, ����ģ��, string֮���,
	// ���ཫ��ֱ��д�̺Ͷ���
	char    m_szGateIP[MAX_GATE][64];  // GateIP��ַ�б�
    int     m_nGatePort[MAX_GATE];     // Gate Port�б�
    int     m_nGateCnt;                // Gate����
    char    m_szInfoIP[64];            // InfoServer IP
    int     m_nInfoPort;               // InfoServer Port
	char	m_szInfoPwd[33];		   // InfoServer��֤��
	int		m_nSection;				   // С����
    char    m_szMapList[MAX_MAP][MAX_MAPNAME_LENGTH];
	BYTE	m_btMapOK[MAX_MAP];		   // ��ͼ�Ƿ��ʼ���ɹ�
	int     m_nMapCnt;				   // ��ͼ����
	char	m_szEqument[MAX_MAPNAME_LENGTH];   // �����ػ���
    char    m_szName[64];			   // ����������
	char	m_szDBIP[64];			   // DB IP
	char	m_szDBUsr[32];			   // DB �û���
	char	m_szDBPass[32];			   // DB ����

	// Add by lark.li 20080321 begin
	char	m_szTradeLogDBIP[64];			   // DB IP
	char	m_szTradeLogDBName[32];			   // DB IP
	char	m_szTradeLogDBUsr[32];			   // DB �û���
	char	m_szTradeLogDBPass[32];			   // DB ����

	BOOL	m_bTradeLogIsConfig;
	// End
	char	m_szDBName[32];            // kong@pkodev.net 09.22.2017

	long	m_lSocketAlive;            // Socket���ֻ
	int		m_nMaxPly;                 // ��������
	int		m_nMaxCha;                 // ����ɫ��
	int		m_nMaxItem;                // ��������
	int		m_nMaxTNpc;                // ���Ի�NPC
	unsigned long	m_ulBaseID;        // ��������ID����
	long	m_lItemShowTime;           // ���ߴ���ʱ��
	long	m_lItemProtTime;           // ���߱���ʱ��
	long	m_lSayInterval;            // �������
	char	m_szResDir[255];		   // ������Դ���ڵ�Ŀ¼
	char	m_szLogDir[255];		   // Log���ڵ�Ŀ¼
	char	m_chMapMask;               // �Ƿ��ȡ���ͼ
	long	m_lDBSave;                 // ���ݿⶨʱ����ʱ����

	BOOL	m_bLogAI;				   // �Ƿ��AI��log
	BOOL	m_bLogCha;				   // �Ƿ�򿪽�ɫ��log
	BOOL	m_bLogCal;				   // �Ƿ����ֵ�����log
	BOOL	m_bLogMission;			   // �Ƿ��Mission��log

	BOOL	m_bSuperCmd;
	
	// Add by lark.li 20080731 begin
	vector<int>	m_vGMCmd;
	// End
	short	m_sGuildNum;
	short	m_sGuildTryNum;
	BOOL	m_bOfflineStall;
	BOOL	m_bDiscStall;
	BOOL	m_bBlindChaos;
	char	m_szChaosMap[32];
	DWORD	m_dwStallTime;		
	
	BOOL	m_bLogDB;				   // �Ƿ�������Ϊ��¼�����ݿ�
	BOOL    m_bInstantIGS;

	long	m_lWeather;

	unsigned char m_cSaveState[32];
};

#define	defCONFIG_FILE_NAME_LEN	512
extern char	szConfigFileN[defCONFIG_FILE_NAME_LEN];

extern CGameConfig g_Config;
extern CGameCommand g_Command;

