//=============================================================================
// FileName: Character.cpp
// Creater: ZhangXuedong
// Date: 2004.10.19
// Comment: CCharacter class 
//=============================================================================

#include "stdafx.h"
#include "GameApp.h"
#include "Character.h"
#include "SubMap.h"
#include "NPC.h"
#include "Item.h"
#include "Script.h"
#include "CharTrade.h"
#include "Parser.h"
#include "GameDB.h"
#include "CommFunc.h"
#include "Player.h"
#include "ItemAttr.h"
#include "JobInitEquip.h"  
#include "GameAppNet.h"
#include "SkillStateRecord.h"
#include "EventHandler.h"
#include "Birthplace.h"
#include "CharBoat.h"
#include "HarmRec.h"
#include "lua_gamectrl.h"
#include "MapEntry.h"
#include "lua_gamectrl.h"
#include "CharStall.h"

#pragma warning(disable: 4355)

// ������ű�֮��Ľ���
Point			g_SSkillPoint;
bool			g_bBeatBack = false;
unsigned char	g_uchFightID;
//
extern char g_skillstate[1024];

_DBC_USING

CCharacter::CCharacter()
: m_CAction(this),
m_CActCache(this),
_dwLastAreaTick(0),
m_AIType(0),
m_AITarget(0),
m_HostCha(0),
_btBlockCnt(0),
m_sChaseRange(1000),
m_btPatrolState(0),
m_pHate(NULL),
m_dwBoatCtrlTick(0),
m_bRelive(false),
m_bVol(false),
m_bInvited(false),
m_ExpScale(100),
_dwStallTick(NULL),
guildCircleColour(-1),
chatColour(0xFFFFFFFF),
guildIcon(0),
bagOfHoldingID(0),
appCheck(),
requestType(0)
{T_B
    m_sPoseState = enumPoseStand;

	memset(&m_SChaPart, 0, sizeof(m_SChaPart));
	memset(&m_STempChaPart, 0, sizeof(m_STempChaPart));
	for (int i = 0; i < enumACTCONTROL_MAX; i++)
		SetActControl(i);

	m_pTradeData = NULL;
	m_lSideID = 0;

	m_pHate = new CHateMgr;

	m_pCKitbagTmp = 0;

T_E}

CCharacter::~CCharacter()
{
	delete m_pHate;
	if(m_pCKitbagTmp)
	{
		delete m_pCKitbagTmp;
		m_pCKitbagTmp = 0;
	}
}


void CCharacter::Initially()
{T_B
	CMoveAble::Initially();

	m_AIType = 0;
	m_AITarget = 0;
	m_nPatrolX = 0;
	m_nPatrolY = 0;
	m_sChaseRange = 0;
	m_btPatrolState = 0;
	m_sPoseState = enumPoseStand;
	m_CKitbag.Init(defDEF_KBITEM_NUM_PER_TYPE);
	memset(&m_CShortcut, 0, sizeof(m_CShortcut));
	memset(&m_SChaPart, 0, sizeof(m_SChaPart));
	for (int i = 0; i < enumACTCONTROL_MAX; i++)
		SetActControl(i);
	m_pHate->ClearHarmRec();
	m_chSelRelive = enumEPLAYER_RELIVE_NONE;
	m_chReliveLv = 0;
	m_szMotto[0] = '\0';
	m_usIcon = 0;
	m_SLean.chState = 1;
	m_SSeat.chIsSeat = 0;
	m_CAction.Interrupt();
	_btBlockCnt = 0;
	memset(&m_STempChaPart, 0, sizeof(m_STempChaPart));

	m_pTradeData = NULL;
	SetKitbagRecDBID(0);
    SetKitbagTmpRecDBID(0);
	SetStoreItemID(0);
	SetStoreBuy(false);
	SetPetNum(0);
	m_dwStoreTime = 0;

    m_timerAI.Begin(500);
    m_timerAreaCheck.Begin(2000);
	m_timerDBUpdate.Begin(g_Config.m_lDBSave);
	m_timerDie.Begin(1000);
	m_timerMission.Begin( 60*1000 );
	m_timerSkillState.Begin(1000);
	m_timerTeam.Begin(1000);
	m_timerScripts.Begin(defCHA_SCRIPT_TIMER);
	m_timerPing.Begin(defPING_INTERVAL);
	m_ulPingDataLen = 0;
	m_timerExit.Reset();
	m_byExit = CHAEXIT_NONE;

	m_chPKCtrl = 0;
	m_lSideID = 0;

	m_dwPing = defDEFAULT_PING_VAL;
	memset(m_dwPingRec, 0, sizeof(DWORD) * defPING_RECORD_NUM);
	m_dwPingSendTick = 0;

	_dwLastSayTick = 0;
	SetInOutMapQueue(false);

	m_ulNetSendLen = 0;
	m_timerNetSendFreq.Begin(1 * 1000);

	_dwLifeTime = 0;

	ResetScriptParam();

	m_pCKitbagTmp = 0;

	memset(m_sTigerItemID, 0, sizeof(m_sTigerItemID));
	memset(m_sTigerSel, 0, sizeof(m_sTigerSel));

    m_ExpScale = 100;

    m_noticeState = 0;//�����Գ�ʼ֪ͨ״̬Ϊ0
	m_retry3 = 0;
	m_retry4 = 0;
	m_retry5 = 0;
    m_retry6 = 0;

	InitCheatX();

	//m_pSkillGridTemp = 0;

	//add by jilinlee 2007/4/20
	SetReadBookState(false);
	m_SReadBook.dwLastReadCallTick=0;

	m_bRelive = false;
	m_bVol = false;
	m_bInvited = false;
	m_bStoreEnable = false;
T_E}

void CCharacter::GetHelmString(char* buffer){
	short helmID = m_SChaPart.SLink->sID;
	if (helmID == 0){
		helmID = m_SChaPart.sHairID;
	}
	sprintf(buffer, "%d_%d", helmID, m_SChaPart.sTypeID);
}

void CCharacter::SendChatLogPacket(char* Channel, char* msg){
	CCharacter *pMainCha = GetPlyMainCha();
	
	WPacket l_wpk = GETWPACKET();
	WRITE_CMD(l_wpk, CMD_MT_DISCORDLOGS);
	WRITE_STRING(l_wpk, pMainCha->GetName());
	WRITE_STRING(l_wpk, Channel);
	WRITE_STRING(l_wpk, msg);
	
	/*
	char helm[32];
	GetHelmString(helm);
	WRITE_STRING(l_wpk, helm);
	*/
	char lookdata[8192];
	LookData2String(&pMainCha->m_SChaPart, lookdata, defLOOK_DATA_STRING_LEN, false);
	WRITE_STRING(l_wpk, lookdata);
	WRITE_CHAR(l_wpk, pMainCha->GetPlayer()->GetGMLev());
	pMainCha->ReflectINFof(pMainCha, l_wpk);
}

void CCharacter::Finally()
{T_B
	try
	{
		m_timerExit.Reset();
		m_byExit = CHAEXIT_NONE;

		if(m_pCKitbagTmp)
		{
			delete m_pCKitbagTmp;
			m_pCKitbagTmp = 0;
		}

		SetPetNum(0);

		m_bRelive = false;
		m_bVol = false;
		m_bInvited = false;
		/*if(m_pSkillGridTemp)
		{
			delete [] m_pSkillGridTemp;
			m_pSkillGridTemp = 0;
		}*/

        m_ExpScale = 100;

		BreakAction();
		m_AITarget = 0;
		if (m_submap)
			m_submap->GoOut(this);
		CMoveAble::Finally();
	}
	catch (...)
	{
		if (!GetPlayer())
			//LG("exception3", "��ɫ[%s]�ͷ�ʱ�����쳣, [CCharacter::Finally]\n", GetLogName());
			LG("exception3", "when character[%s]release occured abnormity, [CCharacter::Finally]\n", GetLogName());
		else
			//LG("exception3", "��ҽ�ɫ[���� %s�����ݿ�ID %u]�ͷ�ʱ�����쳣, [CCharacter::Finally]\n", GetLogName(), GetPlayer()->GetDBActId());
			LG("exception3", "character player[name %s��DatabaseID %u]release occured abnormity, [CCharacter::Finally]\n", GetLogName(), GetPlayer()->GetDBActId());
		throw;
	}
T_E}

void CCharacter::TradeClear( CPlayer& player )
{
	// ������Ľ�����Ϣ���ߴ�ֻ����ȡ��
	if( m_pTradeData )
	{
		g_TradeSystem.Clear( mission::TRADE_CHAR, *this );
	}

	BYTE byNumBoat = player.GetNumBoat();
	for( BYTE i = 0; i < byNumBoat; i++ )
	{
		CCharacter* pBoat = player.GetBoat( i );
		if( pBoat )
		{
			if( pBoat->m_pTradeData )
			{
				g_TradeSystem.Clear( mission::TRADE_BOAT, *pBoat );
			}
		}
	}
}


bool CCharacter::IsPlayerCha(void)
{
	return m_pCPlayer && m_pCPlayer->IsPlayer();
}

bool CCharacter::IsPlayerMainCha(void)
{
	return m_pCPlayer && (m_pCPlayer->GetMainCha() == this);
}

bool CCharacter::IsPlayerCtrlCha(void)
{
	return m_pCPlayer && (m_pCPlayer->GetCtrlCha() == this);
}

CCharacter* CCharacter::GetPlyCtrlCha(void)
{
	if (m_pCPlayer)
		return m_pCPlayer->GetCtrlCha();
	else
		return this;
}

CCharacter* CCharacter::GetPlyMainCha(void)
{
	if (m_pCPlayer)
		return m_pCPlayer->GetMainCha();
	else
		return this;
}

bool CCharacter::IsGMCha()
{
	if(m_pCPlayer && m_pCPlayer->GetGMLev() > 0 && m_pCPlayer->GetGMLev() < 10) return true;
	//if(m_pCPlayer && m_pCPlayer->GetGMLev() > 0) return true;
	
	return false;
}

bool CCharacter::IsGMCha2()
{
	if(m_pCPlayer && m_pCPlayer->GetGMLev() > 0) return true;
	
	return false;
}

inline bool CCharacter::IsPlayerFocusCha(void)
{
	return IsPlayerCha() && (m_pCPlayer->GetCtrlCha() == this);
}

bool CCharacter::IsPlayerOwnCha(void)
{
	return IsPlayerCha() && (getAttr(ATTR_CHATYPE) == enumCHACTRL_PLAYER);
}

void CCharacter::WritePK(WPACKET& wpk) //д����ұ��������и��ӽṹ(���ٻ��޵�)����������
{T_B
	CMoveAble::WritePK(wpk);

	//ToDo:д���Լ�������
T_E}

void CCharacter::WriteCharPartInfo(WPACKET& packet)
{T_B
	WRITE_SEQ(packet, (cChar*)&this->m_SChaPart, sizeof(this->m_SChaPart));
	WRITE_LONG(packet, m_pCChaRecord->lID );
T_E}

void CCharacter::ReadPK(RPACKET& rpk) //�ع���ұ��������и��ӽṹ(���ٻ��޵�)
{T_B
	CMoveAble::ReadPK(rpk);

	//ToDo:�����Լ�������
	m_AITarget = 0;
	m_CAction.Interrupt();
T_E}

//=============================================================================
// ���Լ��ӵ�ǰ��ͼpCSrcMap���л���Ŀ���ͼszTarMapName��[lTarX,lTarY]�ǵ�Ŀ���ͼ���λ��(ϵͳ�������ײ��Ϣ���е�������һ����ȷ����)
// bNeedOutSrcMap ��ʾ�Ƿ���Ҫ��ԭ��ͼGoOut
//=============================================================================
void CCharacter::SwitchMap(SubMap *pCSrcMap, cChar *szTarMapName, Long lTarX, Long lTarY, bool bNeedOutSrcMap, Char chSwitchType, Long lTMapCpyNO)
{T_B
	if (!pCSrcMap)
		return;

	BreakAction();
	
	if( IsPlayerCha() ) 
	{
		SetSubMap( pCSrcMap );
		GetPlayer()->MisGooutMap();
		SetSubMap( NULL );
	}

	if (bNeedOutSrcMap && pCSrcMap)
		pCSrcMap->GoOut(this);

	if (!strcmp(pCSrcMap->GetName(), szTarMapName)) // ͬ��ͼ���л�
	{
		if (GetPlayer())
			//LG("enter_map", "SwitchMap(ͬ��ͼ�л������ƽ�ɫ�� %s[����ɫ�� %s]����ͼ�� %s)--------\n", GetLogName(), GetPlyMainCha()->GetLogName(), szTarMapName);
			LG("enter_map", "SwitchMap(the same map switch��control player name %s[mainplayer %s]��mapname %s)--------\n", GetLogName(), GetPlyMainCha()->GetLogName(), szTarMapName);
		if (m_SMoveRedu.ulStartTick == 0xffffffff)
			m_SMoveRedu.ulStartTick = GetTickCount();
		if(!IsPlayerCha()) // ����ҽ�ɫ�������Լ��ĳ���������
		{
			m_SFightInit.chTarType = 0;
			m_CChaAttr.Init(GetCat());
			Square	SSrcShape = GetShape();
			Square	STarShape = {{lTarX, lTarY}, GetRadius()};
			if (!pCSrcMap->Enter(&STarShape, this))
				pCSrcMap->Enter(&SSrcShape, this);
		}
		else
		{
			SStateData2String(this, g_skillstate, 1024);
			//PStateData2String(this, g_skillstate, 1024);
			if (IsBoat())
				g_strChaState[1] = g_skillstate;
			else
				g_strChaState[0] = g_skillstate;
			Square SSrcShape = GetShape();
			Square STarShape = {{lTarX, lTarY}, SSrcShape.radius};
			if (!pCSrcMap->EnsurePos(&STarShape, this)) // ����ʧ��
			{
				lTarX = SSrcShape.centre.x;
				lTarY = SSrcShape.centre.y;
			}

			GetPlayer()->GetMainCha()->Cmd_EnterMap(szTarMapName, lTMapCpyNO, lTarX, lTarY);

			// ��ɫ����NPCͬ����ͼ�л�			
			GetPlayer()->MisEnterMap();
		}

		SetExistState(enumEXISTS_WAITING);
		return;
	}
	else
	{
		bool bVolunteer = false;
		SubMap	*pCBackM = GetSubMap();
		SetSubMap(pCSrcMap);
		pCSrcMap->BeforePlyOutMap(this);
		//LG("enter_map", "SwitchMap(��ͬServer��ͼ�л������ƽ�ɫ�� %s[����ɫ�� %s]��ԭ��ͼ %s��Ŀ���ͼ %s)--------\n", GetLogName(), GetPlyMainCha()->GetLogName(), pCSrcMap->GetName(), szTarMapName);
		LG("enter_map", "SwitchMap(differ Server map switch��control player name %s[mainplayer %s]��formerly map %s��aimmap %s)--------\n", GetLogName(), GetPlyMainCha()->GetLogName(), pCSrcMap->GetName(), szTarMapName);
		if (GetSubMap())
			//LG("enter_map", "��ɫ��ͼ�� %s\n", GetSubMap()->GetName());
			LG("enter_map", "character map name %s\n", GetSubMap()->GetName());
		// ����д���ݿ�
		CPlayer	*pPlayer = GetPlayer();
		if(!pPlayer)
			return;

		//�������޸�
		if(GetPlyMainCha()->IsVolunteer())
		{
			bVolunteer = true;
			GetPlyMainCha()->Cmd_DelVolunteer();
		}

		game_db.SavePlayer(pPlayer, enumSAVE_TYPE_SWITCH);
		//LG("enter_map", "�������ݳɹ�\n");
		LG("enter_map", "save data succeed\n");

		// ��ɫ����NPCͬ����ͼ�л�
		pPlayer->MisLogout();

		SetSubMap(pCBackM);

		// ����Э��
		WPACKET	l_wpk	=GETWPACKET();
		WRITE_CMD(l_wpk, CMD_MT_SWITCHMAP);
		WRITE_STRING(l_wpk, pCSrcMap->GetName());
		WRITE_LONG(l_wpk, pCSrcMap->GetCopyNO());
		WRITE_LONG(l_wpk, GetShape().centre.x);
		WRITE_LONG(l_wpk, GetShape().centre.y);
		WRITE_STRING(l_wpk, szTarMapName);
		WRITE_LONG(l_wpk, lTMapCpyNO);
		WRITE_LONG(l_wpk, lTarX);
		WRITE_LONG(l_wpk, lTarY);
		if (chSwitchType == enumSWITCHMAP_DIE) // �������µĵ�ͼ�л������Ŀ���ͼ���ɴ��gateǿ��������ߣ����᷵��Դ��ͼ��
			WRITE_CHAR(l_wpk, 1);
		else
			WRITE_CHAR(l_wpk, 0);
		ReflectINFof(this,l_wpk);

        g_pGameApp->DelPlayerIdx(pPlayer->GetDBChaId());
        g_pGameApp->m_dwPlayerCnt--;

		pPlayer->Free();
		// ɾ��gate server��Ӧ��ά����Ϣ
		pPlayer->OnLogoff();
        DELPLAYER(pPlayer);
		//LG("enter_map", "���������ͼ\n\n");
		LG("enter_map", "finish enter map\n\n");
	}
T_E}

void CCharacter::OnBeginSee(Entity *obj)
{T_B
	if(!IsPlayerFocusCha()) // �ý�ɫ������ҵ�ǰ�Ŀ��ƽ���
		return;

	obj->OnBeginSeen(this);	//ToDo:����Ŀ���������Ϣ�Է�ӳ���ͻ���
T_E}

void CCharacter::OnEndSee(Entity *obj)
{T_B
	if(!IsPlayerFocusCha()) // �ý�ɫ������ҵ�ǰ�Ŀ��ƽ���
		return;

	obj->OnEndSeen(this);	//ToDo:�ӿͻ���ɾ��Ŀ��
T_E}

void CCharacter::ReflectINFof(Entity *srcent, WPACKET chginf)
{T_B
	if (!IsPlayerCha()) // �ý�ɫ���������
		return;

	if(srcent ==this)
	{
	}
	WRITE_LONG(chginf, GetPlayer()->GetDBChaId());
	WRITE_LONG(chginf, GetPlayer()->GetGateAddr());
	WRITE_SHORT(chginf, 1);

	m_pCPlayer->GetGate()->SendData(chginf);
T_E}

bool CCharacter::IsPKSilver()
{
	if (!GetSubMap())
		return false;

	return (0 == strcmp(GetSubMap()->GetName(), g_Config.m_szChaosMap));
}

void CCharacter::OnBeginSeen(CCharacter *pCCha)
{T_B
	if (!pCCha->IsPlayerCha()) // �ý�ɫ���������
		return;

	MPTimer tt;
	tt.Begin();

	WPACKET pk	=GETWPACKET();
	WRITE_CMD(pk, CMD_MC_CHABEGINSEE);		//����2�ֽ�
	if (GetPlayer() && GetPlayer() == pCCha->GetPlayer())
		WRITE_CHAR(pk, enumENTITY_SEEN_SWITCH);
	else
		WRITE_CHAR(pk, enumENTITY_SEEN_NEW);

	mission::CEventEntity* pEntity = IsEvent();
	if( pEntity )
	{
		uShort	usEventID = pEntity->GetInfoID();
		
		// ͬ���¼�ʵ��ļ���״̬��Ϣ
		BYTE byData;
		pEntity->GetState( *pCCha, byData );
		usEventID |= byData<<12;
		GetEvent().SetID(usEventID);
	}

	WriteBaseInfo(pk, LOOK_OTHER);

	BYTE byState = 0, byShowType = 0;
	mission::CNpc* pNpc = IsNpc();
	if( pNpc )
	{
		if( pNpc->GetType() == mission::CNpc::TALK )
		{
			// �Ȼ�ȡNPC״̬��Ϣ
			mission::CTalkNpc* pTalk = (mission::CTalkNpc*)pNpc;
			pTalk->MissionProc( *pCCha, byState );
		}
		byShowType = pNpc->GetShowType();
	}

	WRITE_CHAR( pk, byShowType );
	WRITE_CHAR(pk,  byState );

	// pose״̬����
	WRITE_SHORT(pk, m_sPoseState);
	switch (m_sPoseState)
	{
	case	enumPoseLean:
		{
			WRITE_CHAR(pk, m_SLean.chState);
			WRITE_LONG(pk, m_SLean.lPose);
			WRITE_LONG(pk, m_SLean.lAngle);
			WRITE_LONG(pk, m_SLean.lPosX);
			WRITE_LONG(pk, m_SLean.lPosY);
			WRITE_LONG(pk, m_SLean.lHeight);
			break;
		}
	case	enumPoseSeat:
		{
			WRITE_SHORT(pk, m_SSeat.sAngle);
			WRITE_SHORT(pk, m_SSeat.sPose);
			break;
		}
	default:
		{
			break;
		}
	}

	if (IsPlayerCha())
		WriteAttr(pk, 0, ATTR_CLIENT_MAX - 1, enumATTRSYN_INIT);
	else
		WriteMonsAttr(pk, enumATTRSYN_INIT);
	WriteSkillState(pk);
	// WPacket www;
	
	pCCha->ReflectINFof(this,pk);//ͨ��

	//printf("packet size = %d [%s]\n", pk.HasData(), GetName());
T_E}

void CCharacter::OnEndSeen(CCharacter *pCCha)
{T_B
	if (!pCCha->IsPlayerCha()) // �ý�ɫ���������
		return;

	if (m_pCPlayer && pCCha->m_pCPlayer && (GetID() == pCCha->GetID()))
		//LG("��Ұ����", "ͬ����ҽ�ɫ %s ����Ұ�����ǵ�socket��%p��%p.\n", pCCha->GetLogName(), m_pCPlayer->GetGate(), pCCha->m_pCPlayer->GetGate());
		LG("eyeshot error", "the homonymy player %s out of eyeshot��their socket��%p��%p.\n", pCCha->GetLogName(), m_pCPlayer->GetGate(), pCCha->m_pCPlayer->GetGate());

	WPACKET pk	=GETWPACKET();
	WRITE_CMD(pk, CMD_MC_CHAENDSEE);		//����2�ֽ�
	if (GetPlayer() && GetPlayer() == pCCha->GetPlayer() && getAttr(ATTR_CHATYPE) == enumCHACTRL_PLAYER)
		WRITE_CHAR(pk, enumENTITY_SEEN_SWITCH);
	else
		WRITE_CHAR(pk, enumENTITY_SEEN_NEW);

	WRITE_LONG(pk, m_ID);					//ID
	pCCha->ReflectINFof(this,pk);//ͨ��

	// ����npc��Ϣ״̬ͬ��
	mission::CNpc* pNpc = IsNpc();
	if( pNpc )
	{
		if( pNpc->GetType() == mission::CNpc::TALK )
		{
			// �Ȼ�ȡNPC״̬��Ϣ
			mission::CTalkNpc* pTalk = (mission::CTalkNpc*)pNpc;
			pCCha->ClearMissionState( GetID() );
		}
	}
T_E}

bool CCharacter::CanSeen(CCharacter *pCCha)
{
	if (!pCCha)
		return false;

	if (pCCha == this)
		return true;

	if (pCCha->GetActControl(enumACTCONTROL_EYESHOT) && (GetActControl(enumACTCONTROL_NOHIDE) || !GetActControl(enumACTCONTROL_NOSHOW)))
		return true;

	if (IsFriend(pCCha) && !IsGMCha2())
		return true;

	return false;
}

bool CCharacter::CanSeen(CCharacter *pCCha, bool bThisEyeshot, bool bThisNoHide, bool bThisNoShow)
{
	if (!pCCha)
		return false;

	if (pCCha == this)
		return true;

	if (pCCha->GetActControl(enumACTCONTROL_EYESHOT) && (bThisNoHide || !bThisNoShow))
		return true;

	if (IsFriend(pCCha) && !IsGMCha2())
		return true;

	return false;
}

void CCharacter::SetRelive(Char chType, Char chLv, const Char *szInfo)
{
	if (chType == enumEPLAYER_RELIVE_ORIGIN)
	{
		m_chReliveLv = chLv;
		if (m_chReliveLv == 0)
			return;

		if (IsBoat()) // ������ԭ�ظ���
			return;

		GetPlyMainCha()->SetChaRelive();
	}

	WPACKET pk = GETWPACKET();
	WRITE_CMD(pk, CMD_MC_QUERY_RELIVE);
	WRITE_LONG(pk, GetID());
	if (szInfo)
		WRITE_STRING(pk, szInfo);
	else
		WRITE_STRING(pk, "");
	WRITE_CHAR(pk, chType);
	ReflectINFof(this,pk);
}

DWORD CCharacter::GetTeamID()
{
	if (!GetPlayer()) return 0;
	return GetPlayer()->getTeamLeaderID();
}

bool CCharacter::IsTeamLeader()
{
	Long	lTeamID = GetTeamID();

	if (lTeamID == GetPlyMainCha()->GetID())
		return true;

	return false;
}

void CCharacter::SetSideID(Long lSideID)
{
	 if (m_lSideID != lSideID)
	 {
		 m_lSideID = lSideID;
		 SynSideInfo();
	 }
}

// chPosType 1��װ����.2��������
SItemGrid* CCharacter::GetItem(Char chPosType, Long lItemID)
{
	SItemGrid	*pSItemCont = 0;

	if (chPosType == 1)
	{
		for (Char i = enumEQUIP_HEAD; i < enumEQUIP_NUM; i++)
		{
			if (m_SChaPart.SLink[i].sID == (Short)lItemID)
			{
				pSItemCont = m_SChaPart.SLink + i;
				break;
			}
		}
	}
	else if (chPosType == 2)
	{
		Short	sUseGNum = m_CKitbag.GetUseGridNum();
		for (Short i = sUseGNum - 1; i >= 0; i--)
		{
			pSItemCont = m_CKitbag.GetGridContByNum(i);
			if (pSItemCont && pSItemCont->sID == (Short)lItemID)
				break;
			else
				pSItemCont = 0;
		}
	}

	return pSItemCont;
}

// chPosType 1��װ����.2��������
SItemGrid* CCharacter::GetItem2(Char chPosType, Long lPosID)
{
	SItemGrid	*pSItemCont = 0;

	if (chPosType == 1)
	{
		pSItemCont = GetEquipItem((Char)lPosID);
	}
	else if (chPosType == 2)
	{
		pSItemCont = m_CKitbag.GetGridContByID((Short)lPosID);
	}

	return pSItemCont;
}

// ����װ������Ч�ԣ��漰�������������ֵ�ӳɣ��Ѿ�����Լ��ܵ�Ӱ��
bool CCharacter::SetEquipValid(dbc::Char chEquipPos, bool bValid, bool bSyn)
{
	if (!GetPlayer() || IsBoat())
		return false;
	if (chEquipPos < 0 || chEquipPos >= enumEQUIP_NUM)
		return false;
	SItemGrid	*pSEquipIt = m_SChaPart.SLink + chEquipPos;
	if (!g_IsRealItemID(pSEquipIt->sID))
		return false;

	CCharacter	*pCMainCha = GetPlyMainCha();
	if (bSyn)
	{
		pCMainCha->m_CSkillBag.SetChangeFlag(false);
		m_CChaAttr.ResetChangeFlag();
		SetBoatAttrChangeFlag(false);
		m_CSkillState.ResetChangeFlag();
		pCMainCha->SetLookChangeFlag();
	}

	if (bValid)
		CheckItemValid(pSEquipIt);
	ChangeItem(bValid, pSEquipIt, chEquipPos);
	if (!bValid)
		CheckItemValid(pSEquipIt);
	GetPlyCtrlCha()->SkillRefresh();

	if (bSyn)
	{
		GetPlyMainCha()->SynSkillBag(enumSYN_SKILLBAG_MODI);
		SynSkillStateToEyeshot();
		g_CParser.DoString("AttrRecheck", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, DOSTRING_PARAM_END);
		GetPlayer()->RefreshBoatAttr();
		SyncBoatAttr(enumATTRSYN_ITEM_EQUIP);
		SynAttrToSelf(enumATTRSYN_ITEM_EQUIP);
		// check this [garner2]
		pCMainCha->SynLook(enumSYN_LOOK_CHANGE);
	}

	return true;
}

// ���ñ������ߵ���Ч�ԣ�����õ�������������������������������λ�ã������漰���õ�����Ϊװ���Խ�ɫ�ļӳ�
bool CCharacter::SetKitbagItemValid(dbc::Short sPosID, bool bValid, bool bRecheckAttr, bool bSyn)
{
	SItemGrid *pSEspeGrid = m_CKitbag.GetGridContByID(sPosID);
	if (!pSEspeGrid)
		return false;
	if (pSEspeGrid->IsValid() == bValid)
		return true;

	if (bSyn)
		m_CKitbag.SetChangeFlag(false);

	/* //disabled pet slot
	Short sEspeGridID = 1;
	
	if (sPosID == sEspeGridID)
	{
		CItemRecord* pItem = GetItemRecordInfo(pSEspeGrid->sID);
		if(pItem == NULL)
			return false;
		if (pItem->sType == enumItemTypePet) // ����
		{
			if (bSyn)
			{
				m_CChaAttr.ResetChangeFlag();
				SetBoatAttrChangeFlag(false);
			}
			pSEspeGrid->SetValid();
			ChangeItem(bValid, pSEspeGrid, enumEQUIP_HEAD);
			if (bRecheckAttr || bSyn)
			{
				g_CParser.DoString("AttrRecheck", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, DOSTRING_PARAM_END);
				if (GetPlayer())
					GetPlayer()->RefreshBoatAttr();
			}
			if (bSyn)
			{
				if (GetPlayer())
					SyncBoatAttr(enumATTRSYN_ITEM_EQUIP);
				SynAttrToSelf(enumATTRSYN_ITEM_EQUIP);
			}
		}
	}*/
	pSEspeGrid->SetValid(bValid);
	if (bSyn)
		SynKitbagNew(enumSYN_KITBAG_ATTR);

	return true;
}

// ���ñ������ߵ���Ч�ԣ�����õ�������������������������������λ�ã������漰���õ�����Ϊװ���Խ�ɫ�ļӳ�
bool CCharacter::SetKitbagItemValid(SItemGrid* pSItem, bool bValid, bool bRecheckAttr, bool bSyn)
{
	Short	sPosID;
	if (!m_CKitbag.GetPosIDByGrid(pSItem, &sPosID))
		return false;
	return SetKitbagItemValid(sPosID, bValid, bRecheckAttr, bSyn);
}

// Ӱ����۵ĵ��ߣ����򣬳����.
bool CCharacter::ItemIsAppendLook(SItemGrid* pSItem)
{
	if (!pSItem)
		return false;
	CItemRecord* pItemRec = GetItemRecordInfo( pSItem->sID );
	if (!pItemRec)
		return false;
	return pItemRec->IsFaceItem();
}

void CCharacter::SetLookChangeFlag(bool bChange)
{
	for (Char i = enumEQUIP_HEAD; i < enumEQUIP_NUM; i++)
		m_SChaPart.SLink[i].SetChange(bChange);
}

void CCharacter::SetEspeItemChangeFlag(bool bChange)
{
	Short	sEspeGridID = 1;
	SItemGrid *pGrid = m_CKitbag.GetGridContByID(sEspeGridID);
	if (pGrid)
	{
		CItemRecord* pItem = GetItemRecordInfo(pGrid->sID);
		if(pItem && pItem->sType == enumItemTypePet) // �������
			m_CKitbag.SetSingleChangeFlag(sEspeGridID);
	}
}

Char CCharacter::GetLookChangeNum(void)
{
	Char	chNum = 0;
	for (Char i = enumEQUIP_HEAD; i < enumEQUIP_NUM; i++)
		if (m_SChaPart.SLink[i].IsChange())
			chNum++;

	return chNum;
}

bool CCharacter::AddKitbagCapacity(dbc::Short sAddVal)
{
	if (m_CKitbag.AddCapacity(sAddVal))
	{
		SynKitbagCapacity();
		return true;
	}
	else
		return false;
}

// ��龫�������Ƿ�Ϸ���������Ч��
bool CCharacter::CheckForgeItem(SForgeItem *pSItem)
{
	CPlayer	*pCPly = GetPlayer();
	if (!pCPly)
		return false;
	if (!pSItem)
		pSItem = pCPly->GetForgeItem();
	SItemGrid	*pSGridCont;
	for (int i = 0; i < defMAX_ITEM_FORGE_GROUP; i++)
	{
		for (short j = 0; j < pSItem->SGroup[i].sGridNum; j++)
		{
			pSGridCont = m_CKitbag.GetGridContByID(pSItem->SGroup[i].SGrid[j].sGridID);
			if (!pSGridCont || pSGridCont->sNum < pSItem->SGroup[i].SGrid[j].sItemNum)
				return false;
		}
	}

	return true;
}

// �����ߵ���Ч��
void CCharacter::CheckItemValid(SItemGrid* pCItem)
{
	if (!pCItem)
		return;
	pCItem->CheckValid();
	if (pCItem->IsValid())
	{
		g_CParser.DoString("check_item_valid", enumSCRIPT_RETURN_NUMBER, 1, enumSCRIPT_PARAM_LIGHTUSERDATA, 2, this, pCItem, DOSTRING_PARAM_END);
		pCItem->SetValid(g_CParser.GetReturnNumber(0) != 0 ? true : false);
	}
}

// �����������
void CCharacter::CheckEspeItemGrid(void)
{
	Short	sEspeGridID = 1;
	SItemGrid *pGrid = m_CKitbag.GetGridContByID(sEspeGridID);
	if (pGrid)
	{
		CItemRecord* pItem = GetItemRecordInfo(pGrid->sID);
		if(pItem && pItem->sType == enumItemTypePet) // �������
			ChangeItem(true, pGrid, enumEQUIP_HEAD);
	}
}

// ���¼��������Ĳ�������������������ߣ�װ����������
Short CCharacter::KbPushItem(bool bRecheckAttr, bool bSynAttr, SItemGrid *pGrid, Short &sPosID, Short sType, bool bCommit, bool bSureOpr)
{
	if (!pGrid)
		return enumKBACT_ERROR_PUSHITEMID;
	CheckItemValid(pGrid);
	Short	sEspeGridID = 1;
	bool b2HasItem = m_CKitbag.GetGridContByID(sEspeGridID) ? true : false;
	Short sPushRet = m_CKitbag.Push(pGrid, sPosID, sType, bCommit, bSureOpr);
	if (sPushRet == enumKBACT_SUCCESS || sPushRet == enumKBACT_ERROR_FULL)
	{
		if (!b2HasItem && sPosID == sEspeGridID) // �����ԭ���ǿյģ������е�����.
		{
			CItemRecord* pItem = GetItemRecordInfo(pGrid->sID);
			if(pItem == NULL)
				return enumKBACT_ERROR_PUSHITEMID;

			/*//disabled pet slot
			if (pItem->sType == enumItemTypePet)
			{
				if (bSynAttr)
				{
					m_CChaAttr.ResetChangeFlag();
					SetBoatAttrChangeFlag(false);
				}
				//ChangeItem(true, pGrid, enumEQUIP_HEAD);
				if (bRecheckAttr || bSynAttr)
				{
					g_CParser.DoString("AttrRecheck", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, DOSTRING_PARAM_END);
					if (GetPlayer())
						GetPlayer()->RefreshBoatAttr();
				}
				if (bSynAttr)
				{
					if (GetPlayer())
						SyncBoatAttr(enumATTRSYN_ITEM_EQUIP);
					SynAttrToSelf(enumATTRSYN_ITEM_EQUIP);
				}
			}*/
		}
	}

	return sPushRet;
}

