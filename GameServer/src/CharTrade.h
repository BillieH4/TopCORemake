// CharTrade.h Created by knight-gongjian 2004.12.7.
//---------------------------------------------------------
#pragma once

#ifndef _CHARTRADE_H_
#define _CHARTRADE_H_


#include "Character.h"
//---------------------------------------------------------
_DBC_USING

namespace mission
{
	typedef struct _TRADE_DATA
	{
		struct TRADE_ITEM_DATA
		{
			BYTE byType : 2;
			BYTE byIndex: 6;
			BYTE byCount;
			USHORT sItemID;			
		};
		TRADE_ITEM_DATA ItemArray[ROLE_MAXNUM_TRADEDATA];
		BYTE  byItemCount;						// ��Ʒ����
		DWORD dwMoney;							// ����Ǯ��
		DWORD dwIMP;

	} TRADE_DATA, *PTRADE_DATA;

	class CTradeData : public dbc::PreAllocStru
	{
	public:
		CTradeData(dbc::uLong lSize);
		virtual ~CTradeData();

		void Clear()
		{
			pRequest	= NULL;
			pAccept		= NULL;
			byValue		= 0;

			memset( &ReqTradeData, 0, sizeof(TRADE_DATA) );
			memset( &AcpTradeData, 0, sizeof(TRADE_DATA) );
		}

		CCharacter *pRequest, *pAccept;
		union
		{
			struct
			{
				BYTE  bTradeStart : 1;		// �����Ƿ�ʼ
				BYTE  bReqTrade : 1;		// ���󷽽�������ȷ��
				BYTE  bAcpTrade : 1;		// ���ܷ���������ȷ��
				BYTE  bReqOk : 1;			// ���󷽽��ײ���ȷ��
				BYTE  bAcpOk : 1;			// ���ܷ����ײ���ȷ��
				BYTE  byParam : 3;			// ����
			};
			BYTE byValue;
		};
		//USHORT sxPos, syPos;		// ���׵ص���Ϣ
		TRADE_DATA ReqTradeData;	// ���󷽽���������Ϣ
		TRADE_DATA AcpTradeData;	// ���ܷ�����������Ϣ
		
		DWORD dwTradeTime;			// ����ʱ���ʱ(���׳����涨����ʱ�䣬ϵͳȡ�����ײ��������ջ�����)
	};

	class CTradeSystem
	{
	public:
		CTradeSystem();
		~CTradeSystem();

		// ���ײ���
		BOOL Request( BYTE byType, CCharacter& character, DWORD dwAcceptID );
		BOOL Accept( BYTE byType, CCharacter& character,  DWORD dwRequestID );
		BOOL Cancel( BYTE byType, CCharacter& character,  DWORD dwCharID );
		
		// ��ɫ�������������Ϣ
		BOOL Clear( BYTE byType, CCharacter& character );

		// ȷ�Ͻ�����Ϣ�Ͳ���
		BOOL ValidateItemData( BYTE byType, CCharacter& character, DWORD dwCharID );
		BOOL ValidateTrade( BYTE byType, CCharacter& character, DWORD dwCharID );

		// ���û���ȡ����Ʒ��������
		BOOL AddItem( BYTE byType, CCharacter& character, DWORD dwCharID, BYTE byOpType, BYTE byIndex, BYTE byItemIndex, BYTE byCount );
		BOOL AddMoney( BYTE byType, CCharacter& charactar, DWORD dwCharID, BYTE byOpType, DWORD dwMoney );
		BOOL AddIMP(BYTE byType, CCharacter& charactar, DWORD dwCharID, BYTE byOpType, DWORD dwMoney);
		BOOL IsTradeDist( CCharacter& Char1, CCharacter& Char2, DWORD dwDist );
	private:
		void ResetItemState( CCharacter& character, CTradeData& TradeData );

	};

#pragma pack( push, before_InfoNet )
#pragma pack( 8 )

    //�̳Ƕ�����Ϣ
    struct SOrderData
    {        
        long long	lOrderID;  //������
        long		lComID;    //��ƷID
		long		lNum;	   //����
        long		ChaID;	   //��ɫ���ݿ�ID
		long		lRecDBID;
		char		ChaName[defENTITY_NAME_LEN];
		DWORD		dwTickCount;
    };

	//��Ʒ��Ϣ
	struct SItemData
	{
		SItemData():pItemArray(NULL)
		{
		}

		SItemData(const SItemData &rItem)
		{
			memcpy(&store_head, &rItem.store_head, sizeof(StoreInfo));
			int nItemNum = store_head.itemNum;
			if(nItemNum > 0)
			{
				pItemArray = new ItemInfo[nItemNum];
				memcpy(pItemArray, rItem.pItemArray, nItemNum * sizeof(ItemInfo));
			}
			else
				pItemArray = NULL;
		}

