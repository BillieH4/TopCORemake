// Mission.h Created by knight-gongjian 2004.12.13.
//---------------------------------------------------------
#pragma once

#ifndef _MISSION_H_
#define _MISSION_H_

#include "RoleData.h"
//---------------------------------------------------------
_DBC_USING

class CCharacter;
namespace mission
{
	class CCharMission
	{
	public:		
		CCharMission();
		~CCharMission();

		// �����¼��ǩ�����ȡ
		BOOL	MisInit( char* pszBuf );
		BOOL	MisGetData( char* pszBuf, DWORD dwSize );

		// ������ʷ��¼��ǩ�洢
		BOOL	MisInitRecord( char* pszBuf );
		BOOL	MisGetRecord( char* pszBuf, DWORD dwSize );

		// ���񴥷�������Ͷ�ȡ
		BOOL	MisInitTrigger( char* pszBuf );
		BOOL	MisGetTrigger( char* pszBuf, DWORD dwSize );

		// ���������ɼ���
		BOOL	MisInitMissionCount( char* pszBuf );
		BOOL	MisGetMissionCount( char* pszBuf, DWORD dwSize );

		void	MisClear();
		void	SetMisChar( CCharacter& character ) { m_pRoleChar = &character; }

		// �������¼�����
		BOOL	MisEventProc( TRIGGER_EVENT e, WPARAM wParam, LPARAM lParam );

		// ��¼��Ϣ
		BOOL	MisAddTrigger( const TRIGGER_DATA& Data );
		BOOL	MisClearTrigger( WORD wTriggerID );
		BOOL	MisDelTrigger( WORD wTriggerID );

		// �������
		BOOL	MisAddRole( WORD wRoleID, WORD wScriptID );
		BOOL	MisHasRole( WORD wRoleID );
		BOOL	MisClearRole( WORD wRoleID );
		BOOL	MisCancelRole( WORD wRoleID );
		BOOL	MisIsRoleFull() { return m_byNumMission >= ROLE_MAXNUM_MISSION; }
		BOOL	MisGetMisScript( WORD wRoleID, WORD& wScriptID );

		// ����������ɱ��
		BOOL	MisSetMissionComplete( WORD wRoleID );
		BOOL	MisSetMissionPending( WORD wRoleID );
		BOOL	MisSetMissionFailure( WORD wRoleID );
		BOOL	MisHasMissionFailure( WORD wRoleID );

		// �����ǩ
		BOOL	MisSetFlag( WORD wRoleID, WORD wFlag );
		BOOL	MisClearFlag( WORD wRoleID, WORD wFlag );
		BOOL	MisIsSet( WORD wRoleID, WORD wFlag );
		BOOL	MisIsValid( WORD wFlag );
		
		BOOL	MisSetRecord( WORD wRec );
		BOOL	MisClearRecord( WORD wRec );
		BOOL	MisIsRecord( WORD wRec );
		BOOL	MisIsValidRecord( WORD wRec );

		// ��¼��Ұ��NPC����Ըý�ɫ������״̬��Ϣ
		BOOL	MisAddMissionState( DWORD dwNpcID, BYTE byID, BYTE byState );
		BOOL	MisGetMissionState( DWORD dwNpcID, BYTE& byState );
		BOOL    MisClearMissionState( DWORD dwNpcID );
		BOOL	MisGetNumMission( DWORD dwNpcID, BYTE& byNum );
		BOOL	MisGetMissionInfo( DWORD dwNpcID, BYTE byIndex, BYTE& byID, BYTE& byState );
		BOOL	MisGetCharMission( DWORD dwNpcID, BYTE byID, BYTE& byState );
		BOOL	MisGetNextMission( DWORD dwNpcID, BYTE& byIndex, BYTE& byID, BYTE& byState );

		// ��������ʱ������Ϣ����
		void	MisSetMissionPage( DWORD dwNpcID, BYTE byPrev, BYTE byNext, BYTE byState );
		BOOL	MisGetMissionPage( DWORD dwNpcID, BYTE& byPrev, BYTE& byNext, BYTE& byState );
		void	MisSetTempData( DWORD dwNpcID, WORD wID, BYTE byState, BYTE byMisType );
		BOOL	MisGetTempData( DWORD dwNpcID, WORD& wID, BYTE& byState, BYTE& byMisType );
		
		// ��ѯ������־��Ϣ
		void	MisGetMisLog();
		void	MisGetMisLogInfo( WORD wMisID );
		void	MisLogClear( WORD wMisID );
		void	MisLogAdd( WORD wMisID, BYTE byState );

		// �������ӿڲ�����Ϣ
		BOOL	MisHasRandMission( WORD wRoleID );
		BOOL	MisAddRandMission( WORD wRoleID, WORD wScriptID, BYTE byType, BYTE byLevel, DWORD dwExp, DWORD dwMoney, USHORT sPrizeData, USHORT sPrizeType, BYTE byNumData );
		BOOL	MisSetRandMissionData( WORD wRoleID, BYTE byIndex, const mission::MISSION_DATA& RandData );
		BOOL	MisGetRandMission( WORD wRoleID, BYTE& byType, BYTE& byLevel, DWORD& dwExp, DWORD& dwMoney, USHORT& sPrizeData, USHORT& sPrizeType, BYTE& byNumData );
		BOOL	MisGetRandMissionData( WORD wRoleID, BYTE byIndex, mission::MISSION_DATA& RandData );