Short CCharacter::KbPopItem(bool bRecheckAttr, bool bSynAttr, SItemGrid *pGrid, Short sPosID, Short sType, bool bCommit)
{
	if (!pGrid)
		return enumKBACT_ERROR_PUSHITEMID;
	Short	sEspeGridID = 1;
	Short sPushRet = m_CKitbag.Pop(pGrid, sPosID, sType, bCommit);
	if (sPosID == sEspeGridID && sPushRet == enumKBACT_SUCCESS) // �ɹ����������
	{
		bool b2HasItem = m_CKitbag.GetGridContByID(sEspeGridID) ? true : false;
		if (!b2HasItem) // �����û�е�����.
		{
			CItemRecord* pItem = GetItemRecordInfo(pGrid->sID);
			if(pItem == NULL)
				return enumKBACT_ERROR_PUSHITEMID;
			/*//disabled pet slot
			if (pItem->sType == enumItemTypePet)
			{
				if (bSynAttr)
				{
					m_CChaAttr.ResetChangeFlag();
					SetBoatAttrChangeFlag(false);
				}
				ChangeItem(false, pGrid, enumEQUIP_HEAD);
				if (bRecheckAttr || bSynAttr)
				{
					g_CParser.DoString("AttrRecheck", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, DOSTRING_PARAM_END);
					if (GetPlayer())
						GetPlayer()->RefreshBoatAttr();
				}
				if (bSynAttr)
				{
					if (GetPlayer())
						SyncBoatAttr(enumATTRSYN_ITEM_EQUIP);
					SynAttrToSelf(enumATTRSYN_ITEM_EQUIP);
				}
			}*/
		}
	}

	return sPushRet;
}

Short CCharacter::KbClearItem(bool bRecheckAttr, bool bSynAttr, Short sPosID, Short sType)
{
	Short	sEspeGridID = 1;
	if (sPosID == sEspeGridID) // �����
	{
		SItemGrid SGrid;
		SItemGrid *pGrid = m_CKitbag.GetGridContByID(sEspeGridID);
		CItemRecord* pItem = GetItemRecordInfo(pGrid->sID);
		if(pItem == NULL)
			return enumKBACT_ERROR_PUSHITEMID;
		if (pItem->sType == enumItemTypePet)
			SGrid = *pGrid;
		Short sRet = m_CKitbag.Clear(sPosID, sType);
		/* //disabled pet slot
		if (sRet == enumKBACT_SUCCESS)
		{
			if (pItem->sType == enumItemTypePet) // ����
			{
				if (bSynAttr)
				{
					m_CChaAttr.ResetChangeFlag();
					SetBoatAttrChangeFlag(false);
				}
				ChangeItem(false, &SGrid, enumEQUIP_HEAD);
				if (bRecheckAttr || bSynAttr)
				{
					g_CParser.DoString("AttrRecheck", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, DOSTRING_PARAM_END);
					if (GetPlayer())
						GetPlayer()->RefreshBoatAttr();
				}
				if (bSynAttr)
				{
					if (GetPlayer())
						SyncBoatAttr(enumATTRSYN_ITEM_EQUIP);
					SynAttrToSelf(enumATTRSYN_ITEM_EQUIP);
				}
			}
		}*/
		return sRet;
	}
	else
		return m_CKitbag.Clear(sPosID, sType);
}

Short CCharacter::KbClearItem(bool bRecheckAttr, bool bSynAttr, SItemGrid *pGrid, Short sNum)
{
	if (!pGrid)
		return enumKBACT_ERROR_PUSHITEMID;
	CItemRecord* pItem = GetItemRecordInfo(pGrid->sID);
	if(pItem == NULL)
		return enumKBACT_ERROR_PUSHITEMID;
	if (pItem->sType == enumItemTypePet) // ����
	{
		Short sEspeGridID = 1;
		SItemGrid SGrid = *pGrid;
		Short sPosID;
		Short sRet = m_CKitbag.Clear(pGrid, sNum, &sPosID);
		/* disabled pet slot
		if (sRet == enumKBACT_SUCCESS)
		{
			if (sPosID == sEspeGridID) // �����
			{
				if (m_CKitbag.GetNum(sEspeGridID) <= 0)
				{
					if (bSynAttr)
					{
						m_CChaAttr.ResetChangeFlag();
						SetBoatAttrChangeFlag(false);
					}
					ChangeItem(false, &SGrid, enumEQUIP_HEAD);
					if (bRecheckAttr || bSynAttr)
					{
						g_CParser.DoString("AttrRecheck", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, DOSTRING_PARAM_END);
						if (GetPlayer())
							GetPlayer()->RefreshBoatAttr();
					}
					if (bSynAttr)
					{
						if (GetPlayer())
							SyncBoatAttr(enumATTRSYN_ITEM_EQUIP);
						SynAttrToSelf(enumATTRSYN_ITEM_EQUIP);
					}
				}
			}
		}*/
		return sRet;
	}
	else
		return m_CKitbag.Clear(pGrid, sNum);
}

Short CCharacter::KbRegroupItem(bool bRecheckAttr, bool bSynAttr, Short sSrcPosID, Short sSrcNum, Short sTarPosID, Short sType)
{
	/*//disabled pet slot
	Short sEspeGridID = 1;
	if (sSrcPosID == sEspeGridID || sTarPosID == sEspeGridID)
	{
		if (bSynAttr)
		{
			m_CChaAttr.ResetChangeFlag();
			SetBoatAttrChangeFlag(false);
		}

		SItemGrid SEspeGridOld, *pSEspeGridOld = m_CKitbag.GetGridContByID(sEspeGridID);
		if (pSEspeGridOld) SEspeGridOld = *pSEspeGridOld;
		Short sRet = m_CKitbag.Regroup(sSrcPosID, sSrcNum, sTarPosID, sType);
		SItemGrid *pSEspeGridNew = m_CKitbag.GetGridContByID(sEspeGridID);
		if (SEspeGridOld.sID != 0)
		{
			CItemRecord* pItem = GetItemRecordInfo(SEspeGridOld.sID);
			if(pItem == NULL)
				return enumKBACT_ERROR_PUSHITEMID;
			if (pItem->sType == enumItemTypePet) // ����
				ChangeItem(false, &SEspeGridOld, enumEQUIP_HEAD);
		}
		if (pSEspeGridNew)
		{
			CItemRecord* pItem = GetItemRecordInfo(pSEspeGridNew->sID);
			if(pItem == NULL)
				return enumKBACT_ERROR_PUSHITEMID;
			if (pItem->sType == enumItemTypePet) // ����
				ChangeItem(true, pSEspeGridNew, enumEQUIP_HEAD);
		}

		if (bRecheckAttr || bSynAttr)
		{
			g_CParser.DoString("AttrRecheck", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, DOSTRING_PARAM_END);
			if (GetPlayer())
				GetPlayer()->RefreshBoatAttr();
		}
		if (bSynAttr)
		{
			if (GetPlayer())
				SyncBoatAttr(enumATTRSYN_ITEM_EQUIP);
			SynAttrToSelf(enumATTRSYN_ITEM_EQUIP);
		}

		return sRet;
	}
	else*/
		return m_CKitbag.Regroup(sSrcPosID, sSrcNum, sTarPosID, sType);
}

void CCharacter::CheckBagItemValid(CKitbag* pCBag)
{
	if (!pCBag)
		return;
	SItemGrid	*pGridCont;
	Short sUsedNum = pCBag->GetUseGridNum();
	for (int i = 0; i < sUsedNum; i++)
	{
		pGridCont = pCBag->GetGridContByNum(i);
		CheckItemValid(pGridCont);
	}
}

void CCharacter::CheckLookItemValid(void)
{
	for (int i = 0; i < enumEQUIP_NUM; i++)
		CheckItemValid(m_SChaPart.SLink + i);
}

bool CCharacter::String2LookDate(std::string &strData)
{
	if (::Strin2LookData(&m_SChaPart, strData))
	{
		CheckLookItemValid();
		return true;
	}

	return false;
}

bool CCharacter::String2KitbagData(std::string &strData)
{
	if (::String2KitbagData(&m_CKitbag, strData))
	{
		CheckBagItemValid(&m_CKitbag);
		return true;
	}

	return false;
}

bool CCharacter::String2KitbagTmpData(std::string &strData)
{
	if(m_pCKitbagTmp != NULL)
	{
		delete m_pCKitbagTmp;
		m_pCKitbagTmp = 0;
	}
	m_pCKitbagTmp = new CKitbag;
	m_pCKitbagTmp->Init(32);

	if (::String2KitbagData(m_pCKitbagTmp, strData))
	{
		CheckBagItemValid(m_pCKitbagTmp);
		return true;
	}

	return false;
}

// ִ�о����ű�
bool CCharacter::DoForgeLikeScript(dbc::cChar *cszFunc, dbc::Long &lRet)
{
	CPlayer	*pCPly = GetPlayer();
	if (!pCPly)
		return false;
	SForgeItem *pSItem = pCPly->GetForgeItem();

	int	nParamNum = 0;
	int nRetNum = 1;
	lua_getglobal(g_pLuaState, cszFunc);
	if (!lua_isfunction(g_pLuaState, -1)) // ���Ǻ�����
	{
		lua_pop(g_pLuaState, 1);
		return false;
	}
	lua_pushlightuserdata(g_pLuaState, this);
	nParamNum++;
	for (int i = 0; i < defMAX_ITEM_FORGE_GROUP; i++)
	{
		lua_pushnumber(g_pLuaState, pSItem->SGroup[i].sGridNum);
		nParamNum++;
		for (short j = 0; j < pSItem->SGroup[i].sGridNum; j++)
		{
			lua_pushnumber(g_pLuaState, pSItem->SGroup[i].SGrid[j].sGridID);
			lua_pushnumber(g_pLuaState, pSItem->SGroup[i].SGrid[j].sItemNum);
			nParamNum += 2;
		}
	}
	int nState = lua_pcall(g_pLuaState, nParamNum, LUA_MULTRET, 0);
	if (nState != 0)
	{
		LG("lua_err", "DoString %s\n", cszFunc);
		lua_callalert(g_pLuaState, nState);
		lua_settop(g_pLuaState, 0);
		return false;
	}
	lRet = (Long)lua_tonumber(g_pLuaState, -1);
	lua_settop(g_pLuaState, 0);

	return true;
}

bool CCharacter::DoLifeSkillcript(dbc::cChar *cszFunc, dbc::Long &lRet)
{
	CPlayer	*pCPly = GetPlayer();
	if (!pCPly)
		return false;
	SLifeSkillItem *pSItem = pCPly->GetLifeSkillItem();
	if(!pSItem)
		return false;
	int	nParamNum = 0;
	int nRetNum = 1;
	lua_getglobal(g_pLuaState, cszFunc);
	if (!lua_isfunction(g_pLuaState, -1)) // ���Ǻ�����
	{
		lua_pop(g_pLuaState, 1);
		return false;
	}

	lua_pushlightuserdata(g_pLuaState, this);
	nParamNum++;
	lua_pushnumber(g_pLuaState,pSItem->sbagCount);
	nParamNum++;

	for (int i = 0; i < pSItem->sbagCount; i++)
	{
		lua_pushnumber(g_pLuaState, pSItem->sGridID[i]);
		nParamNum ++;
	}
	
	lua_pushnumber(g_pLuaState,pSItem->sReturn);
	nParamNum++;
	int nState = lua_pcall(g_pLuaState, nParamNum, LUA_MULTRET, 0);
	if (nState != 0)
	{
		LG("lua_err", "DoString %s\n", cszFunc);
		lua_callalert(g_pLuaState, nState);
		lua_settop(g_pLuaState, 0);
		return false;
	}
	lRet = (Long)lua_tonumber(g_pLuaState, -1);
	lua_settop(g_pLuaState, 0);
	return true;
}

bool CCharacter::DoTigerScript(dbc::cChar *cszFunc)
{
	CPlayer	*pCPly = GetPlayer();
	if (!pCPly)
		return false;

	if(!strcmp(cszFunc, "TigerStart"))
	{
		int	nParamNum = 0;
		short sRet = 0;
		lua_getglobal(g_pLuaState, cszFunc);
		if (!lua_isfunction(g_pLuaState, -1)) // ���Ǻ�����
		{
			lua_pop(g_pLuaState, 1);
			return false;
		}
		lua_pushlightuserdata(g_pLuaState, this);
		nParamNum++;
		for(int i = 0; i < 3; i++)
		{
			lua_pushnumber(g_pLuaState, m_sTigerSel[i]);
			nParamNum++;
		}
		int nState = lua_pcall(g_pLuaState, nParamNum, LUA_MULTRET, 0);
		if (nState != 0)
		{
			LG("lua_err", "DoString %s\n", cszFunc);
			lua_callalert(g_pLuaState, nState);
			lua_settop(g_pLuaState, 0);
			return false;
		}

		for(i = 0; i < 9; i++)
		{
			sRet = (short)lua_tonumber(g_pLuaState, i+1);
			if(sRet <= 0)
			{
				memset(m_sTigerItemID, 0, sizeof(m_sTigerItemID));
				memset(m_sTigerSel, 0, sizeof(m_sTigerSel));
				LG("lua_err", "DoString %s\n", cszFunc);
				lua_callalert(g_pLuaState, nState);
				lua_settop(g_pLuaState, 0);
				return false;
			}
			m_sTigerItemID[i] = sRet;
		}
		
		lua_settop(g_pLuaState, 0);
	}
	else if(!strcmp(cszFunc, "TigerStop"))
	{
		int	nParamNum = 0;
		lua_getglobal(g_pLuaState, cszFunc);
		if (!lua_isfunction(g_pLuaState, -1)) // ���Ǻ�����
		{
			lua_pop(g_pLuaState, 1);
			return false;
		}
		lua_pushlightuserdata(g_pLuaState, this);
		nParamNum++;
		for(int i = 0; i < 9; i++)
		{
			lua_pushnumber(g_pLuaState, m_sTigerItemID[i]);
			nParamNum++;
		}
		for(i = 0; i < 3; i++)
		{
			lua_pushnumber(g_pLuaState, m_sTigerSel[i]);
			nParamNum++;
		}
		int nState = lua_pcall(g_pLuaState, nParamNum, LUA_MULTRET, 0);
		if (nState != 0)
		{
			LG("lua_err", "DoString %s\n", cszFunc);
			lua_callalert(g_pLuaState, nState);
			lua_settop(g_pLuaState, 0);
			return false;
		}

		lua_settop(g_pLuaState, 0);
	}

	return true;
}

void CCharacter::Reset()
{
	BreakAction();
	m_CSkillState.Reset();
	for (int i = 0; i < enumACTCONTROL_MAX; i++)
		SetActControl(i);
	m_SSeat.chIsSeat = 0;

	setAttr(ATTR_HP, m_CChaAttr.GetAttr(ATTR_MXHP));	// ��ǰHP
	setAttr(ATTR_SP, m_CChaAttr.GetAttr(ATTR_MXSP));	// ��ǰSP
}

void CCharacter::OnDie(DWORD dwCurTime)
{T_B
	if (GetExistState() >= enumEXISTS_WITHERING) // ��ʧ
	{
		if (m_SExistCtrl.lWitherTime == -1)
		{
			return;
		}
		else if (dwCurTime - m_SExistCtrl.ulTick >= (uLong)m_SExistCtrl.lWitherTime)
		{
			if (IsPlayerCha()) // ��ҽ�ɫ�������ڵ�ͼ��ȴ�
			{
				if (m_chSelRelive != enumEPLAYER_RELIVE_NONE)
				{
					if (m_chSelRelive == enumEPLAYER_RELIVE_CITY) // �سǸ���
					{
						if (IsBoat()) // ��ֻ
						{
							BackToCity(true);

							g_CParser.DoString("Relive", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, DOSTRING_PARAM_END);
							if (getAttr(ATTR_HP) <= 0)
								//LG("�����������", "��ɫ %s(%d)�����������HP�Ƿ�\n", GetLogName(), getAttr(ATTR_JOB));
								LG("renascence compute error", "character %s(%d)after renascence compute HP is unlawful\n", GetLogName(), getAttr(ATTR_JOB));

							m_chSelRelive = enumEPLAYER_RELIVE_NONE;
							m_chReliveLv = 0;

						}
						else
						{
							BackToCity(true);
							
							g_CParser.DoString("Relive", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, DOSTRING_PARAM_END);
							if (getAttr(ATTR_HP) <= 0)
								//LG("�����������", "��ɫ %s(%d)�����������HP�Ƿ�\n", GetLogName(), getAttr(ATTR_JOB));
								LG("renascence compute error", "character %s(%d)after renascence compute HP is unlawful\n", GetLogName(), getAttr(ATTR_JOB));

							m_chSelRelive = enumEPLAYER_RELIVE_NONE;
							m_chReliveLv = 0;

						}
					}
					else if (m_chSelRelive == enumEPLAYER_RELIVE_ORIGIN)
					{
						if (m_chReliveLv > 0)
						{
							SubMap	*pCMap = GetSubMap();
							GetSubMap()->GoOut(this);
							SetExistState(enumEXISTS_NATALITY);
							//m_timerScripts.Reset();

							m_chSelRelive = enumEPLAYER_RELIVE_NONE;
							m_chReliveLv = 0;

							g_CParser.DoString("Relive_now", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, enumSCRIPT_PARAM_NUMBER, 1, m_chReliveLv, DOSTRING_PARAM_END);
							if (getAttr(ATTR_HP) <= 0)
								//LG("�����������", "��ɫ %s(%d)�����������HP�Ƿ�\n", GetLogName(), getAttr(ATTR_JOB));
								LG("renascence compute error", "character %s(%d)after renascence compute HP is unlawful\n", GetLogName(), getAttr(ATTR_JOB));
							SwitchMap(pCMap, pCMap->GetName(), GetPos().x, GetPos().y, false, enumSWITCHMAP_DIE, pCMap->GetCopyNO());
						}
					}
					else if (m_chSelRelive == enumEPLAYER_RELIVE_MAP)
					{
						if (GetSubMap() && !GetSubMap()->GetMapRes()->IsRepatriateDie())
						{
							std::string	strScript = "get_repatriate_city_";
							strScript += GetSubMap()->GetName();
							if (g_CParser.DoString(strScript.c_str(), enumSCRIPT_RETURN_STRING, 1, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, DOSTRING_PARAM_END))
								BackToCity(true, g_CParser.GetReturnString(0));
						}
					}
				}
			}
		}
	}
T_E}


void CCharacter::AfterStepMove(void)
{
	// ����ֻ�л���ͼ
	if (IsBoat())
	{
		const long	clSwitchDist = 50 * 100;
		const long	clTarDist = 60 * 100;

		bool	bSwitch = false;
		Char	szTarMapName[MAX_MAPNAME_LENGTH];
		Long	lTarX, lTarY = GetPos().y;
		Point	SrcPos = {0, lTarY};

		SubMap	*pMap = GetSubMap();
		const Rect	&area = pMap->GetRange();
		if (!strcmp(pMap->GetName(), "garner"))
		{
			if (GetPos().x >= area.rbtm.x - clSwitchDist)
			{
				bSwitch = true;
				strcpy(szTarMapName, "magicsea");
				lTarX = clTarDist;
				SrcPos.x = area.rbtm.x - clTarDist;
				pMap->MoveTo(this, SrcPos);
				//LG("enter_map", "��Ҵ�ֻ������garner�л���magicsea\n");
				LG("enter_map", "character boat will switch garner to magicsea\n");
			}
		}
		else if (!strcmp(pMap->GetName(), "magicsea"))
		{
			if (GetPos().x >= area.rbtm.x - clSwitchDist)
			{
				bSwitch = true;
				strcpy(szTarMapName, "darkblue");
				lTarX = clTarDist;
				SrcPos.x = area.rbtm.x - clTarDist;
				pMap->MoveTo(this, SrcPos);
				//LG("enter_map", "��Ҵ�ֻ������magicsea�л���darkblue\n");
				LG("enter_map", "character boat will switch magicsea to darkblue\n");
			}
			else if (GetPos().x <= area.ltop.x + clSwitchDist)
			{
				bSwitch = true;
				strcpy(szTarMapName, "garner");
				lTarX = area.rbtm.x - clTarDist;
				SrcPos.x = area.ltop.x + clTarDist;
				pMap->MoveTo(this, SrcPos);
				//LG("enter_map", "��Ҵ�ֻ������magicsea�л���garner\n");
				LG("enter_map", "character boat will switch magicsea to garner\n");
			}
		}
		else if (!strcmp(pMap->GetName(), "darkblue"))
		{
			if (GetPos().x <= area.ltop.x + clSwitchDist)
			{
				bSwitch = true;
				strcpy(szTarMapName, "magicsea");
				lTarX = area.rbtm.x - clTarDist;
				SrcPos.x = area.ltop.x + clTarDist;
				pMap->MoveTo(this, SrcPos);
				//LG("enter_map", "��Ҵ�ֻ������darkblue�л���magicsea\n");
				LG("enter_map", "character boat will switch darkblue to magicsea\n");
			}
		}

		if (bSwitch) SwitchMap(pMap, szTarMapName, lTarX, lTarY);
	}
	//
}

void CCharacter::SubsequenceMove()
{T_B
	if (!IsLiveing())
	{
		m_SMoveRedu.ulStartTick = GetTickCount();
		return; // �ƶ������������������к�״̬������������������˷�֧
	}

	if (GetMoveState() != enumMSTATE_ON)
	{
		SetExistState(GetMoveStopState());
		if (GetMoveStopState() == enumEXISTS_SLEEPING && m_pCChaRecord->sDormancy == 0)
		{
			// LG("host", "[%s] move to sleep end, set waiting\n", GetName());
			SetExistState(enumEXISTS_WAITING);
		}
	}

	if (GetMoveState() & enumMSTATE_BLOCK)
		AddBlockCnt();

	if (GetMoveState() & enumMSTATE_CANCEL || !(GetMoveState() & enumMSTATE_INRANGE))
		m_SMoveRedu.ulStartTick = GetTickCount();

	m_SMoveInit.STargetInfo.chType = 0;
	if(!m_CAction.DoNext(enumACTION_MOVE, m_SMoveProc.sState))
		m_SMoveRedu.ulStartTick = GetTickCount();
T_E}

void CCharacter::SubsequenceFight()
{T_B
	m_SMoveRedu.ulStartTick = GetTickCount();

	if (!IsLiveing())
	{
		return; // ��ս��������������������˷�֧
	}
	else if (GetFightState() != enumFSTATE_ON)
	{
		SetExistState(GetFightStopState());
	}

	m_CAction.DoNext(enumACTION_SKILL, m_SFightProc.sState);
T_E}

//=============================================================================
// ͨ���ж�ʧ��ԭ��
// chType �ж�����
// chReason ʧ��ԭ�򣬲μ�.\client\scripts\table\NotifySet.txt
//=============================================================================
void CCharacter::FailedActionNoti(Char chType, Char chReason)
{T_B
	WPACKET pk	=GETWPACKET();
	WRITE_CMD(pk, CMD_MC_FAILEDACTION);
	WRITE_LONG(pk, GetID());
	WRITE_CHAR(pk, chType);
	WRITE_CHAR(pk, chReason);

	ReflectINFof(this, pk);
T_E}

void CCharacter::EndAction(RPACKET pk)
{T_B
	if (!IsLiveing())
	{
		m_CLog.Log("$$$PacketID:\t%u\n", m_ulPacketID);
		//m_CLog.Log("�ܾ��ж������������ڣ�\n\n");
		m_CLog.Log("refuse action requset(oneself is inexistence)\n\n");return;
	}

	m_CAction.End();

	// log
	m_CLog.Log("===Recieve(EndAction):\tTick %u\n", GetTickCount());
	//m_CLog.Log("\tȫ���ж���Ŀ��%d����ǰ�ж��ţ�%d.\n", m_CAction.GetActionNum(), m_CAction.GetCurActionNo());
	m_CLog.Log("\tall action numbers��%d,currently actionID%d.\n", m_CAction.GetActionNum(), m_CAction.GetCurActionNo());
	//
T_E}

void CCharacter::BreakAction(RPACKET pk)
{
	m_CAction.Interrupt();
	ResetMove();
	ResetFight();
}

void CCharacter::AfterAttrChange(int nIdx, dbc::Long lOldVal, dbc::Long lNewVal)
{
}

void CCharacter::Die()
{T_B
	SubMap	*pCMap = GetSubMap();

	BreakAction();
	m_CSkillState.Reset();
	for (int i = 0; i < enumACTCONTROL_MAX; i++)
		SetActControl(i);
	m_SSeat.chIsSeat = 0;
	m_chSelRelive = enumEPLAYER_RELIVE_NONE;
	m_chReliveLv = 0;
	if (IsPlayerOwnCha() && pCMap && !pCMap->GetMapRes()->IsRepatriateDie())
	{
		std::string	strScript = "check_repatriate_";
		strScript += pCMap->GetName();
		if (g_CParser.DoString(strScript.c_str(), enumSCRIPT_RETURN_NUMBER, 1, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, DOSTRING_PARAM_END))
		{
			if (!g_CParser.GetReturnNumber(0))
			{
				std::string	strScript = "get_repatriate_hint_";
				strScript += pCMap->GetName();
				if (g_CParser.DoString(strScript.c_str(), enumSCRIPT_RETURN_STRING, 1, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, DOSTRING_PARAM_END))
					SetRelive(enumEPLAYER_RELIVE_MAP, 0, g_CParser.GetReturnString(0));
			}
		}
	}

	//if(IsPlayerCha() && pCMap)//2006.10.12wsj
	//{
	//	pCMap->OnPlayerDie(this);
	//}

	if(IsPlayerCha())
	{
		GetPlyMainCha()->ResetChaRelive();
	}

	if(IsPlayerFocusCha()) // ��ҵ�ǰ�Ŀ��ƽ��㣬����ʱ�����
	{
		SetWitherTime(0);
		if (IsBoat())
			SetResumeTime(0);
		else
			SetResumeTime(-1);
	}

	// ȡ�����Ľ�����Ϣ���ߴ�ֻ����ȡ��

	g_StallSystem.CloseStall(*this);

	if( GetPlayer() && (this == GetPlyMainCha() || IsBoat()) )
	{
		if( GetPlyMainCha()->m_pTradeData )
		{
			DWORD dwCharID = ( GetPlyMainCha() == GetPlyMainCha()->m_pTradeData->pRequest ) ? GetPlyMainCha()->m_pTradeData->pAccept->GetID():
					GetPlyMainCha()->m_pTradeData->pRequest->GetID();
			g_TradeSystem.Cancel( mission::TRADE_CHAR, *GetPlyMainCha(), dwCharID );
		}

		BYTE byNumBoat = GetPlayer()->GetNumBoat();
		for( BYTE i = 0; i < byNumBoat; i++ )
		{
			CCharacter* pBoat = GetPlayer()->GetBoat( i );
			if( pBoat )
			{
				if( pBoat->m_pTradeData )
				{
					DWORD dwCharID = ( pBoat == pBoat->m_pTradeData->pRequest ) ? pBoat->m_pTradeData->pAccept->GetID():
						pBoat->m_pTradeData->pRequest->GetID();
					g_TradeSystem.Cancel( mission::TRADE_BOAT, *pBoat, dwCharID );
				}
			}
		}
	}

	if (!IsPlayerCha() && pCMap)
	{
		if (InOutMapQueue())
		{
			//LG("�ظ���ͼ���ɫ", "��ɫ%s���Ѿ������ڵ�ͼ�����!\n", GetLogName());
		}
		else
		{
			SetInOutMapQueue();
			SSwitchMapInfo	SwitchInfo;

			SwitchInfo.pSrcMap = pCMap;
			strcpy(SwitchInfo.szSrcMapName, pCMap->GetName());
			SwitchInfo.SSrcPos = GetShape().centre;
			m_SFightProc.sState = enumFSTATE_TARGET_NO;
			strcpy(SwitchInfo.szTarMapName, SwitchInfo.szSrcMapName);
			Point SPos;
			SPos = m_STerritory.centre;
			SPos.move(rand() % 360, 3 * 100);
			if (!pCMap->IsValidPos(SPos.x, SPos.y))
				SPos = m_STerritory.centre;
			SwitchInfo.STarPos = SPos;
			pCMap->m_COutMapCha.Add(this, GetID(), &SwitchInfo, enumCHA_TIMEER_ENTERMAP, m_SExistCtrl.lWitherTime, m_SExistCtrl.lResumeTime);
		}
	}
T_E}

void CCharacter::JustDie(CCharacter *pCSrcCha)
{
	g_EventHandler.Event_ChaDie(this, pCSrcCha);
}

//=============================================================================
// ֻ����ͬ�ֽ�ɫ��̬��λ���л�
//=============================================================================
void CCharacter::MoveCity(cChar *szCityName, Long lMapCopyNO, Char chSwitchType)
{
	MPTimer t; t.Begin();

	SBirthPoint	*pSBirthP;
	if (!strcmp(szCityName, ""))
		pSBirthP = GetRandBirthPoint(GetLogName(), GetBirthCity());
	else
		pSBirthP = GetRandBirthPoint(GetLogName(), szCityName);
	SwitchMap(GetSubMap(), pSBirthP->szMapName, (pSBirthP->x + 2 - rand() % 4) * 100, (pSBirthP->y + 2 - rand() % 4) * 100, true, chSwitchType, lMapCopyNO);

	// temp...
	DWORD dwEndTime = t.End();
	if(dwEndTime > 20)
	{
		if (GetSubMap())
			//LG("script_time", "\t��ɫ %s ��ͼ�л���%s-->%s������ʱ����� time = %d\n", GetLogName(), GetSubMap()->GetName(), pSBirthP->szMapName, dwEndTime);
			LG("script_time", "\tcharacter %s map switch(%s-->%s) expend much time:time = %d\n", GetLogName(), GetSubMap()->GetName(), pSBirthP->szMapName, dwEndTime);
		else
			//LG("script_time", "\t��ɫ %s ��ͼ�л���""-->%s������ʱ����� time = %d\n", GetLogName(), pSBirthP->szMapName, dwEndTime);
			LG("script_time", "\tcharacter %s map switch(""-->%s) expend much time:time = %d\n", GetLogName(), pSBirthP->szMapName, dwEndTime);
	}
}

//=============================================================================
// ���س���
//=============================================================================
void CCharacter::BackToCity(bool bDie, cChar *szCityName, Long lMapCpyNO, Char chSwitchType)
{
	SubMap	*pCMap = GetSubMap();
	pCMap->GoOut(this);
	SetToMainCha(bDie);
	CCharacter	*pCMainCha = GetPlyMainCha();
	pCMainCha->SetExistState(enumEXISTS_NATALITY);
	//pCMainCha->m_timerScripts.Reset();

	if(bDie && (!strcmp(pCMap->GetName(), "guildwar")))
	{
		if(GetGuildType() == emGldTypePirate)
		{
			szCityName = "guildwarpirateside";
		}
		else
		{
			szCityName = "guildwarnavyside";
		}
	}
	else if(bDie && (!strcmp(pCMap->GetName(), "guildwar2")))
	{
		if(GetGuildType() == emGldTypePirate)
		{
			szCityName = "guildwarpirateside2";
		}
		else
		{
			szCityName = "guildwarnavyside2";
		}
	}

	if (!szCityName || !strcmp(szCityName, ""))
		pCMainCha->ResetBirthInfo();
	else
	{
		SBirthPoint	*pSBirthP;
		pSBirthP = GetRandBirthPoint(GetLogName(), szCityName);
		SetBirthMap(pSBirthP->szMapName);
		SetPos(pSBirthP->x * 100, pSBirthP->y * 100);
	}
	pCMainCha->SwitchMap(pCMap, pCMainCha->GetBirthMap(), pCMainCha->GetPos().x, pCMainCha->GetPos().y, false, chSwitchType, lMapCpyNO);
}

void CCharacter::BackToCityEx(bool bDie, cChar *szCityName, Long lMapCpyNO, Char chSwitchType)
{
	SubMap	*pCMap = GetSubMap();
	pCMap->GoOut(this);
	SetToMainCha(bDie);
	CCharacter	*pCMainCha = GetPlyMainCha();
	pCMainCha->SetExistState(enumEXISTS_NATALITY);
	//pCMainCha->m_timerScripts.Reset();

	if (!szCityName || !strcmp(szCityName, ""))
		pCMainCha->ResetBirthInfo();
	else
	{
		SBirthPoint	*pSBirthP;
		pSBirthP = GetRandBirthPoint(GetLogName(), szCityName);
		SetBirthMap(pSBirthP->szMapName);
		SetPos(pSBirthP->x * 100, pSBirthP->y * 100);
	}
	pCMainCha->SwitchMap(pCMap, pCMainCha->GetBirthMap(), pCMainCha->GetPos().x, pCMainCha->GetPos().y, false, chSwitchType, lMapCpyNO);
}

void CCharacter::SetToMainCha(bool bDie)
{
	if (!IsPlayerCha())
		return;
	CCharacter	*pCMainC = GetPlyMainCha();
	m_pCPlayer->SetLoginCha(enumLOGIN_CHA_MAIN, 0);
	m_pCPlayer->SetCtrlCha(pCMainC);
	if (IsBoat())
	{
		if (bDie)
			pCMainC->BoatDie(*this, *this);
		SetBirthMap("");
		SetPos(-1, -1);
	}
}

void CCharacter::BickerNotice( const char szData[], ... )
{
	// Modify by lark.li 20080801 begin
	char szTemp[250];
	memset(szTemp, 0, sizeof(szTemp));
	va_list list;
	va_start( list, szData );
	_vsnprintf(szTemp, sizeof(szTemp) - 1, szData, list );
	//vsprintf( szTemp, szData, list );
	// End
	va_end( list );

	WPACKET packet = GETWPACKET();
	WRITE_CMD(packet, CMD_MC_BICKER_NOTICE );
	WRITE_STRING(packet, szTemp);
	
	this->ReflectINFof( this, packet );
}

