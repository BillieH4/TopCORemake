//=============================================================================
// FileName: Character.h
// Creater: ZhangXuedong
// Date: 2004.10.19
// Comment: CCharacter class
//=============================================================================

#ifndef CHARACTER_H
#define CHARACTER_H

#include "MoveAble.h"
#include "GameCommon.h"
#include "Mission.h"
#include "Timer.h"
#include "Kitbag.h"
#include "ShipSet.h"
#include "Action.h"


#define defMAX_ITEM_NUM			10
#define defCHA_SCRIPT_TIMER		1 * 1000
#define defDEFAULT_PING_VAL		500
#define defPING_RECORD_NUM		6
#define defPING_INTERVAL		20 * 1000
#define defCHA_SCRIPT_PARAM_NUM	1

extern CCharacter*		g_pCSystemCha;		// ϵͳ��ɫ

struct SLean // �п�
{
	dbc::uLong	ulPacketID; // ���ݰ���ID
	dbc::Char	chState;	// 0���п���.1��ֹͣ����
	dbc::Long	lPose;
	dbc::Long	lAngle;
	dbc::Long	lPosX, lPosY;
	dbc::Long	lHeight;
};

struct SSeat
{
	dbc::Char	chIsSeat;
	dbc::Short	sAngle;
	dbc::Short	sPose;
};

struct STempChaPart
{
	short	sPartID;
	short	sItemID;
};

struct SCheatX
{
	uInt Xtype;			//1:������ɺ� 2:���ʺ�
	uInt Xerror;		//�������
	uInt Xright;		//������ԵĴ���
	uInt Xcount;		//����Ĵ���
	uInt Xn;
	DWORD dwLastTime;	//��һ�ε�ʱ��
	DWORD dwInterval;	//ʱ����
	string Xnum;		//X number
};

enum EActControl
{
	enumACTCONTROL_MOVE,		// �ƶ�λ
	enumACTCONTROL_USE_GSKILL,	// ʹ����ͨ����λ
	enumACTCONTROL_USE_MSKILL,	// ʹ��ħ������λ
	enumACTCONTROL_BEUSE_SKILL,	// ��ʹ�ü���λ
	enumACTCONTROL_TRADE,		// ����λ
	enumACTCONTROL_USE_ITEM,	// ʹ����Ʒλ
	enumACTCONTROL_BEUSE_ITEM,	// ��ʹ����Ʒλ
	enumACTCONTROL_INVINCIBLE,	// �޵�λ
	enumACTCONTROL_EYESHOT,		// ��Ұλ�����Կ�����Ұ�ڿɼ���ʵ�壩
	enumACTCONTROL_NOHIDE,		// �����Σ����Ա�������
	enumACTCONTROL_NOSHOW,		// ����ǿ�����Σ������ǿ�����Σ������α����Σ�
	enumACTCONTROL_ITEM_OPT,	// ���߲���λ
	enumACTCONTROL_TALKTO_NPC,	// ��NPC�Ի�λ
	enumACTCONTROL_MAX,
};

enum ESwitchMapType
{
	enumSWITCHMAP_CARRY,	// ����
	enumSWITCHMAP_DIE,		// ����
};

enum ELogAssetsType	// ��ö��ֵ��Ӧ�����±�
{
	enumLASSETS_INIT,		// ��ʼ��
	enumLASSETS_TRADE,		// ����
	enumLASSETS_BANK,		// ����
	enumLASSETS_PICKUP,		// ʰȡ
	enumLASSETS_THROW,		// ����
	enumLASSETS_DELETE,		// ɾ��
};

namespace mission
{
	class CStallData;
	class CTradeData;
	class CTalkNpc;
}

#define LOOK_SELF   0
#define LOOK_OTHER  1
#define LOOK_TEAM   2

class CHateMgr;
class CAction;
class CActionCache;

class CCharacter : public CMoveAble
{
	friend class CChaSpawn;
	friend class CTableCha;
	friend class Guild;
	friend class CTableGuild;
	friend class CTableMaster;
public:
	CCharacter();
	~CCharacter();

	void	GetHelmString(char* buffer);
	void	SendChatLogPacket(char* Channel, char* msg);

	void	Initially();
	void	Finally();

	bool	IsPlayerCha(void); // �Ƿ���ҽ�ɫ
	bool	IsGMCha(); // GM�ȼ���0-10��GM��ɫ
	bool	IsGMCha2(); // GM��ɫ
	bool	IsPlayerCtrlCha(void); // ��ҵ�ǰ���ƵĽ�ɫ
	bool	IsPlayerMainCha(void); // ��ҵ��˽�ɫ
	bool	IsPlayerFocusCha(void); // ��ҵ�ǰ���ƵĽ�ɫ�� ͬIsPlayerCtrlCha����
	bool	IsPlayerOwnCha(void); // ��ҽ�ɫ������������������
	CCharacter	*GetPlyCtrlCha(void);
	CCharacter	*GetPlyMainCha(void);

	void	WritePK(WPACKET& wpk);			//д����ұ��������и��ӽṹ(���ٻ��޵�)����������
	void	WriteCharPartInfo(WPACKET& packet);
	void	ReadPK(RPACKET& rpk);			//�ع���ұ��������и��ӽṹ(���ٻ��޵�)
	void	SwitchMap(SubMap *pCSrcMap, cChar *szTarMapName, Long lTarX, Long lTarY, bool bNeedOutSrcMap = true, Char chSwitchType = enumSWITCHMAP_CARRY, Long lTMapCpyNO = -1);

	virtual void	ProcessPacket(uShort usCmd, RPACKET pk);
	virtual void	Run(uLong ulCurTick);
	virtual	void	RunEnd( DWORD dwCurTime );
	virtual void	OnScriptTimer(DWORD dwExecTime, bool bNotice = false);
	virtual void	StartExit();
	virtual void	CancelExit();
	virtual void	Exit();

	void	CheatRun(DWORD dwCurTime);
	void	CheatCheck(cChar *answer);
	void	CheatConfirm();
	void	InitCheatX();
	DWORD	GetCheatInterval(int state);

