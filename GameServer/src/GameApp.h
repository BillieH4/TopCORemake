//=============================================================================
// FileName: GameApp.h
// Creater: ZhangXuedong
// Date: 2004.11.04
// Comment: CChaMgr class
//=============================================================================

#ifndef GAMEAPP_H
#define GAMEAPP_H


//#define DISABLE_GM_CMD				//	�ر�gm����


#include "GameAppNet.h"
#include "point.h"
#include "RoleData.h"
#include "Config.h"
#include "Identity.h"
#include "Player.h"
#include "SkillStateRecord.h"
#include "Script.h"
#include "AreaRecord.h"
#include "SkillTemp.h"
#include "StateCell.h"
#include "Parser.h"
#include "CommFunc.h"
#include "EntityAlloc.h"
#include "MapRes.h"
#include "PicSet.h"

#define defMAX_CHARINFO_NO 2500

class CCharacter;
class CPlayer;
class CItem;
class CPassengerMgr;

namespace mission
{
 class CTalkNpc;
 class CNpc;
 class CTradeData;
 class CStallData;
}

class SubMap;
class CCharacter;
class CItemRecordAttr;

class CChaRecordSet; 	  
class CSkillRecordSet;   
class CItemRecordSet;	  
class CLevelRecordSet;
class CSailLvRecordSet;
class CLifeLvRecordSet;
class CJobEquipRecordSet;
class CForgeRecordSet;
class CHairRecordSet;

class GateServer;


#define MAX_DBLOG_POOL 100

struct SDBLogData
{
	int		nLoc;			// ��pool�е�λ��
	char	szLog[8192];	// �ַ�������
	SDBLogData():nLoc(0) {}
};


class CDBLogMgr // ������д��db��log
{

public:

	CDBLogMgr()
		:_nPerLogCnt(5), _nLogLeft(0), _nPoolUseLoc(0)
	{
		for(int i = 0; i < MAX_DBLOG_POOL; i++)
		{
			_LogPool[i].nLoc = i;
		}
	}
	
	
	
	// ����Log 5���ַ����ֶ�, ���һ������Ϊ8000�ַ�����
	void Log(const char *type, const char *c1, const char *c2, const char *c3, const char *c4, const char *p, BOOL bAddToList = TRUE);

	// Add by lark.li 20080324 begin
	void TradeLog(const char* action, const char *pszChaFrom, const char *pszChaTo, const char *pszTrade);
	// End

	void HandleLogList();
	void FlushLogList();  // GameServer�رյ�ʱ��, ��֤ʣ�µ�log��д��DB
	
	int	GetLogLeft()				{ return _nLogLeft;		} // ��ʣ�µ�û�д����log����, �����������ؽ���
	int	SetPerLogCnt(int nCnt)		{ _nPerLogCnt = nCnt;	} // ����ÿ��д�������, ���Զ�̬����
	int GetPerLogCnt()				{ return _nPerLogCnt;   } // ����������

protected:
	
	list<SDBLogData*>	_LogList;
	int					_nPerLogCnt;				// ÿ��д������log����
	int					_nLogLeft;					// ��ʣ�µ�log����
	SDBLogData			_LogPool[MAX_DBLOG_POOL];	// ���Ҫ��log������, ��pool��ѭ��ʹ��
	int					_nPoolUseLoc;				// ��ǰ����ʹ�õ�pool��λ��
};

struct SVolunteer
{
	char szName[defENTITY_NAME_LEN];	// ����
	unsigned long ulID;					// ID
	long lLevel;						// �ȼ�
	long lJob;							// ְҵ
	char szMapName[256];				// ��ͼ
};

class	CGameApp : public CDBLogMgr
{
public:
	
    CGameApp();
	~CGameApp();

	BOOL	Init();
	BOOL    InitMap();
    void	Run(DWORD dwCurTime);

