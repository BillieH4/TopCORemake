// CharForge.cpp Created by knight-gongjian 2004.12.7.
//---------------------------------------------------------

#include "CharForge.h"
#include <ForgeRecord.h>
#include "GameAppNet.h"
#include "Player.h"

//---------------------------------------------------------

mission::CForgeSystem g_ForgeSystem;

namespace mission
{

	CForgeSystem::CForgeSystem()
	{
		m_pRecordSet = NULL;
	}

	CForgeSystem::~CForgeSystem()
	{
		Clear();
	}

	void CForgeSystem::Clear()
	{
		SAFE_DELETE( m_pRecordSet );
	}

	BOOL CForgeSystem::LoadForgeData( char szName[] )
	{
		Clear();
		
		extern BOOL LoadTable(CRawDataSet *pTable, const char*);
		m_pRecordSet = new CForgeRecordSet( 1, ROLE_MAXNUM_FORGE );
		return LoadTable(m_pRecordSet, szName);
	}

	void CForgeSystem::ForgeItem( CCharacter& character, BYTE byIndex )
	{
		// �ж��Ƿ��ڽ���״̬
		if( character.m_CKitbag.IsLock() )
		{
			character.SystemNotice( "��ı����ѱ������������Ծ�����Ʒ��" );
			return;
		}

        //��������
        if( character.m_CKitbag.IsPwdLocked() )
        {
            character.SystemNotice( "��ı����ѱ����������������Ծ�����Ʒ��" );
			return;
        }
		//add by ALLEN 2007-10-16
				if( character.IsReadBook() )
        {
            character.SystemNotice( "���ڶ��飬�����Ծ�����Ʒ��" );
			return;
        }
		SItemGrid *pItemData;
		if( !(pItemData = character.m_CKitbag.GetGridContByID( byIndex )) )
		{
			character.SystemNotice( "ForgeItem:����ı�����λ������ ID = %d", byIndex );
			return;
		}
		
		CItemRecord* pItem = GetItemRecordInfo( pItemData->sID );
		if( pItem == NULL )
		{
			character.SystemNotice( "ForgeItem:����ľ�����Ʒ�� ID = %d", pItemData->sID );
			return;
		}

		if( pItem->chForgeLv == 0 )
		{
			character.SystemNotice( "��Ʒ��%s�������Ծ�����", pItem->szName );
			return;
		}

		BYTE byLevel = pItemData->chForgeLv;
		if( byLevel >= ROLE_MAXNUM_FORGE )
		{
			character.SystemNotice( "��ġ�%s���Ѿ��Ǿ�������װ����", pItem->szName );
			return;
		}
		
		// ��������һ��
		byLevel++;

		CForgeRecord* pRecord = (CForgeRecord*)m_pRecordSet->GetRawDataInfo( byLevel );
		if( !pRecord )
		{
			character.SystemNotice( "ForgeItem:��Ч�ľ����ȼ���Level = %d", byLevel );
			return;
		}

		if( !character.HasMoney( pRecord->dwMoney ) )
		{
			character.SystemNotice( "�������������Ǯ���㣬����ʧ�ܣ�" );
			return;
		}

		for( int i = 0; i < FORGE_MAXNUM_ITEM; i++ )
		{
			if( pRecord->ForgeItem[i].sItem == 0 )
				break;
			if( !character.HasItem( pRecord->ForgeItem[i].sItem, pRecord->ForgeItem[i].byNum ) )
			{
				char szForgeItem[64] = "δ֪";
				CItemRecord* pForgeItem = (CItemRecord*)GetItemRecordInfo( pRecord->ForgeItem[i].sItem );
				if( pForgeItem )
				{
					strcpy( szForgeItem, pForgeItem->szName );
				}
				character.SystemNotice( "ȱ�پ���������Ʒ��%s��������%d��������ʧ�ܣ�", szForgeItem, pRecord->ForgeItem[i].byNum );
				return;
			}
		}

		BOOL bSuccess = TRUE;
		// �ж��Ƿ񳬳�
		if( byLevel > pItem->chForgeLv )
		{
			// ����������Ʒ�������ȼ�ʧ�ܣ��ͷ�
			bSuccess = FALSE;
		}
		else
		{
			// �ж��Ƿ� ������������ֵ
			if( byLevel > pItem->chForgeSteady )
			{
				// 
				if( rand()%100 >= pRecord->byRate )
				{
					// ������Ʒ������ʧ�ܣ��ͷ�
					bSuccess = FALSE;
				}
			}
		}

		for( int i = 0; i < FORGE_MAXNUM_ITEM; i++ )
		{
			if( pRecord->ForgeItem[i].sItem == 0 )
				break;
			if( !character.TakeItem( pRecord->ForgeItem[i].sItem, pRecord->ForgeItem[i].byNum, "ϵͳ" ) )
			{
				char szForgeItem[64] = "δ֪";
				CItemRecord* pForgeItem = (CItemRecord*)GetItemRecordInfo( pRecord->ForgeItem[i].sItem );
				if( pForgeItem )
				{
					strcpy( szForgeItem, pForgeItem->szName );
				}
				character.SystemNotice( "ȡ�߾���������Ʒ��%s��������%d��ʧ�ܣ�", szForgeItem, pRecord->ForgeItem[i].byNum );
				return;
			}
		}

		if( bSuccess )
		{
			// ���óɹ������ȼ�
			character.m_CKitbag.SetChangeFlag(false);
			SItemGrid* pGrid = character.m_CKitbag.GetGridContByID( byIndex );
			if( pGrid == NULL || !character.ItemForge( pGrid, byLevel ) )
			{
				character.SystemNotice( "���󣺾����ɹ���������Ʒ��%s�������ȼ�(%d)ʧ�ܣ�", pItem->szName, byLevel );
				return;
			}

			character.SystemNotice( "������Ʒ��%s���ɹ�����ǰ�����ȼ�(%d)��", pItem->szName, byLevel );
			character.SynKitbagNew( enumSYN_KITBAG_FORGES );
		}
		else
		{
			if( pRecord->byFailure == BYTE(-1) )
			{
				character.m_CKitbag.SetChangeFlag(false);
				character.KbClearItem( true, true, byIndex );
				character.SystemNotice( "������Ʒ��%s������ʧ�ܣ�������Ʒ��٣�", pItem->szName );
				character.SynKitbagNew( enumSYN_KITBAG_FORGEF );
			}
			else
			{
				// ���óɹ������ȼ�
				character.m_CKitbag.SetChangeFlag(false);
				byLevel = pRecord->byFailure;
				SItemGrid* pGrid = character.m_CKitbag.GetGridContByID( byIndex );
				if( pGrid == NULL ||  !character.ItemForge( pGrid, byLevel ) )
				{
					character.SystemNotice( "����ʧ�ܣ�������Ʒ��%s�������ȼ�(%d)ʧ�ܣ�", pItem->szName, byLevel );
					return;
				}

				character.SystemNotice( "������Ʒ��%s��ʧ�ܣ��˻ص���ǰ�����ȼ�(%d)��", pItem->szName, byLevel );
				character.SynKitbagNew( enumSYN_KITBAG_FORGEF );
			}
		}
	}

}