	// ָ���
	bool		Cmd_EnterMap(dbc::cChar* l_map, dbc::Long lMapCopyNO, dbc::uLong l_x, dbc::uLong l_y, dbc::Char chLogin = 1);
	void		Cmd_BeginMove(dbc::Short sPing, Point *pPath, dbc::Char chPointNum, dbc::Char chStopState = enumEXISTS_WAITING);
	void		Cmd_BeginMoveDirect(Entity *pTar);
	void		Cmd_BeginSkill(dbc::Short sPing, Point *pPath, dbc::Char chPointNum,
				CSkillRecord *pSkill, dbc::Long lSkillLv, dbc::Long lTarInfo1, dbc::Long lTarInfo2, dbc::Char chStopState = enumEXISTS_WAITING);
	void		Cmd_BeginSkillDirect(dbc::Long lSkillNo, Entity *pTar, bool bIntelligent = true);
	void		Cmd_BeginSkillDirect2(dbc::Long lSkillNo, dbc::Long lSkillLv, dbc::Long lPosX, dbc::Long lPosY);
	dbc::Short	Cmd_UseItem(dbc::Short sSrcKbPage, dbc::Short sSrcKbGrid, dbc::Short sTarKbPage, dbc::Short sTarKbGrid);
	dbc::Short	Cmd_UseEquipItem(dbc::Short sKbPage, dbc::Short sKbGrid, bool bRefresh = true,bool rightHand = false);
	dbc::Short	Cmd_UseExpendItem(dbc::Short sSrcKbPage, dbc::Short sSrcKbGrid, dbc::Short sTarKbPage, dbc::Short sTarKbGrid, bool bRefresh = true);
	dbc::Short	Cmd_UnfixItem(dbc::Char chLinkID, dbc::Short *psItemNum, dbc::Char chDir, dbc::Long lParam1, dbc::Long lParam2, bool bPriority = true, bool bRefresh = true, bool bForcible = false);
	dbc::Short	Cmd_PickupItem(dbc::uLong ulID, dbc::Long lHandle);
	dbc::Short	Cmd_ThrowItem(dbc::Short sKbPage, dbc::Short sKbGrid, dbc::Short *psThrowNum, dbc::Long lPosX, dbc::Long lPosY, bool bRefresh = true, bool bForcible = false);
	dbc::Short	Cmd_ItemSwitchPos(dbc::Short sKbPage, dbc::Short sSrcGrid, dbc::Short sSrcNum, dbc::Short sTarGrid);
	dbc::Short	Cmd_DelItem(dbc::Short sKbPage, dbc::Short sKbGrid, dbc::Short *psThrowNum, bool bRefresh = true, bool bForcible = false);
	dbc::Short	Cmd_BankOper(dbc::Char chSrcType, dbc::Short sSrcGridID, dbc::Short sSrcNum, dbc::Char chTarType, dbc::Short sTarGridID);
	dbc::Short	Cmd_GuildBankOper(dbc::Char chSrcType, dbc::Short sSrcGridID, dbc::Short sSrcNum, dbc::Char chTarType, dbc::Short sTarGridID);
	dbc::Short	Cmd_BagOfHoldingOper(dbc::Char chSrcType, dbc::Short sSrcGridID, dbc::Short sSrcNum, dbc::Char chTarType, dbc::Short sTarGridID);

	
    //�Ϸ���ʱ�����ĵ���(sSrcGrid:��ʱ������λ��   sSrcNum:����   sTarGrid:������λ��)
    dbc::Short  Cmd_DragItem(dbc::Short sSrcGrid, dbc::Short sSrcNum, dbc::Short sTarGrid);
    
	void		Cmd_SetInPK(bool bInPK = true) {if (m_chPKCtrl & 0x02) return; if (bInPK) m_chPKCtrl |= 0x01; else m_chPKCtrl &= 0xfe;}
	void		Cmd_SetInGymkhana(bool bInGymkhana = true) {if (bInGymkhana) m_chPKCtrl |= 0x02; else m_chPKCtrl &= 0xfd;}
	void		Cmd_ReassignAttr(RPACKET &pk);
	dbc::Short	Cmd_RemoveItem(dbc::Long lItemID, dbc::Long lItemNum, dbc::Char chFromType, dbc::Short sFromID, dbc::Char chToType, dbc::Short sToID, bool bRefresh = true, bool bForcible = true);

	void		Cmd_ChangeHair(RPACKET &pk);											// �����������
	void		Prl_ChangeHairResult(int nScriptID, const char* szReason, BOOL bNoticeAll = FALSE); // �������͵���Ϣ����
	void		Prl_OpenHair();															// ֪ͨ�ͻ��˴�������	

	void		Cmd_FightAsk(dbc::Char chType, dbc::Long lTarID, dbc::Long lTarHandle);
	void		Cmd_FightAnswer(bool bFight);
	void		Cmd_ItemRepairAsk(dbc::Char chPosType, dbc::Char chPosID);
	void		Cmd_ItemRepairAnswer(bool bRepair);
	void		Cmd_ItemForgeAsk(dbc::Char chType, SForgeItem *pSItem);
	void		Cmd_ItemForgeAnswer(bool bForge);

	// ADd by lark.li 20080515 begin
	void		Cmd_ItemLotteryAsk(SLotteryItem *pSItem);
	void		Cmd_ItemLotteryAnswer(bool bForge);
	// End
	
	//��������
	void		Cmd_Garner2_Reorder(short index);

	//�����
	void		Cmd_LifeSkillItemAsk(long dwType, SLifeSkillItem *pSItem);
	void		Cmd_LifeSkillItemAsR(long dwType,SLifeSkillItem *pSItem);
    //��������
    void        Cmd_LockKitbag();
    void        Cmd_UnlockKitbag(const char szPassword[]);
    void        Cmd_CheckKitbagState();
    void        Cmd_SetKitbagAutoLock(Char cAuto);

	//���ִ�����
	BOOL		Cmd_AddVolunteer();
	BOOL		Cmd_DelVolunteer();
	void		Cmd_ListVolunteer(short sPage, short sNum);
	BOOL		Cmd_ApplyVolunteer(const char *szName);
	CCharacter	*FindVolunteer(const char *szName);
	
	virtual void	ReflectINFof(Entity *srcent, WPACKET chginf);
	virtual CCharacter *IsCharacter(){return this;}

	void	TradeClear( CPlayer& player );
	bool	TradeAction(bool bLock = true) {return SetNarmalSkillState(bLock, SSTATE_TRADE, 1);}
	bool	StallAction(bool bLock = true);
	bool	HairAction(bool bLock = true) {return SetNarmalSkillState(bLock, SSTATE_HAIR, 1);}
	bool	RepairAction(bool bLock = true) {return SetNarmalSkillState(bLock, SSTATE_FORGE, 1);}
	bool	ForgeAction(bool bLock = true) {return SetNarmalSkillState(bLock, SSTATE_FORGE, 1);}