	// ��������Ϣ�йص�4������ӿں���
	void	ProcessNetMsg(int nMsgType, GateServer *pGate, RPACKET pkt);
	void    OnGateConnected(GateServer *pGate, RPACKET pkt);
	void	OnGateDisconnect(GateServer *pGate, RPACKET pkt);
	void	ProcessPacket(GateServer* pGate, RPACKET pkt);
	void	ProcessTeamMsg(GateServer *pGate, RPACKET pkt);
	void	ProcessGuildMsg(GateServer *pGate, RPACKET pkt);
	void	ProcessGuildChallMoney( GateServer *pGate, RPACKET pkt );
	void	ProcessGuildChallPrizeMoney( GateServer *pGate, RPACKET pkt );
	void	ProcessDynMapEntry(GateServer *pGate, RPACKET pkt);
	void	ProcessInterGameMsg(unsigned short usCmd, GateServer *pGate, RPACKET pkt);
	void	ProcessGroupBroadcast(unsigned short usCmd, GateServer *pGate, RPACKET pkt);
	void	ProcessGarner2Update(RPACKET pkt);//�Ҷ�������
    // ����InfoServer��Ϣ
    void    ProcessInfoMsg(pNetMessage msg, short sType, InfoServer *pInfo);
    void    ProcessMsg(pNetMessage msg, InfoServer *pInfo);
    void    OnInfoConnected(InfoServer *pInfo);
    void    OnInfoDisconnected(InfoServer *pInfo);
    
	//------------------------------------
	
	
    CPlayer*    CreateGamePlayer(const char szPassword[], uLong ulChaDBId, uLong ulWorldId, const char *cszMapName, char chType);
    void        ReleaseGamePlayer(CPlayer*);
	void		GoOutGame(CPlayer* pPlayer, bool bOffLine);
	CPlayer*	GetNewPlayer();
	CPlayer*	GetPlayer(long lHandle);
	CPlayer*	IsValidPlayer(long lID, long lHandle);
    CCharacter* GetNewCharacter();
	CItem*		GetNewItem();
	mission::CTalkNpc*	GetNewTNpc();
	Entity*		GetEntity(long lHandle);
	Entity*		IsValidEntity(unsigned long ulID, long lHandle);
	Entity*		IsLiveingEntity(unsigned long ulID, long lHandle);
	Entity*		IsMapEntity(unsigned long ulID, long lHandle);
	Entity*		IsLifeEntity(unsigned long ulID, long lHandle);
	void		BeginGetTNpc(void);
	mission::CTalkNpc*	GetNextTNpc(void);
	void		AddPlayerIdx(DWORD dwDBID, CPlayer* pPlayer);
	void		DelPlayerIdx(DWORD dwDBID);
	CPlayer*    GetPlayerByDBID(DWORD dwDBID);
	CPlayer*	GetPlayerByMainChaName(	const	char*	sMainChaName	);
	void		AfterPlayerLogin(const char *cszPlyName);
	void		NoticePlayerLogin(CPlayer *pCPlayer);

	CMapRes*	GetMap(int no);
	CMapRes*	FindMapByName(const char *mapname, bool bIncUnRun = false);
	void		LoadAllTable(void);
	void		LoadCharacterInfo(void);
	void		LoadSkillInfo(void);
	void		LoadItemInfo(void);

	// ��������npc�ű���Ϣ
	BOOL		ReloadNpcInfo( CCharacter& character );
	mission::CNpc* FindNpc( const char szName[] );

	// �ٻ�NPC����һ��ʱ��
	BOOL		SummonNpc( BYTE byMapID, USHORT sAreaID, const char szNpc[], USHORT sTime );

	// ����ʵ��
	mission::CEventEntity* CreateEntity( BYTE byType ) { return m_pCEntSpace->GetEventEntity( byType ); }
	
	void		NotiGameReset(unsigned long ulLeftSec);
	void		SaveAllPlayer(void);

	void		SetEntityEnableLog(bool bValid = true);
	CSkillTempData*	GetSkillTData(short sSkillNo, char chSkillLv);
	void		SetSkillTDataRange(short *psRange);
	void		SetSkillTDataState(short *psState);
	void		InitSStateTraOnTime();
	long		GetSStateTraOnTime(unsigned char uchStateID, unsigned char uchStateLv);