		// ����͸�npc����Ʒ(������Ʒ��NPCȡ����Ʒ�󣬼�¼һ����ǣ��Ƿ�ȡ���øñ�Ǳ�ʶ)
		BOOL	MisHasSendNpcItemFlag( WORD wRoleID, WORD wNpcID );
		BOOL	MisNoSendNpcItemFlag( WORD wRoleID, WORD wNpcID );
		BOOL	MisTakeRandMissionNpcItem( WORD wRoleID, WORD wNpcID, USHORT& sItemID );
		BOOL	MisTakeAllRandNpcItem( WORD wRoleID );
		BOOL	MisHasRandMissionNpc( WORD wRoleID, WORD wNpcID, WORD wAreaID );		

		// ���һ������NPC
		BOOL	MisAddFollowNpc( WORD wRoleID, BYTE byIndex, WORD wNpcCharID, CCharacter* pNpc, BYTE byAiType );
		BOOL	MisClearFollowNpc( WORD wRoleID, BYTE byIndex );
		BOOL	MisClearAllFollowNpc( WORD wRoleID );
		BOOL	MisHasFollowNpc( WORD wRoleID, BYTE byIndex );
		BOOL	MisIsFollowNpc( WORD wRoleID, BYTE byIndex, WORD wNpcCharID );
		BOOL	MisLowDistFollowNpc( WORD wRoleID, BYTE byIndex );

		// ���һ���������
		BOOL	MisCompleteRandMission( WORD wRoleID );
		BOOL	MisFailureRandMission( WORD wRoleID );
		BOOL	MisAddRandMissionNum( WORD wRoleID );

		BOOL	MisResetRandMission( WORD wRoleID );
		BOOL	MisResetRandMissionNum( WORD wRoleID );
		WORD	MisGetRandMissionCount( WORD wRoleID );
		WORD	MisGetRandMissionNum( WORD wRoleID );

		// ����Ƿ�������Ҫ��Ʒ
		BOOL	MisNeedItem( USHORT sItemID );

		// ����ˢ�»�ȡ��Ʒ�������ļ���
		void	MisRefreshItemCount( USHORT sItemID );
		BOOL	MisGetItemCount( WORD wRoleID, USHORT sItemID, USHORT& sCount );

		// ��ɫ����
		void	MisLogout();
		void	MisLogin();
		void	MisEnterMap();
		void	MisGooutMap();
		void	MisEnterBoat( CCharacter* pBoat );

		// �����¼�ʵ�彻���ȴ�ʱ��
		void	SetEntityTime( DWORD dwTime ) { m_dwEntityTime = dwTime; }
		BOOL	GetEntityTime( DWORD& dwTime ) { dwTime = m_dwEntityTime; return dwTime != 0; }

	protected:
		
		// Ԥ�������û��壬��ʼ����Ϣ
		void	Initially();
		void	Finally();

		void	DeleteTrigger();
		void	ClearTrigger( DWORD dwIndex );
		void	ClearRoleTrigger( WORD wRoleID );
		BOOL	CancelRole( WORD wRoleID, WORD wScriptID );

		// �����¼�������
		void	KillWare( USHORT sWareID );
		void	GetItem( USHORT sItemID, USHORT sCount );
		void	TimeOut( USHORT sTime );
		void	GotoMap( BYTE byMapID, WORD wxPos, WORD wyPos );
		void	LevelUp( USHORT sLevel );
		void	CharBorn();
		void	EquipItem( USHORT sItemID, USHORT sTriID );

		// �������ݿ�洢����Ϣ�汾ת������ǰ�汾��Ϣ
		BOOL ConvertMissionInfo( const char* pszBuf, int nEdition );
		BOOL ConvertTriggerInfo( const char* pszBuf, int nEdition );
		BOOL ConvertMisCountInfo( const char* pszBuf, int nEdition );
		BOOL ConvertMissionRecord( const char* pszBuf, int nEdition );

		// ��ɫ������Ϣ(����������ǩ����ʷ��¼)
		BYTE			m_byNumTrigger;
		TRIGGER_DATA	m_Trigger[ROLE_MAXNUM_CHARTRIGGER];
		BYTE			m_byNumMission;
		MISSION_INFO	m_Mission[ROLE_MAXNUM_MISSION];
		ROLE_RECORDINFO m_RoleRecord;
		BYTE			m_byNumGotoMap; // �л���ͼ��Ԫ�¼�����������
		RAND_MISSION_COUNT	m_MissionCount[ROLE_MAXNUM_MISSIONCOUNT]; // �������������
		BYTE			m_byNumMisCount;
		BYTE			m_byOnline;		// �ý�ɫ�Ƿ�����

		// ��Ұ��NPC����Ըý�ɫ������Ϣ
		MISSION_STATE   m_MissionState[ROLE_MAXNUM_INSIDE_NPCCOUNT];
		BYTE			m_byStateIndex; // ����״̬�������
		BYTE			m_byNumState;

		// ����Ի���Ϣ��ʱ��¼
		DWORD			m_dwTalkNpcID;
		WORD			m_wIndex;
		BYTE			m_byState;
		BYTE			m_byMisType;
		BYTE			m_byStep;
		BYTE			m_byPrev;
		BYTE			m_byNext;

		// �¼�ʵ�彻���ȴ�ʱ��
		DWORD			m_dwEntityTime;

		// ��ɫָ��
		CCharacter*		m_pRoleChar;

	};
}

//---------------------------------------------------------

#endif // _TRIGGER_EVENT_H_