	void	BickerNotice( const char szData[], ... );
	void	SystemNotice( const char szData[], ... );
	void	PopupNotice( const char szData[], ... );
	void	ColourNotice( DWORD rgb, const char szData[], ... );
	bool	IsPKSilver();

	// ���ý�����Ϣ
	void	SetTradeData( mission::CTradeData* pData ) { m_pTradeData = pData; }
	mission::CTradeData* GetTradeData() { return m_pTradeData; }
	
	// ��ֻ
	void	SetBoat( CCharacter* pBoat );
	CCharacter* GetBoat();
	bool	IsBoat(void) {return m_pCChaRecord->chModalType == enumMODAL_BOAT;}
	
	// ��npc����
	BOOL	SafeSale( BYTE byIndex, BYTE byCount, WORD& wItemID, DWORD& dwMoney );
	BOOL	SafeBuy( WORD wItemID, BYTE byCount, BYTE byIndex, DWORD& dwMoney );
	BOOL	SafeSaleGoods( DWORD dwBoatID, BYTE byIndex, BYTE byCount, WORD& wItemID, DWORD& dwMoney );
	BOOL	SafeBuyGoods( DWORD dwBoatID, WORD wItemID, BYTE byCount, BYTE byIndex, DWORD& dwMoney );
	BOOL	GetSaleGoodsItem( DWORD dwBoatID, BYTE byIndex, WORD& wItemID );

	BOOL	ExchangeReq(short sSrcID, short sSrcNum, short sTarID, short sTarNum);

	bool	SetNarmalSkillState(bool bAdd = true, dbc::uChar uchStateID = 1, dbc::uChar uchStateLv = 1);
	bool	HasTradeAction(void) {return m_CSkillState.HasState(85);}
	//
	BOOL	SetMissionPage( DWORD dwNpcID, BYTE byPrev, BYTE byNext, BYTE byState );
	BOOL	GetMissionPage( DWORD dwNpcID, BYTE& byPrev, BYTE& byNext, BYTE& byState );
	BOOL	SetTempData( DWORD dwNpcID, WORD wID, BYTE byState, BYTE byType );
	BOOL	GetTempData( DWORD dwNpcID, WORD& wID, BYTE& byState, BYTE& byType );

	// ����ϵͳ�ӿں���
	BOOL	SaveMissionData();	// ��ɫ������Ϣ����

	BOOL	AddMissionState( DWORD dwNpcID, BYTE byID, BYTE byState );
	BOOL	ResetMissionState( mission::CTalkNpc& npc );
	
	BOOL	GetMissionState( DWORD dwNpcID, BYTE& byState );
	BOOL	GetNumMission( DWORD dwNpcID, BYTE& byNum );
	BOOL	GetMissionInfo( DWORD dwNpcID, BYTE byIndex, BYTE& byID, BYTE& byState );
	BOOL	GetCharMission( DWORD dwNpcID, BYTE byID, BYTE& byState );
	BOOL	GetNextMission( DWORD dwNpcID, BYTE& byIndex, BYTE& byID, BYTE& byState );
	BOOL	ClearMissionState( DWORD dwNpcID );
	
	BOOL	AddTrigger( const mission::TRIGGER_DATA& Data );
	BOOL	ClearTrigger( WORD wTriggerID );
	BOOL	DeleteTrigger( WORD wTriggerID );
	BOOL	GetMisScriptID( WORD wID, WORD& wScriptID );
	BOOL	AddRole( WORD wID, WORD wParam );
	BOOL	HasRole( WORD wID );
	BOOL	ClearRole( WORD wID );
	BOOL	IsRoleFull();
	BOOL	SetFlag( WORD wID, WORD wFlag );
	BOOL	ClearFlag( WORD wID, WORD wFlag );
	BOOL	IsFlag( WORD wID, WORD wFlag );
	BOOL	IsValidFlag( WORD wFlag );
	BOOL	SetRecord( WORD wRec );
	BOOL	ClearRecord( WORD wRec );
	BOOL	IsRecord( WORD wRec );
	BOOL	IsValidRecord( WORD wRec );

	// �������ӿڲ�����Ϣ
	BOOL	HasRandMission( WORD wRoleID );
	BOOL	AddRandMission( WORD wRoleID, WORD wScriptID, BYTE byType, BYTE byLevel, DWORD dwExp, DWORD dwMoney, USHORT sPrizeData, USHORT sPrizeType, BYTE byNumData );
	BOOL	SetRandMissionData( WORD wRoleID, BYTE byIndex, const mission::MISSION_DATA& RandData );
	BOOL	GetRandMission( WORD wRoleID, BYTE& byType, BYTE& byLevel, DWORD& dwExp, DWORD& dwMoney, USHORT& sPrizeData, USHORT& sPrizeType, BYTE& byNumData );
	BOOL	GetRandMissionData( WORD wRoleID, BYTE byIndex, mission::MISSION_DATA& RandData );

	// ����͸�npc����Ʒ(������Ʒ��NPCȡ����Ʒ�󣬼�¼һ����ǣ��Ƿ�ȡ���øñ�Ǳ�ʶ)
	BOOL	HasSendNpcItemFlag( WORD wRoleID, WORD wNpcID );
	BOOL	NoSendNpcItemFlag( WORD wRoleID, WORD wNpcID );
	BOOL	HasRandMissionNpc( WORD wRoleID, WORD wNpcID, WORD wAreaID );

	// ȡ�����������Ʒ
	BOOL	TakeRandNpcItem( WORD wRoleID, WORD wNpcID, const char szNpc[] );
	BOOL	TakeAllRandItem( WORD wRoleID );

	// �Ƿ�������Ҫ����Ʒ
	BOOL	IsMisNeedItem( USHORT sItemID );
	BOOL	GetMisNeedItemCount( WORD wRoleID, USHORT sItemID, USHORT& sCount );
	void	RefreshNeedItem( USHORT sItemID );

	// ������־
	void	MisLog();
	void	MisLogInfo( WORD wMisID );
	void	MisLogClear( WORD wMisID );

	// ���������Ѿ��������״̬
	BOOL	SetMissionComplete( WORD wRoleID );
	BOOL	SetMissionFailure( WORD wRoleID );
	BOOL	HasMissionFailure( WORD wRoleID );

	// ���������ɼ����ӿ�
	BOOL	CompleteRandMission( WORD wRoleID );
	BOOL	FailureRandMission( WORD wRoleID );
	BOOL	AddRandMissionNum( WORD wRoleID );
	BOOL	ResetRandMission( WORD wRoleID );
	BOOL	ResetRandMissionNum( WORD wRoleID );
	BOOL	HasRandMissionCount( WORD wRoleID, WORD wCount );
	BOOL	GetRandMissionCount( WORD wRoleID, WORD& wCount );
	BOOL	GetRandMissionNum( WORD wRoleID, WORD& wNum );