	void		DataStatistic(void); // ��Ϣͳ��
	CCharacter*	FindPlayerChaByName(const char* cszChaName);
	CCharacter*	FindPlayerChaByNameLua(const char* cszChaName);
	int FindPlayerChaByActNameLua(const char* cszChaName,CCharacter* chas[3]);
	bool		DealAllInGuild(int guildID, const char* luaFunc, const char* luaParam);
	CCharacter*	FindPlayerChaByID(unsigned long ulChaID);
	CCharacter* FindMainPlayerChaByID(unsigned long ulChaID);
	CCharacter*	FindChaByID(unsigned long ulChaID);
	CCharacter*	FindChaByName(const char* cszChaName);
	CPlayer*	FindPlayerByDBChaID(unsigned long ulDBChaID);

	void		WorldNotice(const char *szString); // ֪ͨ��������GameServer�ϵ����
	void		GuildNotice(unsigned long guildID, const char *szString);
	void		ScrollNotice(const char * szString,int SetNum);//Add by sunny.sun20080804
	void		GMNotice(const char * szString);//add by sunny.sun 20080821
	void		LocalNotice(const char *szString); // ֪ͨ��GameServer�ϵ����
	void		ChaNotice(const char *szNotiString, const char *szChaName = ""); // ָ֪ͨ�����

	bool		IsChaAttrMaxValInit(void) {return m_bChaAttrMaxValInit;}
	void		ChaAttrMaxValInit(bool bSet) {m_bChaAttrMaxValInit = bSet;}

	void		CheckSeeWithTeamChange(bool CanSeen[][2], CPlayer **pCPlayerList, char chMemberCnt);
	void		RefreshTeamEyeshot(bool CanSeenOld[][2], bool CanSeenNew[][2], CPlayer **pCPlayerList, char chMemberCnt, char chRefType);
	void		RefreshTeamEyeshot(CPlayer **pCPlayerList, char chMemberCnt, char chRefType);

	BOOL		AddVolunteer(CCharacter *pCha);
	BOOL		DelVolunteer(CCharacter *pCha);
	SVolunteer	*GetVolInfo(int nIndex);
	int			GetVolNum();
	SVolunteer	*FindVolunteer(const char *szName);

	void		BanAccount(const char *szString);
	void		UnbanAccount(const char *szString);
	short		GetMapNum() { return m_mapnum; }
	CMapRes**	GetMapList() { return m_MapList; }

	DWORD   m_dwFPS;
	DWORD   m_dwRunCnt;
	DWORD	m_dwChaCnt;
	DWORD	m_dwPlayerCnt;
	DWORD	m_dwActiveMgrUnit;
	dbc::InterLockedLong   m_dwRunStep;	// ���ܼ��
	
	BOOL	m_bExecLuaCmd;
	string	m_strMapNameList;

	dbc::PreAllocHeap<CPassengerMgr>		m_CabinHeap;
	dbc::PreAllocHeap<mission::CTradeData>	m_TradeDataHeap;
	dbc::PreAllocHeap<mission::CStallData>  m_StallDataHeap;
	dbc::PreAllocHeap<CSkillTempData>		m_SkillTDataHeap;
	dbc::PreAllocHeap<CStateCell>			m_MapStateCellHeap;
	dbc::PreAllocHeap<CChaListNode>			m_ChaListHeap;
	dbc::PreAllocHeap<CStateCellNode>		m_StateCellNodeHeap;

	// ������Ϣͳ��
	dbc::Long	m_lCabinHeapNum;
	dbc::Long	m_lTradeDataHeapNum;
	dbc::Long	m_lSkillTDataHeapNum;
	dbc::Long	m_lMapMgrUnitHeapNum;
	dbc::Long	m_lEntityListHeapNum;
	dbc::Long	m_lMgrNodeHeapNum;
	//

	Identity			m_Ident;
	Identity			m_ItemIdent;

	struct // GameServer����
	{
		unsigned long	m_ulLeftSec;
	    CTimer			m_CTimerReset;
	};

	CEntityAlloc		*m_pCEntSpace;
	CPlayerAlloc		*m_pCPlySpace;

protected:
	void	MgrUnitRun(DWORD dwCurTime);
	void	GameItemRun(DWORD dwCurTime);
	void	MapMgrRun(DWORD dwCurTime);

protected:
	CMapRes* m_MapList[MAX_MAP];		// ά��һ��ָ�������ӵ�ͼ���б�
	short	 m_mapnum;
	
