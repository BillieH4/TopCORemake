//=============================================================================
// FileName: Item.cpp
// Creater: ZhangXuedong
// Date: 2004.09.21
// Comment: CItem class
//=============================================================================

#include "Item.h"
#include "GameCommon.h"
#include "GameAppNet.h"
#include "SubMap.h"
#include "gamedb.h"

_DBC_USING

CItem::CItem()
{T_B
	chValid = 0;
	m_pCItemRecord = 0;
	m_SGridContent.sID = 0;
	m_lFromEntityID = 0;
	m_chSpawType = enumITEM_APPE_NATURAL;
T_E}

void CItem::Initially()
{T_B
	Entity::Initially();

	chValid = 0;
	m_pCItemRecord = 0;
	m_SGridContent.sID = 0;
	m_chSpawType = enumITEM_APPE_NATURAL;
	m_ulStartTick = GetTickCount();
	m_ulOnTick = g_Config.m_lItemShowTime * 1000;
	m_ulProtOnTick = g_Config.m_lItemProtTime * 1000;
	m_ulProtID = 0;
	m_ulProtHandle = 0;
	m_chProtType = enumITEM_PROT_OWN;
T_E}

void CItem::Finally()
{T_B
	if (m_submap)
		m_submap->GoOut(this);
	Entity::Finally();
T_E}

void CItem::OnBeginSeen(CCharacter *pCMainCha)
{T_B
	WPACKET pk =GETWPACKET();
	WRITE_CMD(pk, CMD_MC_ITEMBEGINSEE);
	// ��������
	WRITE_LONG(pk, m_ID);							// world ID
	WRITE_LONG(pk, m_lHandle);
	WRITE_LONG(pk, m_pCItemRecord->lID);			// ID
	WRITE_LONG(pk, GetShape().centre.x);			// ��ǰxλ��
	WRITE_LONG(pk, GetShape().centre.y);			// ��ǰyλ��
	WRITE_SHORT(pk, m_sAngle);					// ����
	WRITE_SHORT(pk, m_SGridContent.sNum);			// ����
	//
	WRITE_CHAR(pk, m_chSpawType);
	WRITE_LONG(pk, m_lFromEntityID);
	// �¼���Ϣ
	WriteEventInfo(pk);

	pCMainCha->ReflectINFof(this,pk);//ͨ��
T_E}

void CItem::OnEndSeen(CCharacter *pCMainCha)
{T_B
	WPACKET pk =GETWPACKET();
	WRITE_CMD(pk, CMD_MC_ITEMENDSEE);
	WRITE_LONG(pk, m_ID);				//ID
	pCMainCha->ReflectINFof(this,pk);	//ͨ��
T_E}

void CItem::Run(dbc::uLong ulCurTick)
{
	if (m_ulProtID != 0)
		if (m_ulProtOnTick != 0 && ulCurTick - m_ulStartTick >= m_ulProtOnTick) // ����ʱ����ʧ
			m_ulProtID = 0;

	if (m_ulOnTick != 0 && ulCurTick - m_ulStartTick >= m_ulOnTick)
	{
		// �ж��Ƿ񴬳�֤������
		CItemRecord* pItem = m_pCItemRecord;
		if( pItem != NULL )
		{
			// �ж϶�������֤��
			if( pItem->sType == enumItemTypeBoat )
			{
				game_db.SaveBoatDelTag( this->GetGridContent()->GetDBParam( enumITEMDBP_INST_ID ), 1 );
			}
		}
		if (!m_submap)
			//LG("������ʧ����", "���� %s(ID %u��HANDLE %u��λ��[%d %d]) ����ʧʱ�������ͼΪ��\n", GetName(), GetID(), GetHandle(), GetPos().x, GetPos().y);
			LG("Item disappear error", "item %s(ID %u��HANDLE %u��position[%d %d]) when it disappear find the map is null\n", GetName(), GetID(), GetHandle(), GetPos().x, GetPos().y);
		else
		{
			Free();
		}
	}
}