	// �ٻ�һ��������NPC
	BOOL	ConvoyNpc( WORD wRoleID, BYTE byIndex, WORD wNpcCharID, BYTE byAiType );
	BOOL	ClearConvoyNpc( WORD wRoleID, BYTE byIndex );
	BOOL	ClearAllConvoyNpc( WORD wRoleID );
	BOOL	HasConvoyNpc( WORD wRoleID, BYTE byIndex );
	BOOL	IsConvoyNpc( WORD wRoleID, BYTE byIndex, WORD wNpcCharID );

	// ��ɫ��Ǯ����Ʒ��������
	void	AddMoney( const char szName[], DWORD dwMoney );
	BOOL	TakeMoney( const char szName[], DWORD dwMoney );
	BOOL	HasMoney( DWORD dwMoney );
	BOOL	AddItem( USHORT sItemID, USHORT sCount, const char szName[], BYTE byAddType = enumITEM_INST_TASK, BYTE bySoundType = enumSYN_KITBAG_FROM_NPC );
	BOOL	TakeItem( USHORT sItemID, USHORT sCount, const char szName[] );
	BOOL	GiveItem( USHORT sItemID, USHORT sCount, BYTE byAddType, BYTE bySoundType );
	BOOL	MakeItem( USHORT sItemID, USHORT sCount, USHORT& sItemPos, BYTE byAddType = enumITEM_INST_TASK, BYTE bySoundType = enumSYN_KITBAG_FROM_NPC );
	BOOL	HasItem( USHORT sItemID, USHORT sCount );
	BOOL	GetNumItem( USHORT sItemID, USHORT& sCount );
	BOOL	HasLeaveBagGrid( USHORT sNum );
	BOOL	HasLeaveBagTempGrid( USHORT sNum );
	BOOL	HasItemBagTemp(USHORT sItemID, USHORT sCount);
	BOOL	TakeItemBagTemp(USHORT sItemID, USHORT sCount, const char szName[]);

	//�����Ʒ����ʱ����
	BOOL	AddItem2KitbagTemp( USHORT sItemID, USHORT sCount, ItemInfo *pItemAttr, BYTE bySoundType = enumSYN_KITBAG_FROM_NPC );
	BOOL	AddItem2KitbagTemp( USHORT sItemID, USHORT sCount, const char szName[], BYTE byAddType = enumITEM_INST_TASK, BYTE bySoundType = enumSYN_KITBAG_FROM_NPC );
	BOOL	GiveItem2KitbagTemp( USHORT sItemID, USHORT sCount, ItemInfo *pItemAttr, BYTE bySoundType );
	BOOL	GiveItem2KitbagTemp( USHORT sItemID, USHORT sCount, BYTE byAddType, BYTE bySoundType );

	// �����ɫְҵ����
	BOOL	SetProfession( BYTE byPf );

	bool	LearnSkill(dbc::Short sSkillID, dbc::Char chLv, bool bSetLv = true, bool bUsePoint = true, bool bLimit = true); // ѧϰ���ܣ�ͨ�棩
	bool	AddSkillState(dbc::uChar uchFightID, dbc::uLong ulSrcWorldID, dbc::Long lSrcHandle, dbc::Char chObjType, dbc::Char chObjHabitat, dbc::Char chEffType,
			dbc::uChar uchStateID, dbc::uChar uchStateLv, dbc::Long lOnTick, dbc::Char chType = enumSSTATE_ADD_UNDEFINED, bool bNotice = true); // ���Ӽ���״̬
	bool	DelSkillState(dbc::uChar uchStateID, bool bNotice = true); // ɾ��״̬

	// ��Ϊ����
	bool	GetActControl(dbc::Char chCtrlType) {return m_ActContrl[chCtrlType];}
	void	Hide();
	void	Show();

	// �ָ���ɫ����
	void	RestoreHp( BYTE byHpRate );
	void	RestoreSp( BYTE bySpRate );
	void	RestoreAllHp();
	void	RestoreAllSp();
	void	RestoreAll();

	BOOL	ViewItemInfo( RPACKET& pk );

	BOOL	AddAttr( int nIndex, DWORD dwValue, dbc::Short sNotiType = enumATTRSYN_TASK ); // ��Ҫ����ATTR_CEXP���ԣ���ʹ��CFightAble::AddExp
	BOOL	TakeAttr( int nIndex, DWORD dwValue, dbc::Short sNotiType = enumATTRSYN_TASK );

	bool	IsInPK(void) {return m_chPKCtrl & 0x01 ? true : false;}
	bool	IsInGymkhana(void) {return m_chPKCtrl & 0x02 ? true : false;}
	void	SetPKCtrl(dbc::Char chCtrl) {m_chPKCtrl = chCtrl;}
	dbc::Char	GetPKCtrl(void) {return m_chPKCtrl;}
	bool	CanPK(void) {return IsInPK() || IsInGymkhana();}
	bool	IsInArea(dbc::Short sAreaMask) {return GetAreaAttr() & sAreaMask ? true : false;}
	void	SetRelive(Char chType = enumEPLAYER_RELIVE_ORIGIN, Char chLv = 0, cChar *szInfo = 0);
	void	Reset(void);

	virtual void BreakAction(RPACKET pk = NULL);
	virtual void EndAction(RPACKET pk = NULL);
	// ��ɫ�¼�������
	virtual void AfterObjDie(CCharacter *pCAtk, CCharacter *pCDead);
	virtual void AfterPeekItem(dbc::Short sItemID, dbc::Short sNum);
	virtual void AfterEquipItem(dbc::Short sItemID, dbc::uShort sTriID);
	virtual void EntryMapUnit( BYTE byMapID, WORD wxPos, WORD wyPos );
	virtual void OnMissionTime(); // ����ʱ�䴥�����¼�
	virtual void OnLevelUp( USHORT sLevel );
	virtual void OnSailLvUp( USHORT sLevel );
	virtual void OnLifeLvUp( USHORT sLevel );
	virtual void OnCharBorn();

	// ��ֻ����ӿں���
	BOOL	IsNeedRepair();
	BOOL	IsNeedSupply();
	void	RepairBoat();
	void	SupplyBoat();
	void	BoatDie( CCharacter& Attacker, CCharacter& Boat );
	BOOL	OnBoatDie( CCharacter& Attacker );
	BOOL	GetBoatID( BYTE byIndex, DWORD& dwBoatID );
	BOOL	BoatCreate( const BOAT_DATA& Data );
	BOOL	BoatUpdate( BYTE byIndex, const BOAT_DATA& Data );
	BOOL	BoatLoad( const BOAT_LOAD_INFO& Info );
	