	CChaRecordSet 	   *m_CChaRecordSet;
	CSkillRecordSet    *m_CSkillRecordSet;
	CSkillStateRecordSet	*m_CSkillStateRecordSet;
	CItemRecordSet	   *m_CItemRecordSet;
	CLevelRecordSet	   *m_CLevelRecordSet;
	CJobEquipRecordSet *m_CJobEquipRecordSet;
	CAreaSet           *m_CAreaRecordSet;
	CSailLvRecordSet   *m_CSailLvRecord;
	CLifeLvRecordSet   *m_CLifeLvRecord;
	CHairRecordSet	   *m_CHairRecord;

	map<DWORD, CPlayer*>  _PlayerIdx;  // ��DB IDӳ�䵽Playerָ��

	vector<SVolunteer>	m_vecVolunteerList;	// ־Ը���б�

	struct
	{
		CPlayer		*pCPlayerL;	  // ��¼���Ӧsocket���ӵ����
		CPlayer		*pCCurPlayer; // ���������������
	} m_GatePlayer[MAX_GATE];                                                                

public:
	CPicSet			   *m_PicSet;


private:
    
	DWORD   _dwFPS;
    DWORD   _dwLastTick;
    DWORD   _dwRunCnt;
    DWORD   _dwTempRunCnt;

	// ��¼��ͬ�ȼ���ͬ���ܵ������ֵ
	struct
	{
		CSkillTempData	*m_pCSkillTData[defMAX_SKILL_NO + 1][defMAX_SKILL_LV + 1];
		short			m_sSkillSetNo;
		char			m_chSkillSetLv;
	};

	long	m_lSStateTraOnTime[AREA_STATE_MAXID + 1][SKILL_STATE_LEVEL + 1];	// �ӵر�ת�Ƶ���ɫ���ϵ�״̬�ĳ���ʱ��

    CTimer	m_CTimerItem;

	bool	m_bChaAttrMaxValInit;
};

inline CMapRes* CGameApp::GetMap(int no)
{T_B 
	return m_MapList[no]; 
T_E}
	
inline void CGameApp::AddPlayerIdx(DWORD dwDBID, CPlayer* pPlayer)
{T_B
	_PlayerIdx[dwDBID] = pPlayer;
	//LG("player_idx", "���DB ID = %d��Ӧ��Player\n", dwDBID);
T_E}

inline void	CGameApp::DelPlayerIdx(DWORD dwDBID)
{T_B
	//LG("player_idx", "���DB ID = %d��Ӧ��Player\n", dwDBID);
	map<DWORD, CPlayer*>::iterator it = _PlayerIdx.find(dwDBID);
	if(it!=_PlayerIdx.end())
	{
		_PlayerIdx.erase(it);
	}
	else
	{
		//LG("player_idx", "���PlayerIdx��ʱ����ִ���, DB ID = %d û�з�������\n", dwDBID); 
		LG("player_idx", "when delete PlayerIdx it appear error, DB ID = %d no find index\n", dwDBID); 
	}
	//LG("player_idx", "Idx Size = %d\n\n", _PlayerIdx.size());
T_E}

inline CPlayer* CGameApp::GetPlayerByDBID(DWORD dwDBID)
{T_B
	return _PlayerIdx[dwDBID];
T_E}

inline	CPlayer*	CGameApp::GetPlayerByMainChaName(	const	char*	sMainChaName	)
{T_B
	if(	!sMainChaName	)
	{
		return	NULL;
	};
	std::map<	DWORD,	CPlayer*	>::iterator	it	=	_PlayerIdx.begin();
	while(	it	!=	_PlayerIdx.end()	)
	{
		if(	it->second	)
		{
			if(	it->second->GetMainCha()	)
			{
				if(	!strcmp(	it->second->GetMainCha()->GetName(),	sMainChaName	)	)
				{
					return	it->second;
				}
			}
		};
		++it;
	};
	return	NULL;
T_E};

