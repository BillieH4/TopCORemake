// CharBoat.cpp created by knight 2005.4.18
//---------------------------------------------------------

#include "CharBoat.h"
#include <ShipSet.h>
#include "GameApp.h"
#include "GameDB.h"
#include "SubMap.h"
#include "lua_gamectrl.h"

//---------------------------------------------------------
mission::CCharBoat g_CharBoat;

namespace mission
{	
	CCharBoat::CCharBoat()
	{T_B
		m_pShipSet = NULL;
		m_pShipPartSet = NULL;
	T_E}

	CCharBoat::~CCharBoat()
	{T_B
		Clear();
	T_E}
	
	void CCharBoat::Clear()
	{T_B
		SAFE_DELETE( m_pShipSet );
		SAFE_DELETE( m_pShipPartSet );
	T_E}

	BOOL CCharBoat::Load( const char szBoat[], const char szPart[] )
	{T_B
		
		extern BOOL LoadTable(CRawDataSet *pTable, const char*);
		m_pShipSet = new xShipSet( 1, 400 );
		if( !LoadTable(m_pShipSet, szBoat ) )
		{
			Clear();
			return FALSE;
		}

		m_pShipPartSet = new xShipPartSet( 1, 400 );
		if( !LoadTable(m_pShipPartSet, szPart ) )
		{
			Clear();
			return FALSE;
		}

		return TRUE;
	T_E}

	void CCharBoat::UpdateBoat( const BOAT_DATA& Data )
	{T_B
		
	T_E}

	void CCharBoat::GetBerthName( USHORT sBerthID, char szBerth[], USHORT sLen )
	{T_B
		// ����NPC�ű��Ի�������
		lua_getglobal( g_pLuaState, "GetBerthData" );
		if( !lua_isfunction( g_pLuaState, -1 ) )
		{
			lua_pop( g_pLuaState, 1 );
			LG( "lua_invalidfunc", "GetBerthData" );
			return;
		}

		lua_pushnumber( g_pLuaState, sBerthID );

		int nStatus = lua_pcall( g_pLuaState, 1, 1, 0 );
		if( nStatus )
		{
			lua_callalert( g_pLuaState, nStatus );
			lua_settop(g_pLuaState, 0);
			return;
		}

		const char* pszName = lua_tostring( g_pLuaState, -1 );
		lua_settop(g_pLuaState, 0);
		if( pszName )
		{
			strncpy( szBerth, pszName, sLen );
		}
	T_E}

	BOOL CCharBoat::BoatLimit( CCharacter& owner, USHORT sBoatID )
	{T_B
		xShipInfo* pInfo = (xShipInfo*)m_pShipSet->GetRawDataInfo( sBoatID );
		if( pInfo == NULL ) 
		{
			//owner.SystemNotice( "��ȡ��ֻ��Ϣʧ�ܣ�����Ĵ�ֻ��ϢID[%d]!", sBoatID );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00042), sBoatID );
			return TRUE;
		}

		if( pInfo->sLvLimit > (USHORT)owner.getAttr( ATTR_LV ) )
		{
			//owner.SystemNotice( "�������㣬��Ҫ�ȼ�%d��", pInfo->sLvLimit );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00043), pInfo->sLvLimit );
			return TRUE;
		}

		// ����ְҵ������
		if( pInfo->sPfLimit[0] == USHORT(-1) )
		{
			return FALSE;
		}

		char szData[256];
		//strcpy( szData, "�������㣬��Ҫְҵ��" );
		strcpy( szData, RES_STRING(GM_CHARBOAT_CPP_00044));
		BOOL bRet = TRUE;
		USHORT sPf = (USHORT)owner.getAttr( ATTR_JOB );
		for( BYTE i = 0; i < pInfo->sNumPfLimit; i++ )
		{
			strcat( szData, g_GetJobName( pInfo->sPfLimit[i] ) );
			if( i + 1 < pInfo->sNumPfLimit )
			{
				strcat( szData, "��" );
			}
			else
			{
				strcat( szData, "��" );
			}
			if( pInfo->sPfLimit[i] == sPf ) 
			{
				bRet = FALSE;
			}
		}

		if( bRet )
		{
			owner.SystemNotice( szData );
		}

		return bRet;
	T_E}

	BOOL CCharBoat::SetPartData( CCharacter& boat, USHORT sTypeID, const BOAT_DATA& AttrInfo )
	{T_B
		xShipInfo* pInfo = (xShipInfo*)m_pShipSet->GetRawDataInfo( AttrInfo.sBoat );
		if( pInfo == NULL ) 
		{
			//LG( "boat_error", "���ô�ֻ�����Ϣ������Ĵ�ֻ��ϢID[%d]!", AttrInfo.sBoat );
			LG( "boat_error", "set boat surface informationg,error information of boat  ID[%d]!", AttrInfo.sBoat );
			return FALSE;
		}

		// ��ֻ�Ƿ���Ը������
		xShipPartInfo* pData = (xShipPartInfo*)m_pShipPartSet->GetRawDataInfo( pInfo->sBody );
		if( pData == NULL ) 
		{
			//LG( "boat_error", "���ô�ֻ�����Ϣ������Ĵ�����ϢID[%d]!", pInfo->sBody );
			LG( "boat_error", "set boat surface informationg,error information of boat  ID[%d]!", pInfo->sBody );
			return FALSE;
		}

		stNetChangeChaPart BoatPart;
		memset( &BoatPart, 0, sizeof(stNetChangeChaPart) );
		BoatPart.sPosID = pInfo->sPosID;
		BoatPart.sBoatID = AttrInfo.sBoat;
		BoatPart.sHeader = AttrInfo.sHeader;
		BoatPart.sBody = AttrInfo.sBody;
		BoatPart.sEngine = AttrInfo.sEngine;
		BoatPart.sCannon = AttrInfo.sCannon;
		BoatPart.sEquipment = AttrInfo.sEquipment;

		BoatPart.sTypeID = sTypeID;
		boat.SetBoatLook( BoatPart );
		return TRUE;		
	T_E}

	BOOL CCharBoat::SyncAttr( CCharacter& owner, DWORD dwBoatID, USHORT sCmd, USHORT sBerthID, const BOAT_SYNC_ATTR& AttrInfo )
	{T_B
		xShipInfo* pInfo = (xShipInfo*)m_pShipSet->GetRawDataInfo( AttrInfo.sBoatID );
		if( pInfo == NULL ) 
		{
			//owner.SystemNotice( "������ֻʧ�ܣ�����Ĵ�ֻ��ϢID[%d]!", AttrInfo.sBoatID );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00010), AttrInfo.sBoatID );
			return FALSE;
		}

		// ��ֻ�Ƿ���Ը������
		xShipBuildInfo Data;
		memset( &Data, 0, sizeof(xShipBuildInfo) );
		Data.dwMoney = 0;
		Data.dwMinAttack = pInfo->sMinAttack;
		Data.dwMaxAttack = pInfo->sMaxAttack;
		Data.dwCurEndure = pInfo->sEndure;
		Data.dwMaxEndure = pInfo->sEndure;
		Data.dwSpeed = pInfo->sSpeed;
		Data.dwDistance = pInfo->sDistance;
		Data.dwDefence = pInfo->sDefence;
		Data.dwCurSupply = pInfo->sSupply;
		Data.dwMaxSupply = pInfo->sSupply;
		Data.dwConsume = pInfo->sConsume;
		Data.dwAttackTime = pInfo->sTime;
		Data.sCapacity = pInfo->sCapacity;
		GetBerthName( sBerthID, Data.szBerth, BOAT_MAXSIZE_NAME - 1 );

		WPACKET packet = GETWPACKET();
        WRITE_CMD( packet, sCmd );
		WRITE_LONG( packet, dwBoatID );
		WRITE_STRING( packet, AttrInfo.szName );
        WRITE_STRING( packet, pInfo->szName );
		WRITE_STRING( packet, pInfo->szDesp );
		WRITE_STRING( packet, Data.szBerth );
		WRITE_CHAR( packet, pInfo->byIsUpdate );

		xShipPartInfo* pData = (xShipPartInfo*)m_pShipPartSet->GetRawDataInfo( pInfo->sBody );
		if( pData == NULL ) 
		{
			//owner.SystemNotice( "������ֻʧ�ܣ�����Ĵ�����ϢID[%d]!", pInfo->sBody );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00038), pInfo->sBody );
			return FALSE;
		}
		Data.dwMoney += pData->dwPrice;
		Data.dwMinAttack += pData->sMinAttack;
		Data.dwMaxAttack += pData->sMaxAttack;
		Data.dwCurEndure += pData->sEndure;
		Data.dwMaxEndure += pData->sEndure;
		Data.dwSpeed += pData->sSpeed;
		Data.dwDistance += pData->sDistance;
		Data.dwDefence += pData->sDefence;
		Data.dwCurSupply += pData->sSupply;
		Data.dwMaxSupply += pData->sSupply;
		Data.dwConsume += pData->sConsume;
		Data.dwAttackTime += pData->sTime;
		//Data.sCapacity += pData->sCapacity;

		WRITE_SHORT( packet, pInfo->sPosID );
		WRITE_LONG( packet, pData->dwModel );
		WRITE_STRING( packet, pData->szName );

		if( pInfo->byIsUpdate )
		{
			pData = (xShipPartInfo*)m_pShipPartSet->GetRawDataInfo( AttrInfo.sHeader );
			if( pData == NULL ) 
			{
				//owner.SystemNotice( "������ֻʧ�ܣ�����Ĵ�ͷ��ϢID[%d]!", AttrInfo.sHeader );
				owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00039), AttrInfo.sHeader );
				return FALSE;
			}
			WRITE_CHAR( packet, AttrInfo.byHeader );
			WRITE_LONG( packet, pData->dwModel );
			WRITE_STRING( packet, pData->szName );
			Data.dwMoney += pData->dwPrice;
			Data.dwMinAttack += pData->sMinAttack;
			Data.dwMaxAttack += pData->sMaxAttack;
			Data.dwCurEndure += pData->sEndure;
			Data.dwMaxEndure += pData->sEndure;
			Data.dwSpeed += pData->sSpeed;
			Data.dwDistance += pData->sDistance;
			Data.dwDefence += pData->sDefence;
			Data.dwCurSupply += pData->sSupply;
			Data.dwMaxSupply += pData->sSupply;
			Data.dwConsume += pData->sConsume;
			Data.dwAttackTime += pData->sTime;
			//Data.sCapacity += pData->sCapacity;