		~SItemData()
		{
			if(pItemArray)
			{
				delete [] pItemArray;
				pItemArray = NULL;
			}
		}

		SItemData& operator=(const SItemData &rItem)
		{
			memcpy(&store_head, &rItem.store_head, sizeof(StoreInfo));
			int nItemNum = store_head.itemNum;
			if(nItemNum > 0)
			{
				if(pItemArray)
				{
					delete [] pItemArray;
				}
				pItemArray = new ItemInfo[nItemNum];
				memcpy(pItemArray, rItem.pItemArray, nItemNum * sizeof(ItemInfo));
			}
			else
				pItemArray = NULL;
			return *this;
		}

		StoreInfo   store_head;       //  ��Ʒ��Ϣͷ
		ItemInfo	*pItemArray;      //  ������Ϣ��
	};

    //�̳�ϵͳ
    class CStoreSystem
    {
    public:
        CStoreSystem();
        ~CStoreSystem();

        //�û��������
        BOOL Request( CCharacter *pCha, long lComID );
		BOOL Accept( long long lOrderID, RoleInfo *ChaInfo );
		BOOL Accept( CCharacter *pCha, long lComID );
		BOOL Cancel( long long lOrderID );

		BOOL RequestVIP(CCharacter *pCha, short sVipID, short sMonth);
		BOOL AcceptVIP(long long lOrderID, RoleInfo *ChaInfo, DWORD dwVipParam);
		BOOL CancelVIP(long long lOrderID);

		//�û���ȡ��Ʒ�б�
		BOOL RequestItemList(CCharacter *pCha, long lClsID, short sPage, short sNum);

		//�û�����һ�����
		BOOL RequestChange(CCharacter *pCha, long lNum);
		BOOL AcceptChange(long long lOrderID, RoleInfo *ChaInfo);
		BOOL CancelChange(long long lOrderID);

		//��ѯ��ɫ��Ϣ
		BOOL RequestRoleInfo(CCharacter *pCha);
		BOOL AcceptRoleInfo(long long lOrderID, RoleInfo *ChaInfo);
		BOOL CancelRoleInfo(long long lOrderID);

		//��ѯ���׼�¼
		BOOL RequestRecord(CCharacter *pCha, long lNum);
		BOOL AcceptRecord(long long lOrderID, HistoryInfo *pRecord);
		BOOL CancelRecord(long long lOrderID);

		BOOL RequestGMSend(CCharacter *pCha, cChar *szTitle, cChar *szContent);
		BOOL AcceptGMSend(long long lOrderID, long lMailID);
		BOOL CancelGMSend(long long lOrderID);

		BOOL RequestGMRecv(CCharacter *pCha);
		BOOL AcceptGMRecv(long long lOrderID, MailInfo *pMi);
		BOOL CancelGMRecv(long long lOrderID);

		//�û������̳�
		BOOL Open(CCharacter *pCha, long vip);

		//�̳��б�
		BOOL GetItemList();
		BOOL GetAfficheList();

		BOOL SetItemList(void *pItemList, long lNum);
		BOOL ClearItemList();

		BOOL SetItemClass(ClassInfo *pClassList, long lNum);
		BOOL ClearItemClass();

		BOOL SetAfficheList(AfficheInfo *pAfficheList, long lNum);
		BOOL ClearAfficheList();

		void InValid() { m_bValid = false; }
		void SetValid() { m_bValid = true; }
		BOOL IsValid() { return m_bValid; }

        void Run( DWORD dwCurTime, DWORD dwIntervalTime, DWORD dwOrderTime );    //�������б�
        
    private:
		//��ѯ��Ʒ
		long		GetClassId(long lComID);
		cChar		*GetClassName(long lClsID);
		SItemData	*GetItemData(long lComID);
		BOOL		DelItemData(long lComID);

		//��������
		BOOL		PushOrder(CCharacter *pCha, long long lOrderID, long lComID, long lNum);
		SOrderData	PopOrder(long long lOrderID);
		BOOL		HasOrder(long long lOrderID);

		//map<long long, SOrderData>		m_OrderList;		//�����б�
		vector<SOrderData>				m_OrderList;
        map<long, long>					m_ItemSearchList;	//��ƷID�ͷ����ӳ���ϵ
		map< long, vector<SItemData> >	m_ItemList;			//��Ʒ�б�
		vector<ClassInfo>				m_ItemClass;		//��Ʒ����
		vector<AfficheInfo>				m_AfficheList;		//�����б�
		BOOL							m_bValid;
        
    };

#pragma pack( pop, before_InfoNet )

}

extern mission::CTradeSystem g_TradeSystem;
extern mission::CStoreSystem g_StoreSystem;

//---------------------------------------------------------

#endif // _CHARTRADE_H_