// ȡ��Ӧ�ȼ����ܵ�������ݣ���������ݻ�û�г�ʼ������ִ�г�ʼ��������
inline CSkillTempData* CGameApp::GetSkillTData(short sSkillNo, char chSkillLv)
{
	if (!m_pCSkillTData[sSkillNo][chSkillLv])
	{
		m_pCSkillTData[sSkillNo][chSkillLv] = m_SkillTDataHeap.Get();
		if (!m_pCSkillTData[sSkillNo][chSkillLv])
			return 0;
		CSkillRecord	*pCSkillRec;
		pCSkillRec = GetSkillRecordInfo(sSkillNo);
		if (!pCSkillRec)
			return 0;

		m_sSkillSetNo = sSkillNo;
		m_chSkillSetLv = chSkillLv;
		// SP����
		if (strcmp(pCSkillRec->szUseSP, "0"))
		{
			if (g_CParser.DoString(pCSkillRec->szUseSP, enumSCRIPT_RETURN_NUMBER, 1, enumSCRIPT_PARAM_NUMBER, 1, chSkillLv, DOSTRING_PARAM_END))
				m_pCSkillTData[sSkillNo][chSkillLv]->sUseSP = (Short)g_CParser.GetReturnNumber(0);
		}
		else
			m_pCSkillTData[sSkillNo][chSkillLv]->sUseSP = 0;
		// ���;öȡ�����
		if (strcmp(pCSkillRec->szUseEndure, "0"))
		{
			if (g_CParser.DoString(pCSkillRec->szUseEndure, enumSCRIPT_RETURN_NUMBER, 1, enumSCRIPT_PARAM_NUMBER, 1, chSkillLv, DOSTRING_PARAM_END))
				m_pCSkillTData[sSkillNo][chSkillLv]->sUseEndure = (Short)g_CParser.GetReturnNumber(0);
		}
		else
			m_pCSkillTData[sSkillNo][chSkillLv]->sUseEndure = 0;
		// ������������
		if (strcmp(pCSkillRec->szUseEnergy, "0"))
		{
			if (g_CParser.DoString(pCSkillRec->szUseEnergy, enumSCRIPT_RETURN_NUMBER, 1, enumSCRIPT_PARAM_NUMBER, 1, chSkillLv, DOSTRING_PARAM_END))
				m_pCSkillTData[sSkillNo][chSkillLv]->sUseEnergy = (Short)g_CParser.GetReturnNumber(0);
		}
		else
			m_pCSkillTData[sSkillNo][chSkillLv]->sUseEnergy = 0;
		// ��������
		m_pCSkillTData[sSkillNo][chSkillLv]->sRange[0] = enumRANGE_TYPE_NONE;
		if (strcmp(pCSkillRec->szSetRange, "0"))
			g_CParser.DoString(pCSkillRec->szSetRange, enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_NUMBER, 1, chSkillLv, DOSTRING_PARAM_END);
		// ����״̬
		m_pCSkillTData[sSkillNo][chSkillLv]->sStateParam[0] = SSTATE_NONE;
		if (strcmp(pCSkillRec->szRangeState, "0"))
			g_CParser.DoString(pCSkillRec->szRangeState, enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_NUMBER, 1, chSkillLv, DOSTRING_PARAM_END);
		// ��ʩ�ü��
		if (strcmp(pCSkillRec->szFireSpeed, "0"))
		{
			if (g_CParser.DoString(pCSkillRec->szFireSpeed, enumSCRIPT_RETURN_NUMBER, 1, enumSCRIPT_PARAM_NUMBER, 1, chSkillLv, DOSTRING_PARAM_END))
				m_pCSkillTData[sSkillNo][chSkillLv]->lResumeTime = g_CParser.GetReturnNumber(0);
		}
		else
			m_pCSkillTData[sSkillNo][chSkillLv]->lResumeTime = 0;
	}

	return m_pCSkillTData[sSkillNo][chSkillLv];
}

inline void CGameApp::SetSkillTDataRange(short *psRange)
{
	if (m_sSkillSetNo < 0 || m_sSkillSetNo > defMAX_SKILL_NO)
		return;
	if (m_chSkillSetLv < 0 || m_chSkillSetLv > defMAX_SKILL_LV)
		return;

	memcpy(m_pCSkillTData[m_sSkillSetNo][m_chSkillSetLv]->sRange, psRange, sizeof(short) * defSKILL_RANGE_EXTEP_NUM);
}