	// ��ֻ����ó��
	BOOL	AdjustTradeItemCess( USHORT sLowCess, USHORT sData );
	BOOL	GetTradeItemData( BYTE& byLevel, USHORT& sCess );
	BOOL	SetTradeItemLevel( BYTE byLevel );	
	BOOL	HasTradeItemLevel( BYTE byLevel );
	BOOL	GetTradeItemLevel( BYTE& byLevel );
	BOOL	BoatTrade( USHORT sBerthID );	

	BOOL	BoatBerth( USHORT sBerthID, USHORT sxPos, USHORT syPos, USHORT sDir );	
	BOOL	BoatLaunch( BYTE byIndex, USHORT sBerthID, USHORT sxPos, USHORT syPos, USHORT sDir );
	BOOL	BoatBerthList( DWORD dwNpcID, BYTE byType, USHORT sBerthID, USHORT sxPos, USHORT syPos, USHORT sDir );
	BOOL	BoatSelLuanch( BYTE byIndex );
	BOOL	BoatSelected( BYTE byType, BYTE byIndex );
	BOOL	HasAllBoatInBerth( USHORT sBerthID );
	BOOL	HasBoatInBerth( USHORT sBerthID );
	BOOL	HasDeadBoatInBerth( USHORT sBerthID );
	void	SetBoatLook( const stNetChangeChaPart& Info ) { memcpy( &m_SChaPart, &Info, sizeof(stNetChangeChaPart) ); }
	BOOL	BoatPackBagList( USHORT sBerthID, BYTE byType, BYTE byLevel );
	BOOL	BoatPackBag( BYTE byIndex );
	BOOL	PackBag( CCharacter& boat, BYTE byType, BYTE byLevel );
	BOOL	PackBag( CCharacter& Boat, USHORT sItemID, USHORT sCount, USHORT sPileID, USHORT& sNumPack );
	void	SetBoatAttrChangeFlag(bool bSet = true);
	void	SyncBoatAttr( dbc::Short sSynType, bool bAllBoat = true ); // ͬ������Ĵ�ֻ����

	// ��ֻ���ɾ��
	BOOL	BoatAdd( CCharacter& Boat );
	BOOL	BoatClear( CCharacter& Boat );
	BOOL	BoatAdd( DWORD dwDBID );
	BOOL	BoatClear( DWORD dwDBID );

	// �¼�ʵ�彻��ʱ���¼
	BOOL	SetEntityState( DWORD dwEntityID, BYTE byState );
	void	SetEntityTime( DWORD dwTime );
	DWORD	GetEntityTime();

	// �����ж�����
	BOOL	HasGuild();
	BOOL	IsGuildType( BYTE byType );

	// ��̯
	void	SetStallData( mission::CStallData* pData );
	mission::CStallData* GetStallData();
	BYTE	GetStallNum();

	//add by jilinlee 2007/4/20
	//����
	BOOL IsReadBook();
	void SetReadBookState(bool bIsReadBook = false);


	// 
	void	ChangeItem(bool bEquip, SItemGrid *pItemCont, dbc::Char chLinkID); // ������װ���仯�����Ľ�ɫ���Եı仯
	void	SkillRefresh(); // ���߼����
	// �½���ɫ��ʼ��
	void	NewChaInit(void);
	// �½���ɫ��װ����ʼ��
	void	ChaInitEquip(void);
	void	ResetBirthInfo(void);

	// ͬ����ɫ����
	void	SynKitbagNew(dbc::Char chType); // ͬ��������
	void    SynKitbagTmpNew(dbc::Char chType); // ͬ����ʱ����
	void	SynShortcut(); // ͬ�������
	void	SynLook(dbc::Char chSynType = enumSYN_LOOK_SWITCH); // ͬ����ɫ���
	void	SynLook(dbc::Char chLookType, bool verbose);
	bool	ItemForge(SItemGrid *pItem, dbc::Char chAddLv = 1); // �������߲�ͬ��
	void	SynSkillBag(dbc::Char chType); // ͬ��������
	void	SynPKCtrl(void); // ͬ��PK״̬
	void	SynAddItemCha(CCharacter *pCItemCha);
	void	SynDelItemCha(CCharacter *pCItemCha);
	void	CheckPing(void);
	void	SendPreMoveTime(void);
	void	SynSideInfo(void);
	void	SynBeginItemRepair(void);
	void	SynBeginItemForge(void);
	
	void	SynBeginItemLottery(void);	//Add by lark.li 20080513

	void	SynBeginItemUnite(void);
	void	SynBeginItemMilling(void);
	void	SynBeginItemFusion();
	void    SynBeginItemUpgrade();
	void    SynBeginItemEidolonMetempsychosis();
	void	SynBeginItemEidolonFusion();
	void	SynBeginItemPurify();
	void	SynBeginItemFix();
	void	SynBeginItemEnergy();
	void	SynBeginGetStone();
	void	SynBeginTiger();
	void	SynAppendLook(void);
	void	SynItemUseSuc(dbc::Short sItemID);
	void	SynKitbagCapacity(void);
	void	SynEspeItem(void);
	void	SynVolunteerState(BOOL bState);
	void	SynTigerString(cChar *szString);
	void	SynBeginGMSend();
	void	SynBeginGMRecv(DWORD dwNpcID);

	//

	// ��ɫ���ݱ���֯
	void	WriteBaseInfo(WPACKET &pk, dbc::Char chLookType = LOOK_SELF);
	void	WritePKCtrl(WPACKET &pk);
	void	WriteSkillbag(WPACKET &pk, int nSynType);
	void	WriteKitbag(CKitbag &CKb, WPACKET &pk, int nSynType);
	void	WriteLookData(WPACKET &pk, dbc::Char chLookType = LOOK_SELF, dbc::Char chSynType = enumSYN_LOOK_SWITCH);
	bool	WriteAppendLook(CKitbag &CKb, WPACKET &pk, bool bInit = false);
	void	WriteShortcut(WPACKET &pk);
	void	WriteBoat(WPACKET &pk);
	void	WriteItemChaBoat(WPACKET &pk, CCharacter *pCBoat);
	void	WriteSideInfo(WPACKET &pk);
	//