void CCharacter::ColourNotice( DWORD rgb, const char szData[], ... )
{
	char szTemp[250];
	memset(szTemp, 0, sizeof(szTemp));
	va_list list;
	va_start( list, szData );
	_vsnprintf(szTemp, sizeof(szTemp) - 1, szData, list );
	va_end( list );

	WPACKET packet = GETWPACKET();
	WRITE_CMD(packet, CMD_MC_COLOUR_NOTICE );
	WRITE_LONG(packet, rgb);
	WRITE_STRING(packet, szTemp);
	
	this->ReflectINFof( this, packet );
}

void CCharacter::SystemNotice( const char szData[], ... )
{T_B
	// Modify by lark.li 20080801 begin
	char szTemp[250];
	memset(szTemp, 0, sizeof(szTemp));
	va_list list;
	va_start( list, szData );
	_vsnprintf(szTemp, sizeof(szTemp) - 1, szData, list );
	//vsprintf( szTemp, szData, list );
	// End
	va_end( list );

	WPACKET packet = GETWPACKET();
	WRITE_CMD(packet, CMD_MC_SYSINFO );
	WRITE_SEQ(packet, szTemp, uShort( strlen(szTemp) ) + 1 );
	
	this->ReflectINFof( this, packet );
T_E}

void CCharacter::PopupNotice( const char szData[], ... )
{T_B
	// Modify by lark.li 20080801 begin
	char szTemp[250];
	memset(szTemp, 0, sizeof(szTemp));
	va_list list;
	va_start( list, szData );
	_vsnprintf(szTemp, sizeof(szTemp) - 1, szData, list );
	//vsprintf( szTemp, szData, list );
	// End
	va_end( list );

	WPACKET packet = GETWPACKET();
	WRITE_CMD(packet, CMD_MC_POPUP_NOTICE );
	WRITE_SEQ(packet, szTemp, uShort( strlen(szTemp) ) + 1 );

	this->ReflectINFof( this, packet );
T_E}

BOOL CCharacter::SetMissionPage( DWORD dwNpcID, BYTE byPrev, BYTE byNext, BYTE byState )
{T_B
	if( GetPlayer() )
	{
		GetPlayer()->MisSetMissionPage( dwNpcID, byPrev, byNext, byState );
		return TRUE;
	}
	return FALSE;
T_E}

BOOL CCharacter::GetMissionPage( DWORD dwNpcID, BYTE& byPrev, BYTE& byNext, BYTE& byState )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisGetMissionPage( dwNpcID, byPrev, byNext, byState );
	}
	return FALSE;
T_E}

BOOL CCharacter::SetTempData( DWORD dwNpcID, WORD wID, BYTE byState, BYTE byType )
{T_B
	if( GetPlayer() )
	{
		GetPlayer()->MisSetTempData( dwNpcID, wID, byState, byType );
		return TRUE;
	}
	return FALSE;
T_E}

BOOL CCharacter::GetTempData( DWORD dwNpcID, WORD& wID, BYTE& byState, BYTE& byType )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisGetTempData( dwNpcID, wID, byState, byType );
	}
	return FALSE;
T_E}

BOOL CCharacter::GetNumMission( DWORD dwNpcID, BYTE& byNum )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisGetNumMission( dwNpcID, byNum );
	}
	return FALSE;
T_E}

BOOL CCharacter::GetNextMission( DWORD dwNpcID, BYTE& byIndex, BYTE& byID, BYTE& byState )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisGetNextMission( dwNpcID, byIndex, byID, byState );
	}
	return FALSE;
T_E}

BOOL CCharacter::GetMissionInfo( DWORD dwNpcID, BYTE byIndex, BYTE& byID, BYTE& byState )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisGetMissionInfo( dwNpcID, byIndex, byID, byState );
	}
	return FALSE;
T_E}

BOOL CCharacter::GetCharMission( DWORD dwNpcID, BYTE byID, BYTE& byState )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisGetCharMission( dwNpcID, byID, byState );
	}
	return FALSE;
T_E}

BOOL CCharacter::GetMissionState( DWORD dwNpcID, BYTE& byState )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisGetMissionState( dwNpcID, byState );
	}
	return FALSE;
T_E}

BOOL CCharacter::AddMissionState( DWORD dwNpcID, BYTE byID, BYTE byState )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisAddMissionState( dwNpcID, byID, byState );
	}
	return FALSE;
T_E}

void CCharacter::SetEntityTime( DWORD dwTime )
{
	if( GetPlayer() == NULL )
		return;
	GetPlayer()->SetEntityTime( dwTime );
}

DWORD CCharacter::GetEntityTime()
{
	if( GetPlayer() == NULL )
		return 0;
	
	DWORD dwTime;
	if( !GetPlayer()->GetEntityTime( dwTime ) )
		return 0;

	return GetTickCount() - dwTime;
}

BOOL CCharacter::SetEntityState( DWORD dwEntityID, BYTE byState )
{
	if( GetPlayer() == NULL )
		return FALSE;
	// ͬ�����ͻ���
	WPACKET packet = GETWPACKET();
	WRITE_CMD(packet, CMD_MC_ENTITY_CHGSTATE );
	WRITE_LONG(packet, dwEntityID );
	WRITE_CHAR(packet, byState );
	ReflectINFof( this, packet );
	return TRUE;
}

BOOL CCharacter::ResetMissionState( mission::CTalkNpc& npc )
{T_B
	if( GetPlayer() == NULL )
		return FALSE;
	DWORD dwNpcID = npc.GetID();
	GetPlayer()->MisClearMissionState( dwNpcID );

	BYTE byState = 0;
	npc.MissionProc( *this, byState );

	// ͬ�����ͻ���
	WPACKET packet = GETWPACKET();
	WRITE_CMD(packet, CMD_MC_NPCSTATECHG );
	WRITE_LONG(packet, npc.GetID() );
	WRITE_CHAR(packet, byState );
	ReflectINFof( this, packet );
	return TRUE;
T_E}

BOOL CCharacter::ClearMissionState( DWORD dwNpcID )
{T_B	
	if( GetPlayer() )
	{
		return GetPlayer()->MisClearMissionState( dwNpcID );
	}
	return FALSE;
T_E}

void CCharacter::MisLog()
{T_B
	if( GetPlayer() )
	{
		GetPlayer()->MisGetMisLog();
	}
T_E}

void CCharacter::MisLogInfo( WORD wMisID )
{T_B
	if( GetPlayer() )
	{
		GetPlayer()->MisGetMisLogInfo( wMisID );
	}
T_E}

void CCharacter::MisLogClear( WORD wMisID )
{T_B
	if( GetPlayer() )
	{
		if( GetPlayer()->IsLuanchOut() )
		{
			if( GetPlayer()->GetLuanchOut()->m_pTradeData ) 
			{
				//SystemNotice( "��ǰ����״̬,�������ж�����!" );
				SystemNotice( RES_STRING(GM_CHARACTER_CPP_00001) );
				return;
			}
		}

		if( m_pTradeData )
		{
			//SystemNotice( "��ǰ����״̬,�������ж�����!" );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00001) );
			return;
		}
		GetPlayer()->MisCancelRole( wMisID );
	}
T_E}

BOOL CCharacter::ConvoyNpc( WORD wRoleID, BYTE byIndex, WORD wNpcCharID, BYTE byAiType )
{T_B
	if( GetPlayer() )
	{
		Point pos;
		pos = GetPos();
		pos.x += rand()%100;
		pos.y += rand()%100;
		CCharacter* pNpc = this->GetSubMap()->ChaSpawn( wNpcCharID, enumCHACTRL_NPC, rand()%360, &pos, TRUE );
		if( !pNpc )
		{
			return FALSE;
		}

		// �������������
		pNpc->m_AIType = byAiType;
		pNpc->m_AITarget = this;

		if( !GetPlayer()->MisAddFollowNpc( wRoleID, byIndex, wNpcCharID, pNpc, byAiType ) )
		{
			pNpc->Free();
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
T_E}

BOOL CCharacter::ClearConvoyNpc( WORD wRoleID, BYTE byIndex )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisClearFollowNpc( wRoleID, byIndex );
	}
	return FALSE;
T_E}

BOOL CCharacter::ClearAllConvoyNpc( WORD wRoleID )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisClearAllFollowNpc( wRoleID );
	}
	return FALSE;	
T_E}

BOOL CCharacter::HasConvoyNpc( WORD wRoleID, BYTE byIndex )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisHasFollowNpc( wRoleID, byIndex );
	}
	return FALSE;
T_E}

BOOL CCharacter::IsConvoyNpc( WORD wRoleID, BYTE byIndex, WORD wNpcCharID )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisIsFollowNpc( wRoleID, byIndex, wNpcCharID );
	}
	return FALSE;
T_E}

BOOL CCharacter::AddTrigger( const mission::TRIGGER_DATA& Data )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisAddTrigger( Data );
	}
	return FALSE;
T_E}

BOOL CCharacter::ClearTrigger( WORD wTriggerID )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisClearTrigger( wTriggerID );
	}
	return FALSE;
T_E}

BOOL CCharacter::DeleteTrigger( WORD wTriggerID )
{
	if( GetPlayer() )
	{
		return GetPlayer()->MisDelTrigger( wTriggerID );
	}

	return FALSE;
}

BOOL CCharacter::AddRole( WORD wID, WORD wParam )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisAddRole( wID, wParam );
	}
	return FALSE;
T_E}

BOOL CCharacter::HasRole( WORD wID )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisHasRole( wID );
	}
	return FALSE;
T_E}

BOOL CCharacter::ClearRole( WORD wID )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisClearRole( wID );
	}
	return FALSE;
T_E}

BOOL CCharacter::GetMisScriptID( WORD wID, WORD& wScriptID )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisGetMisScript( wID, wScriptID );
	}
	return FALSE;	
T_E}

BOOL CCharacter::SetMissionComplete( WORD wRoleID )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisSetMissionComplete( wRoleID );
	}
	return FALSE;
T_E}

BOOL CCharacter::SetMissionFailure( WORD wRoleID )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisSetMissionFailure( wRoleID );
	}
	return FALSE;
T_E}

BOOL CCharacter::HasMissionFailure( WORD wRoleID )
{
	if( GetPlayer() )
	{
		return GetPlayer()->MisHasMissionFailure( wRoleID );
	}
	return FALSE;
}

BOOL CCharacter::IsRoleFull()
{T_B
	if( GetPlayer() ) 
	{
		return GetPlayer()->MisIsRoleFull(); 
	}
	return TRUE; 
T_E}

BOOL CCharacter::SetFlag( WORD wID, WORD wFlag )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisSetFlag( wID, wFlag );
	}
	return FALSE;
T_E}

BOOL CCharacter::ClearFlag( WORD wID, WORD wFlag )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisClearFlag( wID, wFlag );
	}
	return FALSE;
T_E}

BOOL CCharacter::IsFlag( WORD wID, WORD wFlag )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisIsSet( wID, wFlag );
	}
	return FALSE;
T_E}

BOOL CCharacter::IsValidFlag( WORD wFlag )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisIsValid( wFlag );
	}
	return FALSE;
T_E}

BOOL CCharacter::SetRecord( WORD wRec )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisSetRecord( wRec );
	}
	return FALSE;
T_E}

BOOL CCharacter::ClearRecord( WORD wRec )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisClearRecord( wRec );
	}
	return FALSE;
T_E}

BOOL CCharacter::IsRecord( WORD wRec )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisIsRecord( wRec );
	}
	return FALSE;
T_E}

BOOL CCharacter::IsValidRecord( WORD wRec )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisIsValidRecord( wRec );
	}
	return FALSE;
T_E}

BOOL CCharacter::HasRandMission( WORD wRoleID )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisHasRandMission( wRoleID );
	}
	return FALSE;
T_E}

BOOL CCharacter::AddRandMission( WORD wRoleID, WORD wScriptID, BYTE byType, BYTE byLevel, DWORD dwExp, DWORD dwMoney, USHORT sPrizeData, USHORT sPrizeType, BYTE byNumData )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisAddRandMission( wRoleID, wScriptID, byType, byLevel, dwExp, dwMoney, sPrizeData, sPrizeType, byNumData );
	}
	return FALSE;
T_E}

BOOL CCharacter::SetRandMissionData( WORD wRoleID, BYTE byIndex, const mission::MISSION_DATA& RandData )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisSetRandMissionData( wRoleID, byIndex, RandData );
	}
	return FALSE;
T_E}

BOOL CCharacter::GetRandMission( WORD wRoleID, BYTE& byType, BYTE& byLevel, DWORD& dwExp, DWORD& dwMoney, USHORT& sPrizeData, USHORT& sPrizeType, BYTE& byNumData )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisGetRandMission( wRoleID, byType, byLevel, dwExp, dwMoney, sPrizeData, sPrizeType, byNumData );
	}
	return FALSE;
T_E}

BOOL CCharacter::GetRandMissionData( WORD wRoleID, BYTE byIndex, mission::MISSION_DATA& RandData )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisGetRandMissionData( wRoleID, byIndex, RandData );
	}
	return FALSE;
T_E}

BOOL CCharacter::HasSendNpcItemFlag( WORD wRoleID, WORD wNpcID )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisHasSendNpcItemFlag( wRoleID, wNpcID );
	}
	return FALSE;
T_E}

BOOL CCharacter::NoSendNpcItemFlag( WORD wRoleID, WORD wNpcID )
{
	if( GetPlayer() )
	{
		return GetPlayer()->MisNoSendNpcItemFlag( wRoleID, wNpcID );
	}
	return FALSE;
}

BOOL CCharacter::HasRandMissionNpc( WORD wRoleID, WORD wNpcID, WORD wAreaID )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisHasRandMissionNpc( wRoleID, wNpcID, wAreaID );
	}
	return FALSE;
T_E}

BOOL CCharacter::CompleteRandMission( WORD wRoleID )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisCompleteRandMission( wRoleID );
	}
	return FALSE;
T_E}

BOOL CCharacter::FailureRandMission( WORD wRoleID )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisFailureRandMission( wRoleID );
	}
	return FALSE;
T_E}

BOOL CCharacter::AddRandMissionNum( WORD wRoleID )
{
	if( GetPlayer() )
	{
		return GetPlayer()->MisAddRandMissionNum( wRoleID );
	}
	return FALSE;
}

BOOL CCharacter::ResetRandMission( WORD wRoleID )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisResetRandMission( wRoleID );
	}
	return FALSE;
T_E}

BOOL CCharacter::ResetRandMissionNum( WORD wRoleID )
{
	return ( GetPlayer() ) ? GetPlayer()->MisResetRandMissionNum( wRoleID ) : FALSE;
}

BOOL CCharacter::HasRandMissionCount( WORD wRoleID, WORD wCount )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisGetRandMissionCount( wRoleID ) >= wCount;
	}
	return FALSE;
T_E}

BOOL CCharacter::GetRandMissionCount( WORD wRoleID, WORD& wCount )
{T_B
	if( GetPlayer() )
	{
		wCount = GetPlayer()->MisGetRandMissionCount( wRoleID );
		return TRUE;
	}
	return FALSE;
T_E}

BOOL CCharacter::GetRandMissionNum( WORD wRoleID, WORD& wNum )
{T_B
	if( GetPlayer() ) 
	{
		wNum = GetPlayer()->MisGetRandMissionNum( wRoleID );
		return TRUE;
	}
	return FALSE;
T_E}

BOOL CCharacter::SafeSale( BYTE byIndex, BYTE byCount, WORD& wItemID, DWORD& dwMoney )
{T_B
	if(GetPlyMainCha()->m_CKitbag.IsPwdLocked())
	{
		//SystemNotice("������������!");
		SystemNotice(RES_STRING(GM_CHARACTER_CPP_00002));
		return FALSE;
	}

    ///stall/trade dupe fix [ mothannakh 15/8/2019]/
    //check if trade mode
    if (GetTradeData())
    {
        SystemNotice(RES_STRING(GM_CHARSTALL_CPP_00029));
        return FALSE;
    }
            //check if player stalling
    if (GetStallData())
    {
        //character.SystemNotice( "���ڰ�̯�������Խ���" );
        SystemNotice(RES_STRING(GM_CHARTRADE_CPP_00003));
        return FALSE;
    }
	
	//add by ALLEN 2007-10-16
		if(GetPlyMainCha()->IsReadBook())
	{
		//SystemNotice("���ڶ��飬�����Խ���!");
		SystemNotice(RES_STRING(GM_CHARACTER_CPP_00003));
		return FALSE;
	}

	USHORT sSize = m_CKitbag.GetCapacity();
	if( byIndex >= sSize )
	{
		//SystemNotice( "��Ʒ��λ��������!ID = %d", byIndex );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00004), byIndex );
		return FALSE;
	}

	wItemID = (WORD)m_CKitbag.GetID(byIndex);
	CItemRecord* pItem = GetItemRecordInfo( wItemID );
	if( pItem == NULL )
	{
		//SystemNotice( "��ƷID�����޷��ҵ�����Ʒ��Ϣ!ID = %d", wItemID );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00005), wItemID );
		return FALSE;
	}

	if( !pItem->chIsTrade )
	{
		//SystemNotice( "����Ʒ��%s�����ɽ���!", pItem->szName );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00006), pItem->szName );
		return FALSE;
	}

	::SItemGrid*	sig	=	m_CKitbag.GetGridContByID(	byIndex	);

	if(	sig	)
	{
		if(	sig->dwDBID )
		{
			SystemNotice(	"This item cannot be traded!"	);
			return	FALSE;
		}
	};
	
	if( !m_CKitbag.HasItem( byIndex ) )
	{
		//SystemNotice( "δ���ָ���Ʒ������λ(%d)����Ʒ��Ϣ!", byIndex );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00007), byIndex );
		return FALSE;
	}

	if( m_CKitbag.GetNum(byIndex) < byCount )
	{
		//SystemNotice( "������Ʒ��%s������(%d)���㣬����(%d)!", pItem->szName, byCount, wItemID );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00008), pItem->szName, byCount, wItemID );
		return FALSE;
	}
	
	// �ж϶�������֤��
	DWORD dwPrice = pItem->lPrice;
	if( pItem->sType == enumItemTypeBoat )
	{
		DWORD dwBoatID = (DWORD)m_CKitbag.GetDBParam( enumITEMDBP_INST_ID, byIndex );
		CCharacter* pBoat = GetPlayer()->GetBoat( dwBoatID );
		if( !pBoat )
		{
			//SystemNotice( "��ȡ��ֻ�۸���Ϣʧ��!" );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00009) );
			return FALSE;
		}
		dwPrice = (long)pBoat->getAttr( ATTR_BOAT_PRICE );

		if( !BoatClear( m_CKitbag.GetDBParam( enumITEMDBP_INST_ID, byIndex ) ) )
		{
			//SystemNotice( "���ۡ�%s��ʧ�ܣ�������ʹ�øô�!", pItem->szName );			
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00010), pItem->szName );			
			return enumITEMOPT_ERROR_UNUSE;
		}
	}

	m_CKitbag.SetChangeFlag( false );
	m_CChaAttr.ResetChangeFlag();
	SetBoatAttrChangeFlag(false);
	
	SItemGrid Grid;
	Grid.sNum = byCount;
	KbPopItem( true, false, &Grid, byIndex );

	dwMoney = (dwPrice>>1) * byCount;
	DWORD dwCharMoney = (long)getAttr( ATTR_GD );
	dwCharMoney += dwMoney;

	setAttr( ATTR_GD, dwCharMoney );
	SynAttr( enumATTRSYN_TRADE );
	SyncBoatAttr(enumATTRSYN_TRADE);
	
	//SystemNotice( "�������%d����%s����Ʒ�������(%d)��Ǯ���ܶ�(%d)!", byCount, pItem->szName, dwMoney, dwCharMoney );
	SystemNotice( RES_STRING(GM_CHARACTER_CPP_00011), byCount, pItem->szName, dwMoney, dwCharMoney );
	char szLog[128] = "";
	sprintf( szLog, "%d��%s", byCount, pItem->szName );
	TL( CHA_SELL, GetName(), "", szLog );

	// ����������Ʒ����
	RefreshNeedItem( wItemID );

	// ͬ����Ʒ����
	SynKitbagNew( enumSYN_KITBAG_FROM_NPC );

	// ���ݿⱣ��
	SaveAssets();
	LogAssets(enumLASSETS_TRADE);

	CItemRecord*	cir	=	::GetItemRecordInfo(	Grid.sID	);

	return TRUE;
T_E}