inline void CGameApp::SetSkillTDataState(short *psState)
{
	if (m_sSkillSetNo < 0 || m_sSkillSetNo > defMAX_SKILL_NO)
		return;
	if (m_chSkillSetLv < 0 || m_chSkillSetLv > defMAX_SKILL_LV)
		return;

	memcpy(m_pCSkillTData[m_sSkillSetNo][m_chSkillSetLv]->sStateParam, psState, sizeof(short) * defSKILL_STATE_PARAM_NUM);
}

// ��ʼ���ر�״̬ת�Ƶ���ɫ������������ʱ��
inline void CGameApp::InitSStateTraOnTime()
{
	memset(m_lSStateTraOnTime, 0, sizeof(m_lSStateTraOnTime));
	CSkillStateRecord	*pSStateR;
	for (int i = 1; i <= AREA_STATE_MAXID; i++)
	{
		pSStateR = GetCSkillStateRecordInfo(i);
		if (!pSStateR)
			continue;
		if (!strcmp(pSStateR->szOnTransfer, "0"))
			continue;
		for (int j = 1; j <= SKILL_STATE_LEVEL; j++)
		{
			if (g_CParser.DoString(pSStateR->szOnTransfer, enumSCRIPT_RETURN_NUMBER, 1, enumSCRIPT_PARAM_NUMBER, 1, j, DOSTRING_PARAM_END))
				m_lSStateTraOnTime[i][j] = g_CParser.GetReturnNumber(0);
		}
	}
}

inline long CGameApp::GetSStateTraOnTime(unsigned char uchStateID, unsigned char uchStateLv)
{
	return m_lSStateTraOnTime[uchStateID][uchStateLv];
}

// �����������������̵����н�ɫ
inline CCharacter* CGameApp::FindPlayerChaByName(const char* cszChaName)
{
	BEGINGETGATE();
	CPlayer	*pCPlayer;
	CCharacter	*pCha = 0;
	GateServer	*pGateServer;
	while (pGateServer = GETNEXTGATE())
	{
		if (!BEGINGETPLAYER(pGateServer))
			continue;
		int nCount = 0;
		while (pCPlayer = (CPlayer *)GETNEXTPLAYER(pGateServer))
		{
			if (++nCount > GETPLAYERCOUNT(pGateServer))
			{
				//LG("����������", "�����Ŀ:%u, %s\n", GETPLAYERCOUNT(pGateServer), "FindPlayerChaByName");
				LG("player chain list error", "player number:%u, %s\n", GETPLAYERCOUNT(pGateServer), "FindPlayerChaByName");
				break;
			}
			pCha = pCPlayer->GetCtrlCha();
			if (!pCha)
				continue;
			if (!strcmp(pCha->GetName(), cszChaName)) // �ҵ���ɫ
				return pCha;
		}
	}

	return 0;
}

inline CCharacter* CGameApp::FindPlayerChaByNameLua(const char* cszChaName)
{
	BEGINGETGATE();
	CPlayer	*pCPlayer;
	CCharacter	*pCha = 0;
	GateServer	*pGateServer;
	while (pGateServer = GETNEXTGATE())
	{
		if (!BEGINGETPLAYER(pGateServer))
			continue;
		int nCount = 0;
		while (pCPlayer = (CPlayer *)GETNEXTPLAYER(pGateServer))
		{
			if (++nCount > GETPLAYERCOUNT(pGateServer))
			{
				//LG("����������", "�����Ŀ:%u, %s\n", GETPLAYERCOUNT(pGateServer), "FindPlayerChaByName");
				LG("player chain list error", "player number:%u, %s\n", GETPLAYERCOUNT(pGateServer), "FindPlayerChaByName");
				break;
			}
			pCha = pCPlayer->GetCtrlCha();

			if (!pCha)
				continue;
			if (!strcmp(pCha->GetPlayer()->GetMainCha()->GetName(), cszChaName)) // �ҵ���ɫ
				return pCha;
		}
	}

	return 0;
}