	// ������ж�ʧ��
	void	FailedActionNoti(dbc::Char chType, dbc::Char chReason);
	// �ն���ʾ��Ϣ
	void	TerminalMessage(dbc::Long lMessageID);
	// ���߲���ʧ��
	void	ItemOprateFailed(dbc::Short sFailedID);

	void		SetMotto(dbc::cChar *szMotto) {if (szMotto) strncpy(m_szMotto, szMotto, defMOTTO_LEN - 1);}
	dbc::cChar*	GetMotto(void) {return m_szMotto;}
	void		SetIcon(dbc::uShort usIcon) {m_usIcon = usIcon;}
	dbc::uShort	GetIcon(void) {return m_usIcon;}
	void		SetGuildName(dbc::cChar *szGuildName);
	dbc::cChar*	GetGuildName(void);
	dbc::cChar*	GetValidGuildName(void);
	void		SetGuildMotto(dbc::cChar *szGuildMotto);
	dbc::cChar*	GetGuildMotto(void);
	dbc::cChar*	GetValidGuildMotto(void) ;
	void		SetGuildID( DWORD dwGuildID );
	DWORD		GetGuildID();
	DWORD		GetValidGuildID();
	DWORD		GetValidGuildCircleColour();
	int			GetValidGuildIcon();
	void		SetGuildType( BYTE byType );
	BYTE		GetGuildType();
	void		SetGuildState( uLong lState );
	uLong		GetGuildState();
	void		SetEnterGymkhana(bool bEnter = true);
	void		SyncGuildInfo();
	void		SetStallName(dbc::cChar *szStallName);
	dbc::cChar*	GetStallName(void);
	void		SynStallName(void);

	void			AddBlockCnt()						{   _btBlockCnt++;					}
	BYTE			GetBlockCnt()						{   return _btBlockCnt;				}
	void			SetBlockCnt(BYTE cnt)				{   _btBlockCnt = cnt;				}

	virtual void	AfterAttrChange(int nIdx, dbc::Long lOldVal, dbc::Long lNewVal);
	virtual void	Die();	// ��������
	void			JustDie(CCharacter *pCSrcCha);	// �����ϣ���������
	void			MoveCity(dbc::cChar *szCityName, Long lMapCpyNO = -1, Char chSwitchType = enumSWITCHMAP_CARRY);
	void			BackToCity(bool Die = false, cChar *szCityName = 0, Long lMapCpyNO = -1, Char chSwitchType = enumSWITCHMAP_DIE);
	void			BackToCityEx(bool Die = false, cChar *szCityName = 0, Long lMapCpyNO = -1, Char chSwitchType = enumSWITCHMAP_DIE);
	void			SetToMainCha(bool bBoatDie = false);
	bool			CanSeen(CCharacter *pCCha);
	bool			CanSeen(CCharacter *pCCha, bool bThisEyeshot, bool bThisNoHide, bool bThisNoShow);
	bool			IsHide() {return !GetActControl(enumACTCONTROL_NOHIDE) && GetActControl(enumACTCONTROL_NOSHOW);}
	SItemGrid*		GetEquipItem(dbc::Char chPart) {if (chPart >= enumEQUIP_NUM || chPart < 0) return 0; if (!g_IsRealItemID(m_SChaPart.SLink[chPart].sID)) return 0; return &m_SChaPart.SLink[chPart];}
	DWORD			GetTeamID();
	bool			IsTeamLeader();
	dbc::Long		GetSideID() {return m_lSideID;}
	void			SetSideID(dbc::Long lSideID);
	SItemGrid*		GetItem(dbc::Char chPosType, dbc::Long lItemID);
	SItemGrid*		GetItem2(dbc::Char chPosType, dbc::Long lPosID);
	bool			SetEquipValid(dbc::Char chEquipPos, bool bValid, bool bSyn = true);
	bool			SetKitbagItemValid(dbc::Short sPosID, bool bValid, bool bRecheckAttr = true, bool bSyn = true);
	bool			SetKitbagItemValid(SItemGrid* pSItem, bool bValid, bool bRecheckAttr = true, bool bSyn = true);
	bool			ItemIsAppendLook(SItemGrid* pSItem);
	void			SetLookChangeFlag(bool bChange = false);
	void			SetEspeItemChangeFlag(bool bChange = false);
	dbc::Char		GetLookChangeNum(void);
	bool			CheckForgeItem(SForgeItem *pSItem = NULL);
	bool			DoForgeLikeScript(dbc::cChar *cszFunc, dbc::Long &lRet);
	bool			DoLifeSkillcript(dbc::cChar *cszFunc, dbc::Long &lRet);
	bool			DoTigerScript(dbc::cChar *cszFunc);
	void			SetInOutMapQueue(bool bOutMap = true) {m_bInOutMapQueue = bOutMap;}
	bool			InOutMapQueue(void) {return m_bInOutMapQueue;}
	bool			AddKitbagCapacity(dbc::Short sAddVal);
	void			CheckItemValid(SItemGrid* pCItem);
	void			CheckEspeItemGrid(void);
	dbc::Short		KbPushItem(bool bRecheckAttr, bool bSynAttr, SItemGrid *pGrid, dbc::Short &sPosID, dbc::Short sType = 0, bool bCommit = true, bool bSureOpr = false);
	dbc::Short		KbPopItem(bool bRecheckAttr, bool bSynAttr, SItemGrid *pGrid, dbc::Short sPosID, dbc::Short sType = 0, bool bCommit = true);
	dbc::Short		KbClearItem(bool bRecheckAttr, bool bSynAttr, dbc::Short sPosID, dbc::Short sType = 0);
	dbc::Short		KbClearItem(bool bRecheckAttr, bool bSynAttr, SItemGrid *pGrid, dbc::Short sNum = 0);
	dbc::Short		KbRegroupItem(bool bRecheckAttr, bool bSynAttr, dbc::Short sSrcPosID, dbc::Short sSrcNum, dbc::Short sTarPosID, dbc::Short sType = 0);
	void			ResetScriptParam(void) {memset(m_lScriptParam, 0, sizeof(m_lScriptParam));}
	dbc::Long		GetScriptParam(dbc::Char chID) {if (chID >= 0 && chID < defCHA_SCRIPT_PARAM_NUM) return m_lScriptParam[chID]; else return -1;}
	bool			SetScriptParam(dbc::Char chID, dbc::Long lVal) {if (chID >= 0 && chID < defCHA_SCRIPT_PARAM_NUM) {m_lScriptParam[chID] = lVal; return true;} else return false;}
	void			CheckBagItemValid(CKitbag* pCBag);
	void			CheckLookItemValid(void);
	bool			String2LookDate(std::string &strData);
	bool			String2KitbagData(std::string &strData);
    bool            String2KitbagTmpData(std::string &strData);