BOOL CCharacter::ExchangeReq(short sSrcID, short sSrcNum, short sTarID, short sTarNum)
{
	//char szNpc[128] = "ϵͳ";
	char szNpc[128];
	strncpy( szNpc, RES_STRING(GM_CHARACTER_CPP_00012), 128 - 1 );

	if (!GetPlyMainCha()->HasItem( sSrcID, sSrcNum ))
	{
		//SystemNotice("��û�жһ��������Ʒ!");
		SystemNotice(RES_STRING(GM_CHARACTER_CPP_00013));

		WPACKET packet = GETWPACKET();
		WRITE_CMD(packet, CMD_MC_BLACKMARKET_EXCHANGE_ASR);
		WRITE_CHAR(packet, 0);
		this->ReflectINFof( this, packet );

		return FALSE;
	}

	//add by jilinlee 2007.8.3  ��ֹ��Ҵ���Դ��������Ϊ0�����и���
	if (g_CParser.DoString("Can_Exchange", enumSCRIPT_RETURN_NUMBER, 1, enumSCRIPT_PARAM_NUMBER, 4, sSrcID, sSrcNum, sTarID, sTarNum, DOSTRING_PARAM_END))
	{
		if(!g_CParser.GetReturnNumber(0))
		{
			//SystemNotice("���ݴ��󣬶һ�ʧ��!");
			SystemNotice(RES_STRING(GM_CHARACTER_CPP_00014));
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
	//~~

	GetPlyMainCha()->TakeItem( sSrcID, sSrcNum, szNpc );
	GetPlyMainCha()->AddItem( sTarID, sTarNum, szNpc );
	//SystemNotice("�һ��ɹ�!");

	WPACKET packet = GETWPACKET();
	WRITE_CMD(packet, CMD_MC_BLACKMARKET_EXCHANGE_ASR);
	WRITE_CHAR(packet, 1);
	WRITE_SHORT(packet, sSrcID);
	WRITE_SHORT(packet, sSrcNum);
	WRITE_SHORT(packet, sTarID);
	WRITE_SHORT(packet, sTarNum);
	this->ReflectINFof( this, packet );

	return TRUE;
}

BOOL CCharacter::SafeBuy( WORD wItemID, BYTE byCount, BYTE byIndex, DWORD& dwMoney )
{T_B
	if(GetPlyMainCha()->m_CKitbag.IsPwdLocked())
	{
		//SystemNotice("������������!");
		SystemNotice(RES_STRING(GM_CHARACTER_CPP_00002));
		return FALSE;
	}

	//add by ALLEN 2007-10-16
		if(GetPlyMainCha()->IsReadBook())
	{
		//SystemNotice("���ڶ��飬�����Խ���!");
		SystemNotice(RES_STRING(GM_CHARACTER_CPP_00003));
		return FALSE;
	}

	CItemRecord* pItem = GetItemRecordInfo( wItemID );
	if( pItem == NULL )
	{
		//SystemNotice( "��ƷID�����޷��ҵ�����Ʒ��Ϣ!ID = %d", wItemID );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00005), wItemID );
		return FALSE;
	}

	if(byCount <= 0)
	{
		//SystemNotice("���ݴ��󣬹���ʧ��!");
		SystemNotice(RES_STRING(GM_CHARACTER_CPP_00015));
		return FALSE;
	}
	
	dwMoney = pItem->lPrice * byCount;
	USHORT sSize = m_CKitbag.GetCapacity();
	if( byIndex >= sSize )
	{
		//SystemNotice( "��Ʒ��λ����(%d)��Ч!", byIndex );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00016), byIndex );
		return FALSE;
	}

	if( m_CKitbag.GetID( byIndex ) == wItemID )
	{
		if( !pItem->GetIsPile() )
		{
			//SystemNotice( "����Ʒ��%s�������Զѵ����!", pItem->szName );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00017), pItem->szName );
			return FALSE;
		}
	}

	DWORD dwCharMoney = (long)getAttr( ATTR_GD );
	if( dwCharMoney < dwMoney )
	{
		//SystemNotice( "��Ľ�Ǯ(%d)��������%d����Ʒ��%s��!����(%d)", dwCharMoney,byCount, pItem->szName, pItem->lPrice );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00018), dwCharMoney,byCount, pItem->szName, pItem->lPrice );
		return FALSE;
	}

	USHORT sNum = byCount;

	SItemGrid SGridCont;
	SGridCont.sID = wItemID;
	SGridCont.sNum = byCount;
	ItemInstance( enumITEM_INST_BUY, &SGridCont );
	
	m_CKitbag.SetChangeFlag(false);
	m_CChaAttr.ResetChangeFlag();
	SetBoatAttrChangeFlag(false);
	// ����ʵ������Ʒ
	Short sPushPos = defKITBAG_DEFPUSH_POS;
	Short sPushRet = KbPushItem( true, false, &SGridCont, sPushPos );
	SynKitbagNew( enumSYN_KITBAG_FROM_NPC );
	if( sPushRet == enumKBACT_ERROR_LOCK ) // ������������
	{
		ItemOprateFailed( enumITEMOPT_ERROR_KBLOCK );
		return FALSE;
	}
	else if( sPushRet == enumKBACT_ERROR_PUSHITEMID ) // ���߲�����
	{
		ItemOprateFailed( enumITEMOPT_ERROR_NONE );
		return FALSE;
	}
	else if( sPushRet == enumKBACT_ERROR_FULL ) // ���������������ٹ�������
	{
		// �����Ʒ�����¼�
		sNum = sNum - SGridCont.sNum;
		if( sNum == 0 ) 
		{
			//SystemNotice( "��ĵ����������������Թ�����Ʒ!" );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00019) );
			return TRUE;
		}
		// �����Ʒ�����¼�
		AfterPeekItem( wItemID, sNum );
	}
	else if( sPushRet == enumKBACT_SUCCESS )
	{
		// �����Ʒ�����¼�
		AfterPeekItem( wItemID, sNum );
	}

	dwMoney = sNum * pItem->lPrice;
	dwCharMoney -= dwMoney;
	setAttr( ATTR_GD, dwCharMoney );
	SynAttr( enumATTRSYN_TRADE );
	SyncBoatAttr(enumATTRSYN_TRADE);

	//SystemNotice( "�㹺����%d����%s��������(%d)��Ǯ!���(%d)", sNum, pItem->szName, dwMoney, dwCharMoney );
	SystemNotice( RES_STRING(GM_CHARACTER_CPP_00020), sNum, pItem->szName, dwMoney, dwCharMoney );
	char szLog[128] = "";
	sprintf( szLog, "%d��%s", sNum, pItem->szName );
	TL( CHA_BUY, GetName(), "", szLog );

	// ���ݿⱣ��
	SaveAssets();
	LogAssets(enumLASSETS_TRADE);

	return TRUE;
T_E}

BOOL CCharacter::GetSaleGoodsItem( DWORD dwBoatID, BYTE byIndex, WORD& wItemID )
{T_B
	if( m_pCPlayer )
	{
		USHORT sBerthID, sxPos, syPos, sDir;
		m_pCPlayer->GetBerth( sBerthID, sxPos, syPos, sDir );

		BYTE byNumBoat = m_pCPlayer->GetNumBoat();
		for( BYTE i = 0; i < byNumBoat; i++ )
		{
			CCharacter* pBoat = m_pCPlayer->GetBoat( i );
			if( pBoat && pBoat->GetID() == dwBoatID && sBerthID == pBoat->getAttr( ATTR_BOAT_BERTH ) )
			{
				CKitbag& Bag = pBoat->m_CKitbag;
				USHORT sSize = Bag.GetCapacity();
				if( byIndex >= sSize )
				{
        			//SystemNotice( "��Ʒ��λ��������!ID = %d", byIndex );
					SystemNotice( RES_STRING(GM_CHARACTER_CPP_00004), byIndex );
					return FALSE;
				}

				wItemID = (WORD)Bag.GetID( byIndex );
				return TRUE;
			}
		}
	}
	return FALSE;
T_E}

BOOL CCharacter::SafeSaleGoods( DWORD dwBoatID, BYTE byIndex, BYTE byCount, WORD& wItemID, DWORD& dwMoney )
{T_B
	if( m_pCPlayer )
	{
		USHORT sBerthID, sxPos, syPos, sDir;
		m_pCPlayer->GetBerth( sBerthID, sxPos, syPos, sDir );

		BYTE byNumBoat = m_pCPlayer->GetNumBoat();
		for( BYTE i = 0; i < byNumBoat; i++ )
		{
			CCharacter* pBoat = m_pCPlayer->GetBoat( i );
			if( pBoat && pBoat->GetID() == dwBoatID && sBerthID == pBoat->getAttr( ATTR_BOAT_BERTH ) )
			{
				CKitbag& Bag = pBoat->m_CKitbag;
				USHORT sSize = Bag.GetCapacity();

				if(GetPlyMainCha()->m_CKitbag.IsPwdLocked())
				{
					//SystemNotice( "������������!" );
					SystemNotice( RES_STRING(GM_CHARACTER_CPP_00002) );
					return FALSE;
				}

				//add by ALLEN 2007-10-16
				if(GetPlyMainCha()->IsReadBook())
				{
					//SystemNotice( "���ڶ��飬�����Խ���!" );
					SystemNotice(RES_STRING(GM_CHARACTER_CPP_00003));
					return FALSE;
				}
				if( byIndex >= sSize )
				{
					//SystemNotice( "��Ʒ��λ��������!ID = %d", byIndex );
					SystemNotice( RES_STRING(GM_CHARACTER_CPP_00004), byIndex );
					return FALSE;
				}

				wItemID = (WORD)Bag.GetID(byIndex);
				CItemRecord* pItem = GetItemRecordInfo( wItemID );
				if( pItem == NULL )
				{
					//SystemNotice( "��ƷID�����޷��ҵ�����Ʒ��Ϣ!ID = %d", wItemID );
					SystemNotice( RES_STRING(GM_CHARACTER_CPP_00005), wItemID );
					return FALSE;
				}

				if( pItem->sType == enumItemTypeMission )
				{
					//SystemNotice( "������ߡ�%s�������Խ���!", pItem->szName );
					SystemNotice( RES_STRING(GM_CHARACTER_CPP_00021), pItem->szName );
					return FALSE;
				}

				if( !Bag.HasItem( byIndex ) )
				{
					//SystemNotice( "δ���ָ���Ʒ������λ(%d)����Ʒ��Ϣ!", byIndex );
					SystemNotice( RES_STRING(GM_CHARACTER_CPP_00007), byIndex );
					return FALSE;
				}

				if( Bag.GetNum(byIndex) < byCount )
				{
					//SystemNotice( "������Ʒ��%s������(%d)���㣬����(%d)!", pItem->szName, byCount, wItemID );
					SystemNotice( RES_STRING(GM_CHARACTER_CPP_00008), pItem->szName, byCount, wItemID );
					return FALSE;
				}

				// �ж϶�������֤��
				if( pItem->sType == enumItemTypeBoat )
				{
					if( !BoatClear( Bag.GetDBParam( enumITEMDBP_INST_ID, byIndex ) ) )
					{
						//SystemNotice( "���ۡ�%s��ʧ�ܣ�������ʹ�øô�!", pItem->szName );
						SystemNotice( RES_STRING(GM_CHARACTER_CPP_00010), pItem->szName );
						return enumITEMOPT_ERROR_UNUSE;
					}
				}

				Bag.SetChangeFlag( false );
				m_CChaAttr.ResetChangeFlag();
				SetBoatAttrChangeFlag(false);

				SItemGrid Grid;
				Grid.sNum = byCount;
				if( pBoat->KbPopItem( true, false, &Grid, byIndex ) != enumKBACT_SUCCESS )
				{
					//SystemNotice( "��ȡ%s���ճ��ۻ���ʧ��!", pBoat->GetName() );
					SystemNotice( RES_STRING(GM_CHARACTER_CPP_00022), pBoat->GetName() );
					//LG( "trade_error", "��ȡ��ɫ%s��ֻ%s���ճ��ۻ���ʧ��!ID[%d], Count[%d]", GetName(), pBoat->GetName(), wItemID, byCount );
					LG( "trade_error", "distill character %s boat %s cabin sale goods failed!ID[%d], Count[%d]", GetName(), pBoat->GetName(), wItemID, byCount );
					return FALSE;
				}

				// ͬ����Ʒ����
				pBoat->SynKitbagNew( enumSYN_KITBAG_FROM_NPC );

				DWORD dwPrice = ( dwMoney > 0 ) ? dwMoney : (pItem->lPrice)>>1;
				dwMoney = dwPrice * byCount;
				DWORD dwCharMoney = (long)getAttr( ATTR_GD );
				dwCharMoney += dwMoney;

				setAttr( ATTR_GD, dwCharMoney );
				SynAttr( enumATTRSYN_TRADE );
				SyncBoatAttr(enumATTRSYN_TRADE);

				// ���ݿⱣ��
				pBoat->SaveAssets();
				SaveAssets();
				pBoat->LogAssets(enumLASSETS_TRADE);

				//SystemNotice( "�������%d����%s����Ʒ�������(%d)��Ǯ���ܶ�(%d)!", byCount, pItem->szName, dwMoney, dwCharMoney );				
				SystemNotice( RES_STRING(GM_CHARACTER_CPP_00011), byCount, pItem->szName, dwMoney, dwCharMoney );				
				
				char szLog[128] = "";
				sprintf( szLog, "%d��%s", byCount, pItem->szName );
				TL( BOAT_SYS, GetName(), "", szLog );
				return TRUE;
			}
		}
	}
	
	return FALSE;
T_E}

BOOL CCharacter::SafeBuyGoods( DWORD dwBoatID, WORD wItemID, BYTE byCount, BYTE byIndex, DWORD& dwMoney )
{T_B
	if( m_pCPlayer )
	{
		USHORT sBerthID, sxPos, syPos, sDir;
		m_pCPlayer->GetBerth( sBerthID, sxPos, syPos, sDir );

		BYTE byNumBoat = m_pCPlayer->GetNumBoat();
		for( BYTE i = 0; i < byNumBoat; i++ )
		{
			CCharacter* pBoat = m_pCPlayer->GetBoat( i );
			if( pBoat && pBoat->GetID() == dwBoatID && sBerthID == pBoat->getAttr( ATTR_BOAT_BERTH ) )
			{
				CItemRecord* pItem = GetItemRecordInfo( wItemID );
				if( pItem == NULL )
				{
					//SystemNotice( "��ƷID�����޷��ҵ�����Ʒ��Ϣ!ID = %d", wItemID );
					SystemNotice( RES_STRING(GM_CHARACTER_CPP_00005), wItemID );
					return FALSE;
				}

				CKitbag& Bag = pBoat->m_CKitbag;
				USHORT sSize = Bag.GetCapacity();
				if(GetPlyMainCha()->m_CKitbag.IsPwdLocked())
				{
					//SystemNotice( "������������!" );
					SystemNotice( RES_STRING(GM_CHARACTER_CPP_00002) );
					return FALSE;
				}

				//add by ALLEN 2007-10-16
				if(GetPlyMainCha()->IsReadBook())
				{
					//SystemNotice( "���ڶ��飬�����Խ���!" );
					SystemNotice(RES_STRING(GM_CHARACTER_CPP_00003));
					return FALSE;
				}

				if( byIndex >= sSize )
				{
					//SystemNotice( "��Ʒ��λ����(%d)��Ч!", byIndex );
					SystemNotice(RES_STRING(GM_CHARACTER_CPP_00016), byIndex );
					return FALSE;
				}

				if( Bag.GetID( byIndex ) == wItemID )
				{
					if( !pItem->GetIsPile() )
					{
						//SystemNotice( "����Ʒ��%s�������Զѵ����!", pItem->szName );
						SystemNotice( RES_STRING(GM_CHARACTER_CPP_00017), pItem->szName );
						return FALSE;
					}
				}

				// ������Ʒ�ܼ�
				USHORT sNum = byCount;
				DWORD dwPrice = ( dwMoney > 0 ) ? dwMoney : pItem->lPrice;
				dwMoney = sNum * dwPrice;

				DWORD dwCharMoney = (long)getAttr( ATTR_GD );
				if( dwCharMoney < dwMoney )
				{
					//SystemNotice( "��Ľ�Ǯ(%d)��������%d����Ʒ��%s��!����(%d)", dwCharMoney,byCount, pItem->szName, dwPrice );
					SystemNotice( RES_STRING(GM_CHARACTER_CPP_00018), dwCharMoney,byCount, pItem->szName, dwPrice );
					return FALSE;
				}				

				SItemGrid SGridCont;
				SGridCont.sID = wItemID;
				SGridCont.sNum = byCount;
				ItemInstance( enumITEM_INST_BUY, &SGridCont );

				Bag.SetChangeFlag(false);
				m_CChaAttr.ResetChangeFlag();
				SetBoatAttrChangeFlag(false);
				// ����ʵ������Ʒ
				Short sPushPos = defKITBAG_DEFPUSH_POS;
				Short sPushRet = pBoat->KbPushItem( true, false, &SGridCont, sPushPos );
				pBoat->SynKitbagNew( enumSYN_KITBAG_FROM_NPC );
				if( sPushRet == enumKBACT_ERROR_LOCK ) // ������������
				{
					ItemOprateFailed( enumITEMOPT_ERROR_KBLOCK );
					return FALSE;
				}
				else if( sPushRet == enumKBACT_ERROR_PUSHITEMID ) // ���߲�����
				{
					ItemOprateFailed( enumITEMOPT_ERROR_NONE );
					return FALSE;
				}
				else if( sPushRet == enumKBACT_ERROR_FULL ) // ���������������ٹ�������
				{
					// �����Ʒ�����¼�
					sNum = sNum - SGridCont.sNum;
					if( sNum == 0 ) 
					{
						//SystemNotice( "��ĵ����������������Թ�����Ʒ!" );
						SystemNotice( RES_STRING(GM_CHARACTER_CPP_00019) );
						return TRUE;
					}
					// �����Ʒ�����¼�
					AfterPeekItem( wItemID, sNum );
				}
				else if( sPushRet == enumKBACT_SUCCESS )
				{
					// �����Ʒ�����¼�
					AfterPeekItem( wItemID, sNum );
				}

				dwCharMoney -= dwMoney;
				setAttr( ATTR_GD, dwCharMoney );
				SynAttr( enumATTRSYN_TRADE );
				SyncBoatAttr(enumATTRSYN_TRADE);

				// ���ݿⱣ��
				pBoat->SaveAssets();
				SaveAssets();
				pBoat->LogAssets(enumLASSETS_TRADE);

				//SystemNotice( "�㹺����%d����%s��������(%d)��Ǯ!���(%d)", sNum, pItem->szName, dwMoney, dwCharMoney );
				SystemNotice( RES_STRING(GM_CHARACTER_CPP_00020), sNum, pItem->szName, dwMoney, dwCharMoney );
				
				char szLog[128] = "";
				sprintf( szLog, "%d��%s", sNum, pItem->szName );
				TL( SYS_BOAT, GetName(), "", szLog );

				return TRUE;
			}
		}
	}
	
	return FALSE;
T_E}

bool CCharacter::SetNarmalSkillState(bool bAdd, uChar uchStateID, uChar uchStateLv)
{T_B
	if (bAdd)
		return AddSkillState(0, GetID(), GetHandle(), enumSKILL_TYPE_SELF, enumSKILL_TAR_LORS, enumSKILL_EFF_HELPFUL, uchStateID, uchStateLv, -1);
	else
		return DelSkillState(uchStateID);
T_E}

bool CCharacter::StallAction(bool bLock)
{T_B
	SSkillGrid	*pSSkillCont = m_CSkillBag.GetSkillContByID(241);
	if (pSSkillCont)
		return SetNarmalSkillState(bLock, SSTATE_STALL, pSSkillCont->chLv);
	else
		return false;
T_E}

void CCharacter::AddMoney( const char szName[], DWORD dwMoney )
{T_B
	m_CChaAttr.ResetChangeFlag();
	DWORD dwCharMoney = (long)this->getAttr( ATTR_GD );
	dwCharMoney += dwMoney;
	setAttr( ATTR_GD, dwCharMoney );

	// ͬ����Ǯ
	SynAttr( enumATTRSYN_TASK );
	//SystemNotice( "%s������%d��Ǯ���ܶ�(%d)!", szName, dwMoney, dwCharMoney );
	//ColourNotice(0xb5eb8e, "Received %dg (Total: %dg)", dwMoney, dwCharMoney );
	SystemNotice( RES_STRING(GM_CHARACTER_CPP_00023), szName, dwMoney, dwCharMoney );
T_E}

BOOL CCharacter::TakeMoney( const char szName[], DWORD dwMoney )
{T_B
	m_CChaAttr.ResetChangeFlag();
	DWORD dwCharMoney = (long)this->getAttr( ATTR_GD );
	if( dwCharMoney < dwMoney )
		return FALSE;
	dwCharMoney -= dwMoney;
	setAttr( ATTR_GD, dwCharMoney );

	// ͬ����Ǯ
	SynAttr( enumATTRSYN_TASK );
	//SystemNotice( "%sȡ������%d��Ǯ�����(%d)!", szName, dwMoney, dwCharMoney );
	SystemNotice( RES_STRING(GM_CHARACTER_CPP_00024), szName, dwMoney, dwCharMoney );
	return TRUE;
T_E}

BOOL CCharacter::HasMoney( DWORD dwMoney )
{T_B
	return (DWORD)getAttr( ATTR_GD ) >= dwMoney;
T_E}

BOOL CCharacter::MakeItem( USHORT sItemID, USHORT sCount, USHORT& sItemPos, BYTE byAddType, BYTE bySoundType )
{T_B
	if( sCount <= 0 ) return FALSE;
	CItemRecord* pItem = GetItemRecordInfo( sItemID );
	if( pItem == NULL )
	{
		//SystemNotice( "MakeItem:�������Ʒ��������!ID = %d", sItemID );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00025), sItemID );
		return FALSE;
	}

	SItemGrid SGridCont;
	SGridCont.sID = sItemID;
	SGridCont.sNum = sCount;
	ItemInstance( byAddType, &SGridCont );
	
	// ����ʵ������Ʒ
	m_CKitbag.SetChangeFlag(false);
	Short sPushPos = defKITBAG_DEFPUSH_POS;
	Short sPushRet = KbPushItem( true, true, &SGridCont, sPushPos );
	if( sPushRet == enumKBACT_ERROR_LOCK ) // ������������
	{
		ItemOprateFailed( enumITEMOPT_ERROR_KBLOCK );
		return FALSE;
	}
	else if( sPushRet == enumKBACT_ERROR_PUSHITEMID ) // ���߲�����
	{
		ItemOprateFailed( enumITEMOPT_ERROR_NONE );
		return FALSE;
	}
	else if( sPushRet == enumKBACT_ERROR_FULL ) // ��������������������
	{
		ItemOprateFailed( enumKBACT_ERROR_FULL );
		return FALSE;
	}
	else if( sPushRet == enumKBACT_SUCCESS )
	{
		// �����Ʒ�����¼�
		AfterPeekItem( sItemID, sCount );
	}

	sItemPos = sPushPos;
	SynKitbagNew( enumSYN_KITBAG_SYSTEM );
	//SystemNotice( "%s������%d����%s����Ʒ!", "ϵͳ", sCount, pItem->szName );
	SystemNotice( RES_STRING(GM_CHARACTER_CPP_00026), RES_STRING(GM_CHARACTER_CPP_00012), sCount, pItem->szName );
	char szLog[128] = "";
	sprintf( szLog, "%d��%s", sCount, pItem->szName );
	TL( CHA_MIS, GetName(), "", szLog );

	return TRUE;
T_E}

BOOL CCharacter::GiveItem( USHORT sItemID, USHORT sCount, BYTE byAddType, BYTE bySoundType )
{T_B
	if( sCount <= 0 ) return TRUE;
	CItemRecord* pItem = GetItemRecordInfo( sItemID );
	if( pItem == NULL )
	{
		//SystemNotice( "GiveItem:�������Ʒ��������!ID = %d", sItemID );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00027), sItemID );
		return FALSE;
	}

	SItemGrid SGridCont;
	SGridCont.sID = sItemID;
	SGridCont.sNum = sCount;
	ItemInstance( byAddType, &SGridCont );
	
	// ����ʵ������Ʒ
	m_CKitbag.SetChangeFlag(false);
	Short sPushPos = defKITBAG_DEFPUSH_POS;
	Short sPushRet = KbPushItem( true, true, &SGridCont, sPushPos );
	if( sPushRet == enumKBACT_ERROR_LOCK ) // ������������
	{
		ItemOprateFailed( enumITEMOPT_ERROR_KBLOCK );
		return FALSE;
	}
	else if( sPushRet == enumKBACT_ERROR_PUSHITEMID ) // ���߲�����
	{
		ItemOprateFailed( enumITEMOPT_ERROR_NONE );
		return FALSE;
	}
	else if( sPushRet == enumKBACT_ERROR_FULL ) // ��������������������
	{
		// �����Ʒ�����¼�
		USHORT sNum = sCount - SGridCont.sNum;
		if( sNum > 0 ) AfterPeekItem( sItemID, sNum );

		CCharacter	*pCCtrlCha = GetPlyCtrlCha(), *pCMainCha = GetPlyMainCha();
		Long	lPosX, lPosY;
		pCCtrlCha->GetTrowItemPos(&lPosX, &lPosY);
		pCCtrlCha->GetSubMap()->ItemSpawn( &SGridCont, lPosX, lPosY, enumITEM_APPE_THROW, pCCtrlCha->GetID(), pCMainCha->GetID(), pCMainCha->GetHandle() );
		ItemOprateFailed( enumITEMOPT_ERROR_KBFULL );
	}
	else if( sPushRet == enumKBACT_SUCCESS )
	{
		// �����Ʒ�����¼�
		AfterPeekItem( sItemID, sCount );
	}

	SynKitbagNew( enumSYN_KITBAG_SYSTEM );

	return TRUE;
T_E}

BOOL CCharacter::GiveItem2KitbagTemp( USHORT sItemID, USHORT sCount, ItemInfo *pItemAttr, BYTE bySoundType )
{T_B
	if( sCount <= 0 ) return TRUE;
	CItemRecord* pItem = GetItemRecordInfo( sItemID );
	if( pItem == NULL )
	{
		//SystemNotice( "GiveItem2KitbagTemp:�������Ʒ��������!ID = %d", sItemID );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00028), sItemID );
		return FALSE;
	}

	SItemGrid SGridCont;
	SGridCont.sID = sItemID;
	SGridCont.sNum = sCount;
	if(pItemAttr == NULL)
	{
		ItemInstance( enumITEM_INST_BUY, &SGridCont );
	}
	else
	{
		ItemInstance( enumITEM_INST_BUY, &SGridCont );

		int i;
		for(i = 0; i < defITEM_INSTANCE_ATTR_NUM; i++)
		{
			SGridCont.sInstAttr[i][0] = (short)pItemAttr->itemAttrID[i];
			SGridCont.sInstAttr[i][1] = (short)pItemAttr->itemAttrVal[i];
		}

		//���ò���
		unsigned long ulForgeP = SGridCont.GetDBParam(enumITEMDBP_FORGE);
		short sHole = static_cast<short>(ulForgeP / 1000000000);
		ulForgeP = ulForgeP + (pItemAttr->itemFlute - sHole) * 1000000000;
		SGridCont.SetDBParam(enumITEMDBP_FORGE, static_cast<long>(ulForgeP));
	}

	// ����ʵ������Ʒ
	m_pCKitbagTmp->SetChangeFlag(false);
	Short sPushPos = defKITBAG_DEFPUSH_POS;
	Short sPushRet = m_pCKitbagTmp->Push(&SGridCont, sPushPos);
	if( sPushRet == enumKBACT_ERROR_LOCK ) // ������������
	{
		ItemOprateFailed( enumITEMOPT_ERROR_KBLOCK );
		return FALSE;
	}
	else if( sPushRet == enumKBACT_ERROR_PUSHITEMID ) // ���߲�����
	{
		ItemOprateFailed( enumITEMOPT_ERROR_NONE );
		return FALSE;
	}
	else if( sPushRet == enumKBACT_ERROR_FULL ) // ��������������������
	{
		// �����Ʒ�����¼�
		USHORT sNum = sCount - SGridCont.sNum;
		if( sNum > 0 ) AfterPeekItem( sItemID, sNum );

		CCharacter	*pCCtrlCha = GetPlyCtrlCha(), *pCMainCha = GetPlyMainCha();
		Long	lPosX, lPosY;
		pCCtrlCha->GetTrowItemPos(&lPosX, &lPosY);
		pCCtrlCha->GetSubMap()->ItemSpawn( &SGridCont, lPosX, lPosY, enumITEM_APPE_THROW, pCCtrlCha->GetID(), pCMainCha->GetID(), pCMainCha->GetHandle() );
		ItemOprateFailed( enumITEMOPT_ERROR_KBFULL );
	}
	else if( sPushRet == enumKBACT_SUCCESS )
	{
		// �����Ʒ�����¼�
		AfterPeekItem( sItemID, sCount );
	}

	SynKitbagTmpNew( enumSYN_KITBAG_SYSTEM );

	return TRUE;
T_E}

BOOL CCharacter::AddItem( USHORT sItemID, USHORT sCount, const char szName[], BYTE byAddType, BYTE bySoundType )
{T_B
	//char szItem[128] = "δ֪";
	char szItem[128] = "";

	CItemRecord* pItem = GetItemRecordInfo( sItemID );
	if( pItem == NULL )
	{
		//SystemNotice( "AddItem:�������Ʒ��������!ID = %d", sItemID );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00029), sItemID );
		return FALSE;
	}
	strncpy( szItem, pItem->szName,128 - 1 );//�˴�Ҫע�⣬szItem����Ҫ����������汾���е�������������ɵ��ߣ�����Ϊ128

	if( GiveItem( sItemID, sCount, byAddType, bySoundType ) )
	{
		//SystemNotice( "%s������%d����%s����Ʒ!", szName, sCount, szItem );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00026), szName, sCount, szItem );
		char szLog[128] = "";
		//sprintf( szLog, "%d��%s", sCount, szItem );
		sprintf( szLog, RES_STRING(GM_CHARACTER_CPP_00096), sCount, szItem );
		TL( CHA_MIS, GetName(), "", szLog );

		return TRUE;
	}
	else
	{
		//SystemNotice( "%s����%d����%s����Ʒ������ʧ��!", szName, sCount, szItem );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00030), szName, sCount, szItem );
	}

	return FALSE;
T_E}

BOOL CCharacter::AddItem2KitbagTemp( USHORT sItemID, USHORT sCount, ItemInfo *pItemAttr, BYTE bySoundType )
{T_B
	//char szItem[32] = "δ֪";
	char szItem[32];
	strncpy( szItem, RES_STRING(GM_CHARACTER_CPP_00031), 32 - 1 );

	CItemRecord* pItem = GetItemRecordInfo( sItemID );
	if( pItem == NULL )
	{
		//SystemNotice( "AddItem2KitbagTemp:�������Ʒ��������!ID = %d", sItemID );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00032), sItemID );
		return FALSE;
	}
	strcpy( szItem, pItem->szName );

	if( GiveItem2KitbagTemp( sItemID, sCount, pItemAttr, bySoundType ) )
	{
		//SystemNotice( "�㹺����%d����%s����Ʒ!", sCount, szItem );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00033), sCount, szItem );		

		return TRUE;
	}
	else
	{
		//SystemNotice( "�㹺��%d����%s����Ʒ������ʧ��!", sCount, szItem );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00034), sCount, szItem );
	}

	return FALSE;
T_E}

BOOL CCharacter::AddItem2KitbagTemp( USHORT sItemID, USHORT sCount, const char szName[], BYTE byAddType, BYTE bySoundType )
{T_B
	//char szItem[32] = "δ֪";
	char szItem[32];
	strncpy( szItem, RES_STRING(GM_CHARACTER_CPP_00031), 32 - 1 );

	CItemRecord* pItem = GetItemRecordInfo( sItemID );
	if( pItem == NULL )
	{
		//SystemNotice( "AddItem2KitbagTemp:�������Ʒ��������!ID = %d", sItemID );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00032), sItemID );
		return FALSE;
	}
	strcpy( szItem, pItem->szName );
	if( GiveItem2KitbagTemp( sItemID, sCount, byAddType, bySoundType ) )
	{
		//SystemNotice( "%s������%d����%s����Ʒ!", szName, sCount, szItem );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00026), szName, sCount, szItem );
		char szLog[128] = "";
		sprintf( szLog, "%d %s", sCount, szItem );
		TL( CHA_MIS, GetName(), "", szLog );
		return TRUE;
	}
	else
	{
		//SystemNotice( "%s����%d����%s����Ʒ������ʧ��!", szName, sCount, szItem );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00030), szName, sCount, szItem );
	}

	return FALSE;
T_E}

BOOL CCharacter::GiveItem2KitbagTemp( USHORT sItemID, USHORT sCount, BYTE byAddType, BYTE bySoundType )
{T_B
	if( sCount <= 0 ) return TRUE;
	CItemRecord* pItem = GetItemRecordInfo( sItemID );
	if( pItem == NULL )
	{
		//SystemNotice( "GiveItem2KitbagTemp:�������Ʒ��������!ID = %d", sItemID );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00028), sItemID );
		return FALSE;
	}
	
	SItemGrid SGridCont;
	SGridCont.sID = sItemID;
	SGridCont.sNum = sCount;
	ItemInstance( byAddType, &SGridCont );

	// ����ʵ������Ʒ
	m_pCKitbagTmp->SetChangeFlag(false);
	Short sPushPos = defKITBAG_DEFPUSH_POS;
	Short sPushRet = m_pCKitbagTmp->Push(&SGridCont, sPushPos);
	if( sPushRet == enumKBACT_ERROR_LOCK ) // ������������
	{
		ItemOprateFailed( enumITEMOPT_ERROR_KBLOCK );
		return FALSE;
	}
	else if( sPushRet == enumKBACT_ERROR_PUSHITEMID ) // ���߲�����
	{
		ItemOprateFailed( enumITEMOPT_ERROR_NONE );
		return FALSE;
	}
	else if( sPushRet == enumKBACT_ERROR_FULL ) // ��������������������
	{
		// �����Ʒ�����¼�
		USHORT sNum = sCount - SGridCont.sNum;
		if( sNum > 0 ) AfterPeekItem( sItemID, sNum );

		CCharacter	*pCCtrlCha = GetPlyCtrlCha(), *pCMainCha = GetPlyMainCha();
		Long	lPosX, lPosY;
		pCCtrlCha->GetTrowItemPos(&lPosX, &lPosY);
		pCCtrlCha->GetSubMap()->ItemSpawn( &SGridCont, lPosX, lPosY, enumITEM_APPE_THROW, pCCtrlCha->GetID(), pCMainCha->GetID(), pCMainCha->GetHandle() );
		ItemOprateFailed( enumITEMOPT_ERROR_KBFULL );
	}
	else if( sPushRet == enumKBACT_SUCCESS )
	{
		// �����Ʒ�����¼�
		AfterPeekItem( sItemID, sCount );
	}

	SynKitbagTmpNew( enumSYN_KITBAG_SYSTEM );
	return TRUE;
T_E}

BOOL CCharacter::TakeItemBagTemp(USHORT sItemID, USHORT sCount, const char szName[])
{T_B
	int nNum = 0, nCount = 0;
	//char szItem[32] = "δ֪";
	char szItem[32];
	strncpy( szItem, RES_STRING(GM_CHARACTER_CPP_00031), 32 - 1 );

	CItemRecord* pItem = GetItemRecordInfo( sItemID );
	if( pItem == NULL )
	{
		//SystemNotice( "TakeItem:�������Ʒ��������!ID = %d", sItemID );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00035), sItemID );
		return FALSE;
	}
	strcpy( szItem, pItem->szName );
	USHORT sSize = m_pCKitbagTmp->GetUseGridNum();
	Short sIndex[defMAX_KBITEM_NUM_PER_TYPE][2];
	memset( sIndex, 0, sizeof(Short)*sSize );
	SItemGrid	*pGrid;
	SItemGrid SGridCont;
	for( int i = 0; i < sSize; i++ )
	{
		pGrid = m_pCKitbagTmp->GetGridContByNum(i);
		if (!pGrid)
			continue;
		if( pGrid->sID == sItemID )
		{
			sIndex[nNum][0] = m_pCKitbagTmp->GetPosIDByNum(i);
			sIndex[nNum][1] = sCount - nCount;
			if (sIndex[nNum][1] > pGrid->sNum)
				sIndex[nNum][1] = pGrid->sNum;
			nNum++;
			nCount += pGrid->sNum;
			if( nCount >= sCount )
			{
				nCount = sCount;
				break;
			}
		}
	}

	if( nCount < sCount )
	{
		//SystemNotice( "��Ҫ��ȡ%d����%s����Ʒ����������(%d)����!", sCount, szItem, nCount );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00036), sCount, szItem, nCount );
		return FALSE;
	}

	m_pCKitbagTmp->SetChangeFlag(false);
	for( int i = 0; i < nNum; i++ )
	{
		SGridCont.sNum = sIndex[i][1];
		m_pCKitbagTmp->Pop(&SGridCont, sIndex[i][0]);
		/*if( KbPopItem(true, true, &SGridCont, sIndex[i][0]) != enumKBACT_SUCCESS )
		{
			SystemNotice( "%s����ȡ�����%d����%s����Ʒ!GridID = %d, NumItem = %d", szName, sCount, szItem, sIndex[i][0], sIndex[i][1] );
			return FALSE;
		}*/
	}

	// ͬ��������Ϣ
	SynKitbagTmpNew( enumSYN_KITBAG_SYSTEM );
	//SystemNotice( "%sȡ�������%d����%s����Ʒ!", szName, sCount, szItem );
	SystemNotice( RES_STRING(GM_CHARACTER_CPP_00038), szName, sCount, szItem );
	char szLog[128] = "";
	sprintf( szLog, "%d��%s", sCount, szItem );
	TL( MIS_CHA, GetName(), "", szLog );

	// ˢ��������߼���
	RefreshNeedItem( sItemID );
	return TRUE;
T_E}

BOOL CCharacter::TakeItem( USHORT sItemID, USHORT sCount, const char szName[] )
{T_B
	int nNum = 0, nCount = 0;
	//char szItem[32] = "δ֪";
	char szItem[43];
	//strncpy( szItem, RES_STRING(GM_CHARACTER_CPP_00031), 32 - 1 );
	strncpy( szItem, RES_STRING(GM_CHARACTER_CPP_00031), 43 - 1 );
	if (_countof(szItem) > 43)
	{
		SystemNotice("Item Name too Long Pm GM !!");
		return false;
	}
	
	
	////
	CItemRecord* pItem = GetItemRecordInfo( sItemID );
	if( pItem == NULL )
	{
		//SystemNotice( "TakeItem:�������Ʒ��������!ID = %d", sItemID );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00035), sItemID );
		return FALSE;
	}
	///mothannakh fix// item name long string return false  ///
	if (strlen(pItem->szName) > 43)
	{
		SystemNotice("Item Name too Long Pm GM !");
		return false;
	}
	// exploit fix end --	
	strcpy( szItem, pItem->szName );
	USHORT sSize = m_CKitbag.GetUseGridNum();
	Short sIndex[defMAX_KBITEM_NUM_PER_TYPE][2];
	memset( sIndex, 0, sizeof(Short)*sSize );
	SItemGrid	*pGrid;
	SItemGrid SGridCont;
	for( int i = 0; i < sSize; i++ )
	{
		pGrid = m_CKitbag.GetGridContByNum(i);
		if (!pGrid)
			continue;
		if( pGrid->sID == sItemID )
		{
			sIndex[nNum][0] = m_CKitbag.GetPosIDByNum(i);
			sIndex[nNum][1] = sCount - nCount;
			if (sIndex[nNum][1] > pGrid->sNum)
				sIndex[nNum][1] = pGrid->sNum;
			nNum++;
			nCount += pGrid->sNum;
			if( nCount >= sCount )
			{
				nCount = sCount;
				break;
			}
		}
	}

	if( nCount < sCount )
	{
		//SystemNotice( "��Ҫ��ȡ%d����%s����Ʒ����������(%d)����!", sCount, szItem, nCount );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00036), sCount, szItem, nCount );
        		return FALSE;
	}

	m_CKitbag.SetChangeFlag(false);
	for( int i = 0; i < nNum; i++ )
	{
		SGridCont.sNum = sIndex[i][1];
		if( KbPopItem(true, true, &SGridCont, sIndex[i][0]) != enumKBACT_SUCCESS )
		{
			//SystemNotice( "%s����ȡ�����%d����%s����Ʒ!GridID = %d, NumItem = %d", szName, sCount, szItem, sIndex[i][0], sIndex[i][1] );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00037), szName, sCount, szItem, sIndex[i][0], sIndex[i][1] );
			return FALSE;
		}
	}

	// ͬ��������Ϣ
	SynKitbagNew( enumSYN_KITBAG_SYSTEM );
	//SystemNotice( "%sȡ�������%d����%s����Ʒ!", szName, sCount, szItem );
	SystemNotice( RES_STRING(GM_CHARACTER_CPP_00038), szName, sCount, szItem );
	char szLog[128] = "";
	sprintf( szLog, "%d��%s", sCount, szItem );
	TL( MIS_CHA, GetName(), "", szLog );

	// ˢ��������߼���
	RefreshNeedItem( sItemID );
	return TRUE;
T_E}

BOOL CCharacter::TakeAllRandItem( WORD wRoleID )
{T_B
	if( GetPlayer() )
	{
		return GetPlayer()->MisTakeAllRandNpcItem( wRoleID );
	}
	return FALSE;
T_E}

BOOL CCharacter::TakeRandNpcItem( WORD wRoleID, WORD wNpcID, const char szNpc[] )
{T_B
	if( GetPlayer() )
	{
		USHORT sItemID;
		if( !GetPlayer()->MisTakeRandMissionNpcItem( wRoleID, wNpcID, sItemID ) )
		{
			//SystemNotice( "TakeRandItem:��ȡ���������Ʒ��Ϣʧ��!RoleID = %d, NpcID = %d", wRoleID, wNpcID );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00039), wRoleID, wNpcID );
			return FALSE;
		}

		//char szNpc[NPC_MAXSIZE_NAME] = "δ֪";
		//if( this->m_submap )
		//{
		//	CNpcRecord* pRec = m_submap->GetNpcInfo( wNpcID );
		//	if( pRec )
		//	{
		//		strncpy( szNpc, pRec->szName, NPC_MAXSIZE_NAME - 1 );
		//	}
		//}

		if( !GetPlyMainCha()->TakeItem( sItemID, 1, szNpc ) )
		{
			//SystemNotice( "TakeRandItem:%sȡ�����������Ʒ����ʧ��!sItemID = %d", szNpc, sItemID );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00040), szNpc, sItemID );
			return FALSE;
		}
		return TRUE;
	}

	return FALSE;
T_E}

BOOL CCharacter::IsMisNeedItem( USHORT sItemID )
{T_B
	return ( GetPlayer() ) ? GetPlayer()->MisNeedItem( sItemID ) : FALSE;
T_E}

BOOL CCharacter::GetMisNeedItemCount( WORD wRoleID, USHORT sItemID, USHORT& sCount )
{
	return ( GetPlayer() ) ? GetPlayer()->MisGetItemCount( wRoleID, sItemID, sCount ) : FALSE;
}

void CCharacter::RefreshNeedItem( USHORT sItemID )
{
	if( GetPlayer() ) {
		GetPlayer()->MisRefreshItemCount( sItemID );
	}
}

BOOL CCharacter::HasItem( USHORT sItemID, USHORT sCount )
{T_B
	int nCount = 0;
	CItemRecord* pItem = GetItemRecordInfo( sItemID );
	if( pItem == NULL )
	{
		//SystemNotice( "HasItem:�������Ʒ��������!ID = %d", sItemID );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00041), sItemID );
		return FALSE;
	}

	USHORT sNum = m_CKitbag.GetUseGridNum();
	SItemGrid *pGridCont;
	if( !pItem->GetIsPile() )
	{
		for( int i = 0; i < sNum; i++ )
		{
			pGridCont = m_CKitbag.GetGridContByNum( i );
			if( pGridCont )
			{
				if( sItemID == pGridCont->sID )
				{
					nCount++;
					if( nCount >= sCount )
						break;
				}
			}
		}
	}
	else
	{
		for( int i = 0; i < sNum; i++ )
		{
			pGridCont = m_CKitbag.GetGridContByNum( i );
			if( pGridCont )
			{
				if( sItemID == pGridCont->sID )
				{
					nCount += (USHORT)pGridCont->sNum;;
					if( nCount >= sCount )
						break;
				}
			}
		}
	}

	return nCount >= sCount;
T_E}

BOOL CCharacter::HasItemBagTemp(USHORT sItemID, USHORT sCount)
{T_B
	int nCount = 0;
	CItemRecord* pItem = GetItemRecordInfo( sItemID );
	if( pItem == NULL )
	{
		//SystemNotice( "HasItemBagTemp:�������Ʒ��������!ID = %d", sItemID );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00042), sItemID );
		return FALSE;
	}

	if(!m_pCKitbagTmp)
	{
		//SystemNotice( "HasItemBagTemp: û����ʱ����!" );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00043) );
		return FALSE;
	}

	USHORT sNum = m_pCKitbagTmp->GetUseGridNum();
	SItemGrid *pGridCont;
	if( !pItem->GetIsPile() )
	{
		for( int i = 0; i < sNum; i++ )
		{
			pGridCont = m_pCKitbagTmp->GetGridContByNum( i );
			if( pGridCont )
			{
				if( sItemID == pGridCont->sID )
				{
					nCount++;
					if( nCount >= sCount )
						break;
				}
			}
		}
	}
	else
	{
		for( int i = 0; i < sNum; i++ )
		{
			pGridCont = m_pCKitbagTmp->GetGridContByNum( i );
			if( pGridCont )
			{
				if( sItemID == pGridCont->sID )
				{
					nCount += (USHORT)pGridCont->sNum;;
					if( nCount >= sCount )
						break;
				}
			}
		}
	}

	return nCount >= sCount;
T_E}

BOOL CCharacter::GetNumItem( USHORT sItemID, USHORT& sCount )
{T_B
	USHORT sNum = m_CKitbag.GetUseGridNum();
	SItemGrid *pGridCont;
	for( int i = 0; i < sNum; i++ )
	{
		pGridCont = m_CKitbag.GetGridContByNum( i );
		if( pGridCont )
		{
			if( sItemID == pGridCont->sID )
			{
				sCount += (USHORT)pGridCont->sNum;
			}
		}
	}
	return TRUE;
T_E}

BOOL CCharacter::HasTradeItemLevel( BYTE byLevel )
{
	USHORT sNum = m_CKitbag.GetUseGridNum();
	SItemGrid *pGridCont;
	for( int i = 0; i < sNum; i++ )
	{
		pGridCont = m_CKitbag.GetGridContByNum( i );
		if( pGridCont )
		{
			CItemRecord* pItem = GetItemRecordInfo( pGridCont->sID );
			if( pItem && pItem->sType == enumItemTypeTrade )
			{
				return pGridCont->sEnergy[0] >= byLevel;
			}
		}
	}

	return FALSE;
}

BOOL CCharacter::SetTradeItemLevel( BYTE byLevel )
{
	USHORT sNum = m_CKitbag.GetUseGridNum();
	SItemGrid *pGridCont;
	Short	sPosID;
	for( int i = 0; i < sNum; i++ )
	{
		pGridCont = m_CKitbag.GetGridContByNum( i );
		if( pGridCont )
		{
			CItemRecord* pItem = GetItemRecordInfo( pGridCont->sID );
			if( pItem && pItem->sType == enumItemTypeTrade )
			{				
				sPosID = m_CKitbag.GetPosIDByNum( i );
				//LG( "TradeCess", "��ɫ%s���ó�׵ȼ�Level = %d, CurLevel = %d.", GetName(), byLevel, m_CKitbag.GetEnergy( false, sPosID ) );
				LG( "TradeCess", "character %s add trade level:Level = %d, CurLevel = %d.", GetName(), byLevel, m_CKitbag.GetEnergy( false, sPosID ) );
				m_CKitbag.SetChangeFlag(false);
				m_CKitbag.SetEnergy(false, byLevel, sPosID);
				m_CKitbag.SetSingleChangeFlag( sPosID );
				SynKitbagNew( enumSYN_KITBAG_FROM_NPC );
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CCharacter::GetTradeItemLevel( BYTE& byLevel )
{
	USHORT sNum = m_CKitbag.GetUseGridNum();
	SItemGrid *pGridCont;
	for( int i = 0; i < sNum; i++ )
	{
		pGridCont = m_CKitbag.GetGridContByNum( i );
		if( pGridCont )
		{
			CItemRecord* pItem = GetItemRecordInfo( pGridCont->sID );
			if( pItem && pItem->sType == enumItemTypeTrade )
			{
				byLevel = (BYTE)pGridCont->sEnergy[0];
				return TRUE;
			}
		}
	}

	return FALSE;
}
BOOL CCharacter::AdjustTradeItemCess( USHORT sLowCess, USHORT sData )
{	
	USHORT sNum = m_CKitbag.GetUseGridNum();
	SItemGrid *pGridCont;
	Short	sPosID;
	for( int i = 0; i < sNum; i++ )
	{
		pGridCont = m_CKitbag.GetGridContByNum( i );
		if( pGridCont )
		{
			CItemRecord* pItem = GetItemRecordInfo( pGridCont->sID );
			if( pItem && pItem->sType == enumItemTypeTrade )
			{
				sPosID = m_CKitbag.GetPosIDByNum( i );
				m_CKitbag.SetChangeFlag(false);
				//LG( "TradeCess", "��ɫ%s���ó��˰��LowCess = %d, sData = %d, CurData = %d.", GetName(), sLowCess, sData, m_CKitbag.GetEnergy( true, sPosID ) );
				LG( "TradeCess", "character %s add trade lowCess:LowCess = %d, sData = %d, CurData = %d.", GetName(), sLowCess, sData, m_CKitbag.GetEnergy( true, sPosID ) );
				if( pGridCont->sEnergy[1] + sData >= sLowCess )
				{
					m_CKitbag.SetEnergy(true, sLowCess, sPosID);
				}
				else
				{
					m_CKitbag.SetEnergy(true, pGridCont->sEnergy[1] + sData, sPosID);
				}
				m_CKitbag.SetSingleChangeFlag( sPosID );
				SynKitbagNew( enumSYN_KITBAG_FROM_NPC );
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CCharacter::GetTradeItemData( BYTE& byLevel, USHORT& sCess )
{
	USHORT sNum = m_CKitbag.GetUseGridNum();
	SItemGrid *pGridCont;
	for( int i = 0; i < sNum; i++ )
	{
		pGridCont = m_CKitbag.GetGridContByNum( i );
		if( pGridCont )
		{
			CItemRecord* pItem = GetItemRecordInfo( pGridCont->sID );
			if( pItem && pItem->sType == enumItemTypeTrade )
			{
				sCess  = (USHORT)pGridCont->sEnergy[1];
				byLevel = (BYTE)pGridCont->sEnergy[0];
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CCharacter::HasLeaveBagGrid( USHORT sNum )
{
	return sNum <= m_CKitbag.GetCapacity() - m_CKitbag.GetUseGridNum();
}

BOOL CCharacter::HasLeaveBagTempGrid( USHORT sNum )
{
	return sNum <= m_pCKitbagTmp->GetCapacity() - m_pCKitbagTmp->GetUseGridNum();
}

// ѧϰ���������ܲ�ͨ��
// sSkillID�����ܱ��.chLv���ȼ�.bSetLv�����õȼ���true�����õȼ� false�����ӵȼ���.bUsePoint���Ƿ����ļ��ܵ�
// �����Ƿ�ѧϰ�ɹ�
bool CCharacter::LearnSkill(Short sSkillID, Char chLv, bool bSetLv, bool bUsePoint, bool bLimit)
{T_B
	//m_CLog.Log("��ʼѧϰ���ܣ���� %d���ȼ� %d���Ƿ����õȼ� %d.\n", sSkillID, chLv, bSetLv);
	m_CLog.Log("start study skill:skillID %d��level %d��whether set level %d.\n", sSkillID, chLv, bSetLv);
	if (sSkillID > defMAX_SKILL_NO)
	{
		SystemNotice("���ܲ����ڣ�����������Χ.%d", sSkillID);
		//m_CLog.Log("ѧϰʧ�ܣ���������� %d\n", defMAX_SKILL_NO);
		m_CLog.Log("study failed:Max_skill_No %d\n", defMAX_SKILL_NO);
		return false;
	}

	CSkillRecord *pCSkill = GetSkillRecordInfo(sSkillID);
	if (!pCSkill)
	{
		//SystemNotice("���ܲ�����");
		SystemNotice(RES_STRING(GM_CHARACTER_CPP_00044));
		//m_CLog.Log("ѧϰʧ�ܣ��Ҳ����ü���\n");
		m_CLog.Log("Study failed: can't find the skill\n");
		return false;
	}
	if (chLv < 0)
	{
		//SystemNotice("ѧϰ�ļ��ܵȼ�[%d]����", chLv);
		SystemNotice(RES_STRING(GM_CHARACTER_CPP_00045), chLv);
		//m_CLog.Log("ѧϰʧ�ܣ��ȼ��Ƿ�\n");
		m_CLog.Log("Study failed:level unlawful\n");
		return false;
	}

	SSkillGrid	*pSkillGrid, SAddSkill;
	Char		chOldLv, chNewLv;
	bool		bIsNewSkill = false;
	pSkillGrid = m_CSkillBag.GetSkillContByID(sSkillID);
	if (!pSkillGrid) // �µļ���
	{
		bIsNewSkill = true;
		chOldLv = 0;
		chNewLv = chLv;
	}
	else
	{
		chOldLv = pSkillGrid->chLv;
		if (bSetLv) // ���õȼ�
		{
			chNewLv = chLv;
			if (chNewLv <= chOldLv)
			{
				//SystemNotice("�����Ѵ��ڣ��ҵȼ�����ѧϰֵ");
				SystemNotice(RES_STRING(GM_CHARACTER_CPP_00046));
				//m_CLog.Log("ѧϰʧ�ܣ��ȼ��Ƿ�����ǰ�ȼ� %d��Ҫ���õĵȼ� %d.\n", chOldLv, chNewLv);
				m_CLog.Log("Study failed:level unlawful,currently level %d,will set level: %d.\n", chOldLv, chNewLv);
				return false;
			}
		}
		else // ���ӵȼ�
		{
			chNewLv = chOldLv + chLv;
		}
		SAddSkill.chState = pSkillGrid->chState;
	}

	if (bLimit && !CanLearnSkill(pCSkill, chNewLv)) // ����ѧϰ�ü���
	{
		//m_CLog.Log("ѧϰʧ�ܣ�����ѧϰ.\n");
		m_CLog.Log("Study failed:can't study.\n");
		return false;
	}

	m_CSkillBag.SetChangeFlag(false);
	m_CChaAttr.ResetChangeFlag();
	SetBoatAttrChangeFlag(false);

	if (bUsePoint)
	{
		Long	lPExpend = pCSkill->chPointExpend * (chNewLv - chOldLv);
		if (pCSkill->chFightType == enumSKILL_LAND_LIVE || pCSkill->chFightType == enumSKILL_SEE_LIVE) // ����ܣ���������ܵ�.
		{
			Long	lCurLP = (long)m_CChaAttr.GetAttr(ATTR_LIFETP);
			if (lPExpend > lCurLP) // ���ܵ㲻��
			{
				//SystemNotice("����ܵ㲻�㣺��ǰ�� %d������� %d.", lCurLP, lPExpend);
				SystemNotice(RES_STRING(GM_CHARACTER_CPP_00047), lCurLP, lPExpend);
				//m_CLog.Log("ѧϰʧ�ܣ����ܵ㲻��.\n");
				m_CLog.Log("Study failed:Skillpoint not enough.\n");
				return false;
			}
			setAttr(ATTR_LIFETP, lCurLP - lPExpend);
		}
		else
		{
			Long	lCurTP = (long)m_CChaAttr.GetAttr(ATTR_TP);
			if (lPExpend > lCurTP) // ���ܵ㲻��
			{
				//SystemNotice("���ܵ㲻�㣺��ǰ�� %d������� %d.", lCurTP, lPExpend);
				SystemNotice(RES_STRING(GM_CHARACTER_CPP_00048), lCurTP, lPExpend);
				//m_CLog.Log("ѧϰʧ�ܣ����ܵ㲻��.\n");
				m_CLog.Log("Study failed:skillpoint not enough.\n");
				return false;
			}
			setAttr(ATTR_TP, lCurTP - lPExpend);
		}
	}

	Long	lLastSkillTick = 0;
	if (pSkillGrid)
		lLastSkillTick = pSkillGrid->lColdDownT;
	SAddSkill.chLv = chNewLv;
	SAddSkill.sID = sSkillID;
	bool	bAddResult = m_CSkillBag.Add(&SAddSkill);
	if (pSkillGrid)
		pSkillGrid->lColdDownT = lLastSkillTick;
	if (!bAddResult)
	{
		//SystemNotice("���뼼�ܰ�ʧ��");
		SystemNotice(RES_STRING(GM_CHARACTER_CPP_00049));
		//m_CLog.Log("ѧϰʧ�ܣ����뼼�ܰ�ʧ��.\n");
		m_CLog.Log("Study failed:add skilpoint failed.\n");
		return false;
	}

	ChangeItem(false, m_SChaPart.SLink + enumEQUIP_LHAND, enumEQUIP_LHAND);
	if (bIsNewSkill)
	{
		GetPlyCtrlCha()->SkillRefresh();
		GetPlyMainCha()->SynSkillBag(enumSYN_SKILLBAG_ADD);
	}
	else
	{
		if (SAddSkill.chState == enumSUSTATE_ACTIVE) // ����ļ��ܣ�����ֹͣ�ı�ȼ���ǰ�ļ��ܣ��ټ���ı�ȼ���ļ���
		{
			g_CParser.DoString(pCSkill->szInactive, enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, enumSCRIPT_PARAM_NUMBER, 1, chOldLv, DOSTRING_PARAM_END);
			g_CParser.DoString(pCSkill->szActive, enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, enumSCRIPT_PARAM_NUMBER, 1, chNewLv, DOSTRING_PARAM_END);
		}
		GetPlyMainCha()->SynSkillBag(enumSYN_SKILLBAG_MODI);
	}

	ChangeItem(true, m_SChaPart.SLink + enumEQUIP_LHAND, enumEQUIP_LHAND);

	g_CParser.DoString("AttrRecheck", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, DOSTRING_PARAM_END);
	if (GetPlayer())
	{
		GetPlayer()->RefreshBoatAttr();
		SyncBoatAttr(enumATTRSYN_REASSIGN);
	}
	SynAttrToSelf(enumATTRSYN_REASSIGN);

	//m_CLog.Log("ѧϰ�ɹ�.\n");
	m_CLog.Log("study succeed.\n");
	return true;
T_E}

// ְҵ���ȼ���ǰ�ü��ܣ��ȼ������ж�
bool CCharacter::CanLearnSkill(CSkillRecord *pCSkill, Char chToLv)
{
	bool	bJobOk = false;
	char	chJob = (char)m_CChaAttr.GetAttr(ATTR_JOB);
	for (int i = 0; i < defSKILL_JOB_SELECT_NUM; i++)
	{
		if (pCSkill->chJobSelect[i][0] == cchSkillRecordKeyValue)
			break;
		if (pCSkill->chJobSelect[i][0] == -1)
		{
			if (chToLv <= pCSkill->chJobSelect[i][1])
				bJobOk = true;
			break;
		}
		if (pCSkill->chJobSelect[i][0] == chJob)
		{
			if (chToLv <= pCSkill->chJobSelect[i][1])
				bJobOk = true;
			break;
		}
	}
	if (!bJobOk) // ְҵ�����ܵȼ�������
	{
		//SystemNotice("ְҵ��������ѧϰ�ȼ�������ְҵ������");
		SystemNotice(RES_STRING(GM_CHARACTER_CPP_00050));
		//m_CLog.Log("����ѧϰ��ְҵ�����ܵȼ�������\n");
		m_CLog.Log("can't study:profession,skill level is not accord\n");
		return false;
	}

	if (pCSkill->sLevelDemand > m_CChaAttr.GetAttr(ATTR_LV)) // �ȼ�����
	{
		//SystemNotice("��ɫ�ȼ���������ǰ�ȼ� %d������ȼ� %d.", m_CChaAttr.GetAttr(ATTR_LV), pCSkill->sLevelDemand);
		SystemNotice("RES_STRING(GM_CHARACTER_CPP_00051)", m_CChaAttr.GetAttr(ATTR_LV), pCSkill->sLevelDemand);
		//m_CLog.Log("����ѧϰ����ɫ�ȼ���������ǰ�ȼ� %d������ȼ� %d.\n", m_CChaAttr.GetAttr(ATTR_LV), pCSkill->sLevelDemand);
		m_CLog.Log("can't study:character level not enough%d��remand level %d.\n", m_CChaAttr.GetAttr(ATTR_LV), pCSkill->sLevelDemand);
		return false;
	}

	bool	bNeedSkill = true;
	SSkillGrid	*pSkillGrid;
	for (int i = 0; i < defSKILL_PRE_SKILL_NUM; i++)
	{
		if (pCSkill->sPremissSkill[i][0] == cchSkillRecordKeyValue)
			break;
		if (pCSkill->sPremissSkill[i][0] == -1)
			break;
		pSkillGrid = m_CSkillBag.GetSkillContByID(pCSkill->sPremissSkill[i][0]);
		if (!pSkillGrid || pSkillGrid->chLv < pCSkill->sPremissSkill[i][1])
		{
			bNeedSkill = false;
			break;
		}
	}
	if (!bNeedSkill) // ǰ�ü��ܲ�����
	{

		//SystemNotice("ǰ�ü��ܲ���");
		SystemNotice(RES_STRING(GM_CHARACTER_CPP_00052));
		//m_CLog.Log("����ѧϰ��ǰ�ü��ܲ����� .\n");
		m_CLog.Log("can't study:bNeedSkill is not accord.\n");
		return false;
	}

	return true;
}

// �޸ģ���������ۺϵ���װ���������
dbc::Short CCharacter::CanEquipItemNew(dbc::Short sItemID1, dbc::Short sItemID2 )
{
	CItemRecord* pItem1 = GetItemRecordInfo( sItemID1 );
	CItemRecord* pItem2 = ( sItemID2 > 0 ) ? GetItemRecordInfo( sItemID2 ) : NULL;

	if( !pItem1 ) return enumITEMOPT_ERROR_NONE;
	if( !pItem1->IsAllowEquip( m_pCChaRecord->lID ) ) {
		return enumITEMOPT_ERROR_BODY;
	}
	if( pItem2 && !pItem2->IsAllowEquip( m_pCChaRecord->lID ) )	{
		return enumITEMOPT_ERROR_BODY;
	}

	if( pItem2 )
	{
		if( pItem1->sNeedLv > pItem2->sNeedLv )
		{
			if( m_CChaAttr.GetAttr(ATTR_LV) < pItem1->sNeedLv )
			{
				return enumITEMOPT_ERROR_EQUIPLV;
			}
		}
		else
		{
			if( m_CChaAttr.GetAttr(ATTR_LV) < pItem2->sNeedLv )
			{
				return enumITEMOPT_ERROR_EQUIPLV;
			}
		}
	}
	else if( m_CChaAttr.GetAttr(ATTR_LV) < pItem1->sNeedLv )
	{
		return enumITEMOPT_ERROR_EQUIPLV;
	}

	char chJob = (char)m_CChaAttr.GetAttr( ATTR_JOB );
	for( char i = 0; i < MAX_JOB_TYPE; ++i )
	{
		if( pItem1->szWork[i] == cchItemRecordKeyValue ) 
		{
			break;
		}
		else if( pItem1->szWork[i] == char(-1) || pItem1->szWork[i] == chJob )
		{
			if( !pItem2 ) {
				return enumITEMOPT_SUCCESS;
			}

			for( char j = 0; j < MAX_JOB_TYPE; ++j )
			{
				if( pItem2->szWork[j] == char(-1) || pItem2->szWork[j] == chJob ) {
					return enumITEMOPT_SUCCESS;
				}
				else if( pItem2->szWork[j] == cchItemRecordKeyValue ) {
					break;
				}
			}
			break;
		}
	}

	return enumITEMOPT_ERROR_EQUIPJOB;
}

dbc::Short CCharacter::CanEquipItem(SItemGrid* pSEquipIt)
{
	if (!pSEquipIt)
		return enumITEMOPT_ERROR_NONE;

	CItemRecord	*pCItemRec = GetItemRecordInfo(pSEquipIt->sID);
	if (!pCItemRec->IsAllowEquip(m_pCChaRecord->lID))
		return enumITEMOPT_ERROR_BODY;

	if (m_CChaAttr.GetAttr(ATTR_LV) < pSEquipIt->sNeedLv)
		return enumITEMOPT_ERROR_EQUIPLV;

	for (char i = 0; i < MAX_JOB_TYPE; i++)
	{
		if (pCItemRec->szWork[i] == cchItemRecordKeyValue)
			break;
		if (pCItemRec->szWork[i] == -1)
			return enumITEMOPT_SUCCESS;
		if (m_CChaAttr.GetAttr(ATTR_JOB) == pCItemRec->szWork[i])
			return enumITEMOPT_SUCCESS;
	}

	return enumITEMOPT_ERROR_EQUIPJOB;
}

Short CCharacter::CanEquipItem(dbc::Short sItemID)
{
	CItemRecord	*pCItemRec = GetItemRecordInfo(sItemID);
	if (!pCItemRec)
	{
		return enumITEMOPT_ERROR_NONE;
	}
	if (!pCItemRec->IsAllowEquip(m_pCChaRecord->lID))
	{
		//ColourNotice(0xBC0000, "Unable to equip %s", pCItemRec->szName);
		return enumITEMOPT_ERROR_BODY;
	}

	if (m_CChaAttr.GetAttr(ATTR_LV) < pCItemRec->sNeedLv)
		return enumITEMOPT_ERROR_EQUIPLV;
	for (char i = 0; i < MAX_JOB_TYPE; i++)
	{
		if (pCItemRec->szWork[i] == cchItemRecordKeyValue)
			break;
		if (pCItemRec->szWork[i] == -1)
			return enumITEMOPT_SUCCESS;
		if (m_CChaAttr.GetAttr(ATTR_JOB) == pCItemRec->szWork[i])
			return enumITEMOPT_SUCCESS;
	}

	return enumITEMOPT_ERROR_EQUIPJOB;
}

// ���Ӽ���״̬
bool CCharacter::AddSkillState(uChar uchFightID, uLong ulSrcWorldID, Long lSrcHandle, Char chObjType, Char chObjHabitat, Char chEffType,
							   uChar uchStateID, uChar uchStateLv, Long lOnTick, dbc::Char chType, bool bNotice)
{T_B
	if (uchStateID > SKILL_STATE_MAXID || uchStateLv > SKILL_STATE_LEVEL)
		return false;

	CCharacter	*pCCha = 0;
	Entity	*pCEnt = g_pGameApp->IsValidEntity(ulSrcWorldID, lSrcHandle);
	if (!pCEnt)
		return false;
	pCCha = pCEnt->IsCharacter();

	if (bNotice)
	{
		GetPlyMainCha()->SetLookChangeFlag();
		m_CChaAttr.ResetChangeFlag();
		m_CSkillState.ResetChangeFlag();
		if (pCCha != g_pCSystemCha)
		{
			pCCha->GetPlyMainCha()->SetLookChangeFlag();
			pCCha->m_CChaAttr.ResetChangeFlag();
			pCCha->m_CSkillState.ResetChangeFlag();
		}
	}

	CSkillStateRecord	*pSSkillState = GetCSkillStateRecordInfo(uchStateID);
	if (!pSSkillState)
		return false;

	SSkillStateUnit	*pState = m_CSkillState.GetSStateByID(uchStateID);
	bool	bAlreadyHas = false;
	uChar	uchOldLv = 0;
	if (pState)
	{
		bAlreadyHas = true;
		uchOldLv = pState->GetStateLv();
	}
	Char chAddType = pSSkillState->chAddType;
	if (chType != enumSSTATE_ADD_UNDEFINED)
		chAddType = chType;
	if (!m_CSkillState.Add(uchFightID, ulSrcWorldID, lSrcHandle, chObjType, chObjHabitat, chEffType, uchStateID, uchStateLv, GetTickCount(), lOnTick, chAddType))
		return false;
	if (!bAlreadyHas)
	{
		if (!pSSkillState->bCanMove)
			SetActControl(enumACTCONTROL_MOVE, false);
		if (!pSSkillState->bCanGSkill)
			SetActControl(enumACTCONTROL_USE_GSKILL, false);
		if (!pSSkillState->bCanMSkill)
			SetActControl(enumACTCONTROL_USE_MSKILL, false);
		if (!pSSkillState->bCanTrade)
			SetActControl(enumACTCONTROL_TRADE, false);
		if (!pSSkillState->bCanItem)
			SetActControl(enumACTCONTROL_USE_ITEM, false);
		if (!pSSkillState->bCanUnbeatable)
			SetActControl(enumACTCONTROL_INVINCIBLE, false);
		if (!pSSkillState->bCanItemmed)
			SetActControl(enumACTCONTROL_BEUSE_ITEM, false);
		if (!pSSkillState->bCanSkilled)
			SetActControl(enumACTCONTROL_BEUSE_SKILL, false);
		if (!pSSkillState->bOptItem)
			SetActControl(enumACTCONTROL_ITEM_OPT, false);
		if (!pSSkillState->bTalkToNPC)
			SetActControl(enumACTCONTROL_TALKTO_NPC, false);
		if (!pSSkillState->bNoHide)
		{
			if (GetSubMap())
				GetSubMap()->RefreshEyeshot(this, GetActControl(enumACTCONTROL_EYESHOT), false, GetActControl(enumACTCONTROL_NOSHOW));
			SetActControl(enumACTCONTROL_NOHIDE, false);
		}
		if (!pSSkillState->bNoShow)
		{
			if (GetSubMap())
				GetSubMap()->RefreshEyeshot(this, GetActControl(enumACTCONTROL_EYESHOT), GetActControl(enumACTCONTROL_NOHIDE), false);
			SetActControl(enumACTCONTROL_NOSHOW, false);
		}
	}

	Long	lOldHP = (long)m_CChaAttr.GetAttr(ATTR_HP);
	bool	bDie = false;
	if (bAlreadyHas)
	{
		if (uchOldLv != uchStateLv)
		{
			g_CParser.DoString(pSSkillState->szSubState, enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, enumSCRIPT_PARAM_NUMBER, 1, uchOldLv, DOSTRING_PARAM_END);
			g_CParser.DoString(pSSkillState->szAddState, enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, enumSCRIPT_PARAM_NUMBER, 1, uchStateLv, DOSTRING_PARAM_END);
		}
		else
			return false;
	}
	else
		g_CParser.DoString(pSSkillState->szAddState, enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, enumSCRIPT_PARAM_NUMBER, 1, uchStateLv, DOSTRING_PARAM_END);
	BeUseSkill(lOldHP, (long)m_CChaAttr.GetAttr(ATTR_HP), pCCha, chEffType);

	if (lOldHP > 0 && m_CChaAttr.GetAttr(ATTR_HP) <= 0) // ����
	{
		SetDie(pCCha);
		bDie = true;
	}

	if (bNotice)
	{
		// check this [garner2]
		GetPlyMainCha()->SynLook(enumSYN_LOOK_CHANGE);
		SynSkillStateToEyeshot();
		SynAttr(enumATTRSYN_SKILL_STATE);
		if (pCCha != g_pCSystemCha)
		{
			// check this [garner2]
			pCCha->GetPlyMainCha()->SynLook(enumSYN_LOOK_CHANGE);
			pCCha->SynSkillStateToEyeshot();
			pCCha->SynAttr(enumATTRSYN_ATTACK);
		}
	}

	if (bDie) // ����
	{
		m_CLog.Log("!!!����\tTick %u\n", GetTickCount());
		Die();
		return true;
	}

	return true;
T_E}

// ɾ��״̬
bool CCharacter::DelSkillState(dbc::uChar uchStateID, bool bNotice)
{T_B
	if (bNotice)
	{
		m_CChaAttr.ResetChangeFlag();
		m_CSkillState.ResetChangeFlag();
	}

	CSkillStateRecord	*pSSkillState = GetCSkillStateRecordInfo(uchStateID);
	if (!pSSkillState)
		return false;
	SSkillStateUnit	*pState = m_CSkillState.GetSStateByID(uchStateID);
	if (!pState)
		return false;
	uChar	uchStateLv = pState->GetStateLv();
	bool	bDie = false;
	if (pState)
	{
		CCharacter	*pCCha = 0;
		Entity	*pCEnt = g_pGameApp->IsValidEntity(pState->ulSrcWorldID, pState->lSrcHandle);
		if (pCEnt)
			pCCha = pCEnt->IsCharacter();
		char	chEffType = pState->chEffType;

		if (!m_CSkillState.Del(uchStateID))
			return false;
		bool	bNoHide = GetActControl(enumACTCONTROL_NOHIDE);
		bool	bNoShow = GetActControl(enumACTCONTROL_NOSHOW);
		bool	bToNoHide = true, bToNoShow = true;
		for (int i = 0; i < enumACTCONTROL_MAX; i++)
			SetActControl(i);
		SetActControl(enumACTCONTROL_NOHIDE, bNoHide);
		SetActControl(enumACTCONTROL_NOSHOW, bNoShow);
		CSkillStateRecord	*pSTempSkillState;
		SSkillStateUnit		*pTempState;
		for (int i = 0; i < m_CSkillState.GetStateNum(); i++)
		{
			pTempState = m_CSkillState.GetSStateByNum(i);
			if (!pTempState)
				continue;
			pSTempSkillState = GetCSkillStateRecordInfo(pTempState->GetStateID());
			if (!pSTempSkillState)
				continue;
			if (!pSTempSkillState->bCanMove)
				SetActControl(enumACTCONTROL_MOVE, false);
			if (!pSTempSkillState->bCanGSkill)
				SetActControl(enumACTCONTROL_USE_GSKILL, false);
			if (!pSTempSkillState->bCanMSkill)
				SetActControl(enumACTCONTROL_USE_MSKILL, false);
			if (!pSTempSkillState->bCanTrade)
				SetActControl(enumACTCONTROL_TRADE, false);
			if (!pSTempSkillState->bCanItem)
				SetActControl(enumACTCONTROL_USE_ITEM, false);
			if (!pSTempSkillState->bCanUnbeatable)
				SetActControl(enumACTCONTROL_INVINCIBLE, false);
			if (!pSTempSkillState->bCanItemmed)
				SetActControl(enumACTCONTROL_BEUSE_ITEM, false);
			if (!pSTempSkillState->bCanSkilled)
				SetActControl(enumACTCONTROL_BEUSE_SKILL, false);
			if (!pSTempSkillState->bOptItem)
				SetActControl(enumACTCONTROL_ITEM_OPT, false);
			if (!pSTempSkillState->bTalkToNPC)
				SetActControl(enumACTCONTROL_TALKTO_NPC, false);
			if (!pSTempSkillState->bNoHide)
				bToNoHide = false;
			if (!pSTempSkillState->bNoShow)
				bToNoShow = false;
		}
		if (bToNoHide != bNoHide || bToNoShow != bNoShow)
			if (GetSubMap())
				GetSubMap()->RefreshEyeshot(this, true, bToNoHide, bToNoShow);
		SetActControl(enumACTCONTROL_NOHIDE, bToNoHide);
		SetActControl(enumACTCONTROL_NOSHOW, bToNoShow);

		Long	lOldHP = (long)m_CChaAttr.GetAttr(ATTR_HP);

		g_CParser.DoString(pSSkillState->szSubState, enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, enumSCRIPT_PARAM_NUMBER, 1, uchStateLv, DOSTRING_PARAM_END);

		BeUseSkill(lOldHP, (long)m_CChaAttr.GetAttr(ATTR_HP), pCCha, chEffType);
		if (lOldHP > 0 && m_CChaAttr.GetAttr(ATTR_HP) <= 0) // ����
		{
			SetDie(pCCha);
			bDie = true;
		}
	}

	if (bNotice)
	{
		SynSkillStateToEyeshot();
		SynAttr(enumATTRSYN_SKILL_STATE);
	}

	if (bDie) // ����
	{
		m_CLog.Log("!!!����\tTick %u\n", GetTickCount());
		Die();
		return true;
	}

	return true;
T_E}

void CCharacter::RestoreHp( BYTE byHpRate )
{T_B
	m_CChaAttr.ResetChangeFlag();
	DWORD dwCharHp = (long)this->getAttr( ATTR_HP );
	dwCharHp += byHpRate*(long)getAttr( ATTR_MXHP )/100;
	if( dwCharHp > (DWORD)getAttr( ATTR_MXHP ) )
	{
		dwCharHp = (long)getAttr( ATTR_MXHP );
	}
	DWORD dwHp = dwCharHp - (long)getAttr( ATTR_HP );
	setAttr( ATTR_HP, dwCharHp );
	SynAttr( enumATTRSYN_TASK );
	//SystemNotice( "�ָ�HPֵ(%d)�㣬��ǰHP(%d).", dwHp, dwCharHp );
	SystemNotice( RES_STRING(GM_CHARACTER_CPP_00053), dwHp, dwCharHp );
T_E}

void CCharacter::RestoreSp( BYTE bySpRate )
{T_B
	m_CChaAttr.ResetChangeFlag();
	DWORD dwCharSp = (long)this->getAttr( ATTR_SP );
	dwCharSp += bySpRate*(long)getAttr( ATTR_MXSP )/100;
	if( dwCharSp > (DWORD)getAttr( ATTR_MXSP ) )
	{
		dwCharSp = (long)getAttr( ATTR_MXSP );
	}
	DWORD dwSp = dwCharSp - (DWORD)getAttr( ATTR_SP );
	setAttr( ATTR_SP, dwCharSp );
	SynAttr( enumATTRSYN_TASK );
	//SystemNotice( "�ָ�SPֵ(%d)�㣬��ǰSP(%d).", dwSp, dwCharSp );
	SystemNotice( RES_STRING(GM_CHARACTER_CPP_00054), dwSp, dwCharSp );
T_E}

void CCharacter::RestoreAllHp()
{T_B
	m_CChaAttr.ResetChangeFlag();
	setAttr( ATTR_HP, (long)getAttr( ATTR_MXHP ) );
	SynAttr( enumATTRSYN_TASK );
	//SystemNotice( "�ָ�����HPֵ����ǰHP(%d).", getAttr( ATTR_HP ) );
	SystemNotice( RES_STRING(GM_CHARACTER_CPP_00055), getAttr( ATTR_HP ) );
T_E}

void CCharacter::RestoreAllSp()
{T_B
	m_CChaAttr.ResetChangeFlag();
	setAttr( ATTR_SP, (long)getAttr( ATTR_MXSP ) );
	SynAttr( enumATTRSYN_TASK ); 
	//SystemNotice( "�ָ�����SPֵ����ǰSP(%d).", getAttr( ATTR_SP ) );
	SystemNotice( RES_STRING(GM_CHARACTER_CPP_00056), getAttr( ATTR_SP ) );
T_E}

void CCharacter::RestoreAll()
{T_B
	m_CChaAttr.ResetChangeFlag();
	setAttr( ATTR_HP, (long)getAttr( ATTR_MXHP ) );
	setAttr( ATTR_SP, (long)getAttr( ATTR_MXSP ) );
	SynAttr( enumATTRSYN_TASK );
	//SystemNotice( "�ָ�����HPֵ����ǰHP(%d).", getAttr( ATTR_HP ) );
	SystemNotice( RES_STRING(GM_CHARACTER_CPP_00055), getAttr( ATTR_HP ) );
	//SystemNotice( "�ָ�����SPֵ����ǰSP(%d).", getAttr( ATTR_SP ) );
	SystemNotice(RES_STRING(GM_CHARACTER_CPP_00056), getAttr( ATTR_SP ) );
T_E}

long CCharacter::ExecuteEvent(Entity *pCObj, dbc::uShort usEventID)
{T_B
	long	lRet = 1;

	switch (pCObj->GetEvent().GetTouchType())
	{
	case	enumEVENTT_RANGE:
		{
			if (!IsRangePoint(pCObj->GetPos(), defRANGE_TOUCH_DIS))
				break;

			uShort	usEventEType = pCObj->GetEvent().GetExecType();
			void	*pTableRec = pCObj->GetEvent().GetTableRec();
			if (usEventEType == enumEVENTE_SMAP_ENTRY)
			{
				CSwitchMapRecord *pCSwitchMapRecord = (CSwitchMapRecord *)pTableRec;
				//m_CLog.Log("�ӵ�ǰ��ͼ[%s],�л���Ŀ���ͼ[%s]\n\n", m_submap->GetName(), pCSwitchMapRecord->szTarMapName);
				m_CLog.Log("from currently map[%s],switch to aim map[%s]\n\n", m_submap->GetName(), pCSwitchMapRecord->szTarMapName);

				SwitchMap(GetSubMap(), pCSwitchMapRecord->szTarMapName, pCSwitchMapRecord->STarPos.x, pCSwitchMapRecord->STarPos.y);
			}
			else if (usEventEType == enumEVENTE_DMAP_ENTRY)
			{
				CDynMapEntryCell	*pCEntry = (CDynMapEntryCell*)pTableRec;
				CMapEntryCopyCell	*pCCopyInfo = pCEntry->GetCopy(0);
				if (!pCCopyInfo)
				{
					//SystemNotice("����������");
					SystemNotice(RES_STRING(GM_CHARACTER_CPP_00057));
					break;
				}
				if (!pCCopyInfo->HasFreePlyCount(1)) // ��������
				{
					//SystemNotice("��������");
					SystemNotice(RES_STRING(GM_CHARACTER_CPP_00058));
					break;
				}
				string	strScript = "check_can_enter_";
				strScript += pCEntry->GetTMapName();
				if (g_CParser.StringIsFunction(strScript.c_str()))
				{
					if (g_CParser.DoString(strScript.c_str(), enumSCRIPT_RETURN_NUMBER, 1, enumSCRIPT_PARAM_LIGHTUSERDATA, 2, this, pCCopyInfo, DOSTRING_PARAM_END))
					{
						if (!g_CParser.GetReturnNumber(0))
							break;
					}
				}
				pCCopyInfo->AddCurPlyNum(1);

				string	strScript1 = "begin_enter_";
				strScript1 += pCEntry->GetTMapName();

				g_CParser.DoString(strScript1.c_str(), enumSCRIPT_RETURN_NUMBER, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 2, this, pCCopyInfo, DOSTRING_PARAM_END);
			}
		}
		break;
	default:
		break;
	}

	return lRet;
T_E}

void CCharacter::AfterObjDie(CCharacter *pCAtk, CCharacter *pCDead)
{T_B
	if (GetPlayer())
	{
		bool	bExecProc = true;
		if (pCAtk != this)
		{
			g_CParser.DoString("CheckExpShare", enumSCRIPT_RETURN_NUMBER, 1, enumSCRIPT_PARAM_LIGHTUSERDATA, 2, this, pCAtk, DOSTRING_PARAM_END);
			if (g_CParser.GetReturnNumber(0) == 0)
				bExecProc = false;
		}
		if (bExecProc)
			GetPlayer()->MisEventProc( mission::TE_KILL, (uShort)pCDead->GetCat(), pCDead->GetID() );
	}
T_E}

void CCharacter::AfterPeekItem(dbc::Short sItemID, dbc::Short sNum) 
{T_B
	if( GetPlayer() )
	{
		GetPlayer()->MisEventProc( mission::TE_GET_ITEM, sItemID, sNum );
	}
T_E}

void CCharacter::AfterEquipItem(dbc::Short sItemID, dbc::uShort sTriID)
{T_B
	if( GetPlayer() && sTriID != 0 )
	{
		GetPlayer()->MisEventProc( mission::TE_EQUIP_ITEM, sItemID, sTriID );
	}
T_E}

void CCharacter::EntryMapUnit( BYTE byMapID, WORD wxPos, WORD wyPos )
{T_B
	if( GetPlayer() )
	{
		GetPlayer()->MisEventProc( mission::TE_GOTO_MAP, byMapID, (wxPos<<16)|wyPos );
	}
T_E}

void CCharacter::OnMissionTime()
{T_B
	if( GetPlayer() )
	{
		GetPlayer()->MisEventProc( mission::TE_GAME_TIME, 0, 0 );
	}
	mission::CNpc* pNpc = this->IsNpc();
	if( pNpc )
	{
		pNpc->EventProc( mission::TE_GAME_TIME, 0, 0 );
	}
T_E}

void CCharacter::OnLevelUp( USHORT sLevel )
{T_B
	if( GetPlayer() )
	{
		
		//֪ͨGroupServer
		if(sLevel == 41)
		{
			CCharacter *pMainCha = GetPlyMainCha();
			WPacket l_wpk = GETWPACKET();
			WRITE_CMD(l_wpk,CMD_MP_MASTER_FINISH);
			WRITE_LONG(l_wpk,pMainCha->GetPlayer()->GetDBChaId());
			pMainCha->ReflectINFof(pMainCha,l_wpk);
		}
		
		GetPlayer()->MisEventProc( mission::TE_LEVEL_UP, sLevel, 0 );
	}
T_E}

void CCharacter::OnSailLvUp( USHORT sLevel )
{T_B
T_E}

void CCharacter::OnLifeLvUp( USHORT sLevel )
{T_B
T_E}

void CCharacter::OnCharBorn()
{T_B
	if( GetPlayer() )
	{
		GetPlayer()->MisEventProc( mission::TE_MAP_INIT, 0, 0 );
	}
T_E}

void CCharacter::Hide()
{T_B
	SSkillStateUnit	*pCState = m_CSkillState.GetSStateByID(SSTATE_HIDE);
	if (pCState)
		return;

	AddSkillState(0, g_pCSystemCha->GetID(), g_pCSystemCha->GetHandle(), enumSKILL_TYPE_SELF, enumSKILL_TAR_LORS, enumSKILL_EFF_HELPFUL, SSTATE_HIDE, 1, 10);
T_E}

void CCharacter::Show()
{T_B
	SSkillStateUnit	*pCState = m_CSkillState.GetSStateByID(SSTATE_HIDE);
	if (!pCState)
		return;

	DelSkillState(SSTATE_HIDE);
T_E}


bool IsFramestone(int id) {
	return (id == 1124 || id == 2530 || id == 2533 || id == 2536 || id == 2539 || id == 2542 || id == 2545);
}

bool IsClawstone(int id) {
	return (id == 1125 || id == 2531 || id == 2534 || id == 2537 || id == 2540 || id == 2543 || id == 2546);
}

bool IsPawstone(int id) {
	return (id == 1126 || id == 2532 || id == 2535 || id == 2538 || id == 2541 || id == 2544 || id == 2547);
}

bool IsCrownstone(int id) {
	return (id == 1127 || id == 2548);
}


bool RequiresApparel(int id){
	return IsFramestone(id) || IsClawstone(id) || IsPawstone(id) || IsCrownstone(id);
}

int GetEquipSlot(Char chLinkID){
	if (chLinkID >= enumEQUIP_HEADAPP && chLinkID <= enumEQUIP_SHOESAPP){
		return chLinkID - 19;
	}
	//int slot = chLinkID;
	//int id = pItemCont->sID;
	//CItemRecord* item = GetItemRecordInfo(id);
	//int itemType = item->sType;
	switch (chLinkID){
		
		case enumEQUIP_BOWAPP:
		case enumEQUIP_SHIELDAPP:
		case enumEQUIP_SWORD2APP:{
			return enumEQUIP_LHAND;
		}
		case enumEQUIP_GREATSWORDAPP:
		case enumEQUIP_STAFFAPP:
		case enumEQUIP_DAGGERAPP:
		case enumEQUIP_GUNAPP:
		case enumEQUIP_SWORD1APP:{
			return enumEQUIP_RHAND;
		}

		case enumEQUIP_FAIRYAPP:{
			return enumEQUIP_FAIRY;
		}

		default:{
			return chLinkID;
		}
		
	}
	
}

int GetApparelSlot(Char chLinkID, SItemGrid *pItemCont){
	if (chLinkID >= enumEQUIP_HEAD && chLinkID <= enumEQUIP_SHOES){
		return chLinkID + 19;
	}

	int slot = chLinkID;
	int id = pItemCont->sID;

	CItemRecord* item = GetItemRecordInfo(id);
	int itemType = item->sType;

	if (itemType == enumItemTypeSword && chLinkID == enumEQUIP_LHAND){
		slot = enumEQUIP_SWORD2APP;
	}
	else if (itemType == enumItemTypeSword && chLinkID == enumEQUIP_RHAND){
		slot = enumEQUIP_SWORD1APP;
	}
	else if (itemType == enumItemTypeGlave){
		slot = enumEQUIP_GREATSWORDAPP;
	}
	else if (itemType == enumItemTypeBow){
		slot = enumEQUIP_BOWAPP;
	}
	else if (itemType == enumItemTypeHarquebus){
		slot = enumEQUIP_GUNAPP;
	}
	else if (itemType == enumItemTypeStylet){
		slot = enumEQUIP_DAGGERAPP;
	}
	else if (itemType == enumItemTypeCosh){
		slot = enumEQUIP_STAFFAPP;
	}
	else if (itemType == enumItemTypeShield){
		slot = enumEQUIP_SHIELDAPP;
	}else if (chLinkID=enumEQUIP_FAIRY){
		slot = enumEQUIP_FAIRYAPP;
	}
	return slot;
}

//=============================================================================
// ������װ���仯�����Ľ�ɫ���Եı仯
// bEquip��0��ж��װ��.1��װ��.
// lItemID��װ�����
//=============================================================================
void CCharacter::ChangeItem(bool bEquip, SItemGrid *pItemCont, Char chLinkID)
{T_B

	//add by ALLEN 2007-10-16
	if (this->IsReadBook())
	return;
	
	if (!pItemCont->IsValid())
	return;


	CItemRecord	*pCItemRec = GetItemRecordInfo(pItemCont->sID);
	if (!pCItemRec) // ���߱���û�иõ���
		return;

	if (chLinkID >= enumEQUIP_HEADAPP && chLinkID <= enumEQUIP_SHIELDAPP){
		Char linkid = GetEquipSlot(chLinkID);// GetApparelSlot(chLinkID, pItemCont);
		short eqid = m_SChaPart.SLink[linkid].sID;
		if (RequiresApparel(eqid)){
			if (!bEquip && appCheck[linkid]){
				//remove stats if app removed.
				ChangeItem(false, &m_SChaPart.SLink[linkid], linkid);
			}else if (bEquip && !appCheck[linkid]){
				//add stats if app added.
				ChangeItem(true,&m_SChaPart.SLink[linkid], linkid);
			}
		}
		return;
	}

	if (chLinkID >= enumEQUIP_HEAD && chLinkID < enumEQUIP_HEADAPP){
		short id = pItemCont->sID;
		Char appSlot = GetApparelSlot(chLinkID, pItemCont);
		short eqid = m_SChaPart.SLink[appSlot].sID;
		if (RequiresApparel(id) && eqid == 0){
			if (!appCheck[chLinkID]){
				return;
			}
			bEquip = false;
			//no stats if no apparel.
			//appCheck[chLinkID] = false;
			//return;
		}
	}


	


	appCheck[chLinkID] = bEquip;
	char	chType = 1;
	if (!bEquip) // ж��װ��
		chType = -1;

	float	fBalance;
	if (chLinkID == enumEQUIP_LHAND)
		fBalance = 1 - pCItemRec->sLHandValu * (100 - m_CChaAttr.GetAttr(ATTR_LHAND_ITEMV)) / float(100);
	else
		fBalance = 1;

	long	lChaAttrType;
	float fLvEffect1 = 1.0;
	float fLvEffect2 = 0.0;
	float fLvEffect3 = 0.0;
	int nLv = 10;

	// modify by ning.yan  20080821  begin
	//if( pItemCont->sID >= CItemRecord::enumItemFusionStart && pItemCont->sID < CItemRecord::enumItemFusionEnd && pItemCont->GetFusionItemID() )
	CItemRecord * pItem = GetItemRecordInfo(pItemCont->sID);
	//if(CItemRecord::IsVaildFusionID(pItem) && pItemCont->GetFusionItemID() ) // ning.yan end
	//{

	//now items not checked if it is fused, just check lvel.
	//also changed from 2% to 1%
	if (pItemCont->GetItemLevel() > 0){
		fLvEffect1 = float(1);// float(80) / 100;
		fLvEffect2 = float(pItemCont->GetItemLevel())/100;
		fLvEffect3 = float(pItemCont->GetItemLevel() - nLv) / 100;//remove the starting at 80%
		for (int i = ITEMATTR_COE_STR; i <= ITEMATTR_COE_COL; i++)
		{
			lChaAttrType = g_ConvItemAttrTypeToCha(i);
			long lTemp = pItemCont->GetAttr(i);
			m_CChaAttr.AddAttr(lChaAttrType, Long(chType * lTemp * (lTemp > 0 ? fLvEffect1 + fLvEffect2 : float(1.0) - fLvEffect3) ));
		}
		for (int i = ITEMATTR_VAL_STR; i <= ITEMATTR_VAL_PDEF; i++)
		{
			lChaAttrType = g_ConvItemAttrTypeToCha(i);
			long lTemp = pItemCont->GetAttr(i);
			m_CChaAttr.AddAttr(lChaAttrType, Long(chType * lTemp * (lTemp > 0 ? fLvEffect1 + fLvEffect2 : float(1.0) - fLvEffect3) ));
		}
	}
	else
	{
		for (int i = ITEMATTR_COE_STR; i <= ITEMATTR_COE_COL; i++)
		{
			lChaAttrType = g_ConvItemAttrTypeToCha(i);
			long lTemp = pItemCont->GetAttr(i);
			m_CChaAttr.AddAttr(lChaAttrType, Long(chType * pItemCont->GetAttr(i)));
		}
		for (int i = ITEMATTR_VAL_STR; i <= ITEMATTR_VAL_PDEF; i++)
		{
			lChaAttrType = g_ConvItemAttrTypeToCha(i);
			m_CChaAttr.AddAttr(lChaAttrType, Long(chType * pItemCont->GetAttr(i)));
		}
	}

	m_CChaAttr.AddAttr(ATTR_ITEMV_MNATK, -1 * chType * pItemCont->GetAttr(ITEMATTR_VAL_MNATK));
	m_CChaAttr.AddAttr(ATTR_ITEMV_MXATK, -1 * chType * pItemCont->GetAttr(ITEMATTR_VAL_MXATK));
	m_CChaAttr.AddAttr(ATTR_ITEMV_MNATK, Long(chType * pItemCont->GetAttr(ITEMATTR_VAL_MNATK) * fBalance));
	m_CChaAttr.AddAttr(ATTR_ITEMV_MXATK, Long(chType * pItemCont->GetAttr(ITEMATTR_VAL_MXATK) * fBalance));
T_E}

void CCharacter::SkillRefresh()
{T_B
	CCharacter	*pCMainCha = GetPlyMainCha();
	CCharacter	*pCCtrlCha = GetPlyCtrlCha();
	CCharacter	*pCExecCha;

	bool		bIsBoat = pCCtrlCha->IsBoat();

	CSkillBag	*pCSkillBag = &pCMainCha->m_CSkillBag;
	stNetChangeChaPart	*pCLook = &pCMainCha->m_SChaPart;

	pCMainCha->m_sDefSkillNo = 0;
	SSkillGrid		*pSkillGrid;
	short sSkillNum = pCSkillBag->GetSkillNum();
	int nActive;
	CSkillRecord	*pCSkillRecord;
	for (short i = 0; i < sSkillNum; i++)
	{
		pSkillGrid = pCSkillBag->GetSkillContByNum(i);
		if (!pSkillGrid)
			continue;
		pCSkillRecord = GetSkillRecordInfo(pSkillGrid->sID);
		if (!pCSkillRecord)
			continue;
		if (pCSkillRecord->chFightType == enumSKILL_SEE_LIVE) // ���������
			nActive = g_IsUseSeaLiveSkill((long)getAttr(ATTR_BOAT_PART), pCSkillRecord);
		else
			nActive = g_IsUseSkill(pCLook, pCSkillRecord);

		if (pCSkillRecord->chType == enumSKILL_ACTIVE || pCSkillRecord->chType == enumSKILL_INBORN) // �������ܻ�Ĭ�ϼ��ܣ����ݽ�ɫ��̬�������Ƿ񼤻�
		{
			//if (IsPlayerCha()) // �����ɫ������м���ˢ��
			{
				if (bIsBoat && (pCSkillRecord->chSrcType == enumSKILL_SRC_HUMAN))
					nActive = 0;
				else if (!bIsBoat && (pCSkillRecord->chSrcType == enumSKILL_SRC_BOAT))
					nActive = 0;
			}
		}

		if (nActive == 1)
		{
			if (pCSkillRecord->chType == enumSKILL_INBORN)
				pCMainCha->m_sDefSkillNo = pSkillGrid->sID;
			if (pSkillGrid->chState != enumSUSTATE_ACTIVE)
			{
				if (strcmp(pCSkillRecord->szActive, "0"))
				{
					if (pCSkillRecord->chType == enumSKILL_PASSIVE) // ��������,�����ν�ɫ����
						pCExecCha = pCMainCha;
					else
						pCExecCha = pCCtrlCha;
					g_CParser.DoString(pCSkillRecord->szActive, enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, pCExecCha, enumSCRIPT_PARAM_NUMBER, 1, pSkillGrid->chLv, DOSTRING_PARAM_END);
				}
				pCSkillBag->SetState(pSkillGrid->sID, enumSUSTATE_ACTIVE);
			}
		}
		else if (nActive == 0)
		{
			if (pSkillGrid->chState != enumSUSTATE_INACTIVE)
			{
				if (strcmp(pCSkillRecord->szInactive, "0"))
				{
					if (pCSkillRecord->chType == enumSKILL_PASSIVE) // ��������,�����ν�ɫ����
						pCExecCha = pCMainCha;
					else
						pCExecCha = pCCtrlCha;
					g_CParser.DoString(pCSkillRecord->szInactive, enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, pCExecCha, enumSCRIPT_PARAM_NUMBER, 1, pSkillGrid->chLv, DOSTRING_PARAM_END);
				}
				pCSkillBag->SetState(pSkillGrid->sID, enumSUSTATE_INACTIVE);
			}
		}
	}

	if (bIsBoat) // ����ɫ
	{
		pSkillGrid = pCCtrlCha->m_CSkillBag.GetSkillContByNum(0);
		if (pSkillGrid)
			if (GetPlayer())
				pCMainCha->m_sDefSkillNo = pSkillGrid->sID;
	}
T_E}

// תְ
BOOL CCharacter::SetProfession( BYTE byPf )
{
	m_CChaAttr.ResetChangeFlag();
	setAttr(ATTR_JOB, byPf);
	SetBoatAttrChangeFlag(false);
	g_CParser.DoString("AttrRecheck", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, DOSTRING_PARAM_END);
	if (GetPlayer())
	{
		GetPlayer()->RefreshBoatAttr();
		SyncBoatAttr(enumATTRSYN_CHANGE_JOB);
	}
	SynAttrToSelf(enumATTRSYN_CHANGE_JOB);
	return TRUE;
}

// ͬ��������
void CCharacter::SynKitbagNew(Char chType)
{T_B
	if (!m_CKitbag.IsChange())
		return;

	WPACKET WtPk = GETWPACKET();
	WRITE_CMD(WtPk, CMD_MC_NOTIACTION);	// ͨ���ж�
	WRITE_LONG(WtPk, GetID());
	WRITE_LONG(WtPk, m_ulPacketID);
	WRITE_CHAR(WtPk, enumACTION_KITBAG);
	WriteKitbag(m_CKitbag, WtPk, chType);
	ReflectINFof(this, WtPk);

	SynAppendLook();
T_E}

//ͬ����ʱ����
void CCharacter::SynKitbagTmpNew(Char chType)
{T_B
	if (!m_pCKitbagTmp->IsChange())
		return;

	WPACKET WtPk = GETWPACKET();
	WRITE_CMD(WtPk, CMD_MC_NOTIACTION);	// ͨ���ж�
	WRITE_LONG(WtPk, GetID());
	WRITE_LONG(WtPk, m_ulPacketID);
	WRITE_CHAR(WtPk, enumACTION_KITBAGTMP);
	WriteKitbag(*m_pCKitbagTmp, WtPk, chType);
	ReflectINFof(this, WtPk);

	//SynAppendLook();
T_E}

// ͬ�������
void CCharacter::SynShortcut()
{T_B
	WPACKET WtPk = GETWPACKET();
	WRITE_CMD(WtPk, CMD_MC_NOTIACTION);	//ͨ���ж�
	WRITE_LONG(WtPk, GetID());
	WRITE_LONG(WtPk, m_ulPacketID);
	WRITE_CHAR(WtPk, enumACTION_SHORTCUT);
	WriteShortcut(WtPk);
	ReflectINFof(this, WtPk);
T_E}

// ͬ����ɫ���(��ɫ�����Ϣд��)
void CCharacter::SynLook(dbc::Char chSynType)
{T_B
	if (GetLookChangeNum() == 0)
		return;

	WPACKET WtPk=GETWPACKET();
	WRITE_CMD(WtPk, CMD_MC_NOTIACTION);	//ͨ���ж�
	WRITE_LONG(WtPk, GetID());
	WRITE_LONG(WtPk, m_ulPacketID);
	WRITE_CHAR(WtPk, enumACTION_LOOK);
	WriteLookData(WtPk, 0, chSynType);

	if (chSynType == enumSYN_LOOK_SWITCH)
		NotiChgToEyeshot(WtPk);//ͨ��
	else
		ReflectINFof(this, WtPk);
T_E}

// synching only to self [chaos argent]
void CCharacter::SynLook(dbc::Char chLookType, bool verbose)
{T_B
	if (GetLookChangeNum() == 0)
		return;

	WPACKET WtPk=GETWPACKET();
	WRITE_CMD(WtPk, CMD_MC_NOTIACTION);
	WRITE_LONG(WtPk, GetID());
	WRITE_LONG(WtPk, m_ulPacketID);
	WRITE_CHAR(WtPk, enumACTION_LOOK);
	WriteLookData(WtPk, chLookType);
	ReflectINFof(this, WtPk);

	if (verbose)
	{
		WPACKET WtPk=GETWPACKET();
		WRITE_CMD(WtPk, CMD_MC_NOTIACTION);
		WRITE_LONG(WtPk, GetID());
		WRITE_LONG(WtPk, m_ulPacketID);
		WRITE_CHAR(WtPk, enumACTION_LOOK);
		WriteLookData(WtPk, LOOK_OTHER);
		NotiChgToEyeshot(WtPk, false);
	}
T_E}

void CCharacter::ChaInitEquip(void)
{T_B
	CJobEquipRecord	*pCInitEquip = GetJobEquipRecordInfo((long)m_CChaAttr.GetAttr(ATTR_JOB));
	if (!pCInitEquip)
		return;

	SItemGrid	GridCont;
	for (short i = 0; i < defJOB_INIT_EQUIP_MAX; i++)
	{
		if (pCInitEquip->sItemID[i] > 0)
		{
			GridCont.sID = pCInitEquip->sItemID[i];
			GridCont.sNum = 1;
			GridCont.SetDBParam(-1, 0);
			ItemInstance(enumITEM_INST_BUY, &GridCont);
			KbPushItem(false, false, &GridCont, i);
		}
	}
T_E}

void CCharacter::ResetBirthInfo(void)
{
	SBirthPoint	*pSBirthP = GetRandBirthPoint(GetLogName(), GetBirthCity());
	SetBirthMap(pSBirthP->szMapName);
	SetPos(pSBirthP->x * 100, pSBirthP->y * 100);
}

void CCharacter::NewChaInit(void)
{T_B
	m_CChaAttr.Init(GetCat());
	m_CKitbag.Init(m_CKitbag.GetCapacity());
	ChaInitEquip();

	EnrichSkillBag();
T_E}

// �������߲�ͬ��
bool CCharacter::ItemForge(SItemGrid *pItem, dbc::Char chAddLv)
{T_B
	bool	bForge = false;
	// ���ݵ��߰���ֵ���ж��Ƿ����ɹ�
	bForge = true;

	if (bForge)
	{
		//pItem->sForgeAttr[0][0] = 0;
		//pItem->chForgeLv = chAddLv;
		//g_CParser.DoString("Creat_Item", enumSCRIPT_RETURN_NUMBER, nRetNum, enumSCRIPT_PARAM_NUMBER, 3, pCItemRec->sType, pCItemRec->sNeedLv, chType);
	}

	return bForge;
T_E}

//=============================================================================
// ͬ��������
// chType ͬ������.
// ��chType == enumSYN_SKILLBAG_MODIʱ��sModiSkillID��ʾ�޸ĵļ���ID(-1Ϊȫ���޸�).
// chTypeΪ��������ʱ��sModiSkillID������
//=============================================================================
void CCharacter::SynSkillBag(Char chType)
{T_B
	WPACKET pk	=GETWPACKET();
	WRITE_CMD(pk, CMD_MC_SYNSKILLBAG);
	WRITE_LONG(pk, GetID());
	WriteSkillbag(pk, chType);

	ReflectINFof(this, pk);
T_E}

void CCharacter::SynAddItemCha(CCharacter *pCItemCha)
{
	WPACKET pk	=GETWPACKET();
	WRITE_CMD(pk, CMD_MC_ADD_ITEM_CHA);
	WRITE_LONG(pk, GetPlayer()->GetMainCha()->GetID());
	WriteItemChaBoat(pk, pCItemCha);

	ReflectINFof(this, pk);
}

void CCharacter::SynDelItemCha(CCharacter *pCItemCha)
{
	WPACKET pk	=GETWPACKET();
	WRITE_CMD(pk, CMD_MC_DEL_ITEM_CHA);
	WRITE_LONG(pk, GetPlayer()->GetMainCha()->GetID());
	WRITE_LONG(pk, pCItemCha->GetID());

	ReflectINFof(this, pk);
}

void CCharacter::CheckPing(void)
{
	WPACKET WtPk	=GETWPACKET();
	WRITE_CMD(WtPk, CMD_MC_CHECK_PING);	//ͨ���ж�
	for (uLong i = 0; i < m_ulPingDataLen; i++)
		WRITE_CHAR(WtPk, rand()/255);
	ReflectINFof(this, WtPk);//ͨ��

	m_dwPingSendTick = GetTickCount();
}

void CCharacter::SendPreMoveTime(void)
{
	WPACKET WtPk	=GETWPACKET();
	WRITE_CMD(WtPk, CMD_MC_PREMOVE_TIME);	//ͨ���ж�
	if (m_lSetPing >= 0)
		WRITE_LONG(WtPk, m_lSetPing);
	else
		WRITE_LONG(WtPk, m_dwPing);
	ReflectINFof(this, WtPk);//ͨ��
}

void CCharacter::SynPKCtrl(void)
{
	WPACKET WtPk	=GETWPACKET();
	WRITE_CMD(WtPk, CMD_MC_NOTIACTION);	//ͨ���ж�
	WRITE_LONG(WtPk, m_ID);
	WRITE_LONG(WtPk, m_ulPacketID);
	WRITE_CHAR(WtPk, enumACTION_PK_CTRL);
	WritePKCtrl(WtPk);
	NotiChgToEyeshot(WtPk);//ͨ��
	//ReflectINFof(this, WtPk);//ͨ��
}

void CCharacter::SynSideInfo(void)
{
	WPACKET WtPk	=GETWPACKET();
	WRITE_CMD(WtPk, CMD_MC_SIDE_INFO);	//ͨ���ж�
	WRITE_LONG(WtPk, m_ID);
	WriteSideInfo(WtPk);
	NotiChgToEyeshot(WtPk);//ͨ��
}

void CCharacter::TerminalMessage(Long lMessageID)
{T_B
	WPACKET pk	=GETWPACKET();
	WRITE_CMD(pk, CMD_MC_MESSAGE);
	WRITE_LONG(pk, GetID());
	WRITE_LONG(pk, lMessageID);

	ReflectINFof(this, pk);
T_E}

void CCharacter::ItemOprateFailed(Short sFailedID)
{
	WPACKET WtPk	=GETWPACKET();
	WRITE_CMD(WtPk, CMD_MC_NOTIACTION);	//ͨ���ж�
	WRITE_LONG(WtPk, m_ID);
	WRITE_LONG(WtPk, m_ulPacketID);
	WRITE_CHAR(WtPk, enumACTION_ITEM_FAILED);
	WRITE_SHORT(WtPk, sFailedID);
	ReflectINFof(this, WtPk);//ͨ��
}

void CCharacter::AreaChange(void)
{
	//if ((m_usAreaAttr[0] & enumAREA_TYPE_NOT_PK) != (m_usAreaAttr[1] & enumAREA_TYPE_NOT_PK))
	//{
	//	Cmd_SetInGymkhana(m_usAreaAttr[1] & enumAREA_TYPE_NOT_PK);
	//	SynPKCtrl();
	//}
}

void CCharacter::SetEnterGymkhana(bool bEnter)
{
	CPlayer	*pPlayer = GetPlayer();
	if(!pPlayer)
		return;
	//if (bEnter)
	//	game_db.SavePlayerPos(pPlayer);

	Cmd_SetInGymkhana(bEnter);
	SynPKCtrl();
}

// ��ֻ�����ӿں���
// ���鴬ֻ�����͸��������Ƿ����Ҫ��,���������ݿ�
BOOL CCharacter::BoatCreate( const BOAT_DATA& Data )
{T_B	

	return FALSE;
T_E}

BOOL CCharacter::BoatUpdate( BYTE byIndex, const BOAT_DATA& Data )
{T_B
	return FALSE;
T_E}

// ��ɫ��ֻװ�ش���
BOOL CCharacter::BoatLoad( const BOAT_LOAD_INFO& Info )
{T_B
	return FALSE;
T_E}

// ��ֻ��������
void CCharacter::BoatDie( CCharacter& Attacker, CCharacter& Boat )
{
	GetPlayer()->SetLuanchOut( -1 );
	if( Boat.OnBoatDie( Attacker ) )
	{
		//BickerNotice( "��ֻ%s�������޷��޲��ѱ�����!", Boat.GetName() );
		BickerNotice( RES_STRING(GM_CHARACTER_CPP_00059), Boat.GetName() );
		
		// ���ٴ���֤��
		DWORD dwBoatID = (long)Boat.getAttr( ATTR_BOAT_DBID );
		USHORT sNumGird = m_CKitbag.GetUseGridNum();
		for( int i = 0; i < sNumGird; i++ )
		{
			SItemGrid *pGridCont = m_CKitbag.GetGridContByNum( i );
			if( pGridCont )
			{
				CItemRecord* pItem = GetItemRecordInfo( pGridCont->sID );
				if( pItem == NULL )
				{
					//SystemNotice( "��ƷID�����޷��ҵ�����Ʒ��Ϣ!ID = %d", pGridCont->sID );
					SystemNotice( RES_STRING(GM_CHARACTER_CPP_00005), pGridCont->sID );
					//LG( "boat_error", "��ƷID�����޷��ҵ�����Ʒ��Ϣ!ID = %d", pGridCont->sID );
					//LG( "boat_error", "��ƷID�����޷��ҵ�����Ʒ��Ϣ!ID = %d", pGridCont->sID );
					LG( "boat_error", "GridID error��can't find the gridID = %d", pGridCont->sID );
					continue;
				}
				if( pItem->sType == enumItemTypeBoat && dwBoatID == pGridCont->GetDBParam( enumITEMDBP_INST_ID ) )
				{
					short sPosID = m_CKitbag.GetPosIDByNum(i);
					if (sPosID < 0)
					{
						//SystemNotice( "��ƷID�����޷��ҵ�����Ʒ��Ϣ!ID = %d", pGridCont->sID );
						SystemNotice( RES_STRING(GM_CHARACTER_CPP_00005), pGridCont->sID );
						//LG( "boat_error", "��ƷID�����޷��ҵ�����Ʒ��Ϣ!ID = %d", pGridCont->sID );
						LG( "boat_error", "GridID error��can't find the gridID = %d", pGridCont->sID );
						continue;
					}
					if( KbClearItem(true, true, sPosID) != enumKBACT_SUCCESS )
					{
						// ���ٴ���֤��ʧ��
						//SystemNotice( "BoatDie:���ٴ���֤��ʧ��!ID[0x%X]", dwBoatID );
						SystemNotice( RES_STRING(GM_CHARACTER_CPP_00060), dwBoatID );
						//LG( "boat_error", "BoatDie:���ٴ���֤��ʧ��!ID[0x%X]", dwBoatID );
						LG( "boat_error", "BoatDie:destroy captain prove failed! ID[0x%X]", dwBoatID );
						break;
					}
				}
			}
		}
		
		// ���ٴ�ֻ
		if( !GetPlayer()->ClearBoat( dwBoatID ) )
		{
			char szData[128];
			//sprintf( szData, "BoatDie:���ٴ�ֻ%sʧ��!ID[%d]", Boat.GetName(), Boat.getAttr( ATTR_BOAT_DBID ) );
			sprintf( szData, RES_STRING(GM_CHARACTER_CPP_00061), Boat.GetName(), Boat.getAttr( ATTR_BOAT_DBID ) );
			SystemNotice( szData );
			LG( "boat_error", szData );
		}
		return;
	}

	//g_CParser.DoString( "Ship_ShipDieAttr", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, &Boat, DOSTRING_PARAM_END );
	//BickerNotice( "��ֻ%s�𻵳��뺣�ף����ҳ���NPC����!", Boat.GetName() );
	BickerNotice( RES_STRING(GM_CHARACTER_CPP_00062), Boat.GetName() );
}

BOOL CCharacter::OnBoatDie( CCharacter& Attacker )
{
	setAttr( ATTR_BOAT_ISDEAD, 1 );
	game_db.SaveBoatTempData( *this );

	return FALSE;
}

BOOL CCharacter::GetBoatID( BYTE byIndex, DWORD& dwBoatID )
{
	if( GetPlayer() )
	{
		USHORT sBerthID, sxPos, syPos, sDir;
		GetPlayer()->GetBerth( sBerthID, sxPos, syPos, sDir );

		BOAT_BERTH_DATA Data;
		memset( &Data, 0, sizeof(BOAT_BERTH_DATA) );
		BYTE byNumBoat;
		GetPlayer()->GetBerthBoat( sBerthID, byNumBoat, Data );
		if( byNumBoat == 0 )
		{
			//SystemNotice( "BoatSelected:��û��ͣ���ڸøۿڵ�����ֻ!" );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00063) );
			return TRUE;
		}

		if( byIndex >= byNumBoat )
		{
			//SystemNotice( "BoatSelected:ѡ������ֻID[%d]����!", byIndex );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00064), byIndex );
			return FALSE;
		}

		CCharacter* pBoat = GetPlayer()->GetBoat( Data.byID[byIndex] );
		if( !pBoat )
		{
			return FALSE;
		}
		dwBoatID = pBoat->GetID();
		return TRUE;
	}
	return FALSE;
}

// ��ֻͣ��
BOOL CCharacter::BoatBerth( USHORT sBerthID, USHORT sxPos, USHORT syPos, USHORT sDir )
{T_B
	CCharacter* pBoat = GetPlayer()->GetLuanchOut();
	if( !pBoat || pBoat != this ) {
		//SystemNotice( "�Ҳ�����ĳ�����ֻ!" );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00065) );
		return FALSE;
	}

	// ���ô�ֻλ���µĸۿ�
	this->setAttr( ATTR_BOAT_BERTH, sBerthID );

	if (!pBoat->SkillOutBoat(sxPos * 100, syPos * 100, sDir))
		return FALSE;

	g_CParser.DoString( "Ship_ExAttrCheck", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 2, pBoat->GetPlayer()->GetMainCha(), pBoat, DOSTRING_PARAM_END );

	//	2008-8-21	yangyinyu	add	begin!
	CCharacter*	c	=	this->GetBoat();
	this->GetPlayer()->GetMainCha()->SetBoat(	pBoat	);
	this->GetPlayer()->GetMainCha()->SetBoat(	c	);
	//	2008-8-21	yangyinyu	add	end!

	pBoat->SkillPushBoat(pBoat, false);

	// ����������
	m_pCPlayer->SetLuanchOut( -1 );

	return TRUE;
T_E}

// ��ֻ����
BOOL CCharacter::BoatEnterMap( CCharacter& Boat, DWORD dwxPos, DWORD dwyPos, USHORT sDir )
{T_B
	// ��ֻ�����ͼ
	if (!SkillPopBoat(&Boat, dwxPos, dwyPos, sDir))
	{
		//SystemNotice( "��ֻ�����ͼʧ��!" );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00066) );
		return FALSE;
	}
	SkillInBoat(&Boat);

	// ���ó�����ֻ
	DWORD dwBoatID = (DWORD)Boat.getAttr( ATTR_BOAT_DBID );
	m_pCPlayer->SetLuanchOut( dwBoatID );

	//	2008-8-21	yangyinyu	add	begin!
	CCharacter*	c	=	this->GetBoat();
	this->SetBoat(	&Boat	);
	this->SetBoat(	c	);
	//	2008-8-21	yangyinyu	add	end!

	return TRUE;
T_E}

// ��ֻ����
BOOL CCharacter::BoatLaunch( BYTE byIndex, USHORT sBerthID, USHORT sxPos, USHORT syPos, USHORT sDir )
{T_B
	// �Ѿ�����
	if( m_pCPlayer->IsLuanchOut() )
	{
		//SystemNotice( "��Ĵ�ֻ�Ѿ�������!" );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00067) );
		return FALSE;
	}

	CCharacter* pBoat = GetPlayer()->GetBoat( byIndex );
	if( !pBoat )
	{
		return FALSE;
	}

	if( pBoat->getAttr( ATTR_BOAT_ISDEAD ) != 0 )
	{
		//SystemNotice( "��ֻ%s�Ѿ���û�����ȴ���!", pBoat->GetName() );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00068), pBoat->GetName() );
		return TRUE;
	}

	// �ж��Ƿ������������
	if( g_CharBoat.BoatLimit( *GetPlayer()->GetMainCha(), (USHORT)pBoat->getAttr( ATTR_BOAT_SHIP ) ) )
	{
		return TRUE;
	}

	if( pBoat->getAttr( ATTR_HP ) <= 0 )
	{
		//SystemNotice( "��ֻ�����أ���Ҫ�������ܳ���!" );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00069) );
		return TRUE;
	}

	//if( pBoat->getAttr( ATTR_HP ) < pBoat->getAttr( ATTR_MXHP ) )
	//{
	//	SystemNotice( "��ֻ�𻵣���Ҫ����!" );
	//}

	//if( pBoat->getAttr( ATTR_SP ) < pBoat->getAttr( ATTR_MXSP ) )
	//{
	//	SystemNotice( "��ֻ��Ҫ����!" );
	//}

	if(g_CParser.DoString("RemoveYS", enumSCRIPT_RETURN_NUMBER, 1, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, DOSTRING_PARAM_END))
	{
		int ret = g_CParser.GetReturnNumber(0);
		if(ret != 1)
		{
			//LG("RemoveYS_error", "RemoveYSʧ��!\n");
			LG("RemoveYS_error", "RemoveYS failed\n");
		}
	}

	// ��ֻ�����ͼ
	if( !BoatEnterMap( *pBoat, sxPos * 100, syPos * 100, sDir ) )
	{
		//SystemNotice( "��ֻ�����ͼʧ��!" );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00070) );
		return FALSE;
	}

	return TRUE;
T_E}

BOOL CCharacter::BoatSelLuanch( BYTE byIndex )
{
	if( GetPlayer() )
	{
        if(m_CKitbag.IsLock())
        {
           // SystemNotice( "��������ʱ���������!" );
			 SystemNotice( RES_STRING(GM_CHARACTER_CPP_00071) );
            return FALSE;
        }
		// ���ý�ɫ����������Ϣ
		USHORT sBerthID, sxPos, syPos, sDir;
		GetPlayer()->GetBerth( sBerthID, sxPos, syPos, sDir );
		
		// ����ɫ�Ƿ���npc20�׷�Χ��
		//if( !IsDist( GetShape().centre.x, GetShape().centre.y, sxPos*100, syPos*100, 40 ) )
		//{
		//	SystemNotice( "�����ڳ�����λ�ò��ڸۿ�!" );
		//	return FALSE;
		//}

		BOAT_BERTH_DATA Data;
		memset( &Data, 0, sizeof(BOAT_BERTH_DATA) );
		BYTE byNumBoat;
		GetPlayer()->GetAllBerthBoat( sBerthID, byNumBoat, Data );
		if( byNumBoat == 0 )
		{
			//SystemNotice( "��û��ͣ���ڸøۿڵĴ�ֻ!" );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00072) );
			return TRUE;
		}

		if( byIndex >= byNumBoat )
		{
			//SystemNotice( "BoatSelLuance:ѡ�������ֻID[%d]����!", byIndex );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00073), byIndex );
			return FALSE;
		}

		return BoatLaunch( Data.byID[byIndex], sBerthID, sxPos, syPos, sDir );
	}
	return TRUE;
}

// ��ֻ�������ݽ���
BOOL CCharacter::BoatTrade( USHORT sBerthID )
{T_B
	// ���ý�ɫ����������Ϣ
	if( m_pCPlayer )
	{
		m_pCPlayer->SetBerth( sBerthID, 0, 0, 0 );
		return TRUE;
	}

	return FALSE;
T_E}

BOOL CCharacter::HasAllBoatInBerth( USHORT sBerthID )
{
	return ( GetPlayer() ) ? GetPlayer()->HasAllBoatInBerth( sBerthID ) : FALSE; 
}

BOOL CCharacter::HasBoatInBerth( USHORT sBerthID )
{ 
	return ( GetPlayer() ) ? GetPlayer()->HasBoatInBerth( sBerthID ) : FALSE; 
}

BOOL CCharacter::HasDeadBoatInBerth( USHORT sBerthID )
{
	return ( GetPlayer() ) ? GetPlayer()->HasDeadBoatInBerth( sBerthID ) : FALSE; 
}

BOOL CCharacter::IsNeedRepair()
{
	if( GetPlayer() )
	{
		CCharacter* pBoat = GetPlayer()->GetLuanchOut();
		if( pBoat == NULL )
		{
			return FALSE;
		}
		return pBoat->getAttr( ATTR_BOAT_DIECOUNT ) > 0;
	}
	return FALSE;
}

BOOL CCharacter::IsNeedSupply()
{
	if( GetPlayer() )
	{
		CCharacter* pBoat = GetPlayer()->GetLuanchOut();
		if( pBoat == NULL )
		{
			return FALSE;
		}
		return pBoat->getAttr( ATTR_MXSP ) > pBoat->getAttr( ATTR_SP );
	}
	return FALSE;
}

void CCharacter::RepairBoat()
{
	if( GetPlayer() )
	{
		CCharacter* pChar = GetPlayer()->GetMainCha();
		CCharacter* pBoat = GetPlayer()->GetLuanchOut();
		if( pBoat == NULL )
		{
			//SystemNotice( "��Ĵ�ֻ��û�г�������������ʧ��!" );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00074) );
			return;
		}
		DWORD dwMaxHp = (DWORD)pBoat->getAttr( ATTR_MXHP );		
		if( dwMaxHp - pBoat->getAttr( ATTR_HP ) == 0 || dwMaxHp <= (DWORD)pBoat->getAttr( ATTR_HP ) )
		{
			//SystemNotice( "��ֻ%s״���ܺã�����Ҫ����.", pBoat->GetName() );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00075), pBoat->GetName() );
			return;
		}

		DWORD dwReHp = dwMaxHp - (long)pBoat->getAttr( ATTR_HP );
		USHORT sLv = (USHORT)pChar->getAttr( ATTR_LV );
		if( sLv > 10 )
		{
			DWORD dwCharMoney = (long)pChar->getAttr( ATTR_GD );			
			DWORD dwMoney = DWORD(float(dwReHp)*0.05) + sLv * 20;
			//if( !pChar->TakeMoney( "ϵͳ", dwMoney ) )
			if( !pChar->TakeMoney( RES_STRING(GM_CHARACTER_CPP_00012), dwMoney ) )
			{
				//SystemNotice( "����ֻ%s��Ҫ��Ǯ(%d)G����Ľ�Ǯ(%d)����.", pBoat->GetName(), dwMoney, dwCharMoney );
				SystemNotice( RES_STRING(GM_CHARACTER_CPP_00076), pBoat->GetName(), dwMoney, dwCharMoney );
				return;
			}
		}
		
		pBoat->m_CChaAttr.ResetChangeFlag();
		pBoat->setAttr( ATTR_HP, dwMaxHp );
		pBoat->SyncBoatAttr( enumATTRSYN_TASK, FALSE );
		//SystemNotice( "��ֻ��%s��������ϣ��ָ��;�%d��!", pBoat->GetName(), dwReHp );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00077), pBoat->GetName(), dwReHp );
		g_CParser.DoString( "Ship_ExAttrCheck", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 2, pChar, pBoat, DOSTRING_PARAM_END );
	}
}

void CCharacter::SupplyBoat()
{
	if( GetPlayer() )
	{
		CCharacter* pChar = GetPlayer()->GetMainCha();
		CCharacter* pBoat = GetPlayer()->GetLuanchOut();
		if( pBoat == NULL )
		{
			//SystemNotice( "��Ĵ�ֻ��û�г��������ٲ���ʧ��!" );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00078) );
			return;
		}

		DWORD dwMaxSp = (DWORD)pBoat->getAttr( ATTR_MXSP );		
		if( dwMaxSp - pBoat->getAttr( ATTR_SP ) == 0  || dwMaxSp <= (DWORD)pBoat->getAttr( ATTR_SP ) )
		{
			//SystemNotice( "��ֻ%s��������!", pBoat->GetName() );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00079), pBoat->GetName() );
			return;
		}

		DWORD dwReSp = dwMaxSp - (long)pBoat->getAttr( ATTR_SP );
		USHORT sLv = (USHORT)pChar->getAttr( ATTR_LV );
		if( sLv > 10 )
		{
			DWORD dwCharMoney = (long)pChar->getAttr( ATTR_GD );
			DWORD dwMoney = dwReSp + sLv * 20;
			//if( !pChar->TakeMoney( "ϵͳ", dwMoney ) )
			if( !pChar->TakeMoney( RES_STRING(GM_CHARACTER_CPP_00012), dwMoney ) )
			{
				//SystemNotice( "������ֻ%s��Ҫ��Ǯ(%d)G����Ľ�Ǯ(%d)����.", pBoat->GetName(), dwMoney, dwCharMoney );
				SystemNotice( RES_STRING(GM_CHARACTER_CPP_00080), pBoat->GetName(), dwMoney, dwCharMoney );
				return;
			}
		}

		pBoat->m_CChaAttr.ResetChangeFlag();
		pBoat->setAttr( ATTR_SP, dwMaxSp );
		pBoat->SyncBoatAttr( enumATTRSYN_TASK, FALSE );
		//SystemNotice( "��ֻ��%s��������ϣ��������%d��!", pBoat->GetName(), dwReSp );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00081), pBoat->GetName(), dwReSp );

		// ���贬ֻ����
		g_CParser.DoString( "Ship_ExAttrCheck", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 2, GetPlayer()->GetMainCha(), pBoat, DOSTRING_PARAM_END );
	}
}

BOOL CCharacter::BoatSelected( BYTE byType, BYTE byIndex )
{
	if( !GetPlayer() ) {
		return FALSE;
	}

	// �ж��Ƿ��ڽ���״̬
	if( GetTradeData() )
	{
		//SystemNotice( "�����ں�������ɫ���ף������Ժ�npc�Ի�!" );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00082) );
		return FALSE;
	}

	// ���ý�ɫ����������Ϣ
	USHORT sBerthID, sxPos, syPos, sDir;
	GetPlayer()->GetBerth( sBerthID, sxPos, syPos, sDir );
	CCharacter* pChar = GetPlayer()->GetMainCha();

	if( byType == mission::BERTH_REPAIR_LIST )
	{
		BOAT_BERTH_DATA Data;
		memset( &Data, 0, sizeof(BOAT_BERTH_DATA) );
		BYTE byNumBoat;
		GetPlayer()->GetBerthBoat( sBerthID, byNumBoat, Data );
		if( byNumBoat == 0 )
		{
			//SystemNotice( "BoatSelected:��û��ͣ���ڸøۿڵ�����ֻ!" );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00083) );
			return TRUE;
		}
		
		if( byIndex >= byNumBoat )
		{
			//SystemNotice( "BoatSelected:ѡ������ֻID[%d]����!", byIndex );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00064), byIndex );
			return FALSE;
		}

		CCharacter* pBoat = GetPlayer()->GetBoat( Data.byID[byIndex] );
		if( !pBoat )
		{
			//SystemNotice( "BoatSelected:ѡ������ֻID[%d]ָ�����!", byIndex );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00084), byIndex );
			return FALSE;
		}

		DWORD dwMaxHp = (DWORD)pBoat->getAttr( ATTR_MXHP );		
		if( dwMaxHp - pBoat->getAttr( ATTR_HP ) == 0 || dwMaxHp <= (DWORD)pBoat->getAttr( ATTR_HP ) )
		{
			//SystemNotice( "��ֻ%s״���ܺã�����Ҫ����.", pBoat->GetName() );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00075), pBoat->GetName() );
			return TRUE;
		}

		DWORD dwReHp = dwMaxHp - (long)pBoat->getAttr( ATTR_HP );
		USHORT sLv = (USHORT)pChar->getAttr( ATTR_LV );
		if( sLv > 10 )
		{
			DWORD dwCharMoney = (long)pChar->getAttr( ATTR_GD );			
			DWORD dwMoney = DWORD(float(dwReHp)*0.05) + sLv * 20;
			//if( !pChar->TakeMoney( "ϵͳ", dwMoney ) )
			if( !pChar->TakeMoney( RES_STRING(GM_CHARACTER_CPP_00012), dwMoney ) )
			{
				//SystemNotice( "����ֻ%s��Ҫ��Ǯ(%d)G����Ľ�Ǯ(%d)����.", pBoat->GetName(), dwMoney, dwCharMoney );
				SystemNotice( RES_STRING(GM_CHARACTER_CPP_00076), pBoat->GetName(), dwMoney, dwCharMoney );
				return TRUE;
			}
		}
		
		pBoat->m_CChaAttr.ResetChangeFlag();
		pBoat->setAttr( ATTR_HP, dwMaxHp );
		pBoat->SyncBoatAttr( enumATTRSYN_TASK, FALSE );
		//SystemNotice( "��ֻ��%s��������ϣ��ָ��;�%d��!", pBoat->GetName(), dwReHp );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00077), pBoat->GetName(), dwReHp );
		g_CParser.DoString( "Ship_ExAttrCheck", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 2, pChar, pBoat, DOSTRING_PARAM_END );
	}
	else if( byType == mission::BERTH_SALVAGE_LIST )
	{
		BOAT_BERTH_DATA Data;
		memset( &Data, 0, sizeof(BOAT_BERTH_DATA) );
		BYTE byNumBoat;
		GetPlayer()->GetDeadBerthBoat( sBerthID, byNumBoat, Data );
		if( byNumBoat == 0 )
		{
			//SystemNotice( "BoatSelected:��û��ͣ���ڸøۿڵĳ�û��ֻ!" );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00085) );
			return TRUE;
		}

		if( byIndex >= byNumBoat )
		{
			//SystemNotice( "BoatSelected:ѡ����̳�û��ֻID[%d]����!", byIndex );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00086), byIndex );
			return FALSE;
		}

		CCharacter* pBoat = GetPlayer()->GetBoat( Data.byID[byIndex] );
		if( !pBoat )
		{
			//SystemNotice( "BoatSelected:ѡ����̳�û��ֻID[%d]ָ�����!", byIndex );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00087), byIndex );
			return FALSE;
		}
		
		DWORD dwCharMoney = (long)pChar->getAttr( ATTR_GD );
		DWORD dwMoney = 1000;
		//if( !pChar->TakeMoney( "ϵͳ", dwMoney ) )
		if( !pChar->TakeMoney( RES_STRING(GM_CHARACTER_CPP_00012), dwMoney ) )
		{
			//SystemNotice( "���̴�ֻ%s��Ҫ��Ǯ(%d)G����Ľ�Ǯ(%d)����.", pBoat->GetName(), dwMoney, dwCharMoney );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00088), pBoat->GetName(), dwMoney, dwCharMoney );
			return FALSE;
		}

		pBoat->setAttr( ATTR_BOAT_ISDEAD, 0 );
		if( !game_db.SaveBoatTempData( *pBoat ) )
		{
			//SystemNotice( "BoatSelected:���̴�ֻ��ȡ���ݿ����ʧ��!" );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00089) );
			//LG( "boat_error", "BoatSelected:���̴�ֻ��ȡ���ݿ����ʧ��!" );
			LG( "boat_error", "BoatSelected:salve boat deposit data operator failed!" );
		}
		else
		{
			//SystemNotice( "��ֻ��%s���ѱ��ɹ�����!", pBoat->GetName() );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00090), pBoat->GetName() );
		}
		g_CParser.DoString( "Ship_ExAttrCheck", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 2, pChar, pBoat, DOSTRING_PARAM_END );
	}
	else if( byType == mission::BERTH_SUPPLY_LIST )
	{
		BOAT_BERTH_DATA Data;
		memset( &Data, 0, sizeof(BOAT_BERTH_DATA) );
		BYTE byNumBoat;
		GetPlayer()->GetBerthBoat( sBerthID, byNumBoat, Data );
		if( byNumBoat == 0 )
		{
			//SystemNotice( "BoatSelected:��û��ͣ���ڸøۿڵĲ�����ֻ!" );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00091) );
			return TRUE;
		}

		if( byIndex >= byNumBoat )
		{
			//SystemNotice( "BoatSelected:ѡ�񲹸���ֻID[%d]����!", byIndex );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00092), byIndex );
			return FALSE;
		}

		CCharacter* pBoat = GetPlayer()->GetBoat( Data.byID[byIndex] );
		if( !pBoat )
		{
			//SystemNotice( "BoatSelected:ѡ�񲹸���ֻID[%d]ָ�����!", byIndex );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00093), byIndex );
			return FALSE;
		}
		
		DWORD dwMaxSp = (DWORD)pBoat->getAttr( ATTR_MXSP );		
		if( dwMaxSp - pBoat->getAttr( ATTR_SP ) == 0  || dwMaxSp <= (DWORD)pBoat->getAttr( ATTR_SP ))
		{
			//SystemNotice( "��ֻ%s��������!", pBoat->GetName() );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00079), pBoat->GetName() );
			return TRUE;
		}

		DWORD dwReSp = dwMaxSp - (long)pBoat->getAttr( ATTR_SP );
		USHORT sLv = (USHORT)pChar->getAttr( ATTR_LV );
		if( sLv > 10 )
		{
			DWORD dwCharMoney = (long)pChar->getAttr( ATTR_GD );
			DWORD dwMoney = dwReSp + sLv * 20;
			//if( !pChar->TakeMoney( "ϵͳ", dwMoney ) )
			if( !pChar->TakeMoney( RES_STRING(GM_CHARACTER_CPP_00012), dwMoney ) )
			{
				//SystemNotice( "������ֻ%s��Ҫ��Ǯ%dG����Ľ�Ǯ(%d)����.", pBoat->GetName(), dwMoney, dwCharMoney );
				SystemNotice( RES_STRING(GM_CHARACTER_CPP_00080), pBoat->GetName(), dwMoney, dwCharMoney );
				return TRUE;
			}
		}
		
		pBoat->m_CChaAttr.ResetChangeFlag();
		pBoat->setAttr( ATTR_SP, dwMaxSp );
		pBoat->SyncBoatAttr( enumATTRSYN_TASK, FALSE );
		//SystemNotice( "��ֻ��%s��������ϣ��������%d��!", pBoat->GetName(), dwReSp );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00081), pBoat->GetName(), dwReSp );
		g_CParser.DoString( "Ship_ExAttrCheck", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 2, pChar, pBoat, DOSTRING_PARAM_END );
	}
	else if( byType == mission::BERTH_BOATLEVEL_LIST )
	{
		BOAT_BERTH_DATA Data;
		memset( &Data, 0, sizeof(BOAT_BERTH_DATA) );
		BYTE byNumBoat;
		GetPlayer()->GetBerthBoat( sBerthID, byNumBoat, Data );
		if( byNumBoat == 0 )
		{
			//SystemNotice( "BoatSelected:��û��ͣ���ڸøۿڵĲ�����ֻ!" );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00091) );
			return TRUE;
		}

		if( byIndex >= byNumBoat )
		{
			//SystemNotice( "BoatSelected:ѡ�񲹸���ֻID[%d]����!", byIndex );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00092), byIndex );
			return FALSE;
		}

		CCharacter* pBoat = GetPlayer()->GetBoat( Data.byID[byIndex] );
		if( !pBoat )
		{
			//SystemNotice( "BoatSelected:ѡ�񲹸���ֻID[%d]ָ�����!", byIndex );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00093), byIndex );
			return FALSE;
		}

		// ��ֻ������������
		lua_getglobal( g_pLuaState, "BoatLevelUp" );
		if( !lua_isfunction( g_pLuaState, -1 ) )
		{
			lua_pop( g_pLuaState, 1 );
			LG( "lua_invalidfunc", "BoatLevelUp" );
			return FALSE;
		}

		lua_pushlightuserdata( g_pLuaState, (void*)this );
		lua_pushlightuserdata( g_pLuaState, (void*)pBoat );
		lua_pushnumber( g_pLuaState, pBoat->getAttr( ATTR_LV ) + 1 );
		int nStatus = lua_pcall( g_pLuaState, 3, 1, 0 );
		if( nStatus )
		{
			//SystemNotice( "��ɫ[%s]�Ľű���ֻ����������[BoatLevelUp]����ʧ��!", m_name );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00094), m_name );
			lua_settop(g_pLuaState, 0);
			return FALSE;
		}

		DWORD dwResult = (DWORD)lua_tonumber( g_pLuaState, -1 );
		lua_settop(g_pLuaState, 0);
		if( dwResult != LUA_TRUE )
		{
			//SystemNotice( "��ɫ[%s]�Ľű���ֻ����������[BoatLevelUp]����ʧ��!", m_name );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00095), m_name );
			return FALSE;
		}

		return TRUE;
	}
	else
	{
		//SystemNotice( "BoatSelected:��ֻѡ����������Type[%d]", byType );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00096), byType );
		return FALSE;
	}
	
	return TRUE;
}

BOOL CCharacter::BoatBerthList( DWORD dwNpcID, BYTE byType, USHORT sBerthID, USHORT sxPos, USHORT syPos, USHORT sDir )
{
	if( GetPlayer() )
	{
		BOAT_BERTH_DATA Data;
		memset( &Data, 0, sizeof(BOAT_BERTH_DATA) );
		BYTE byNumBoat;
		if( byType == mission::BERTH_SALVAGE_LIST )
		{
			GetPlayer()->GetDeadBerthBoat( sBerthID, byNumBoat, Data );
		}
		else if( byType == mission::BERTH_LUANCH_LIST )
		{
			GetPlayer()->GetAllBerthBoat( sBerthID, byNumBoat, Data );
		}
		else
		{
			GetPlayer()->GetBerthBoat( sBerthID, byNumBoat, Data );
		}
		if( byNumBoat == 0 )
		{
			//SystemNotice( "��û��ͣ���ڸøۿڵĴ�ֻ!" );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00072) );
			return TRUE;
		}

		// ���ý�ɫ����������Ϣ
		GetPlayer()->SetBerth( sBerthID, sxPos, syPos, sDir );

		WPACKET packet = GETWPACKET();
		WRITE_CMD( packet, CMD_MC_BERTH_LIST );
		WRITE_LONG( packet, dwNpcID );
		WRITE_CHAR( packet, byType );
		WRITE_CHAR( packet, byNumBoat );

		for( BYTE i = 0;i < byNumBoat; i++ )
		{
			WRITE_STRING( packet, Data.szName[i] );
		}

		ReflectINFof( this, packet );
		return TRUE;
	}

	return FALSE;
}

BOOL CCharacter::BoatAdd( CCharacter& Boat )
{
	WPACKET packet = GETWPACKET();
	WRITE_CMD( packet, CMD_MC_BOAT_ADD );
	WRITE_LONG( packet, Boat.GetID() );

	ReflectINFof( this, packet );
	return TRUE;
}

BOOL CCharacter::BoatClear( CCharacter& Boat )
{
	WPACKET packet = GETWPACKET();
	WRITE_CMD( packet, CMD_MC_BOAT_CLEAR );
	WRITE_LONG( packet, Boat.GetID() );

	ReflectINFof( this, packet );

	return TRUE;
}

BOOL CCharacter::BoatAdd( DWORD dwDBID )
{
	if( GetPlayer()->GetBoat( dwDBID ) )
		return FALSE;
	if( g_CharBoat.CreateBoat( *this, dwDBID, 2 ) )
	{
		CCharacter* pBoat = GetPlayer()->GetBoat( dwDBID );
		if( pBoat )
		{
			g_CParser.DoString("Ship_Tran", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 2, this, pBoat, DOSTRING_PARAM_END);
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CCharacter::BoatClear( DWORD dwDBID )
{
	if( GetPlayer() )
	{
		if( GetPlayer()->GetLuanchID() == dwDBID )
			return FALSE;
		return GetPlayer()->ClearBoat( dwDBID );
	}
	return FALSE;
}

BOOL CCharacter::BoatPackBagList( USHORT sBerthID, BYTE byType, BYTE byLevel )
{
	if( GetPlayer() )
	{
		BOAT_BERTH_DATA Data;
		memset( &Data, 0, sizeof(BOAT_BERTH_DATA) );
		BYTE byNumBoat;
		GetPlayer()->GetBerthBoat( sBerthID, byNumBoat, Data );
		if( byNumBoat == 0 )
		{
			//SystemNotice( "��û��ͣ���ڸøۿڵĴ�ֻ!" );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00072) );
			return TRUE;
		}

		// ���ý�ɫ����������Ϣ
		GetPlayer()->SetBerth( sBerthID, byType, byLevel, 0 );

		WPACKET packet = GETWPACKET();
		WRITE_CMD( packet, CMD_MC_BERTH_LIST );
		WRITE_LONG( packet, 0 );
		WRITE_CHAR( packet, mission::BERTH_BAG_LIST );
		WRITE_CHAR( packet, byNumBoat );

		for( BYTE i = 0;i < byNumBoat; i++ )
		{
			WRITE_STRING( packet, Data.szName[i] );
		}

		ReflectINFof( this, packet );
		return TRUE;
	}

	return FALSE;
}

BOOL CCharacter::PackBag( CCharacter& Boat, USHORT sItemID, USHORT sCount, USHORT sPileID, USHORT& sNumPack )
{
	USHORT sTemp = Boat.m_CKitbag.GetCapacity() - Boat.m_CKitbag.GetUseGridNum();
	if( sTemp == 0 )
	{
		sNumPack = 0;
		//SystemNotice( "��ֻ%s��������!", Boat.GetName() );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00097), Boat.GetName() );
		return TRUE;
	}

	struct GRID_DATA
	{
		BYTE byIndex;
		USHORT sNum;
	};
	GRID_DATA Data[100];
	memset( &Data, 0, sizeof(GRID_DATA)*100 );

	USHORT sGridID = 0;
	USHORT sNumItem = 0;
	USHORT sNumGird = m_CKitbag.GetUseGridNum();
	for( int i = 0; i < sNumGird; i++ )
	{
		SItemGrid *pGridCont = m_CKitbag.GetGridContByNum( i );
		if( pGridCont && pGridCont->sID == sItemID )
		{
			sNumItem += pGridCont->sNum;
			Data[sGridID].byIndex = (BYTE)m_CKitbag.GetPosIDByNum( i );
			Data[sGridID].sNum = pGridCont->sNum;
			if( ++sGridID >= 100 )
			{
				break;
			}
		}
	}

	m_CKitbag.SetChangeFlag( false );
	Boat.m_CKitbag.SetChangeFlag( false );

	USHORT sStartGrid = 0;
	sNumPack = sNumItem/sCount;	
	if( sNumPack > sTemp ) 
	{
		sNumPack = sTemp;
	}
	if( sNumPack == 0 )
	{
		//SystemNotice( "��Ʒ�����������!" );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00098) );
		return TRUE;
	}

	for( int i = 0; i < sNumPack && !Boat.m_CKitbag.IsFull(); i++ )
	{
		USHORT sNum = sCount;
		for( int n = sStartGrid; n < sGridID; n++ )
		{
			SItemGrid g;
			if( Data[n].sNum >= sNum )
			{
				// ���һ����
				g.sNum = sNum;
				if( KbPopItem( true, false, &g, Data[n].byIndex ) != enumKBACT_SUCCESS )
				{
					//SystemNotice( "���󣺴����ȡ��Ʒ��λID[%d]��%d����Ʒʧ��!" );
					//SystemNotice( "���󣺴����ȡ��Ʒʧ��!" );
					SystemNotice( RES_STRING(GM_CHARACTER_CPP_00099) );
					return FALSE;
				}
				//if( !Boat.AddItem( sPileID, 1, "ϵͳ" ) )
				if( !Boat.AddItem( sPileID, 1, RES_STRING(GM_CHARACTER_CPP_00012) ) )
				{
					//SystemNotice( "����ϵͳ����%d�����Ʒʧ��!ID[%d]", 1, sPileID );
					SystemNotice( RES_STRING(GM_CHARACTER_CPP_00100), 1, sPileID );
					return FALSE;
				}
				Data[n].sNum -= sNum;
				if( Data[n].sNum == 0 ) 
				{
					sStartGrid = n + 1;
				}
				else
				{
					sStartGrid = n;
				}
				break;
			}
			else
			{
				g.sNum = Data[n].sNum;
				if( KbPopItem( true, false, &g, Data[n].byIndex ) != enumKBACT_SUCCESS )
				{
					//SystemNotice( "���󣺴����ȡ��Ʒ��λID[%d]��%d����Ʒʧ��!" );
					//SystemNotice( "���󣺴����ȡ��Ʒʧ��!" );
					SystemNotice( RES_STRING(GM_CHARACTER_CPP_00099) );
					return FALSE;
				}
				sNum -= Data[n].sNum;
				Data[n].sNum = 0;
				sStartGrid = n;
			}
		}
	}

	SynKitbagNew( enumSYN_KITBAG_SYSTEM );
	Boat.SynKitbagNew( enumSYN_KITBAG_SYSTEM );
	return TRUE;
}

BOOL CCharacter::PackBag( CCharacter& boat, BYTE byType, BYTE byLevel )
{
	// ���ýű�������������
	lua_getglobal( g_pLuaState, "PackBagGoods" );
	if( !lua_isfunction( g_pLuaState, -1 ) )
	{
		lua_pop( g_pLuaState, 1 );
		LG( "lua_invalidfunc", "PackBagGoods" );
		return FALSE;
	}

	lua_pushlightuserdata( g_pLuaState, (void*)this );
	lua_pushlightuserdata( g_pLuaState, (void*)&boat );
	lua_pushnumber( g_pLuaState, byType );
	lua_pushnumber( g_pLuaState, byLevel );
	int nStatus = lua_pcall( g_pLuaState, 4, 1, 0 );
	if( nStatus )
	{
		//SystemNotice( "��ɫ[%s]�Ľű����������[PackBagGoods]����ʧ��!", m_name );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00101), m_name );
		lua_callalert( g_pLuaState, nStatus );
		lua_settop(g_pLuaState, 0);
		return FALSE;
	}

	DWORD dwResult = (DWORD)lua_tonumber( g_pLuaState, -1 );
	lua_settop(g_pLuaState, 0);
	if( dwResult != LUA_TRUE )
	{
		//SystemNotice( "��ɫ[%s]�Ľű����������[PackBagGoods]����ʧ��!", m_name );
		SystemNotice( RES_STRING(GM_CHARACTER_CPP_00102), m_name );
		return FALSE;
	}

	return TRUE;
}

BOOL CCharacter::BoatPackBag( BYTE byIndex )
{
	if( GetPlayer() )
	{
        if(GetPlyMainCha()->m_CKitbag.IsPwdLocked())
        {
            //GetPlyMainCha()->SystemNotice( "������������!" );
			GetPlyMainCha()->SystemNotice( RES_STRING(GM_CHARACTER_CPP_00002) );
			return FALSE;
        }
        
		// ���ý�ɫ����������Ϣ
		USHORT sBerthID, sType, sLevel, sDir;
		GetPlayer()->GetBerth( sBerthID, sType, sLevel, sDir );

		BOAT_BERTH_DATA Data;
		memset( &Data, 0, sizeof(BOAT_BERTH_DATA) );
		BYTE byNumBoat;
		GetPlayer()->GetBerthBoat( sBerthID, byNumBoat, Data );
		if( byNumBoat == 0 )
		{
			//SystemNotice( "��û��ͣ���ڸøۿڵĿ��Դ����Ʒ�Ĵ�ֻ!" );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00103) );
			return TRUE;
		}

		if( byIndex >= byNumBoat )
		{
			//SystemNotice( "BoatPackBag:ѡ�������ֻID[%d]����!", byIndex );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00104), byIndex );
			return FALSE;
		}

		// �������		
		if( m_pCPlayer->IsLuanchOut() )
		{
			//SystemNotice( "��Ĵ�ֻ�Ѿ�������!" );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00067) );
			return FALSE;
		}

		CCharacter* pBoat = GetPlayer()->GetBoat( Data.byID[byIndex] );
		if( !pBoat )
		{
			return FALSE;
		}

		if( pBoat->m_CKitbag.IsFull() )
		{
			//SystemNotice( "��ѡ��Ĵ�ֻ������������ѡ��������ֻ���!" );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00105) );
			return FALSE;
		}

		return PackBag( *pBoat, (BYTE)sType, (BYTE)sLevel );
	}
	return TRUE;
}

void CCharacter::SetGuildName(dbc::cChar *szGuildName) 
{
	if (GetPlayer())
	{
		strncpy(GetPlayer()->m_szGuildName, szGuildName, defGUILD_NAME_LEN - 1);
		GetPlayer()->m_szGuildName[defGUILD_NAME_LEN - 1] = '\0';
	}
}

dbc::cChar*	CCharacter::GetGuildName(void) 
{
	if (GetPlayer())
		return GetPlayer()->m_szGuildName;
	else
		return "";
}

dbc::cChar*	CCharacter::GetValidGuildName(void) 
{
	if (GetPlayer() && GetGuildState() == emGldMembStatNormal)
		return GetPlayer()->m_szGuildName;
	else
		return "";
}

void CCharacter::SetGuildMotto(dbc::cChar *szGuildMotto) 
{
	if (GetPlayer())
	{
		strncpy(GetPlayer()->m_szGuildMotto, szGuildMotto, defGUILD_MOTTO_LEN - 1);
		GetPlayer()->m_szGuildMotto[defGUILD_MOTTO_LEN - 1] = '\0';
	}
}

dbc::cChar*	CCharacter::GetGuildMotto(void) 
{
	if (GetPlayer())
		return GetPlayer()->m_szGuildMotto;
	else
		return "";
}

void CCharacter::SetStallName(dbc::cChar *szStallName) 
{
	if (GetPlayer())
	{
		strncpy(GetPlayer()->m_szStallName, szStallName, ROLE_MAXNUM_STALL_NUM - 1);
		GetPlayer()->m_szStallName[ROLE_MAXNUM_STALL_NUM -1] = '\0';
	}
}

dbc::cChar*	CCharacter::GetStallName(void) 
{
	if (GetPlayer())
		return GetPlayer()->m_szStallName;
	else
		return "";
}

dbc::cChar*	CCharacter::GetValidGuildMotto(void) 
{
	if (GetPlayer() && GetGuildState() == emGldMembStatNormal)
		return GetPlayer()->m_szGuildMotto;
	else
		return "";
}

void CCharacter::SetGuildID( DWORD dwGuildID )
{
	if (GetPlayer())
		GetPlayer()->m_lGuildID = dwGuildID;
}

DWORD CCharacter::GetGuildID()
{
	if (GetPlayer())
		return GetPlayer()->m_lGuildID;
	else
		return 0;
}

int CCharacter::GetValidGuildIcon(){
	if (GetPlayer())
	{
		if (GetGuildState() == emGldMembStatNormal && GetGuildID() > 0)
			return guildIcon;
		else
			return 0;
	}
	else
		return 0;
}
DWORD CCharacter::GetValidGuildCircleColour()
{
	if (GetPlayer())
	{
		if (GetGuildState() == emGldMembStatNormal && GetGuildID() > 0)
			return guildCircleColour;
		else
			return -1;
	}
	else
		return -1;
}

DWORD CCharacter::GetValidGuildID()
{
	if (GetPlayer())
	{
		if (GetGuildState() == emGldMembStatNormal)
			return GetPlayer()->m_lGuildID;
		else
			return 0;
	}
	else
		return 0;
}
void CCharacter::SetGuildType( BYTE byType )
{
	GetPlayer()->m_cGuildType = byType;
}

BYTE CCharacter::GetGuildType()
{
	if( GetPlayer() && GetGuildState() == emGldMembStatNormal )
	{
		return GetPlayer()->m_cGuildType;
	}
	return 0;
}

void CCharacter::SetGuildState( uLong lState )
{
	GetPlayer()->m_GuildStatus = lState;
}

uLong CCharacter::GetGuildState()
{
	return GetPlayer()->m_GuildStatus;
}

void CCharacter::SyncGuildInfo()
{
	WPACKET packet = GETWPACKET();
	WRITE_CMD( packet, CMD_MC_GUILD_INFO );
	WRITE_LONG( packet, this->GetID() );
	WRITE_LONG( packet, this->GetPlayer()->m_lGuildID );
	WRITE_STRING( packet, this->GetGuildName() );
	WRITE_STRING( packet, this->GetGuildMotto() );
	WRITE_LONG(packet, this->guildPermission);
	WRITE_LONG(packet, this->GetValidGuildCircleColour());
	WRITE_LONG(packet, this->GetValidGuildIcon());
	this->NotiChgToEyeshot( packet );
}

void CCharacter::SynStallName()
{
	WPACKET packet = GETWPACKET();
	WRITE_CMD( packet, CMD_MC_STALL_NAME );
	WRITE_LONG( packet, GetID() );
	WRITE_STRING( packet, GetStallName() );
	NotiChgToEyeshot( packet );
}

void CCharacter::SynBeginItemRepair()
{
	WPACKET packet = GETWPACKET();
	WRITE_CMD( packet, CMD_MC_BEGIN_ITEM_REPAIR );
	ReflectINFof(this, packet);
}

void CCharacter::SynBeginItemForge()
{
	WPACKET packet = GETWPACKET();
	WRITE_CMD( packet, CMD_MC_BEGIN_ITEM_FORGE );
	ReflectINFof(this, packet);
}

// Add by lark.li 20080514 begin
void CCharacter::SynBeginItemLottery()
{
	WPACKET packet = GETWPACKET();
	WRITE_CMD( packet, CMD_MC_BEGIN_ITEM_LOTTERY );
	ReflectINFof(this, packet);
}
// End

void CCharacter::SynBeginItemUnite()
{
	WPACKET packet = GETWPACKET();
	WRITE_CMD( packet, CMD_MC_BEGIN_ITEM_UNITE );
	ReflectINFof(this, packet);
}

void CCharacter::SynBeginItemMilling()
{
	WPACKET packet = GETWPACKET();
	WRITE_CMD( packet, CMD_MC_BEGIN_ITEM_MILLING );
	ReflectINFof(this, packet);
}

void CCharacter::SynBeginItemFusion()
{
	WPACKET packet = GETWPACKET();
	WRITE_CMD( packet, CMD_MC_BEGIN_ITEM_FUSION );
	ReflectINFof(this, packet);
}

void CCharacter::SynBeginItemUpgrade()
{
	WPACKET packet = GETWPACKET();
	WRITE_CMD( packet, CMD_MC_BEGIN_ITEM_UPGRADE );
	ReflectINFof(this, packet);
}

void CCharacter::SynBeginItemEidolonMetempsychosis()
{
	WPACKET packet = GETWPACKET();
	WRITE_CMD( packet, CMD_MC_BEGIN_ITEM_EIDOLON_METEMPSYCHOSIS );
	ReflectINFof(this, packet);
}

void CCharacter::SynBeginItemEidolonFusion()
{
	WPACKET packet = GETWPACKET();
	WRITE_CMD( packet, CMD_MC_BEGIN_ITEM_EIDOLON_FUSION );
	ReflectINFof(this, packet);
}

void CCharacter::SynBeginItemPurify()
{
	WPACKET packet = GETWPACKET();
	WRITE_CMD( packet, CMD_MC_BEGIN_ITEM_PURIFY );
	ReflectINFof(this, packet);
}

void CCharacter::SynBeginItemFix()
{
	WPACKET packet = GETWPACKET();
	WRITE_CMD( packet, CMD_MC_BEGIN_ITEM_FIX );
	ReflectINFof(this, packet);
} 

void CCharacter::SynBeginItemEnergy()
{
	WPACKET packet = GETWPACKET();
	WRITE_CMD( packet, CMD_MC_BEGIN_ITEM_ENERGY );
	ReflectINFof(this, packet);
}

void CCharacter::SynBeginGMSend()
{
	WPACKET packet = GETWPACKET();
	WRITE_CMD( packet, CMD_MC_BEGIN_GM_SEND );
	ReflectINFof(this, packet);
}

void CCharacter::SynBeginGMRecv(DWORD dwNpcID)
{
	WPACKET packet = GETWPACKET();
	WRITE_CMD( packet, CMD_MC_BEGIN_GM_RECV );
	WRITE_LONG(packet, dwNpcID);
	ReflectINFof(this, packet);
}

void CCharacter::SynBeginGetStone()
{
	WPACKET packet = GETWPACKET();
	WRITE_CMD( packet, CMD_MC_BEGIN_GET_STONE );
	ReflectINFof(this, packet);
}

void CCharacter::SynBeginTiger()
{
	WPACKET packet = GETWPACKET();
	WRITE_CMD( packet, CMD_MC_BEGIN_TIGER );
	ReflectINFof(this, packet);
}

void CCharacter::SynAppendLook()
{
	WPACKET packet = GETWPACKET();
	WRITE_CMD(packet, CMD_MC_APPEND_LOOK);
	WRITE_LONG(packet, GetID());
	if (WriteAppendLook(m_CKitbag, packet))
		NotiChgToEyeshot(packet);
}

void CCharacter::SynItemUseSuc(Short sItemID)
{
	WPACKET packet = GETWPACKET();
	WRITE_CMD(packet, CMD_MC_ITEM_USE_SUC);
	WRITE_LONG(packet, GetID());
	WRITE_SHORT(packet, sItemID);
	NotiChgToEyeshot(packet);
}

void CCharacter::SynKitbagCapacity(void)
{
	WPACKET packet = GETWPACKET();
	WRITE_CMD(packet, CMD_MC_KITBAG_CAPACITY);
	WRITE_LONG(packet, GetID());
	WRITE_SHORT(packet, m_CKitbag.GetCapacity());
	ReflectINFof(this, packet);
}

void CCharacter::SynEspeItem(void)
{
	Short	sEspeGridID = 1;
	SItemGrid *pGrid = m_CKitbag.GetGridContByID(sEspeGridID);
	if (pGrid)
	{
		CItemRecord* pItem = GetItemRecordInfo(pGrid->sID);
		if(pItem && pItem->sType == enumItemTypePet) // �������
			if (m_CKitbag.IsSingleChange(sEspeGridID))
			{
				WPACKET packet = GETWPACKET();
				WRITE_CMD(packet, CMD_MC_ESPE_ITEM);
				WRITE_LONG(packet, GetID());
				WRITE_CHAR(packet, 1);
				WRITE_SHORT(packet, sEspeGridID);
				WRITE_SHORT(packet, pGrid->sEndure[0]);
				WRITE_SHORT(packet, pGrid->sEnergy[0]);
				ReflectINFof(this, packet);
			}
	}
}

void CCharacter::SynVolunteerState(BOOL bState)
{
	if (!GetPlayer())
		return;
	char chState = (bState ? 1 : 0);
	WPACKET packet = GETWPACKET();
	WRITE_CMD(packet, CMD_MC_VOLUNTER_STATE);
	WRITE_CHAR(packet, chState);
	ReflectINFof(this, packet);
}

void CCharacter::SynTigerString(cChar *szString)
{
	if (!GetPlayer())
		return;
	WPACKET packet = GETWPACKET();
	WRITE_CMD(packet, CMD_MC_TIGER_STOP);
	WRITE_STRING(packet, szString);
	ReflectINFof(this, packet);
}

void CCharacter::SyncBoatAttr(Short sSynType, bool bAllBoat)
{
	if (!GetPlayer())
		return;

	if (!bAllBoat) // ֻͬ���Լ�
	{
		SynAttrToSelf(sSynType);
		return;
	}

	CCharacter*	pBoat;
	BYTE byNumBoat = GetPlayer()->GetNumBoat();
	for (BYTE i = 0; i < byNumBoat; i++)
	{
		pBoat = GetPlayer()->GetBoat( i );
		if( !pBoat ) continue;

		pBoat->SynAttrToSelf(sSynType);
	}
}

void CCharacter::SetBoatAttrChangeFlag(bool bSet)
{
	if (!GetPlayer())
		return;

	BYTE byNumBoat = GetPlayer()->GetNumBoat();

	for (BYTE i = 0; i < byNumBoat; i++)
	{
		CCharacter* pBoat = GetPlayer()->GetBoat( i );
		if( !pBoat ) continue;

		if (bSet)
			pBoat->m_CChaAttr.SetChangeFlag();
		else
			pBoat->m_CChaAttr.ResetChangeFlag();
	}
}

BOOL CCharacter::AddAttr( int nIndex, DWORD dwValue, dbc::Short sNotiType )
{
	m_CChaAttr.ResetChangeFlag();
	setAttr(nIndex, m_CChaAttr.GetAttr( nIndex ) + dwValue);
	SynAttr(sNotiType);
	return TRUE;
}

BOOL CCharacter::TakeAttr( int nIndex, DWORD dwValue, dbc::Short sNotiType )
{
	m_CChaAttr.ResetChangeFlag();
	DWORD dwTemp = ( (DWORD)m_CChaAttr.GetAttr( nIndex ) > dwValue ) ? (long)m_CChaAttr.GetAttr( nIndex ) - dwValue : 0;
	setAttr(nIndex, dwTemp);
	SynAttr(sNotiType);
	return TRUE;
}

void CCharacter::SetBoat( CCharacter* pBoat ) 
{ 
	GetPlayer()->SetMakingBoat( pBoat ); 
}

CCharacter* CCharacter::GetBoat() 
{ 
	return GetPlayer()->GetMakingBoat(); 
}

BOOL CCharacter::ViewItemInfo( RPACKET& pk )
{
	BYTE byType = READ_CHAR( pk );
	if( byType == mission::VIEW_CHAR_BAG )
	{
		Short	sGridID = READ_SHORT(pk);
		CItemRecord* pItem = (CItemRecord*)GetItemRecordInfo( m_CKitbag.GetID( sGridID ) );
		if( pItem == NULL )
		{
			//SystemNotice( "ViewItemInfo::��ƷID�����޷��ҵ�����Ʒ��Ϣ!ID = %d, grid = %d", m_CKitbag.GetID( sGridID ), sGridID );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00106), m_CKitbag.GetID( sGridID ), sGridID );
			return FALSE;
		}

		if( pItem->sType == enumItemTypeBoat )
		{
			return g_CharBoat.GetBoatInfo( *this, m_CKitbag.GetDBParam( enumITEMDBP_INST_ID, sGridID ) );
		}
	}
	else 
	{
		BYTE byIndex = (BYTE)READ_SHORT( pk );
		USHORT sItemID;
		DWORD dwBoatID;
		CCharacter* pOwner = NULL;
		if( byType == mission::VIEW_CHARTRADE_SELF )
		{

			if( this->m_pTradeData )
			{
				if( m_pTradeData->pAccept == this )
				{
					if( m_pTradeData->AcpTradeData.ItemArray[byIndex].sItemID == 0 || 
						!m_CKitbag.HasItem( m_pTradeData->AcpTradeData.ItemArray[byIndex].byIndex ) )
					{
						return FALSE;
					}

					sItemID = m_CKitbag.GetID( m_pTradeData->AcpTradeData.ItemArray[byIndex].byIndex );
					dwBoatID = m_CKitbag.GetDBParam( enumITEMDBP_INST_ID, m_pTradeData->AcpTradeData.ItemArray[byIndex].byIndex );
					pOwner = this;
				}
				else if( m_pTradeData->pRequest == this )
				{
					if( m_pTradeData->ReqTradeData.ItemArray[byIndex].sItemID == 0 || 
						!m_CKitbag.HasItem( m_pTradeData->ReqTradeData.ItemArray[byIndex].byIndex ) )
					{
						return FALSE;
					}

					sItemID = m_CKitbag.GetID( m_pTradeData->ReqTradeData.ItemArray[byIndex].byIndex );
					dwBoatID = m_CKitbag.GetDBParam( enumITEMDBP_INST_ID, m_pTradeData->ReqTradeData.ItemArray[byIndex].byIndex );
					pOwner = this;
				}
				else
				{
					return FALSE;
				}
			}
		}
		else if( byType == mission::VIEW_CHARTRADE_OTHER )
		{
			if( this->m_pTradeData )
			{
				if( m_pTradeData->pAccept == this )
				{
					if( m_pTradeData->ReqTradeData.ItemArray[byIndex].sItemID == 0 || 
						!m_pTradeData->pRequest->m_CKitbag.HasItem( m_pTradeData->ReqTradeData.ItemArray[byIndex].byIndex ) )
					{
						return FALSE;
					}

					sItemID = m_pTradeData->pRequest->m_CKitbag.GetID( m_pTradeData->ReqTradeData.ItemArray[byIndex].byIndex );
					dwBoatID = m_pTradeData->pRequest->m_CKitbag.GetDBParam( enumITEMDBP_INST_ID, m_pTradeData->ReqTradeData.ItemArray[byIndex].byIndex );
					pOwner = m_pTradeData->pRequest;
				}
				else if( m_pTradeData->pRequest == this )
				{
					if( m_pTradeData->AcpTradeData.ItemArray[byIndex].sItemID == 0 || 
						!m_pTradeData->pAccept->m_CKitbag.HasItem( m_pTradeData->AcpTradeData.ItemArray[byIndex].byIndex ) )
					{
						return FALSE;
					}

					sItemID = m_pTradeData->pAccept->m_CKitbag.GetID( m_pTradeData->AcpTradeData.ItemArray[byIndex].byIndex );
					dwBoatID = m_pTradeData->pAccept->m_CKitbag.GetDBParam( enumITEMDBP_INST_ID, m_pTradeData->AcpTradeData.ItemArray[byIndex].byIndex );
					pOwner = m_pTradeData->pAccept;
				}
				else
				{
				}
			}
		}
		else
		{
			return FALSE;
		}

		CItemRecord* pItem = (CItemRecord*)GetItemRecordInfo( sItemID );
		if( pItem == NULL )
		{
			//SystemNotice( "ViewItemInfo:��ƷID�����޷��ҵ�����Ʒ��Ϣ!Index = %d, ID = %d", byIndex, sItemID );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00107), byIndex, sItemID );
			return FALSE;
		}

		if( pItem->sType == enumItemTypeBoat )
		{
			return g_CharBoat.GetTradeBoatInfo( *this, *pOwner, dwBoatID );
		}
		else
		{
			//SystemNotice( "viewiteminfo:����Ʒû�д�ֻ��Ϣ!Index[%d], ID[%d]", byIndex, sItemID );
			SystemNotice( RES_STRING(GM_CHARACTER_CPP_00108), byIndex, sItemID );
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CCharacter::HasGuild()
{
	return GetGuildID() > 0 && GetGuildState() == emGldMembStatNormal;
}

BOOL CCharacter::IsGuildType( BYTE byType )
{	
	return ( HasGuild() ) ? GetGuildType() == byType : FALSE;
}

void CCharacter::SetStallData( mission::CStallData* pData )
{
	if( GetPlayer() ) GetPlayer()->SetStallData( pData );
}

mission::CStallData* CCharacter::GetStallData()
{
	return GetPlayer() ? GetPlayer()->GetStallData() : NULL;
}

BYTE CCharacter::GetStallNum()
{
	if (!GetActControl(enumACTCONTROL_USE_MSKILL)) // ����ʹ�ð�̯����
		return 0;

	Char	chLv;

	SSkillGrid	*pSSkillCont = m_CSkillBag.GetSkillContByID(241);
	if (!pSSkillCont)
		chLv = 0;
	else
		chLv = pSSkillCont->chLv;

	return chLv * 6;
}

//add by jilinlee 2007/4/20
//����
BOOL CCharacter::IsReadBook()
{
	return m_SReadBook.bIsReadState;
}
void CCharacter::SetReadBookState(bool bIsReadBook)
{
	m_SReadBook.bIsReadState = bIsReadBook;
	m_SReadBook.dwLastReadCallTick = 0;
}

extern char	g_kitbag[];
extern char g_kitbagTmp[];
void CCharacter::LogAssets(Char chLType)
{
	return;
	//char	*szLTypeStr[] = {"��ʼ��", "����", "����", "ʰȡ", "����", "ɾ��"};
	const char	*szLTypeStr[] = {
		RES_STRING(GM_CHARACTER_CPP_00109), 
		RES_STRING(GM_CHARACTER_CPP_00110),
		RES_STRING(GM_CHARACTER_CPP_00111), 
		RES_STRING(GM_CHARACTER_CPP_00112),
		RES_STRING(GM_CHARACTER_CPP_00113),
		RES_STRING(GM_CHARACTER_CPP_00114)};

	short	sItemNum = m_CKitbag.GetUseGridNum();
    short   sItemTmpNum = m_pCKitbagTmp->GetUseGridNum();
	g_kitbag[0] = '\0';
    g_kitbagTmp[0] = '\0';
	sprintf(g_kitbag, "%d@", sItemNum);
    sprintf(g_kitbagTmp, "%d@", sItemTmpNum);
	SItemGrid *pGridCont;
	CItemRecord *pCItem;
	for (short i = 0; i < sItemNum; i++)
	{
		pGridCont = m_CKitbag.GetGridContByNum(i);
		if (!pGridCont)
			continue;
		pCItem = GetItemRecordInfo(pGridCont->sID);
		if (!pCItem)
			continue;
		sprintf(g_kitbag + strlen(g_kitbag), "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
	}
    for (short i = 0; i < sItemTmpNum; i++)
	{
		pGridCont = m_pCKitbagTmp->GetGridContByNum(i);
		if (!pGridCont)
			continue;
		pCItem = GetItemRecordInfo(pGridCont->sID);
		if (!pCItem)
			continue;
		sprintf(g_kitbagTmp + strlen(g_kitbagTmp), "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
	}
	//LG("����ʲ�", "��ɫ%s(%s)��%s��������Ǯ %u������%s, ��ʱ����%s.\n", GetLogName(), GetPlyMainCha()->GetLogName(), szLTypeStr[chLType], GetPlyMainCha()->getAttr(ATTR_GD), g_kitbag, g_kitbagTmp);
	LG("character assets", "player %s(%s)��%s operator;coin %u,kitbag %s,Tempkitbag %s.\n", GetLogName(), GetPlyMainCha()->GetLogName(), szLTypeStr[chLType], GetPlyMainCha()->getAttr(ATTR_GD), g_kitbag, g_kitbagTmp);
}

bool CCharacter::SaveAssets(void)
{
	return game_db.SaveChaAssets(this);
}

int CCharacter::GetLotteryIssue()
{
	int issue;

	if(game_db.GetLotteryIssue(issue))
		return issue;

	return 0;
}

bool CCharacter::IsRangePoint(dbc::Long lPosX, dbc::Long lPosY, dbc::Long lDist)
{
	Point	CurPos = GetPlyCtrlCha()->GetPos();
	__int64	llErr = 100 * 100;

	__int64	llDistX = lPosX - CurPos.x;
	__int64 llDistY = lPosY - CurPos.y;
	__int64 llDistXY2 = llDistX * llDistX + llDistY * llDistY;
	if (llDistXY2 > (lDist * lDist + llErr))
		return false;

	return true;
}

bool CCharacter::IsRangePoint2(dbc::Long lPosX, dbc::Long lPosY, dbc::Long lDist2)
{
	Point	CurPos = GetPlyCtrlCha()->GetPos();
	__int64	llErr = 0;

	__int64	llDistX = lPosX - CurPos.x;
	__int64 llDistY = lPosY - CurPos.y;
	__int64 llDistXY2 = llDistX * llDistX + llDistY * llDistY;
	if (llDistXY2 > (lDist2 + llErr))
		return false;

	return true;
}

void CCharacter::AddMasterCredit(long lCredit)
{
	unsigned long lMasterID = GetMasterDBID();

	if(lMasterID == 0)
	{
		return;
	}

	CPlayer *pMasterPly = g_pGameApp->GetPlayerByDBID(lMasterID);
	CCharacter *pMaster = NULL;
	if(pMasterPly)
	{
		pMaster = pMasterPly->GetMainCha();
	}

	if(!pMaster)
	{
		//game_db.AddCreditByDBID(lMasterID, lCredit);
		WPACKET WtPk	=GETWPACKET();
		WRITE_CMD(WtPk, CMD_MM_ADDCREDIT);
		WRITE_LONG(WtPk, GetID());
		WRITE_LONG(WtPk, lMasterID);
		WRITE_LONG(WtPk, lCredit);
		ReflectINFof(this, WtPk);//ͨ��
		return;
	}

	pMaster->SetCredit((long)pMaster->GetCredit() + lCredit);
	pMaster->SynAttr(enumATTRSYN_TASK);

	return;
}

unsigned long CCharacter::GetMasterDBID()
{
	return game_db.GetPlayerMasterDBID(GetPlayer());
}

void CCharacter::InitCheatX()
{
	m_sCheatX.dwInterval =  GetCheatInterval(0);
	m_sCheatX.dwLastTime = GetTickCount();
	m_sCheatX.Xerror = 0;
	m_sCheatX.Xnum.clear();
	m_sCheatX.Xtype = 1;
	m_sCheatX.Xright = 0;
	m_sCheatX.Xcount = 0;
	m_sCheatX.Xn = 2;
}

DWORD CCharacter::GetCheatInterval(int state)
{
	#define RAND_IN_NUM(x) (rand() % ((x) + 1))
	const int MS_IN_ONE_MINUTE = 60 * 1000;
	const int MS_IN_ONE_SECOND = 1000;

	DWORD ret = 0;
	
	switch(state)
	{
	case 0://������
		ret = 20 * MS_IN_ONE_SECOND + 100 * RAND_IN_NUM(MS_IN_ONE_SECOND);
		break;
	case 1://�ش�������ʱ
		ret = 65 * MS_IN_ONE_SECOND;
		break;
	case 3://���ʼ��
		ret = (m_sCheatX.Xn > 3) ? (40 * MS_IN_ONE_MINUTE) : (60 * RAND_IN_NUM(MS_IN_ONE_SECOND) + 10 * m_sCheatX.Xn * MS_IN_ONE_MINUTE);
		break;
	default:
		ret = 20 * MS_IN_ONE_SECOND + 100 * RAND_IN_NUM(MS_IN_ONE_SECOND);
		break;
	}
	return ret;
}

void CCharacter::CheatRun(DWORD dwCurTime)
{
	if(dwCurTime - m_sCheatX.dwLastTime < m_sCheatX.dwInterval)
	{
		return;
	}

	switch(m_sCheatX.Xtype)
	{
	case 1://������ɺ�
		{
			if(GetStallData() || IsStoreEnable())
			{
				if(m_sCheatX.Xcount > 0)
				{
					m_sCheatX.dwInterval = GetCheatInterval(3);
				}
				else
				{
					m_sCheatX.dwInterval = GetCheatInterval(0);
				}
			}
			else
			{
				m_sCheatX.Xtype = 2;
				char buf[5];
				buf[0] = g_pGameApp->m_PicSet->RandGetID();
				buf[1] = g_pGameApp->m_PicSet->RandGetID();
				buf[2] = g_pGameApp->m_PicSet->RandGetID();
				buf[3] = g_pGameApp->m_PicSet->RandGetID();
				buf[4] = '\0';
				m_sCheatX.Xnum = buf;

				WPACKET WtPk = GETWPACKET();
				WRITE_CMD(WtPk, CMD_MC_CHEAT_CHECK);
				WRITE_SHORT(WtPk, 4);
				for(int i = 0; i < 4; i++)
				{
					CPicture *pPic = g_pGameApp->m_PicSet->GetPicture(buf[i]);
					uInt nSize = pPic->GetSize();

					WRITE_SHORT(WtPk, nSize);
					for(int j = 0; (uInt)j < nSize; j++)
					{
						WRITE_CHAR(WtPk, pPic->GetImgByte(j));
					}
				}
				ReflectINFof(this, WtPk);

				m_sCheatX.dwInterval = GetCheatInterval(1);
				m_sCheatX.Xcount++;
			}

			m_sCheatX.dwLastTime = dwCurTime;
		}
		break;

	case 2://���ʺ�
		{
			m_sCheatX.Xn = (m_sCheatX.Xn > 0) ? (m_sCheatX.Xn - 1) : 0;
			m_sCheatX.dwInterval = GetCheatInterval(3);
			m_sCheatX.dwLastTime = dwCurTime;
			m_sCheatX.Xerror++;
			m_sCheatX.Xright = 0;
			m_sCheatX.Xtype = 1;
			m_sCheatX.Xnum.clear();

			if(m_sCheatX.Xerror >= 3)
			{
				CheatConfirm();
			}
			else
			{
				//SystemNotice("��û�д�,�㻹��%d�λ���!", 3 - m_sCheatX.Xerror);
				SystemNotice(RES_STRING(GM_CHARACTER_CPP_00115), 3 - m_sCheatX.Xerror);
			}
		}
		break;

	default:
		{
			InitCheatX();
		}
		break;
	}
}

void CCharacter::CheatCheck(cChar *answer)
{
	if(m_sCheatX.Xtype != 2)
		return;

	if(!m_sCheatX.Xnum.empty() && !lstrcmpi(answer, m_sCheatX.Xnum.c_str()))
	{
		m_sCheatX.dwLastTime = GetTickCount();
		m_sCheatX.Xerror = 0;
		m_sCheatX.Xright++;
		m_sCheatX.Xnum.clear();
		m_sCheatX.Xtype = 1;
		m_sCheatX.Xn++;
		m_sCheatX.dwInterval = GetCheatInterval(3);

		if(m_sCheatX.Xcount > 1)
		{
			//����
			g_CParser.DoString("WGPrizeBegin", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, enumSCRIPT_PARAM_NUMBER, 1, m_sCheatX.Xright, DOSTRING_PARAM_END);
		}
	}
	else
	{
		m_sCheatX.dwLastTime = GetTickCount();
		m_sCheatX.Xerror++;
		m_sCheatX.Xright = 0;
		m_sCheatX.Xnum.clear();
		m_sCheatX.Xtype = 1;
		m_sCheatX.Xn = (m_sCheatX.Xn > 0) ? (m_sCheatX.Xn - 1) : 0;
		m_sCheatX.dwInterval = GetCheatInterval(3);

		if(m_sCheatX.Xerror >= 3)
		{
			CheatConfirm();
		}
		else
		{
			//SystemNotice("������,�㻹��%d�λ���!", 3 - m_sCheatX.Xerror);
			SystemNotice(RES_STRING(GM_CHARACTER_CPP_00116), 3 - m_sCheatX.Xerror);
		}
	}
}

void CCharacter::CheatConfirm()
{
	if(IsStoreEnable())
	{
		m_sCheatX.dwLastTime = GetTickCount();
		m_sCheatX.Xright = 0;
		m_sCheatX.Xtype = 1;
		m_sCheatX.Xn = 2;
		m_sCheatX.dwInterval = GetCheatInterval(0);
	}
	else
	{
		//LG("Cheat", "��� %s ʹ�����,��������!\n", GetName());
		LG("Cheat", "character %s use waigua,kick it!\n", GetName());

		GatePlayer *pGatePlyer = (GatePlayer *)GetPlayer();
		g_gmsvr->KickPlayer2(pGatePlyer);
		g_pGameApp->GoOutGame(GetPlayer(), true);
	}
}

//=============================================================================
// ����״̬����ת��Ϊ�ַ���
char* SStateData2String(CCharacter *pCCha, char *szSStateBuf, int nLen)
{
	if (!pCCha || !szSStateBuf) return NULL;

	CSkillState *pSState = &pCCha->m_CSkillState;

	char	szData[512];
	int		nBufLen = 0, nDataLen;
	szSStateBuf[0] = '\0';

	sprintf(szData, "%d", pSState->GetStateNum());
	nDataLen = (int)strlen(szData);
	if (nBufLen + nDataLen >= nLen) return NULL;
	strcat(szSStateBuf, szData);
	nBufLen += nDataLen;

	SSkillStateUnit *pStateUnit;
	long	lOnTick, lOverTick;
	for (unsigned char i = 0; i < pSState->GetStateNum(); i++)
	{
		pStateUnit = pSState->GetSStateByNum(i);
		if (!pStateUnit)
			continue;

		lOnTick = pStateUnit->lOnTick;
		if (lOnTick <= 0)
			continue;

		lOverTick = (pStateUnit->ulLastTick - pStateUnit->ulStartTick) / 1000;
		if (lOnTick > lOverTick)
			lOnTick -= lOverTick;
		else // ʱ���Ѿ�����
			continue;

		sprintf(szData, ";%d,%d,%d",
			pStateUnit->GetStateID(), pStateUnit->GetStateLv(), lOnTick);
		nDataLen = (int)strlen(szData);
		if (nBufLen + nDataLen >= nLen) return NULL;
		strcat(szSStateBuf, szData);
		nBufLen += nDataLen;
	}

	return szSStateBuf;
}

char* PStateData2String(CCharacter *pCCha, char *szSStateBuf, int nLen)
{
	if (!pCCha || !szSStateBuf)
		return NULL;

	CSkillState *pSState = &pCCha->m_CSkillState;

	SSkillStateUnit *pStateUnit;
	long	lOnTick, lOverTick;

	char	szTmp[512];
	char	szStateBuf[512];
	int		nTmpBufLen = 0, nTmpLen;

	int		nCnt = 0;
	int		nPersCount = sizeof(g_Config.m_cSaveState) / sizeof(g_Config.m_cSaveState[0]);
	for (unsigned char i = 0; i < pSState->GetStateNum(); i++)
	{
		pStateUnit = pSState->GetSStateByNum(i);
		if (!pStateUnit)
			continue;

		if (pStateUnit->GetStateID() == 83)
			goto label_es;

		lOnTick = pStateUnit->lOnTick;
		if (lOnTick <= 0)
			continue;

		lOverTick = (pStateUnit->ulLastTick - pStateUnit->ulStartTick) / 1000;
		if (lOnTick > lOverTick)
			lOnTick -= lOverTick;
		else
			continue;

		label_es:
		for (int i = 0; i < nPersCount; i++)
		{
			if (g_Config.m_cSaveState[i] == 0)
				break;

			if (g_Config.m_cSaveState[i] == pStateUnit->GetStateID())
			{
				time_t seconds;
				seconds = time(NULL);

				long int t = static_cast<long int>(seconds);
				t += lOnTick;

				sprintf(szTmp, ";%d,%d,%d", pStateUnit->GetStateID(), pStateUnit->GetStateLv(), t);
				nTmpLen = (int)strlen(szTmp);
				if (nTmpBufLen + nTmpLen >= nLen) return NULL;
				strcat(szStateBuf, szTmp);
				nTmpBufLen += nTmpLen;

				nCnt++;
				break;
			}
		}
	}

	char	szData[512];
	int		nBufLen = 0, nDataLen;
	szSStateBuf[0] = '\0';

	sprintf(szData, "%d", nCnt);
	nDataLen = (int)strlen(szData);
	if (nBufLen + nDataLen >= nLen)
		return NULL;

	strcat(szSStateBuf, szData);
	strcat(szSStateBuf, szStateBuf);
	return szSStateBuf;
}

// �ַ���ת��Ϊ����״̬����
bool Strin2SStateData(CCharacter *pCCha, std::string &strData)
{
	if (!pCCha)
		return false;

	std::string strList[SKILL_STATE_MAXID + 1];
	const short csSubNum = 3;
	std::string strSubList[csSubNum];
	int nSegNum = Util_ResolveTextLine(strData.c_str(), strList, SKILL_STATE_MAXID + 1, ';');
	if (nSegNum < 1)
		return false;

	Util_ResolveTextLine(strList[0].c_str(), strSubList, 3, ','); // ״̬����
	uChar	uchStateNum = Str2Int(strSubList[0]);
	uChar	uchStateID, uchStateLv;
	Long	lOnTick;

	short	sTCount;

	int nPersCount = sizeof(g_Config.m_cSaveState) / sizeof(g_Config.m_cSaveState[0]);
	time_t seconds;

	for (uChar i = 0; i < uchStateNum; i++)
	{
		sTCount = 0;
		Util_ResolveTextLine(strList[i + 1].c_str(), strSubList, csSubNum, ',');
		uchStateID = Str2Int(strSubList[sTCount++]);
		uchStateLv = Str2Int(strSubList[sTCount++]);
		lOnTick = Str2Int(strSubList[sTCount++]);

		if (uchStateID == 83)
			lOnTick = -1;

		for (int i = 0; i < nPersCount; i++)
		{
			if (g_Config.m_cSaveState[i] == 0)
				break;

			if (g_Config.m_cSaveState[i] == uchStateID)
			{
				seconds = time(NULL);
				if (seconds >= lOnTick)
					break;

				lOnTick = lOnTick - seconds;
			}
		}
		pCCha->AddSkillState(0, g_pCSystemCha->GetID(), g_pCSystemCha->GetHandle(), 0, 0, 0, uchStateID, uchStateLv, lOnTick, enumSSTATE_ADD_EQUALORLARGER, false);
	}

	return true;
}

// ��չ����ת�����ַ���
char*	ChaExtendAttr2String(CCharacter *pCCha, char *szAttrBuf, int nLen)
{
	if (!pCCha || !szAttrBuf)
		return NULL;

	sprintf(szAttrBuf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
						(int)pCCha->getAttr(ATTR_EXTEND0), (int)pCCha->getAttr(ATTR_EXTEND1), (int)pCCha->getAttr(ATTR_EXTEND2), 
						(int)pCCha->getAttr(ATTR_EXTEND3), (int)pCCha->getAttr(ATTR_EXTEND4), (int)pCCha->getAttr(ATTR_EXTEND5), 
						(int)pCCha->getAttr(ATTR_EXTEND6), (int)pCCha->getAttr(ATTR_EXTEND7), (int)pCCha->getAttr(ATTR_EXTEND8), 
						(int)pCCha->getAttr(ATTR_EXTEND9));
	return szAttrBuf;
}

// �ַ���ת������չ����
bool		Strin2ChaExtendAttr(CCharacter *pCCha, std::string &strAttr)
{
	if (!pCCha || strAttr.length() < 19)
		return false;

	int val[10];

	sscanf(strAttr.c_str(), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &val[0], &val[1], &val[2], &val[3], &val[4], &val[5], 
															&val[6], &val[7], &val[8], &val[9]);

	for(int i=0;i<10;i++)
	{
		pCCha->setAttr(ATTR_COUNT_BASE10 + i, val[i]);
	}

	return true;
}