inline int CGameApp::FindPlayerChaByActNameLua(const char* cszChaName, CCharacter* chas[3])
{
	BEGINGETGATE();
	CPlayer	*pCPlayer;
	CCharacter	*pCha = 0;
	GateServer	*pGateServer;

	int count = 0;

	while (pGateServer = GETNEXTGATE())
	{
		if (!BEGINGETPLAYER(pGateServer))
			continue;
		int nCount = 0;
		while (pCPlayer = (CPlayer *)GETNEXTPLAYER(pGateServer))
		{
			if (++nCount > GETPLAYERCOUNT(pGateServer))
			{
				//LG("����������", "�����Ŀ:%u, %s\n", GETPLAYERCOUNT(pGateServer), "FindPlayerChaByName");
				LG("player chain list error", "player number:%u, %s\n", GETPLAYERCOUNT(pGateServer), "FindPlayerChaByName");
				break;
			}
			pCha = pCPlayer->GetCtrlCha();

			if (!pCha)
				continue;
			if (!strcmp(pCha->GetPlayer()->GetActName(), cszChaName)) // �ҵ���ɫ
				chas[count++] = pCha;
		}
	}

	return count;
}
/*
inline bool CGameApp::DealAllInGuild(int guildID, const char* luaFunc, const char* luaParam){
	BEGINGETGATE();
	CPlayer	*pCPlayer;
	CCharacter	*pCha = 0;
	GateServer	*pGateServer;
	while (pGateServer = GETNEXTGATE()){
		if (!BEGINGETPLAYER(pGateServer))
			continue;
		int nCount = 0;
		while (pCPlayer = (CPlayer *)GETNEXTPLAYER(pGateServer))
		{
			if (++nCount > GETPLAYERCOUNT(pGateServer)){
				LG("player chain list error", "player number:%u, %s\n", GETPLAYERCOUNT(pGateServer), "DealAllInGuild");
				break;
			}
			pCha = pCPlayer->GetCtrlCha();

			if (!pCha)
				continue;
			if (pCha->GetPlayer()->GetMainCha()->GetValidGuildID() == guildID){
				g_CParser.DoString(luaFunc, enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, pCha,enumSCRIPT_PARAM_STRING,1,luaParam, DOSTRING_PARAM_END);
			}
		}
	}
	return true;
}
*/
inline bool CGameApp::DealAllInGuild(int guildID, const char* luaFunc, const char* luaParam) {
	BEGINGETGATE();
	CPlayer    *pCPlayer;
	CCharacter    *pCha = 0;
	GateServer    *pGateServer;
	while (pGateServer = GETNEXTGATE()) {
		if (!BEGINGETPLAYER(pGateServer))
			continue;
		int nCount = 0;
		while (pCPlayer = (CPlayer *)GETNEXTPLAYER(pGateServer))
		{
			if (++nCount > GETPLAYERCOUNT(pGateServer)) {
				LG("player chain list error", "player number:%u, %s\n", GETPLAYERCOUNT(pGateServer), "DealAllInGuild");
				break;
			}
			pCha = pCPlayer->GetCtrlCha();

			if (!pCha)
				continue;
			if (pCha->GetPlayer()->GetMainCha()->GetValidGuildID() == guildID) {
				//g_CParser.DoString(luaFunc, enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, pCha,enumSCRIPT_PARAM_STRING,1,luaParam, DOSTRING_PARAM_END);
				// angelix@pkodev.net 8/11/2019
				if (luaParam != "") {
					g_CParser.DoString(luaFunc, enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, pCha, enumSCRIPT_PARAM_STRING, 1, luaParam, DOSTRING_PARAM_END);
				}
				else {
					g_CParser.DoString(luaFunc, enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, pCha, DOSTRING_PARAM_END);
				}
			}
		}
	}
	return true;
}
// ����WorldID���������̵����н�ɫ
inline CCharacter* CGameApp::FindPlayerChaByID(unsigned long ulChaID)
{
	BEGINGETGATE();
	CPlayer	*pCPlayer;
	CCharacter	*pCha = 0;
	GateServer	*pGateServer;
	while (pGateServer = GETNEXTGATE())
	{
		if (!BEGINGETPLAYER(pGateServer))
			continue;
		int nCount = 0;
		while (pCPlayer = (CPlayer *)GETNEXTPLAYER(pGateServer))
		{
			if (++nCount > GETPLAYERCOUNT(pGateServer))
			{
				//LG("����������", "�����Ŀ:%u, %s\n", GETPLAYERCOUNT(pGateServer), "FindPlayerChaByID");
				LG("player chain error", "player number:%u, %s\n", GETPLAYERCOUNT(pGateServer), "FindPlayerChaByID");
				break;
			}
			pCha = pCPlayer->GetCtrlCha();
			if (!pCha)
				continue;
			if (pCha->GetID() == ulChaID) // �ҵ���ɫ
				return pCha;
		}
	}

	return 0;
}