			pData = (xShipPartInfo*)m_pShipPartSet->GetRawDataInfo( AttrInfo.sEngine );
			if( pData == NULL ) 
			{
				//owner.SystemNotice( "������ֻʧ�ܣ�����Ĵ�������ϢID[%d]!", AttrInfo.sEngine );
				owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00040), AttrInfo.sEngine );
				return FALSE;
			}
			WRITE_CHAR( packet, AttrInfo.byEngine );
			WRITE_LONG( packet, pData->dwModel );
			WRITE_STRING( packet, pData->szName );
			Data.dwMoney += pData->dwPrice;
			Data.dwMinAttack += pData->sMinAttack;
			Data.dwMaxAttack += pData->sMaxAttack;
			Data.dwCurEndure += pData->sEndure;
			Data.dwMaxEndure += pData->sEndure;
			Data.dwSpeed += pData->sSpeed;
			Data.dwDistance += pData->sDistance;
			Data.dwDefence += pData->sDefence;
			Data.dwCurSupply += pData->sSupply;
			Data.dwMaxSupply += pData->sSupply;
			Data.dwConsume += pData->sConsume;
			Data.dwAttackTime += pData->sTime;
			//Data.sCapacity += pData->sCapacity;

			for( int i = 0; i < BOAT_MAXNUM_MOTOR; i++ )
			{
				xShipPartInfo* pMotorData = (xShipPartInfo*)m_pShipPartSet->GetRawDataInfo( pData->sMotor[i] );
				if( pMotorData == NULL ) 
				{
					WRITE_LONG( packet, 0 );
				}
				else
				{
					WRITE_LONG( packet, pMotorData->dwModel );
					Data.dwMoney += pMotorData->dwPrice;
					Data.dwMinAttack += pMotorData->sMinAttack;
					Data.dwMaxAttack += pMotorData->sMaxAttack;
					Data.dwCurEndure += pMotorData->sEndure;
					Data.dwMaxEndure += pMotorData->sEndure;
					Data.dwSpeed += pMotorData->sSpeed;
					Data.dwDistance += pMotorData->sDistance;
					Data.dwDefence += pMotorData->sDefence;
					Data.dwCurSupply += pMotorData->sSupply;
					Data.dwMaxSupply += pMotorData->sSupply;
					Data.dwConsume += pMotorData->sConsume;
					Data.dwAttackTime += pMotorData->sTime;
					//Data.sCapacity += pMotorData->sCapacity;
				}
			}
		}

		pData = (xShipPartInfo*)m_pShipPartSet->GetRawDataInfo( AttrInfo.sCannon );
		if( pData == NULL ) 
		{
			WRITE_CHAR( packet, AttrInfo.byCannon );
			//WRITE_STRING( packet, "δװ�û���" );
			WRITE_STRING( packet, RES_STRING(GM_CHARBOAT_CPP_00001) );

			//owner.SystemNotice( "������ֻʧ�ܣ�����Ĵ�������ϢID[%d]!", AttrInfo.sCannon );
			//return FALSE;
		}
		else
		{
			WRITE_CHAR( packet, AttrInfo.byCannon );
			WRITE_STRING( packet, pData->szName );
			Data.dwMoney += pData->dwPrice;
			Data.dwMinAttack += pData->sMinAttack;
			Data.dwMaxAttack += pData->sMaxAttack;
			Data.dwCurEndure += pData->sEndure;
			Data.dwMaxEndure += pData->sEndure;
			Data.dwSpeed += pData->sSpeed;
			Data.dwDistance += pData->sDistance;
			Data.dwDefence += pData->sDefence;
			Data.dwCurSupply += pData->sSupply;
			Data.dwMaxSupply += pData->sSupply;
			Data.dwConsume += pData->sConsume;
			Data.dwAttackTime += pData->sTime;
			//Data.sCapacity += pData->sCapacity;
		}

		pData = (xShipPartInfo*)m_pShipPartSet->GetRawDataInfo( AttrInfo.sEquipment );
		if( pData == NULL ) 
		{
			WRITE_CHAR( packet, 0 );
			//WRITE_STRING( packet, "δװ�����" );
			WRITE_STRING( packet, RES_STRING(GM_CHARBOAT_CPP_00002) );

			//owner.SystemNotice( "������ֻʧ�ܣ�����Ĵ������ϢID[%d]!", AttrInfo.sEquipment );
			//return FALSE;
		}
		else
		{
			WRITE_CHAR( packet, AttrInfo.byEquipment );
			WRITE_STRING( packet, pData->szName );
			Data.dwMoney += pData->dwPrice;
			Data.dwMinAttack += pData->sMinAttack;
			Data.dwMaxAttack += pData->sMaxAttack;
			Data.dwCurEndure += pData->sEndure;
			Data.dwMaxEndure += pData->sEndure;
			Data.dwSpeed += pData->sSpeed;
			Data.dwDistance += pData->sDistance;
			Data.dwDefence += pData->sDefence;
			Data.dwCurSupply += pData->sSupply;
			Data.dwMaxSupply += pData->sSupply;
			Data.dwConsume += pData->sConsume;
			Data.dwAttackTime += pData->sTime;
			//Data.sCapacity += pData->sCapacity;
		}

		WRITE_LONG( packet, Data.dwMoney );
		WRITE_LONG( packet, Data.dwMinAttack );
		WRITE_LONG( packet, Data.dwMaxAttack );
		WRITE_LONG( packet, Data.dwCurEndure );
		WRITE_LONG( packet, Data.dwMaxEndure );
		WRITE_LONG( packet, Data.dwSpeed );
		WRITE_LONG( packet, Data.dwDistance );
		WRITE_LONG( packet, Data.dwDefence );
		WRITE_LONG( packet, Data.dwCurSupply );
		WRITE_LONG( packet, Data.dwMaxSupply );
		WRITE_LONG( packet, Data.dwConsume );
		WRITE_LONG( packet, Data.dwAttackTime );
		WRITE_SHORT( packet, Data.sCapacity );

		owner.ReflectINFof( &owner, packet );
		return TRUE;
	T_E}

	void CCharBoat::Cancel( CCharacter& owner )
	{T_B
		CCharacter* pBoat = owner.GetBoat();
		if( pBoat )
		{
			pBoat->Free();
			owner.SetBoat( NULL );
			//owner.SystemNotice( "���Ѿ��ɹ�ȡ���˽��촬ֻ�ļƻ���" );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00003) );
		}
	T_E}

	BOOL CCharBoat::Create( CCharacter& owner, USHORT sBoatID, USHORT sBerthID )
	{T_B
		if( owner.GetPlayer()->IsLuanchOut() )
		{
			//owner.SystemNotice( "��ǰ����״̬,�����Խ��촬ֻ��" );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00004) );
			return FALSE;
		}

		if( owner.GetTradeData() )
		{
			//owner.SystemNotice( "��ǰ����״̬,�����Խ��촬ֻ��" );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00005) );
			return FALSE;
		}

		if( owner.GetStallData() )
		{
			//owner.SystemNotice( "���ڰ�̯�������Խ��촬ֻ��" );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00006) );
			return FALSE;
		}

		if( owner.m_CKitbag.IsLock() || !owner.GetActControl(enumACTCONTROL_ITEM_OPT) )
		{
			//owner.SystemNotice( "�����ѱ������������Խ��촬ֻ��" );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00007) );
			return FALSE;
		}

        if( owner.m_CKitbag.IsPwdLocked() )
        {
            //owner.SystemNotice( "�����ѱ����������������Խ��촬ֻ��" );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00008) );
			return FALSE;
        }

		//add by ALLEN 2007-10-16
				if(owner.IsReadBook())
		{
			//owner.SystemNotice("���ڶ��飬�����Խ��촬ֻ��");
			owner.SystemNotice(RES_STRING(GM_CHARBOAT_CPP_00009));
			return FALSE;
		}

		xShipInfo* pInfo = (xShipInfo*)m_pShipSet->GetRawDataInfo( sBoatID );
		if( pInfo == NULL ) 
		{
			//owner.SystemNotice( "������ֻʧ�ܣ�����Ĵ�ֻ��ϢID[%d]!", sBoatID );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00010), sBoatID );
			return FALSE;
		}

		if( owner.GetBoat() )
		{
			// fixed me to modify
			owner.GetBoat()->Free();
			owner.SetBoat( NULL );
			//owner.SystemNotice( "�ý�ɫ�Ѿ���ʼ�˴�ֻ����ID[%d]��", sBoatID );
			//return FALSE;
		}

		if( owner.GetPlayer()->IsBoatFull() )
		{
			//owner.SystemNotice( "��ӵ�еĴ�ֻ�����������������ٽ����µĴ�ֻ��" );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00011) );
			return  FALSE;
		}

		if( owner.m_CKitbag.IsFull() )
		{
			//owner.BickerNotice( "���������������촬ֻʧ�ܣ�" );
			owner.BickerNotice( RES_STRING(GM_CHARBOAT_CPP_00012) );
			return FALSE;
		}

		CCharacter* pBoat = g_pGameApp->GetNewCharacter();
		if( pBoat == NULL )
		{
			//owner.SystemNotice( "������ֻʧ�ܣ����䴬ֻ�ڴ�ʧ��!ID[%d]", sBoatID );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00013), sBoatID );
			return FALSE;
		}

		owner.SetBoat( pBoat );
		pBoat->setAttr( ATTR_BOAT_SHIP, sBoatID );
		pBoat->setAttr( ATTR_BOAT_BERTH, sBerthID );

		// ͬ����ֻ������
		BOAT_SYNC_ATTR AttrInfo;
		memset( &AttrInfo, 0, sizeof(BOAT_SYNC_ATTR) );
		AttrInfo.byHeader = 0;
		AttrInfo.sHeader = pInfo->sHeader[0];
		AttrInfo.byEngine = 0;
		AttrInfo.sEngine = pInfo->sEngine[0];
		AttrInfo.byCannon = 0;
		AttrInfo.sCannon = pInfo->sCannon[0];
		AttrInfo.byEquipment = 0;
		AttrInfo.sEquipment = pInfo->sEquipment[0];
		AttrInfo.sBoatID = sBoatID;
		
		if( !SyncAttr( owner, 0, CMD_MC_CREATEBOAT, sBerthID, AttrInfo ) )
		{
			// ͬ����ֻ����ʧ�ܣ�ȡ����ֻ����
			pBoat->Free();
			owner.SetBoat( NULL );
			//owner.SystemNotice( "�����ֻ����������Ϣ��" );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00014) );
			return FALSE;
		}
		return TRUE;
	T_E}

	BOOL CCharBoat::GetBoatInfo( CCharacter& owner, DWORD dwBoatID )
	{
		CPlayer* pPlayer = owner.GetPlayer();
		if( !pPlayer )
		{
			return FALSE;
		}

		CCharacter* pBoat = pPlayer->GetBoat( dwBoatID );
		if( !pBoat )
		{
			//owner.SystemNotice( "�Ҳ����ô�����Ϣ��ID[0x%X]", dwBoatID );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00015), dwBoatID );
			return FALSE;
		}

		BOAT_SYNC_ATTR AttrInfo;
		memset( &AttrInfo, 0, sizeof(BOAT_SYNC_ATTR) );
		strncpy( AttrInfo.szName, pBoat->GetName(), BOAT_MAXSIZE_NAME - 1 );
		AttrInfo.sHeader = (USHORT)pBoat->getAttr( ATTR_BOAT_HEADER );
		AttrInfo.sEngine = (USHORT)pBoat->getAttr( ATTR_BOAT_ENGINE );
		AttrInfo.sCannon = (USHORT)pBoat->getAttr( ATTR_BOAT_CANNON );
		AttrInfo.sEquipment = (USHORT)pBoat->getAttr( ATTR_BOAT_PART );
		AttrInfo.sBoatID = (USHORT)pBoat->getAttr( ATTR_BOAT_SHIP );		
		USHORT sBerthID = (USHORT)pBoat->getAttr( ATTR_BOAT_BERTH );

		return SyncAttr( owner, pBoat->GetID(), CMD_MC_BOATINFO, sBerthID, AttrInfo );
	}

	BOOL CCharBoat::GetTradeBoatInfo( CCharacter& viewer, CCharacter& owner, DWORD dwBoatID )
	{
		CPlayer* pPlayer = owner.GetPlayer();
		if( !pPlayer )
		{
			return FALSE;
		}

		CCharacter* pBoat = pPlayer->GetBoat( dwBoatID );
		if( !pBoat )
		{
			//owner.SystemNotice( "�Ҳ����ô�����Ϣ��ID[0x%X]", dwBoatID );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00015), dwBoatID );
			return FALSE;
		}

		BOAT_SYNC_ATTR AttrInfo;
		memset( &AttrInfo, 0, sizeof(BOAT_SYNC_ATTR) );
		strncpy( AttrInfo.szName, pBoat->GetName(), BOAT_MAXSIZE_NAME - 1 );
		AttrInfo.sHeader = (USHORT)pBoat->getAttr( ATTR_BOAT_HEADER );
		AttrInfo.sEngine = (USHORT)pBoat->getAttr( ATTR_BOAT_ENGINE );
		AttrInfo.sCannon = (USHORT)pBoat->getAttr( ATTR_BOAT_CANNON );
		AttrInfo.sEquipment = (USHORT)pBoat->getAttr( ATTR_BOAT_PART );
		AttrInfo.sBoatID = (USHORT)pBoat->getAttr( ATTR_BOAT_SHIP );		
		USHORT sBerthID = (USHORT)pBoat->getAttr( ATTR_BOAT_BERTH );

		return SyncAttr( viewer, pBoat->GetID(), CMD_MC_BOATINFO, sBerthID, AttrInfo );
	}

	BOOL CCharBoat::Update( CCharacter& owner, RPACKET& packet )
	{T_B
		CCharacter* pBoat = owner.GetBoat();
		if( !pBoat ) 
		{
			//owner.SystemNotice( "Update:�㻹δ��ʼ���촬ֻ�����ȿ�ʼ��" );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00016) );
			return FALSE;
		}
		char szHeader = READ_CHAR( packet );
		char szEngine = READ_CHAR( packet );
		char szCannon = READ_CHAR( packet );
		char szEquipment = READ_CHAR( packet );

		USHORT sBoatID = (USHORT)owner.GetBoat()->getAttr( ATTR_BOAT_SHIP );
		USHORT sBerthID = (USHORT)pBoat->getAttr( ATTR_BOAT_BERTH );
		xShipInfo* pInfo = (xShipInfo*)m_pShipSet->GetRawDataInfo( sBoatID );
		if( pInfo == NULL ) 
		{
			//owner.SystemNotice( "������ֻʧ�ܣ�����Ĵ�ֻ��ϢID[%d]!", sBoatID );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00010), sBoatID );
			return FALSE;
		}

		USHORT sHeader, sEngine, sCannon, sEquipment;
		if( pInfo->byIsUpdate )
		{
			if( szHeader < 0 )
			{
				szHeader = pInfo->sNumHeader - 1;
				sHeader = pInfo->sHeader[szHeader];
			}
			else 
			{
				if( szHeader >= pInfo->sNumHeader )
				{
					szHeader = 0;
				}
				sHeader = pInfo->sHeader[szHeader];
			}

			if( szEngine < 0 )
			{
				szEngine = pInfo->sNumEngine - 1;
				sEngine = pInfo->sEngine[szEngine];
			}
			else 
			{
				if( szEngine >= pInfo->sNumEngine )
				{
					szEngine = 0;
				}
				sEngine = pInfo->sEngine[szEngine];
			}
		}
		else
		{
			szHeader = 0;
			sHeader = pInfo->sHeader[szHeader];
			szEngine = 0;
			sEngine = pInfo->sEngine[szEngine];
		}

		if( szCannon < 0 )
		{
			szCannon = pInfo->sNumCannon - 1;
			sCannon = pInfo->sCannon[szCannon];
		}
		else 
		{
			if( szCannon >= pInfo->sNumCannon )
			{
				szCannon = 0;
			}
			sCannon = pInfo->sCannon[szCannon];
		}

		if( szEquipment < 0 )
		{
			szEquipment = pInfo->sNumEquipment - 1;
			sEquipment = pInfo->sEquipment[szEquipment];
		}
		else 
		{
			if( szEquipment >= pInfo->sNumEquipment )
			{
				szEquipment = 0;
			}
			sEquipment = pInfo->sEquipment[szEquipment];
		}

		BOAT_SYNC_ATTR AttrInfo;
		memset( &AttrInfo, 0, sizeof(BOAT_SYNC_ATTR) );
		AttrInfo.byHeader = szHeader;
		AttrInfo.sHeader = sHeader;
		AttrInfo.byEngine = szEngine;
		AttrInfo.sEngine = sEngine;
		AttrInfo.byCannon = szCannon;
		AttrInfo.sCannon = sCannon;
		AttrInfo.byEquipment = szEquipment;
		AttrInfo.sEquipment = sEquipment;
		AttrInfo.sBoatID = sBoatID;

		return SyncAttr( owner, 0, CMD_MC_UPDATEBOAT, sBerthID, AttrInfo );
	T_E}

	BOOL CCharBoat::MakeBoat( CCharacter& owner, RPACKET& packet )
	{T_B
		if( owner.GetPlayer()->IsLuanchOut() )
		{
			//owner.SystemNotice( "��ǰ����״̬,�����Խ��촬ֻ��" );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00004) );
			return FALSE;
		}

		if( owner.GetTradeData() )
		{
			//owner.SystemNotice( "��ǰ����״̬,�����Խ��촬ֻ��" );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00005) );
			return FALSE;
		}

		if( owner.GetStallData() )
		{
			//owner.SystemNotice( "���ڰ�̯�������Խ��촬ֻ��" );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00006) );
			return FALSE;
		}

		if( owner.m_CKitbag.IsLock() || !owner.GetActControl(enumACTCONTROL_ITEM_OPT) )
		{
			//owner.SystemNotice( "�����ѱ������������Խ��촬ֻ��" );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00007) );
			return FALSE;
		}

        if( owner.m_CKitbag.IsPwdLocked() )
        {
            //owner.SystemNotice( "�����ѱ����������������Խ��촬ֻ��" );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00008) );
			return FALSE;
        }

		//add by ALLEN 2007-10-16
		if( owner.IsReadBook() )
        {
           // owner.SystemNotice( "���ڶ��飬�����Խ��촬ֻ��" );
			 owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00009) );
			return FALSE;
        }

		CCharacter* pBoat = owner.GetBoat();
		if( !pBoat ) 
		{
			//owner.SystemNotice( "MakeBoat:�㻹δ��ʼ���촬ֻ�����ȿ�ʼ��" );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00017) );
			return FALSE;
		}

		if( owner.m_CKitbag.IsFull() )
		{
			pBoat->Free();
			owner.SetBoat( NULL );
			//owner.BickerNotice( "���������������촬ֻʧ�ܣ�" );
			owner.BickerNotice( RES_STRING(GM_CHARBOAT_CPP_00012) );
			return FALSE;
		}

		// ��ӵ�player������
		CPlayer* pPlayer = owner.GetPlayer();
		if( !pPlayer )
		{
			pBoat->Free();
			owner.SetBoat( NULL );
			//owner.SystemNotice( "�����Ҳ��������ߵ�player���ݣ�" );
			owner.SystemNotice(RES_STRING(GM_CHARBOAT_CPP_00018) );
			return FALSE;
		}

		// ����Ƿ������������ֻ
		if( pPlayer->IsBoatFull() )
		{
			pBoat->Free();
			owner.SetBoat( NULL );
			//owner.SystemNotice( "��Ĵ�ֻ�����������������ٽ��죡" );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00019) );
			return FALSE;
		}

		BOAT_DATA Data;
		memset( &Data, 0, sizeof(BOAT_DATA) );
		const char* pszData = READ_STRING( packet );
		if( pszData )
		{
			strncpy( Data.szName, pszData, BOAT_MAXSIZE_BOATNAME - 1 );
			size_t nLen = strlen( Data.szName );
			if( nLen < BOAT_MAXSIZE_MINNAME || nLen >= BOAT_MAXSIZE_BOATNAME  )
			{
				pBoat->Free();
				owner.SetBoat( NULL );
				//owner.BickerNotice( "��ֻ�����ַ����ȱ�����%d-%d���ַ�֮�䣡", BOAT_MAXSIZE_MINNAME, BOAT_MAXSIZE_BOATNAME - 1 );
				owner.BickerNotice( RES_STRING(GM_CHARBOAT_CPP_00020), BOAT_MAXSIZE_MINNAME, BOAT_MAXSIZE_BOATNAME - 1 );
				return FALSE;
			}

			// У�鴬���Ƿ���ȷ
			if( !IsValidName( pszData, (USHORT)nLen ) || !CTextFilter::IsLegalText( CTextFilter::NAME_TABLE, pszData ) )
			{
				pBoat->Free();
				owner.SetBoat( NULL );
				//owner.BickerNotice( "��ֻ���ơ�%s���Ƿ���", pszData );
				owner.BickerNotice( RES_STRING(GM_CHARBOAT_CPP_00021), pszData );
				return FALSE;
			}
		}
		else
		{
			pBoat->Free();
			owner.SetBoat( NULL );
			//owner.SystemNotice( "��ɫ%s�����˷Ƿ����ַ�ָ�룡", owner.GetName() );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00022), owner.GetName() );
			//LG( "boat_error", "��ɫ%s�����˷Ƿ����ַ�ָ�룡", owner.GetName() );
			LG( "boat_error", "character%s pass unlawful character pointer��", owner.GetName() );
			return FALSE;			
		}

		char szHeader = READ_CHAR( packet );
		char szEngine = READ_CHAR( packet );
		char szCannon = READ_CHAR( packet );
		char szEquipment = READ_CHAR( packet );

		Data.dwOwnerID = owner.GetPlayer()->GetDBChaId();
		Data.sBoat = (USHORT)owner.GetBoat()->getAttr( ATTR_BOAT_SHIP );		
		Data.sBerth = (USHORT)owner.GetBoat()->getAttr( ATTR_BOAT_BERTH );
		xShipInfo* pInfo = (xShipInfo*)m_pShipSet->GetRawDataInfo( Data.sBoat );
		if( pInfo == NULL ) 
		{
			pBoat->Free();
			owner.SetBoat( NULL );
			//owner.SystemNotice( "������ֻʧ�ܣ�����Ĵ�ֻ��ϢID[%d]!", Data.sBoat );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00010), Data.sBoat );
			return FALSE;
		}

		Data.sCapacity = pInfo->sCapacity;
		Data.sBody = pInfo->sBody;
		USHORT sHeader = 0, sEngine = 0, sCannon = 0, sEquipment = 0;
		if( pInfo->byIsUpdate ) 
		{
			if( szHeader < 0 )
			{
				szHeader = pInfo->sNumHeader - 1;
				Data.sHeader = pInfo->sHeader[szHeader];
			}
			else 
			{
				if( szHeader >= pInfo->sNumHeader )
				{
					szHeader = 0;
				}
				Data.sHeader = pInfo->sHeader[szHeader];
			}

			if( szEngine < 0 )
			{
				szEngine = pInfo->sNumEngine - 1;
				Data.sEngine = pInfo->sEngine[szEngine];
			}
			else 
			{
				if( szEngine >= pInfo->sNumEngine )
				{
					szEngine = 0;
				}
				Data.sEngine = pInfo->sEngine[szEngine];
			}

			if( szCannon < 0 )
			{
				szCannon = pInfo->sNumCannon - 1;
				Data.sCannon = pInfo->sCannon[szCannon];
			}
			else 
			{
				if( szCannon >= pInfo->sNumCannon )
				{
					szCannon = 0;
				}
				Data.sCannon = pInfo->sCannon[szCannon];
			}

			if( szEquipment < 0 )
			{
				szEquipment = pInfo->sNumEquipment - 1;
				Data.sEquipment = pInfo->sEquipment[szEquipment];
			}
			else 
			{
				if( szEquipment >= pInfo->sNumEquipment )
				{
					szEquipment = 0;
				}
				Data.sEquipment = pInfo->sEquipment[szEquipment];
			}
		}
		else
		{
			if( pInfo->sHeader[0] != USHORT(-1) )
			{
				Data.sHeader = pInfo->sHeader[0];
			}
			if( pInfo->sEngine[0] != USHORT(-1) )
			{
				Data.sEngine = pInfo->sEngine[0];
			}
			if( pInfo->sCannon[0] != USHORT(-1) )
			{
				Data.sCannon = pInfo->sCannon[0];
			}
			if( pInfo->sEquipment[0] != USHORT(-1) )
			{
				Data.sEquipment = pInfo->sEquipment[0];
			}
		}
		
		// ���㴬ֻ������Լӳ���Ϣ
		xShipAttrInfo Info;
		memset( &Info, 0, sizeof(xShipAttrInfo) );
		if( !GetData( owner, pInfo->byIsUpdate, Data, Info ) )
		{
			pBoat->Free();
			owner.SetBoat( NULL );
			//owner.SystemNotice( "���촬ֻ����������Լӳ���Ϣ����" );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00023) );
			return FALSE;
		}

		if( !owner.TakeMoney( "", Info.dwMoney ) )
		{
			pBoat->Free();
			owner.SetBoat( NULL );
			//owner.SystemNotice( "���촬ֻ%s��Ҫ��Ǯ%d����ȷ�����Ƿ����㹻��Ǯ��", pInfo->szName, Info.dwMoney );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00024), pInfo->szName, Info.dwMoney );
			return TRUE;
		}

		DWORD dwBoatID;
		if( !game_db.Create( dwBoatID, Data ) )
		{
			pBoat->Free();
			owner.SetBoat( NULL );
			//owner.SystemNotice( "���촬ֻ���ݿ�洢ʧ�ܣ�" );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00025) );
			return FALSE;
		}

		pBoat->m_CKitbag.SetCapacity( Data.sCapacity );

		// ��鴬��֤��������Ϣ
		SItemGrid SGridCont;
		SGridCont.sID = pInfo->sItemID;
		SGridCont.sNum = 1;
		owner.ItemInstance( enumITEM_INST_TASK, &SGridCont );
		SGridCont.lDBParam[enumITEMDBP_INST_ID] = dwBoatID;

		// ����ʵ������Ʒ
		owner.m_CKitbag.SetChangeFlag(false);
		Short sPushPos = defKITBAG_DEFPUSH_POS;
		Short sPushRet = owner.KbPushItem( false, false, &SGridCont, sPushPos );
		if( sPushRet == enumKBACT_ERROR_LOCK ) // ������������
		{
			pBoat->Free();
			owner.SetBoat( NULL );
			//owner.BickerNotice( "�������ѱ����������촬ֻʧ�ܣ�" );
			owner.BickerNotice( RES_STRING(GM_CHARBOAT_CPP_00026) );
			return FALSE;
		}
		else if( sPushRet == enumKBACT_ERROR_PUSHITEMID ) // ���߲�����
		{
			pBoat->Free();
			owner.SetBoat( NULL );
			//owner.BickerNotice( "�õ��߲����ڣ����촬ֻʧ�ܣ�" );
			owner.BickerNotice( RES_STRING(GM_CHARBOAT_CPP_00027) );
			return FALSE;
		}
		else if( sPushRet == enumKBACT_ERROR_FULL ) // ��������������������
		{
			pBoat->Free();
			owner.SetBoat( NULL );
			//owner.BickerNotice( "���������������촬ֻʧ�ܣ�" );
			owner.BickerNotice( RES_STRING(GM_CHARBOAT_CPP_00012) );
			return FALSE;
		}

		// ���ô�ֻ��֤��ID
		pBoat->setAttr( ATTR_CHATYPE, enumCHACTRL_PLAYER );		
		pBoat->setAttr( ATTR_BOAT_DBID, SGridCont.lDBParam[enumITEMDBP_INST_ID] );
		pBoat->setAttr( ATTR_BOAT_DIECOUNT, 0 );
		pBoat->setAttr( ATTR_BOAT_ISDEAD, 0 );

		pBoat->setAttr( ATTR_BOAT_HEADER, Data.sHeader );
		pBoat->setAttr( ATTR_BOAT_BODY, Data.sBody );
		pBoat->setAttr( ATTR_BOAT_ENGINE, Data.sEngine );
		pBoat->setAttr( ATTR_BOAT_CANNON, Data.sCannon );
		pBoat->setAttr( ATTR_BOAT_PART, Data.sEquipment );

		pBoat->SetName( Data.szName );
		pBoat->setAttr( ATTR_LV, 1 );

		// ��ȡ��ֻ��ɫ������Ϣ
		pBoat->SetCat( pInfo->sCharID );
		CChaRecord* pRec = GetChaRecordInfo( pInfo->sCharID );
		if( pRec == NULL )
		{
			//owner.SystemNotice( "������ֻ���޷��õ���Ч������Ϣ��ID[%d]", pInfo->sCharID );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00028), pInfo->sCharID );
			pBoat->Free();
			owner.SetBoat( NULL );
			return FALSE;
		}
		pBoat->m_pCChaRecord = pRec;
		pBoat->m_CChaAttr.Init( pInfo->sCharID, FALSE );
		pBoat->SetID(g_pGameApp->m_Ident.GetID());
		pBoat->SetRadius(pBoat->m_pCChaRecord->sRadii);
		pBoat->SetShip(g_pGameApp->m_CabinHeap.Get());
		pBoat->setAttr(ATTR_CHATYPE, enumCHACTRL_PLAYER);
		pBoat->EnrichSkillBag();

		if( !SetPartData( *pBoat, pInfo->sCharID, Data ) )
		{
			pBoat->Free();
			owner.SetBoat( NULL );
			//owner.SystemNotice( "������ֻʧ�ܣ����ô�ֻ�������ʧ��!ID[%d]", Data.sBoat );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00029), Data.sBoat );
			return FALSE;
		}

		if( !SetBoatAttr( owner, *pBoat, *pInfo, Data ) )
		{
			pBoat->Free();
			owner.SetBoat( NULL );
			//owner.SystemNotice( "���ô�ֻ����ʧ�ܣ�" );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00030) );
			return FALSE;
		}

		pBoat->SetPlayer( owner.GetPlayer() );
		if( !game_db.SaveBoat( *pBoat, enumSAVE_TYPE_TIMER ) )
		{
			//owner.SystemNotice( "��ɫ%s�Ĵ�ֻ%s��ʱ���ݴ洢ʧ�ܣ�", owner.GetName(), pBoat->GetName() );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00031), owner.GetName(), pBoat->GetName() );
			//LG( "boat_error", "��ɫ%s�Ĵ�ֻ%s��ʱ���ݴ洢ʧ�ܣ�", owner.GetName(), pBoat->GetName() ); 
			LG( "boat_error", "character%s boat %s temporary data memory failed��", owner.GetName(), pBoat->GetName() ); 
		}

		if( !pPlayer->AddBoat( *pBoat ) )
		{
			pBoat->Free();
			owner.SetBoat( NULL );
			//owner.SystemNotice( "��Ӵ�ֻ���ݵ�player��ʧ�ܣ�" );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00032) );
			return FALSE;
		}

		owner.SynKitbagNew( enumSYN_KITBAG_SYSTEM );

		// ������ʱ�촬����Ϊ��
		owner.SetBoat( NULL );
		
		Char szLogName[defLOG_NAME_LEN] = "";
		sprintf(szLogName, "Cha-%s+%u", pBoat->GetName(), pBoat->GetID());
		pBoat->m_CLog.SetLogName(szLogName);
		pBoat->m_CLog.SetEnable(g_bLogEntity);

		return TRUE;
	T_E}

	BOOL CCharBoat::SetBoatAttr( CCharacter& owner, CCharacter& boat, const xShipInfo& ShipInfo, const BOAT_DATA& Data, bool bFromFile, bool bLoadState )
	{T_B
		// ��ֻ�Ƿ���Ը������
		xShipAttrInfo Info;
		memset( &Info, 0, sizeof(xShipAttrInfo) );
		Info.dwMinAttack = ShipInfo.sMinAttack;
		Info.dwMaxAttack = ShipInfo.sMaxAttack;
		Info.dwCurEndure = ShipInfo.sEndure;
		Info.dwMaxEndure = ShipInfo.sEndure;
		Info.dwSpeed = ShipInfo.sSpeed;
		Info.dwDistance = ShipInfo.sDistance;
		Info.dwDefence = ShipInfo.sDefence;
		Info.dwCurSupply = ShipInfo.sSupply;
		Info.dwMaxSupply = ShipInfo.sSupply;
		Info.dwAttackTime = ShipInfo.sTime;
		Info.sCapacity = ShipInfo.sCapacity;
		
		Info.dwResume = ShipInfo.sResume;
		Info.dwConsume = ShipInfo.sConsume;
		Info.dwResist = ShipInfo.sResist;
		Info.dwScope = ShipInfo.sScope;
		Info.dwCannonSpeed = ShipInfo.sCannonSpeed;

		// ���㴬ֻ������Լӳ���Ϣ
		if( !GetData( owner, ShipInfo.byIsUpdate, Data, Info ) )
		{
			return FALSE;
		}
		
		boat.setAttr( ATTR_BMNATK, Info.dwMinAttack );
		boat.setAttr( ATTR_BMXATK, Info.dwMaxAttack );
		if (bFromFile)
		{
			boat.setAttr( ATTR_HP, Info.dwCurEndure );
			boat.setAttr( ATTR_BMXHP, Info.dwMaxEndure );
			boat.setAttr( ATTR_SP, Info.dwMaxSupply );
			boat.setAttr( ATTR_BMXSP, Info.dwMaxSupply );			
		}
		
		if( boat.getAttr( ATTR_HP ) < 0 )
		{
			//LG( "boatattr_error", "SetBoatAttr:��ֻ��ǰHP��ֵ����ȷ���Զ��ָ���HP = %d", boat.getAttr( ATTR_HP ) );
			LG( "boatattr_error", "SetBoatAttr: boat currently HP value error��automatism resume��HP = %d", boat.getAttr( ATTR_HP ) );
			boat.setAttr( ATTR_HP, 1 );
		}

		if( boat.getAttr( ATTR_SP ) < 0 )
		{
			//LG( "boatattr_error", "SetBoatAttr:��ֻ��ǰSP��ֵ����ȷ���Զ��ָ���SP = %d", boat.getAttr( ATTR_SP ) );
			LG( "boatattr_error", "SetBoatAttr: boat currently SP value error��automatism resume��SP = %d", boat.getAttr( ATTR_SP ) );
			boat.setAttr( ATTR_SP, 1 );
		}

		if( boat.getAttr( ATTR_BMXSP ) <= 1 || boat.getAttr( ATTR_BMXHP ) <= 1 )
		{
			/*LG( "boatattr_error", "SetBoatAttr:��ֻ���HP����SP��ֵ����ȷ���Զ��ָ���MXHP = %d, MXSP = %d", 
				boat.getAttr( ATTR_BMXHP ), boat.getAttr( ATTR_BMXSP ) );*/
			LG( "boatattr_error", "SetBoatAttr: boat max HP or SP value error��automatism��MXHP = %d, MXSP = %d", 
				boat.getAttr( ATTR_BMXHP ), boat.getAttr( ATTR_BMXSP ) );
			boat.setAttr( ATTR_BMXSP, Info.dwMaxSupply );
			boat.setAttr( ATTR_BMXHP, Info.dwMaxEndure );
		}

		boat.setAttr( ATTR_BMSPD, Info.dwSpeed );
		boat.setAttr( ATTR_BADIS, Info.dwDistance );
		boat.setAttr( ATTR_BDEF, Info.dwDefence );
		boat.setAttr( ATTR_BASPD, Info.dwAttackTime );
		boat.m_CKitbag.SetCapacity( Info.sCapacity );

		boat.setAttr( ATTR_BOAT_PRICE, Info.dwMoney );
		boat.setAttr( ATTR_BHREC, Info.dwResume );
		boat.setAttr( ATTR_BSREC, Info.dwConsume );
		boat.setAttr( ATTR_BOAT_CRANGE, Info.dwScope );
		boat.setAttr( ATTR_BOAT_CSPD, Info.dwCannonSpeed );
		boat.setAttr( ATTR_BASPD, Info.dwAttackTime );

		if (bLoadState)
			Strin2SStateData(&boat, g_strChaState[1]);

		g_CParser.DoString( "Ship_ExAttrCheck", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 2, &owner, &boat, DOSTRING_PARAM_END );

		return TRUE;
	T_E}

	// chType��0����ɫ���ߡ�1����ͼ�л�
	BOOL CCharBoat::LoadBoat( CCharacter& owner, char chType )
	{T_B
		USHORT sNum = owner.m_CKitbag.GetUseGridNum();
		SItemGrid	*pGridCont;
		for( int i = 0; i < sNum; i++ )
		{
			pGridCont = owner.m_CKitbag.GetGridContByNum( i );
			if( pGridCont )
			{
				CItemRecord* pItem = GetItemRecordInfo( pGridCont->sID );
				if( !pItem )
				{
					/*owner.SystemNotice( "δ���ֽ�ɫ%s�����е�%d�����Ʒ��ϢID[%d]��", owner.GetName(), 
						pGridCont->sID );*/
					owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00033), owner.GetName(), 
						pGridCont->sID );
					continue;
				}
				
				if( pItem->sType == enumItemTypeBoat )
				{
					if(!CreateBoat( owner, pGridCont->GetDBParam( enumITEMDBP_INST_ID ), chType ))
                    {
                        return FALSE;
                    }
					if( owner.GetPlayer()->IsBoatFull() )
					{
						return TRUE;
					}
				}
			}
		}
		return TRUE;
	T_E}
	
	CCharacter* CCharBoat::SummonBoat( USHORT sBoatID )
	{T_B
		CCharacter* pBoat = g_pGameApp->GetNewCharacter();
		if( pBoat == NULL )
		{
			//LG( "monsterboat_error", "��ɫ����װ�ش�ֻ���䴬ֻ�ڴ�ʧ��!" );
			LG( "monsterboat_error", "when character online,loading boat and assign memory failed " );
			return FALSE;
		}

		pBoat->setAttr( ATTR_BOAT_DBID, -1 );
		pBoat->setAttr( ATTR_CHATYPE, enumCHACTRL_MONS );

		BOAT_DATA Info;
		memset( &Info, 0, sizeof(BOAT_DATA) );
		// Info.sBerth = (USHORT)pBoat->getAttr( ATTR_BOAT_BERTH );
		Info.sBoat = sBoatID;

		// ���ô��Ļ�������
		xShipInfo* pInfo = (xShipInfo*)m_pShipSet->GetRawDataInfo( Info.sBoat );
		if( pInfo == NULL ) 
		{
			//LG( "monsterboat_error", "������ֻʧ�ܣ�����Ĵ�ֻ��ϢID[%d]!", Info.sBoat );
			LG( "monsterboat_error", "craete boat failed , error information of boat ID[%d]!", Info.sBoat );
			return FALSE;
		}
		pBoat->SetName( pInfo->szName );

		BYTE byIndex = 0;
		if( pInfo->sNumHeader > 1 )
		{
			byIndex = rand()%pInfo->sNumHeader;
		}
		Info.sHeader = pInfo->sHeader[byIndex];
		Info.sBody = pInfo->sBody;

		byIndex = 0;
		if( pInfo->sNumEngine > 1 )
		{
			byIndex = rand()%pInfo->sNumEngine;
		}
		Info.sEngine = pInfo->sEngine[byIndex];

		byIndex = 0;
		if( pInfo->sNumCannon > 1 )
		{
			byIndex = rand()%pInfo->sNumCannon;
		}
		Info.sCannon = pInfo->sCannon[byIndex];

		byIndex = 0;
		if( pInfo->sNumEquipment > 1 )
		{
			byIndex = rand()%pInfo->sNumEquipment;
		}
		Info.sEquipment = pInfo->sEquipment[byIndex];

		// ��ȡ��ֻ��ɫ������Ϣ
		CChaRecord* pRec = GetChaRecordInfo( pInfo->sCharID );
		if( pRec == NULL )
		{
			//LG( "monsterboat_error", "������ֻ���޷��õ���Ч������Ϣ��ID[%d]", pInfo->sCharID );
			LG( "monsterboat_error", "create boat��cannot get efficiency attribute information��ID[%d]", pInfo->sCharID );
			pBoat->Free();
			return FALSE;
		}
		pBoat->m_pCChaRecord = pRec;		

		//LG( "monsterboat_init", "�ɹ�װ�ش�ֻ��%s��", pBoat->GetName() );
		LG( "monsterboat_init", "succeed loading boat��%s��", pBoat->GetName() );

		// ��ֻ�Ƿ���Ը������
		xShipAttrInfo Data;
		memset( &Data, 0, sizeof(xShipAttrInfo) );
		Data.dwMinAttack = pInfo->sMinAttack;
		Data.dwMaxAttack = pInfo->sMaxAttack;
		Data.dwCurEndure = pInfo->sEndure;
		Data.dwMaxEndure = pInfo->sEndure;
		Data.dwSpeed = pInfo->sSpeed;
		Data.dwDistance = pInfo->sDistance;
		Data.dwDefence = pInfo->sDefence;
		Data.dwCurSupply = pInfo->sSupply;
		Data.dwMaxSupply = pInfo->sSupply;
		Data.dwAttackTime = pInfo->sTime;
		Data.sCapacity = pInfo->sCapacity;
		
		Data.dwResume = pInfo->sResume;
		Data.dwResist = pInfo->sResist;
		Data.dwScope = pInfo->sScope;
		Data.dwCannonSpeed = pInfo->sCannonSpeed;

		// ���㴬ֻ������Լӳ���Ϣ
		if( !GetData( *pBoat, pInfo->byIsUpdate, Info, Data ) )
		{
			return FALSE;
		}

		pBoat->setAttr( ATTR_MNATK, Data.dwMinAttack );
		pBoat->setAttr( ATTR_BMXATK, Data.dwMaxAttack );
		pBoat->setAttr( ATTR_HP, Data.dwCurEndure );
		pBoat->setAttr( ATTR_MXHP, Data.dwMaxEndure );
		pBoat->setAttr( ATTR_BMSPD, Data.dwSpeed );
		pBoat->setAttr( ATTR_BADIS, Data.dwDistance );
		pBoat->setAttr( ATTR_BDEF, Data.dwDefence );
		pBoat->setAttr( ATTR_BMXSP, Data.dwCurSupply );
		pBoat->setAttr( ATTR_BMXSP, Data.dwMaxSupply );
		pBoat->setAttr( ATTR_BASPD, Data.dwAttackTime );
		pBoat->m_CKitbag.SetCapacity( Data.sCapacity );

		pBoat->setAttr( ATTR_BOAT_PRICE, Data.dwMoney );
		pBoat->setAttr( ATTR_BHREC, Data.dwResume );
		pBoat->setAttr( ATTR_BSREC, Data.dwConsume );
		pBoat->setAttr( ATTR_BOAT_CRANGE, Data.dwScope );
		pBoat->setAttr( ATTR_BOAT_CSPD, Data.dwCannonSpeed );
		pBoat->setAttr( ATTR_BASPD, Data.dwAttackTime );
	
		return NULL;
	T_E}

	// chType��0����ɫ���ߡ�1����ͼ�л���2����Ϸ��ͨ��
	BOOL CCharBoat::CreateBoat( CCharacter& owner, DWORD dwBoatID, char chType )
	{T_B
		CCharacter* pBoat = g_pGameApp->GetNewCharacter();
		if( pBoat == NULL )
		{
			//owner.SystemNotice( "��ɫ����װ�ش�ֻ���䴬ֻ�ڴ�ʧ��!" );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00034) );
			//LG( "boat_error", "��ɫ%s����װ�ش�ֻ���䴬ֻID[%d]�ڴ�ʧ��!", owner.GetName(), dwBoatID );
			LG( "boat_error", "character %s go up loading boat and assign boatID[%d]memory failed!", owner.GetName(), dwBoatID );
			return FALSE;
		}
		pBoat->setAttr( ATTR_BOAT_DBID, dwBoatID );
		pBoat->setAttr( ATTR_CHATYPE, enumCHACTRL_PLAYER );

		if( !game_db.GetBoat( *pBoat ) )
		{
			pBoat->Free();
			//owner.SystemNotice( "��ȡ��ɫ%s��ֻID[%d]���ݿ�����ʧ�ܣ�", owner.GetName(), dwBoatID );
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00035), owner.GetName(), dwBoatID );
			//LG( "boat_error", "��ȡ��ɫ%s��ֻID[%d]���ݿ�����ʧ�ܣ�\n", owner.GetName(), dwBoatID );
			LG( "boat_error", "get character %s boat ID[%d]DB data failed��\n", owner.GetName(), dwBoatID );
			return FALSE;
		}

		BOAT_DATA Info;
		memset( &Info, 0, sizeof(BOAT_DATA) );
		// Info.sBerth = (USHORT)pBoat->getAttr( ATTR_BOAT_BERTH );
		Info.sBoat = (USHORT)pBoat->getAttr( ATTR_BOAT_SHIP );
		Info.sHeader = (USHORT)pBoat->getAttr( ATTR_BOAT_HEADER );
		Info.sBody = (USHORT)pBoat->getAttr( ATTR_BOAT_BODY );
		Info.sEngine = (USHORT)pBoat->getAttr( ATTR_BOAT_ENGINE );
		Info.sCannon = (USHORT)pBoat->getAttr( ATTR_BOAT_CANNON );
		Info.sEquipment = (USHORT)pBoat->getAttr( ATTR_BOAT_PART );

		// ���ô��Ļ�������
		xShipInfo* pInfo = (xShipInfo*)m_pShipSet->GetRawDataInfo( Info.sBoat );
		if( pInfo == NULL ) 
		{
			pBoat->Free();
			/*owner.SystemNotice( "������ֻʧ�ܣ�����Ĵ�ֻ��ϢID[%d]!", Info.sBoat );
			LG( "boat_error", "������ֻʧ�ܣ�����Ĵ�ֻ��ϢID[%d]!", Info.sBoat );*/
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00010), Info.sBoat );
			LG( "boat_error", "create boat failed��error information of boat ID[%d]!", Info.sBoat );
			return FALSE;
		}

		// ��ȡ��ֻ��ɫ������Ϣ
		pBoat->SetCat( pInfo->sCharID );
		CChaRecord* pRec = GetChaRecordInfo( pInfo->sCharID );
		if( pRec == NULL )
		{
			pBoat->Free();
			/*owner.SystemNotice( "������ֻ���޷��õ���Ч������Ϣ��ID[%d]", pInfo->sCharID );
			LG( "boat_error", "������ֻ���޷��õ���Ч������Ϣ��ID[%d]", pInfo->sCharID );*/
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00028), pInfo->sCharID );
			LG( "boat_error", "create boat,cannot get efficiency attribute information��ID[%d]", pInfo->sCharID );
			return FALSE;
		}
		pBoat->m_pCChaRecord = pRec;
		pBoat->m_CChaAttr.Init( pInfo->sCharID, FALSE );
		pBoat->SetID(g_pGameApp->m_Ident.GetID());
		pBoat->SetRadius(pBoat->m_pCChaRecord->sRadii);
		pBoat->SetShip(g_pGameApp->m_CabinHeap.Get());
		pBoat->setAttr(ATTR_CHATYPE, enumCHACTRL_PLAYER);
		pBoat->EnrichSkillBag();

		if( !SetPartData( *pBoat, pInfo->sCharID, Info ) )
		{
			pBoat->Free();
			/*owner.SystemNotice( "������ֻʧ�ܣ����ô�ֻ�������ʧ��!ID[%d]", Info.sBoat );
			LG( "boat_error", "������ֻʧ�ܣ����ô�ֻ�������ʧ��!ID[%d]", Info.sBoat );*/
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00029), Info.sBoat );
			LG( "boat_error", "create boat failed,set boat surface data failed!ID[%d]", Info.sBoat );
			return FALSE;
		}

		bool	bBoatState = false;
		if (chType == 1)
			bBoatState = true;
		else
			pBoat->m_CSkillState.Reset();
		if( !SetBoatAttr( owner, *pBoat, *pInfo, Info, false ) )
		{
			pBoat->Free();
			/*owner.SystemNotice( "���ô�ֻ����ʧ�ܣ�" );
			LG( "boat_error", "���ý�ɫ%s��ֻ����ʧ��,��ֻ��%s��ID[%d]����ɫplayer������Ϣʧ�ܣ�", 
				owner.GetName(), pBoat->GetName(), dwBoatID );*/
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00030) );
			LG( "boat_error", "set character %s boat attribute failed,boat��%s��ID[%d] to character player data information failed��", 
				owner.GetName(), pBoat->GetName(), dwBoatID );
			return FALSE;
		}
		
		if( !owner.GetPlayer()->AddBoat( *pBoat ) )
		{
			pBoat->Free();
			/*owner.SystemNotice( "��Ӵ�ֻ��%s��ID[%d]����ɫ%splayer������Ϣʧ�ܣ�", 
				pBoat->GetName(), dwBoatID, owner.GetName() );
			LG( "boat_error", "��Ӵ�ֻ��%s��ID[%d]����ɫ%splayer������Ϣʧ�ܣ�", 
				pBoat->GetName(), dwBoatID, owner.GetName() );*/
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00036), 
				pBoat->GetName(), dwBoatID, owner.GetName() );
			LG( "boat_error", "add boat��%s��ID[%d] to character %s player data information failed��", 
				pBoat->GetName(), dwBoatID, owner.GetName() );
			return FALSE;
		}

		/*owner.SystemNotice( "�ɹ�װ�ص�%d�Ҵ�ֻ��%s��", owner.GetPlayer()->GetNumBoat(), 
			pBoat->GetName() );*/
		owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00037), owner.GetPlayer()->GetNumBoat(), 
			pBoat->GetName() );

		Char szLogName[defLOG_NAME_LEN] = "";
		sprintf(szLogName, "Cha-%s+%u", pBoat->GetName(), pBoat->GetID());
		pBoat->m_CLog.SetLogName(szLogName);
		pBoat->m_CLog.SetEnable(g_bLogEntity);

		return TRUE;
	T_E}

	BOOL CCharBoat::GetData( CCharacter& owner, BYTE byIsUpdate, const BOAT_DATA& Info, xShipAttrInfo& Data )
	{
		// ��ֻ�Ƿ���Ը������
		xShipPartInfo* pData = (xShipPartInfo*)m_pShipPartSet->GetRawDataInfo( Info.sBody );
		if( pData == NULL ) 
		{
			/*owner.SystemNotice( "������ֻʧ�ܣ�����Ĵ�����ϢID[%d]!", Info.sBody );
			LG( "boat_error", "������ֻʧ�ܣ�����Ĵ�����ϢID[%d]!", Info.sBody );*/
			owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00038), Info.sBody );
			LG( "boat_error", "create boat failed��error information of hull ID[%d]!", Info.sBody );
		}
		else
		{
			Data.dwMinAttack += pData->sMinAttack;
			Data.dwMaxAttack += pData->sMaxAttack;
			Data.dwCurEndure += pData->sEndure;
			Data.dwMaxEndure += pData->sEndure;
			Data.dwSpeed += pData->sSpeed;
			Data.dwDistance += pData->sDistance;
			Data.dwDefence += pData->sDefence;
			Data.dwCurSupply += pData->sSupply;
			Data.dwMaxSupply += pData->sSupply;
			Data.dwAttackTime += pData->sTime;
			Data.sCapacity += pData->sCapacity;

			Data.dwResume += pData->sResume;
			Data.dwConsume += pData->sConsume;
			Data.dwResist += pData->sResist;
			Data.dwScope += pData->sScope;
			Data.dwCannonSpeed += pData->sCannonSpeed;

			Data.dwMoney += pData->dwPrice;
		}

		if( byIsUpdate )
		{
			pData = (xShipPartInfo*)m_pShipPartSet->GetRawDataInfo( Info.sHeader );
			if( pData == NULL ) 
			{
				/*owner.SystemNotice( "������ֻʧ�ܣ�����Ĵ�ͷ��ϢID[%d]!", Info.sHeader );
				LG( "boat_error", "������ֻʧ�ܣ�����Ĵ�ͷ��ϢID[%d]!", Info.sHeader );*/
				owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00039), Info.sHeader );
				LG( "boat_error", "create boat failed��error information of fore ID[%d]!", Info.sHeader );
			}
			else
			{
				Data.dwMinAttack += pData->sMinAttack;
				Data.dwMaxAttack += pData->sMaxAttack;
				Data.dwCurEndure += pData->sEndure;
				Data.dwMaxEndure += pData->sEndure;
				Data.dwSpeed += pData->sSpeed;
				Data.dwDistance += pData->sDistance;
				Data.dwDefence += pData->sDefence;
				Data.dwCurSupply += pData->sSupply;
				Data.dwMaxSupply += pData->sSupply;
				Data.dwAttackTime += pData->sTime;
				Data.sCapacity += pData->sCapacity;

				Data.dwResume += pData->sResume;
				Data.dwConsume += pData->sConsume;
				Data.dwResist += pData->sResist;
				Data.dwScope += pData->sScope;
				Data.dwCannonSpeed += pData->sCannonSpeed;

				Data.dwMoney += pData->dwPrice;
			}

			pData = (xShipPartInfo*)m_pShipPartSet->GetRawDataInfo( Info.sEngine );
			if( pData == NULL ) 
			{
				/*owner.SystemNotice( "������ֻʧ�ܣ�����Ĵ�������ϢID[%d]!", Info.sEngine );
				LG( "boat_error", "������ֻʧ�ܣ�����Ĵ�������ϢID[%d]!", Info.sEngine );*/
				owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00040), Info.sEngine );
				LG( "boat_error", "create boat failed��error information of boat drive ID[%d]!", Info.sEngine );
			}
			else
			{
				Data.dwMinAttack += pData->sMinAttack;
				Data.dwMaxAttack += pData->sMaxAttack;
				Data.dwCurEndure += pData->sEndure;
				Data.dwMaxEndure += pData->sEndure;
				Data.dwSpeed += pData->sSpeed;
				Data.dwDistance += pData->sDistance;
				Data.dwDefence += pData->sDefence;
				Data.dwCurSupply += pData->sSupply;
				Data.dwMaxSupply += pData->sSupply;
				Data.dwAttackTime += pData->sTime;
				Data.sCapacity += pData->sCapacity;

				Data.dwResume += pData->sResume;
				Data.dwConsume += pData->sConsume;
				Data.dwResist += pData->sResist;
				Data.dwScope += pData->sScope;
				Data.dwCannonSpeed += pData->sCannonSpeed;

				Data.dwMoney += pData->dwPrice;
			}
			for( int i = 0; i < BOAT_MAXNUM_MOTOR; i++ )
			{
				xShipPartInfo* pMotorData = (xShipPartInfo*)m_pShipPartSet->GetRawDataInfo( pData->sMotor[i] );
				if( pMotorData == NULL ) 
				{
					break;
				}
				else
				{
					Data.dwMinAttack += pMotorData->sMinAttack;
					Data.dwMaxAttack += pMotorData->sMaxAttack;
					Data.dwCurEndure += pMotorData->sEndure;
					Data.dwMaxEndure += pMotorData->sEndure;
					Data.dwSpeed += pMotorData->sSpeed;
					Data.dwDistance += pMotorData->sDistance;
					Data.dwDefence += pMotorData->sDefence;
					Data.dwCurSupply += pMotorData->sSupply;
					Data.dwMaxSupply += pMotorData->sSupply;
					Data.dwAttackTime += pMotorData->sTime;
					Data.sCapacity += pMotorData->sCapacity;

					Data.dwResume += pMotorData->sResume;
					Data.dwConsume += pMotorData->sConsume;
					Data.dwResist += pMotorData->sResist;
					Data.dwScope += pMotorData->sScope;
					Data.dwCannonSpeed += pMotorData->sCannonSpeed;

					Data.dwMoney += pMotorData->dwPrice;
				}
			}

			pData = (xShipPartInfo*)m_pShipPartSet->GetRawDataInfo( Info.sCannon );
			if( pData == NULL ) 
			{
				/*owner.SystemNotice( "������ֻʧ�ܣ�����Ĵ�������ϢID[%d]!", Info.sCannon );
				LG( "boat_error", "������ֻʧ�ܣ�����Ĵ�������ϢID[%d]!", Info.sCannon );*/
				owner.SystemNotice( RES_STRING(GM_CHARBOAT_CPP_00041), Info.sCannon );
				LG( "boat_error", "create boat failed��error information of boat artillery ID[%d]!", Info.sCannon );
			}
			else
			{
				Data.dwMinAttack += pData->sMinAttack;
				Data.dwMaxAttack += pData->sMaxAttack;
				Data.dwCurEndure += pData->sEndure;
				Data.dwMaxEndure += pData->sEndure;
				Data.dwSpeed += pData->sSpeed;
				Data.dwDistance += pData->sDistance;
				Data.dwDefence += pData->sDefence;
				Data.dwCurSupply += pData->sSupply;
				Data.dwMaxSupply += pData->sSupply;
				Data.dwAttackTime += pData->sTime;
				Data.sCapacity += pData->sCapacity;

				Data.dwResume += pData->sResume;
				Data.dwConsume += pData->sConsume;
				Data.dwResist += pData->sResist;
				Data.dwScope += pData->sScope;
				Data.dwCannonSpeed += pData->sCannonSpeed;

				Data.dwMoney += pData->dwPrice;
			}

			pData = (xShipPartInfo*)m_pShipPartSet->GetRawDataInfo( Info.sEquipment );
			if( pData == NULL ) 
			{
				//owner.SystemNotice( "������ֻʧ�ܣ�����Ĵ������ϢID[%d]!", Info.sEquipment );
				//return FALSE;
			}
			else
			{
				Data.dwMinAttack += pData->sMinAttack;
				Data.dwMaxAttack += pData->sMaxAttack;
				Data.dwCurEndure += pData->sEndure;
				Data.dwMaxEndure += pData->sEndure;
				Data.dwSpeed += pData->sSpeed;
				Data.dwDistance += pData->sDistance;
				Data.dwDefence += pData->sDefence;
				Data.dwCurSupply += pData->sSupply;
				Data.dwMaxSupply += pData->sSupply;
				Data.dwAttackTime += pData->sTime;
				Data.sCapacity += pData->sCapacity;

				Data.dwResume += pData->sResume;
				Data.dwConsume += pData->sConsume;
				Data.dwResist += pData->sResist;
				Data.dwScope += pData->sScope;
				Data.dwCannonSpeed += pData->sCannonSpeed;

				Data.dwMoney += pData->dwPrice;
			}
		}
		return TRUE;
	}

}
