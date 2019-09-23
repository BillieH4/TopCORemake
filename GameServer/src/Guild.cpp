#include "Guild.h"
#include "GameDB.h"
#include "util.h"
#include "dstring.h"
#include "parser.h"
#include "GameCommon.h"
#include "GameApp.h"

#include "stdafx.h"

BOOL Guild::lua_CreateGuild(CCharacter* pCha, char guildtype)//��������,1-����,2-����
{
	if(pCha->GetPlayer()->m_GuildState.IsFalse(emGuildGetName))//�����û����빫����������
	{
		pCha->GetPlayer()->m_GuildState.SetBit(emGuildGetName);
		pCha->GetPlayer()->m_cGuildType	=guildtype;

		WPACKET	l_wpk =GETWPACKET();
		WRITE_CMD(l_wpk, CMD_MC_GUILD_GETNAME);
		WRITE_CHAR(l_wpk, 1);
		pCha->ReflectINFof(pCha,l_wpk);
	}
	return TRUE;
}
void Guild::cmd_CreateGuild(CCharacter* pCha, bool confirm, cChar *guildname, cChar *passwd)
{
	if(pCha->GetPlayer()->m_GuildState.IsFalse(emGuildGetName))
	{
		return;
	}else
	{        
		pCha->GetPlayer()->m_GuildState.ClearBit(emGuildGetName);
		if(!guildname || !passwd || !confirm)
		{
			return;
		}

        if(pCha->m_CKitbag.IsPwdLocked())
        {
            //pCha->SystemNotice("������������,�޷���������");
			pCha->SystemNotice(RES_STRING(GM_GUILD_CPP_00001));
			return;
        }

		//add by ALLEN 2007-10-16
		if(pCha->IsReadBook())
        {
            //pCha->SystemNotice("����״̬,�޷���������");
			pCha->SystemNotice(RES_STRING(GM_GUILD_CPP_00002));
			return;
        }
        
		short l_len	=short(strlen(guildname));
		if(l_len <1)
		{
			//pCha->SystemNotice("������̫��");
			pCha->SystemNotice(RES_STRING(GM_GUILD_CPP_00003));
			return;
		}else if(l_len >16)
		{
			//pCha->SystemNotice("������̫��");
			pCha->SystemNotice(RES_STRING(GM_GUILD_CPP_00004));
			return;
		}else if(strlen(passwd) >16)
		{
			//pCha->SystemNotice("�������̫��");
			pCha->SystemNotice(RES_STRING(GM_GUILD_CPP_00005));
			return;
		}
		//else if(!IsValidGuildName(guildname,l_len))
		//{
		//	pCha->SystemNotice("Guild name includes illegal character");
		//	return;
		//}
	}
	if (g_CParser.DoString("AskGuildItem", enumSCRIPT_RETURN_NUMBER, 1, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, pCha,enumSCRIPT_PARAM_NUMBER, 1, pCha->GetPlayer()->m_cGuildType, DOSTRING_PARAM_END))
	{
		if(!g_CParser.GetReturnNumber(0))	//����������
			return;
	}
	else
		return;
	char l_guildname[32];
	strcpy(l_guildname,guildname);
	long l_guildid =game_db.CreateGuild(pCha,l_guildname,passwd);
	if(!l_guildid)
	{
		return;
	}

	//����ͬ���������ֺ���
	pCha->SetGuildName( l_guildname );//���ù�������
	pCha->SetGuildID( l_guildid	);		//���ù���ID
	pCha->SetGuildType( pCha->GetPlayer()->m_cGuildType );
	pCha->SetGuildState( 0 );
	pCha->guildPermission = emGldPermMax;
	pCha->SyncGuildInfo();

	//pCha->setAttr(ATTR_GUILD,l_guildid);		
	//pCha->setAttr(ATTR_GUILD_TYPE,pCha->GetPlayer()->m_cGuildType);//���ù�������
	//pCha->setAttr(ATTR_GUILD_STATE,0);			//���ù���״̬

	//����lua�ű��۳���ƷBegin
	g_CParser.DoString("DeductGuildItem", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, pCha,enumSCRIPT_PARAM_NUMBER, 1, pCha->GetPlayer()->m_cGuildType, DOSTRING_PARAM_END);
	//����lua�ű��۳���ƷEnd
	
	//pCha->SystemNotice("���ᴴ���ɹ�.");
	pCha->SystemNotice(RES_STRING(GM_GUILD_CPP_00006));

	char l_str[512];
	sprintf(l_str, "Player [%s] has founded [%s] guild. All Pirates may now apply!", 
			pCha->GetName(), 
			//pCha->GetPlayer()->m_cGuildType == 0 ? "Navy Division" : "Pirate Guild",
			l_guildname);
			//pCha->GetPlayer()->m_cGuildType == 0 ? "Navys" : "Pirates" );

	g_pGameApp->ScrollNotice(l_str, 2);
}
BOOL Guild::lua_ListAllGuild(CCharacter* pCha, char guildtype)			//��ʼ���ͻ��˴����б���ͻ��˷���һ����ʼ�б������NPC�Ի�����һ��
{
	cmd_ListAllGuild(pCha,guildtype);
	return TRUE;
}
void Guild::cmd_ListAllGuild(CCharacter* pCha, char guildtype)			//����һ�η���20�У��ɿͻ�������ִε��ã�ֱ������������
{
	pCha->GetPlayer()->m_cGuildType	=1;
	game_db.ListAllGuild(pCha,7);
}
void Guild::cmd_GuildTryFor(CCharacter* pCha, uLong guildid)			//������빫��
{
	if(!guildid)
	{
		//pCha->SystemNotice("�Բ���!�����ܲ��������κλ�Ա.");
		pCha->SystemNotice(RES_STRING(GM_GUILD_CPP_00007));
	}else if(guildid >199)
	{
		//pCha->SystemNotice("����Ĺ���Ƿ�!");
		pCha->SystemNotice(RES_STRING(GM_GUILD_CPP_00008));
	}else
	{
		char	l_gldtype = 1;// game_db.GetGuildTypeByID(pCha, guildid);
		//if(l_gldtype != pCha->GetPlayer()->m_cGuildType)
		//{
		//	return;
		//}
		if (g_CParser.DoString("AskJoinGuild", enumSCRIPT_RETURN_NUMBER, 1, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, pCha,enumSCRIPT_PARAM_NUMBER, 1, l_gldtype, DOSTRING_PARAM_END))
		{
			if(!g_CParser.GetReturnNumber(0))	//����������
				return;
		}
		else
			return;
		game_db.GuildTryFor(pCha,guildid);
	}
}
void Guild::cmd_GuildTryForComfirm(CCharacter* pCha, char IsReplace)
{
	if(pCha->GetPlayer()->m_GuildState.IsTrue(emGuildReplaceOldTry))
	{
		if(IsReplace ==1)
		{
			game_db.GuildTryForConfirm(pCha,pCha->GetPlayer()->m_lTempGuildID);
		}
		pCha->GetPlayer()->m_GuildState.ClearBit(emGuildReplaceOldTry);
	}
}
void Guild::cmd_GuildListTryPlayer(CCharacter* pCha)
{

	game_db.GuildListTryPlayer(pCha,7);
}
void Guild::cmd_GuildApprove(CCharacter* pCha,uLong chaid)
{

	game_db.GuildApprove(pCha,chaid);
}
void Guild::cmd_GuildReject(CCharacter* pCha,uLong chaid)
{

	game_db.GuildReject(pCha,chaid);
}
void Guild::cmd_GuildKick(CCharacter* pCha,uLong chaid)
{

	game_db.GuildKick(pCha,chaid);
}
void Guild::cmd_GuildLeave(CCharacter* pCha)
{

	game_db.GuildLeave(pCha);
}
void Guild::cmd_GuildDisband(CCharacter* pCha,cChar *passwd)
{

	game_db.GuildDisband(pCha,passwd);
}
void Guild::cmd_GuildMotto(CCharacter* pCha,cChar *motto)
{

	game_db.GuildMotto(pCha,motto);
}
void Guild::cmd_GuildChallenge( CCharacter* pCha, BYTE byLevel, DWORD dwMoney )
{
    if(pCha->m_CKitbag.IsPwdLocked())
    {
        //pCha->SystemNotice("������������,�޷���ս����");
		pCha->SystemNotice(RES_STRING(GM_GUILD_CPP_00009));
        return;
    }

	//add by ALLEN 2007-10-16
	if(pCha->IsReadBook())
    {
       // pCha->SystemNotice("����״̬,�޷���ս����");
		 pCha->SystemNotice(RES_STRING(GM_GUILD_CPP_00010));
        return;
    }
    
	game_db.Challenge( pCha, byLevel, dwMoney );
}
void Guild::cmd_GuildLeizhu( CCharacter* pCha, BYTE byLevel, DWORD dwMoney )
{
	game_db.Leizhu( pCha, byLevel, dwMoney );	
}
void Guild::cmd_PMDisband(CCharacter *pCha)
{
	//pCha->m_CChaAttr.ResetChangeFlag();

	//pCha->setAttr(ATTR_GUILD,0);			//���ù���ID
	//pCha->setAttr(ATTR_GUILD_STATE,0);		//���ù���״̬

	//pCha->SynAttr(enumATTRSYN_TRADE);

	//pCha->SetGuildName("");
}