// ��������ID���������̵��������
inline CPlayer* CGameApp::FindPlayerByDBChaID(unsigned long ulDBChaID)
{
	BEGINGETGATE();
	CPlayer	*pCPlayer;
	GateServer	*pGateServer;
	while (pGateServer = GETNEXTGATE())
	{
		if (!BEGINGETPLAYER(pGateServer))
			continue;
		int nCount = 0;
		while (pCPlayer = (CPlayer *)GETNEXTPLAYER(pGateServer))
		{
			if (++nCount > GETPLAYERCOUNT(pGateServer))
			{
				//LG("����������", "�����Ŀ:%u, %s\n", GETPLAYERCOUNT(pGateServer), "FindPlayerChaByID");
				LG("player chain error", "player number:%u, %s\n", GETPLAYERCOUNT(pGateServer), "FindPlayerChaByID");
				break;
			}
			if (pCPlayer->GetDBChaId() == ulDBChaID) // �ҵ���ɫ
				return pCPlayer;
		}
	}

	return 0;
}

// ����WorldID���������̵���������ɫ
inline CCharacter* CGameApp::FindMainPlayerChaByID(unsigned long ulChaID)
{
	BEGINGETGATE();
	CPlayer	*pCPlayer;
	CCharacter	*pCha = 0;
	GateServer	*pGateServer;
	while (pGateServer = GETNEXTGATE())
	{
		if (!BEGINGETPLAYER(pGateServer))
			continue;
		int nCount = 0;
		while (pCPlayer = (CPlayer *)GETNEXTPLAYER(pGateServer))
		{
			if (++nCount > GETPLAYERCOUNT(pGateServer))
			{
				//LG("����������", "�����Ŀ:%u, %s\n", GETPLAYERCOUNT(pGateServer), "FindPlayerChaByID");
				LG("player chain error", "player number:%u, %s\n", GETPLAYERCOUNT(pGateServer), "FindPlayerChaByID");
				break;
			}
			pCha = pCPlayer->GetMainCha();
			if (!pCha)
				continue;
			if (pCha->GetID() == ulChaID) // �ҵ���ɫ
				return pCha;
		}
	}

	return 0;
}

// ����WorldID���������̵����н�ɫ
inline CCharacter* CGameApp::FindChaByID(unsigned long ulChaID)
{
	CCharacter	*pCCha;
	m_pCEntSpace->BeginGetCha();
	while (pCCha = m_pCEntSpace->GetNextCha())
	{
		if (pCCha->GetID() == ulChaID)
			return pCCha;
	}

	return 0;
}

inline CCharacter* CGameApp::FindChaByName(const char* cszChaName)
{
	CCharacter	*pCCha;
	m_pCEntSpace->BeginGetCha();
	while (pCCha = m_pCEntSpace->GetNextCha())
	{
		if (!strcmp(pCCha->GetName(), cszChaName))
			return pCCha;
	}

	return 0;
}

enum EChaTimerAction
{
	enumCHA_TIMEER_ENTERMAP,
};


struct SSwitchMapInfo // ��ͼ�л���Ϣ
{
	SubMap		*pSrcMap;
	char		szSrcMapName[256];
	Point		SSrcPos;
	char		szTarMapName[256];
	Point		STarPos;
};

extern bool             g_bLogEntity;

extern CGameApp*        g_pGameApp;
extern CItemRecordAttr* g_pCItemAttr;
extern CCharacter*		g_pCSystemCha;		// ϵͳ��ɫ
extern SubMap *			g_pScriptMap;		// �ű��ó�ʼ����ͼ��Ϣȫ�ֱ���
extern long				g_lDeftMMaskLight;	// ���ͼĬ��������Χ
extern string			g_strChaState[2];	// 0���������ɫ�ļ���״̬��1����Ŵ��ļ���״̬
extern uLong			g_ulCurID;
extern Long				g_lCurHandle;
extern HANDLE			hConsole;

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

#define defINVALID_CHA_ID		0
#define defINVALID_CHA_HANDLE	-1

#endif // GAMEAPP_H