	void	SetKitbagRecDBID(long lDBID) {m_lKbRecDBID = lDBID;}
	long	GetKitbagRecDBID(void) {return m_lKbRecDBID;}

    //��ʱ����ID
    void	SetKitbagTmpRecDBID(long lDBID) {m_lKbTmpRecDBID = lDBID;}
	long	GetKitbagTmpRecDBID(void) {return m_lKbTmpRecDBID;}

	void	LogAssets(dbc::Char chLType);
	bool	SaveAssets(void);
	bool	IsRangePoint(dbc::Long lPosX, dbc::Long lPosY, dbc::Long lDist);
	bool	IsRangePoint(const Point &SPos, dbc::Long lDist) {return IsRangePoint(SPos.x, SPos.y, lDist);}
	bool	IsRangePoint2(dbc::Long lPosX, dbc::Long lPosY, dbc::Long lDist2);
	bool	IsRangePoint2(const Point &SPos, dbc::Long lDist2) {return IsRangePoint2(SPos.x, SPos.y, lDist2);}
	void	SetDBSaveInterval(long lIntl) {m_timerDBUpdate.Begin(lIntl);}
	long	GetDBSaveInterval(void) {return m_timerDBUpdate.GetInterval();}
	void	ResetPosState(void) {m_sPoseState = enumPoseStand; m_SSeat.chIsSeat = 0;}

	int		GetLotteryIssue();

	DWORD				m_dwBoatCtrlTick; // �����ȼ�ʱ��

	// AIʹ�õı����ͽӿں���----------------------------------------------------------------
	// ��õķ�ʽ�Ƿ�װһ�� CAICharacter, ����Character�̳�
	BYTE				m_AIType;
	CCharacter*			m_AITarget;		
	CCharacter*			m_HostCha;		  // ���������
	int					m_nPatrolX;       // Ѳ�ߵ�����
	int					m_nPatrolY;
	short				m_sChaseRange;    // ��ɫ��׷�ٷ�Χ, ���������Χ��ɫ�ͻ�س���������
	BYTE				m_btPatrolState;  // Ѳ��״̬, 0 ��ʾͣ���ڵ�1
	                                      //           1 ��ʾͣ���ڵ�2
                                          //           2 ��ʾ���ӵ�1ǰ����2
	                                      //           3 ��ʾ���ӵ�2ǰ����1
public:

	void	ResetAIState();				  // ����AI״̬, �ڽ�ɫ������ʱ�����

	BOOL		GetChaRelive() { return m_bRelive; }
	void		SetChaRelive() { m_bRelive = true; }
	void		ResetChaRelive() { m_bRelive = false; }

	void		SetVolunteer(BOOL bVol) { m_bVol = bVol; }
	BOOL		IsVolunteer() { return m_bVol; }
	void		SetInvited(BOOL bInvited) { m_bInvited = bInvited; }
	BOOL		IsInvited() { return m_bInvited; }

	void			SetCredit(long lCredit) { setAttr(ATTR_FAME, lCredit); }
	dbc::Long			GetCredit() { return getAttr(ATTR_FAME); }
	void			AddMasterCredit(long lCredit);
	unsigned long	GetMasterDBID();

	long			GetStoreItemID() { return m_lStoreItemID; }
	void			SetStoreItemID(long lStoreItemID) { m_lStoreItemID = lStoreItemID; }
	bool			IsStoreBuy() { return m_bStoreBuy; }
	void			SetStoreBuy(bool bValue) { m_bStoreBuy = bValue; }
	int				GetPetNum() { return m_nPetNum; }
	void			SetPetNum(int nPetNum) { m_nPetNum = nPetNum; }

	bool			CheckStoreTime(DWORD dwInterval) { return (GetTickCount() - m_dwStoreTime > dwInterval) ? true : false; }
	void			ResetStoreTime() { m_dwStoreTime = GetTickCount(); }

	bool			IsStoreEnable() { return m_bStoreEnable; }
	void			SetStoreEnable(bool bStoreEnable) { m_bStoreEnable = bStoreEnable; }

    //  ������
    bool IsScaleFlag(){return m_expFlag;}
    void SetScaleFlag(){m_expFlag = true;}
    void SetExpScale(DWORD scale){ m_ExpScale = scale; }
    DWORD GetExpScale(){ return m_ExpScale; }
    int m_noticeState;//������ʱ��֪ͨ״̬
	int m_retry3;
	int m_retry4;
	int m_retry5;
    int m_retry6;

	unsigned long guildPermission;
	unsigned int guildCircleColour;

	unsigned int chatColour;

	int guildIcon;
	int bagOfHoldingID;

protected:

public:

    virtual void OnBeginSee(Entity *);
	virtual void OnEndSee(Entity *);
    virtual void OnBeginSeen(CCharacter *pCCha);
	virtual void OnEndSeen(CCharacter *pCCha);
	virtual void AreaChange(void);

	virtual void OnAI(DWORD dwCurTime);
    virtual void OnAreaCheck(DWORD dwCurTime);
	virtual void OnTeamNotice(DWORD dwCurTime);
    virtual void OnDBUpdate(DWORD dwCurTime);

	virtual void BeginAction(RPACKET pk);
	virtual void AfterStepMove(void);
	virtual void SubsequenceFight();
	virtual void SubsequenceMove();

	void	OnDie(DWORD dwCurTime);
	void	SrcFightTar(CFightAble *pTar, dbc::Short sSkillID);
	
	void	DoCommand(dbc::cChar *cszCommand, dbc::uLong ulLen);
	BOOL	DoGMCommand(const char *pszCmd, const char *pszParam);
	void	DoCommand_CheckStatus(dbc::cChar *pszCommand, dbc::uLong ulLen);
	void	HandleHelp(dbc::cChar *pszCommand, dbc::uLong ulLen);

	long	ExecuteEvent(Entity *pCObj, dbc::uShort usEventID);

	void	SetActControl(dbc::Char chCtrlType, bool bSet = true) {m_ActContrl[chCtrlType] = bSet;}

	bool		CanLearnSkill(CSkillRecord *pCSkill, dbc::Char chToLv);
	dbc::Short	CanEquipItem(dbc::Short sItemID);
	dbc::Short	CanEquipItemNew(dbc::Short sItemID1, dbc::Short sItemID2 = 0);

