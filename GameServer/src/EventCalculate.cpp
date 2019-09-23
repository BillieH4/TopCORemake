#include "StdAfx.h"
#include "EventHandler.h"
#include "Character.h"
#include "Player.h"
#include "Parser.h"
#include "lua_gamectrl.h"



//-------------------------------------
// �¼� : ��ɫ����
// �����ɫ����, �����߾������, ������
//-------------------------------------
void CEventHandler::Event_ChaDie(CCharacter *pDead, CCharacter *pAtk)
{
	BOOL bTeam  = FALSE;
	
	// ��¼��ص����н�ɫ, ��������������������˺����ǵĶ���
	CCharacter *pValidCha[25] = { NULL,NULL,NULL,NULL,NULL,
								  NULL,NULL,NULL,NULL,NULL,
								  NULL,NULL,NULL,NULL,NULL,
								  NULL,NULL,NULL,NULL,NULL,
								  NULL,NULL,NULL,NULL,NULL};
	
	int nValidCha = 0;
	
	CPlayer *pPlayer = pAtk->GetPlayer();
	if(pPlayer==NULL) // ����ҽ�ɫ
	{
		MPTimer t;
		t.Begin();
		// ��������㾭��۳�
		extern lua_State *g_pLuaState;
		lua_getglobal(g_pLuaState, "GetExp_New");
		if( !lua_isfunction( g_pLuaState, -1 ) )
		{
			lua_pop( g_pLuaState, 1 );
			LG( "lua_invalidfunc", "GetExp_New" );
			return;
		}

		lua_pushlightuserdata(g_pLuaState, (void *)pDead);
		lua_pushlightuserdata(g_pLuaState, (void *)pAtk);
		int r = lua_pcall(g_pLuaState, 2, 0, 0); 
		if(r!=0) // ִ�г���
		{
			//LG("lua_err", "GetExp_Newִ�г���, ������(��)[%s], ����������[%s]!\n", pAtk->GetName(), pDead->GetName());
			LG("lua_err", "GetExp_New transact error, attacker(bugbear)[%s], people was bring down[%s]!\n", pAtk->GetName(), pDead->GetName());
			lua_callalert(g_pLuaState, r); 	
		}
		lua_settop(g_pLuaState, 0);
		pDead->ItemCount(pAtk); // ����

		DWORD dwEndTime = t.End();
		if(dwEndTime > 10)
		{
			//LG("script_time", "����������þ������ű�, ����ʱ�����! time = %d\n", dwEndTime);
			LG("script_time", "when player dead transfer experience assign script,account time too long! time = %d\n", dwEndTime);
		}
		return; // ���﹥�����, ������ִ��
	}
	
	for(int i = 0; i < MAX_HARM_REC; i++)
	{
		SHarmRec *pHarm = pDead->m_pHate->GetHarmRec(i);
		if(pHarm->btValid > 0 && pHarm->IsChaValid())
		{
			BOOL bAdd = TRUE;
			for(int j = 0; j < nValidCha; j++)
			{
				if(pHarm->pAtk==pValidCha[j])
				{
					bAdd = FALSE;
					break;
				}
			}
			if(bAdd)
			{
				pValidCha[nValidCha] = pHarm->pAtk;
				nValidCha++;
			}
			
			// �ѵ�ǰ��ɫ�����ж���Ҳ��ӽ���
			pPlayer = pHarm->pAtk->GetPlayer();
			if(pPlayer==NULL)
			{
				//LG("team_error", "�����ѷ־���ʱ�����ر����, playerָ��Ϊ��!, ��ɫ��[%s]\n", pHarm->pAtk->GetName());
				LG("team_error", "it appear especially error when check teammate experience assign, player finger is null!, character name[%s]\n", pHarm->pAtk->GetName());
				break;
			}
			
			for(int i = 0; i < pPlayer->GetTeamMemberCnt(); i++) // ������ÿһ���˶���Ҫ��������
			{
				CCharacter *pOther = pPlayer->GetTeamMemberCha(i);
				if (!pOther)			 continue;
				if(!pOther->IsLiveing()) continue;

				BOOL bAdd = TRUE;
				for(int j = 0; j < nValidCha; j++)
				{
					if(pOther==pValidCha[j])
					{
						bAdd = FALSE;
						break;
					}
				}
				if(bAdd)
				{
					pValidCha[nValidCha] = pOther;
					nValidCha++;
				}
			}
		}
	}
	
	// Ϊ��Щ��ɫ�������Ը��±��
	for(int i =0; i < nValidCha; i++)
	{
		CCharacter *pCur = pValidCha[i];
		if(pCur!=pAtk)	pCur->GetPlyMainCha()->m_CChaAttr.ResetChangeFlag();
	}
	
	MPTimer t;
	t.Begin();
	
	// ��������㾭��
	extern lua_State *g_pLuaState;
	lua_getglobal(g_pLuaState, "GetExp_New");
	if( !lua_isfunction( g_pLuaState, -1 ) )
	{
		lua_pop( g_pLuaState, 1 );
		LG( "lua_invalidfunc", "GetExp_New" );
		return;
	}

	MPTimer tLua; tLua.Begin();
	lua_pushlightuserdata(g_pLuaState, (void *)pDead);
	lua_pushlightuserdata(g_pLuaState, (void *)pAtk);
	int r = lua_pcall(g_pLuaState, 2, 0, 0); 
	if(r!=0) // ִ�г���
	{
		//LG("lua_err", "GetExp_Newִ�г���, ������[%s], �������Ĺ�[%s]!\n", pAtk->GetName(), pDead->GetName());
		LG("lua_err", "GetExp_New transact error, attacker[%s], bugbear wsa bring down [%s]!\n", pAtk->GetName(), pDead->GetName());
		lua_callalert(g_pLuaState, r); 	
	}
	lua_settop(g_pLuaState, 0);
	tLua.End();
	
	// Ϊ��Щ��ɫ�������Ա��֪ͨ
	for(int i =0; i < nValidCha; i++)
	{
		CCharacter *pCur = pValidCha[i];
		if(pCur!=pAtk) pCur->GetPlyMainCha()->SynAttr(enumATTRSYN_ATTACK);
	}

	MPTimer tMission; tMission.Begin();
	// �����ߺ����Ķ��ѵõ�����֪ͨ
	pPlayer = pAtk->GetPlayer();
	pAtk->AfterObjDie(pAtk, pDead);
	for(int i = 0; i < pPlayer->GetTeamMemberCnt(); i++) // ������ÿһ���˶���Ҫ��������
	{
		CCharacter *pOther = pPlayer->GetTeamMemberCha(i);
		if (!pOther)			 continue;
		if(!pOther->IsLiveing()) continue;
		pOther->AfterObjDie(pAtk, pDead);
	}
	tMission.End();
		
	MPTimer tItem; tItem.Begin();
	// ����
	pDead->ItemCount(pAtk);

	tItem.End();
	
	DWORD dwEndTime = t.End();
	if(dwEndTime > 10)
	{
		//LG("script_time", "�������������������, ����ʱ�����, time = %d, exp = %d, upgrade = %d, item = %d!\n", dwEndTime, tLua.GetTimeCount(), tMission.GetTimeCount(), tItem.GetTimeCount());
		LG("script_time", "the flow of assign experience when bugbear dead, calculate time too long, time = %d, exp = %d, upgrade = %d, item = %d!\n", dwEndTime, tLua.GetTimeCount(), tMission.GetTimeCount(), tItem.GetTimeCount());
	}
}