	dbc::Short	CanEquipItem(SItemGrid* pSEquipIt);

public:
	CKitbag				m_CKitbag;			// ������
	CKitbag				*m_pCKitbagTmp;       // ��ʱ����
	stNetShortCut		m_CShortcut;		// �����
	long				m_lKbRecDBID;		// ����������Դ���е�ID
	long				m_lKbTmpRecDBID;	// ��ʱ��������Դ���е�ID
	long				m_lStoreItemID;
	bool				m_bStoreBuy;
	DWORD				m_dwStoreTime;
	bool				m_bStoreEnable;

	int					m_nPetNum;
	
	stNetChangeChaPart	m_SChaPart;
	bool				m_ActContrl[enumACTCONTROL_MAX];	// ��ɫ���ж�����
	CTimer				m_timerScripts;						// ����HP��SP�ȵĻָ�

	CHateMgr*			m_pHate;							// ��޶�

	BOOL				m_bRelive;							// �Ƿ����ڱ����ܸ���

	BOOL				m_bVol;								// �Ƿ���־Ը��
	BOOL				m_bInvited;							// �Ƿ����ڱ�����

	//SSkillGrid			*m_pSkillGridTemp;

	// ������Ϣ
	struct
	{
		Char	m_chSelRelive;	// ѡ�񸴻ʽ
		Char	m_chReliveLv;	// ����ȼ����������0�����ʾ���ڷ���Ȼ����Ŀ���.
	};

	CActionCache		m_CActCache;

	DWORD	m_dwCellRunTime[16];

	short	m_sTigerItemID[9];	// �ϻ�����9������ID
	short	m_sTigerSel[3];		// Ͷע��־
	
private:
	BOOL BoatEnterMap( CCharacter& Boat, DWORD dwxPos, DWORD dwyPos, USHORT sDir );

	dbc::Char			m_szMotto[defMOTTO_LEN];
	dbc::uShort			m_usIcon;

    bool m_expFlag;
    DWORD m_ExpScale;       //  �����ԣ��������

	struct
	{
		short m_sPoseState; // 0��վ.1���п�
	};
	//add by jilinlee 2007/4/20
	struct SReadBook
	{
		bool bIsReadState;       //0,���ڶ���״̬.1���ڶ���״̬.
        DWORD dwLastReadCallTick; //�ϴε���Reading_Book�ű�������ʱ��.
	};
    
	SReadBook m_SReadBook;

	CAction		        m_CAction;
	SLean		        m_SLean;
	SSeat				m_SSeat;

	SCheatX				m_sCheatX;
	
	BYTE				_btBlockCnt;
	STempChaPart        m_STempChaPart;

	// ������Ϣ
	mission::CTradeData*	m_pTradeData;

	#define CHAEXIT_NONE				0		// ��Ч״̬����ɫ����ʹ����...��
	#define CHAEXIT_BEGIN				1<<0	// �Ѿ���ʼ�˳�״̬	

	BYTE				m_byExit;
	CTimer				m_timerExit;
    CTimer              m_timerAI;
    CTimer              m_timerAreaCheck;
	CTimer				m_timerDBUpdate;
	CTimer				m_timerDie;
	CTimer				m_timerMission;			// ����ϵͳ���Ӽ�ʱ�������¼�
    CTimer				m_timerSkillState;		// ����״̬��ʱ��
	CTimer              m_timerTeam;			// Team��Ϣ��ʱ֪ͨ�ͻ���
	struct
	{
		CTimer			m_timerPing;
		dbc::uLong		m_ulPingDataLen;
	};

	dbc::Char			m_chPKCtrl;				// ���λ��ʾ�Ƿ���PK�������λ��ʾPK�����Ƿ��.������PK��ʱ��ϵͳ��Ϊ�����Ǵ򿪵ģ�����ֹ���ÿ���.�����ش�ʱ���Ϳ��Թ��������ң��������⣩����ʱ������ܻ����Ŀ����ǹرյģ���Ҫ�۹������ġ�������ֵ

	dbc::Long			m_lSideID;				// �ֱ߱��
	bool				m_bInOutMapQueue;
	struct // Ping
	{
		DWORD	m_dwPing;
		DWORD	m_dwPingRec[defPING_RECORD_NUM];
		DWORD	m_dwPingSendTick;
	};

	struct // for test net state
	{
		dbc::uLong	m_ulNetSendLen;
		CTimer		m_timerNetSendFreq;
	};

	dbc::Long	m_lScriptParam[defCHA_SCRIPT_PARAM_NUM];

protected:

    DWORD   _dwLastAreaTick;
    BYTE    _btLastAreaNo;

	DWORD	_dwLastSayTick;
	

public:
	void	ResetLifeTime(DWORD dwTime)
	{
		_dwLifeTime = dwTime;
		_dwLifeTimeTick = GetTickCount();
	}

	BOOL	CheckLifeTime()
	{
		if(_dwLifeTime==0) return FALSE; // ����Ҫ����ʱ���ʱ
		
		if((GetTickCount() - _dwLifeTimeTick) > _dwLifeTime) // ����ʱ���ѵ�
		{
			return TRUE;
		}
		return FALSE;
	}

	DWORD GetLifeTime() { return _dwLifeTime; }
	
	DWORD	_dwLifeTime;
	DWORD	_dwLifeTimeTick;
	DWORD	_dwStallTick;

	bool	appCheck[enumEQUIP_NUM];

	BYTE	requestType;
	Square	requestPos; // must check if player is in same position

	bool IsReqPosEqualRealPos() {
		return (requestPos.centre.x == GetShape().centre.x &&
				requestPos.centre.y == GetShape().centre.y);
	}
};

// ������ű�֮��Ľ���
extern Point		g_SSkillPoint;
extern bool			g_bBeatBack;
extern unsigned char	g_uchFightID;
extern char			g_chUseItemFailed[2];
extern char			g_chUseItemGiveMission[2];
//

extern char*	SStateData2String(CCharacter *pCCha, char *szSStateBuf, int nLen);

/**
 * [p]ersist
 * Saves any player stats that were configured in GameServer[x].cfg
 * 07.27.2018
 */
extern char*	PStateData2String(CCharacter *pCCha, char *szSStateBuf, int nLen);
extern bool		Strin2SStateData(CCharacter *pCCha, std::string &strData);
//Add by lark.li 20080723
extern char*	ChaExtendAttr2String(CCharacter *pCCha, char *szAttrBuf, int nLen);
extern bool		Strin2ChaExtendAttr(CCharacter *pCCha, std::string &strAttr);
 //������־��¼�ӿ�
extern void TL(int nType, const char *pszCha1, const char *pszCha2, const char *pszTrade);

#endif // CHARACTER_H