#include <iostream>
#include "GroupServerApp.h"
#include "GameCommon.h"

void GroupServerApp::CP_FRND_INVITE(Player *ply,DataSocket *datasock,RPacket &pk)
{
	if(ply->m_CurrFriendNum >=const_frnd.FriendMax)
	{
		//ply->SendSysInfo("���ĺ������Ѿ��ﵽ����������ˣ��������뱻ȡ����");
		ply->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00001));
	}else
	{
		Invited				*	l_invited	=0;
		uShort				l_len;
		cChar			*	l_invited_name	=pk.ReadString(&l_len);
		if(!l_invited_name || l_len >16)
		{
			return;
		}
		Player			*	l_invited_ply	=FindPlayerByChaName(l_invited_name);
		MutexArmor l_lockDB(m_mtxDB);
		if(!l_invited_ply || l_invited_ply->m_currcha <0 || l_invited_ply ==ply)
		{
			char l_buf[256];
			//sprintf(l_buf,"�����������ҡ�%s����ǰ�������ϡ�",l_invited_name);
			sprintf(l_buf,RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00002),l_invited_name);
			ply->SendSysInfo(l_buf);
		}else if(l_invited =l_invited_ply->FrndFindInvitedByInviterChaID(ply->m_chaid[ply->m_currcha]))
		{
			//ply->SendSysInfo(dstring("����ǰ�ԡ�")<<l_invited_name<<"���Ѿ���һ��δ���ĺ������룬���԰����ꡣ");
			char l_buf[256];
			sprintf(l_buf,RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00003),l_invited_name);
			ply->SendSysInfo(l_buf);
		}else if(l_invited =ply->FrndFindInvitedByInviterChaID(l_invited_ply->m_chaid[l_invited_ply->m_currcha]))
		{
			//ply->SendSysInfo(dstring("��")<<l_invited_name<<"����ǰ�Ѿ���һ������ĺ������룬����ܼ��ɡ�");
			char l_buf[256];
			sprintf(l_buf,RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00005),l_invited_name);
			ply->SendSysInfo(l_buf);
		}else if(l_invited_ply->m_CurrFriendNum >=const_frnd.FriendMax)
		{
			char l_buf[256];
			//sprintf(l_buf,"�����������ҡ�%s���ĺ�����Ŀ�Ѿ��ﵽ����������ˡ�",l_invited_name);
			sprintf(l_buf,RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00006),l_invited_name);
			ply->SendSysInfo(l_buf);
		}else if(m_tblfriends->GetFriendsCount(ply->m_chaid[ply->m_currcha],l_invited_ply->m_chaid[l_invited_ply->m_currcha])>0)
		{
			//ply->SendSysInfo(dstring("������ҡ�")<<l_invited_name<<"���Ѿ��Ǻ����ˡ�");
			char l_buf[256];
			sprintf(l_buf,RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00007),l_invited_name);
			ply->SendSysInfo(l_buf);
		}else
		{
			PtInviter l_ptinviter	=l_invited_ply->FrndBeginInvited(ply);
			if(l_ptinviter )
			{
				char l_buf[256];
				//sprintf(l_buf,"��������ĺ�����ҡ�%s�����ڱ������������ڷ�æ״̬,�����������ѱ�ϵͳȡ����",l_invited_name);
				sprintf(l_buf,RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00009),l_invited_name);
				l_ptinviter->SendSysInfo(l_buf);

				WPacket	l_wpk	=GetWPacket();
				l_wpk.WriteCmd(CMD_PC_FRND_CANCEL);
				l_wpk.WriteChar(MSG_FRND_CANCLE_BUSY);
				l_wpk.WriteLong(l_ptinviter.m_chaid);
				SendToClient(l_invited_ply,l_wpk);
			}
			WPacket	l_wpk	=GetWPacket();
			l_wpk.WriteCmd(CMD_PC_FRND_INVITE);
			l_wpk.WriteString(ply->m_chaname[ply->m_currcha].c_str());
			l_wpk.WriteLong(ply->m_chaid[ply->m_currcha]);
			l_wpk.WriteShort(ply->m_icon[ply->m_currcha]);
			SendToClient(l_invited_ply,l_wpk);
		}
	}
}
void GroupServerApp::CP_FRND_REFUSE(Player *ply,DataSocket *datasock,RPacket &pk)
{
	uLong		l_inviter_chaid	=pk.ReadLong();
	PtInviter	l_inviter		=ply->FrndEndInvited(l_inviter_chaid);
	if(l_inviter && l_inviter->m_currcha >=0 && l_inviter.m_chaid ==l_inviter->m_chaid[l_inviter->m_currcha])
	{
		char l_buf[256];
		//sprintf(l_buf,"��ҡ�%s���ܾ������ĺ������롣",ply->m_chaname[ply->m_currcha].c_str());
		sprintf(l_buf,RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00010),ply->m_chaname[ply->m_currcha].c_str());
		l_inviter->SendSysInfo(l_buf);
	}
}
void GroupServerApp::CP_FRND_ACCEPT(Player *ply,DataSocket *datasock,RPacket &pk)
{
	uLong		l_inviter_chaid	=pk.ReadLong();
	PtInviter	l_inviter		=ply->FrndEndInvited(l_inviter_chaid);
	if(l_inviter && l_inviter->m_currcha >=0 && l_inviter.m_chaid ==l_inviter->m_chaid[l_inviter->m_currcha])
	{
		MutexArmor l_lockDB(m_mtxDB);
		if((++(ply->m_CurrFriendNum)) >const_frnd.FriendMax)
		{
			--(ply->m_CurrFriendNum);
			//ply->SendSysInfo("���ĺ������Ѵﵽ����������ˡ�");
			ply->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00011));
		}else if((++(l_inviter->m_CurrFriendNum)) >const_frnd.FriendMax)
		{
			--(ply->m_CurrFriendNum);
			--(l_inviter->m_CurrFriendNum);
			//ply->SendSysInfo(dstring("�����ߡ�")<<l_inviter->m_chaname[l_inviter->m_currcha].c_str()<<"���ĺ������Ѵﵽ����������ˡ�");
			char l_buf[256];
			sprintf(l_buf,RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00012),l_inviter->m_chaname[l_inviter->m_currcha].c_str());
			ply->SendSysInfo(l_buf);
		}else if(m_tblfriends->GetFriendsCount(ply->m_chaid[ply->m_currcha],l_inviter->m_chaid[l_inviter->m_currcha])>0)
		{
			--(ply->m_CurrFriendNum);
			--(l_inviter->m_CurrFriendNum);
			//ply->SendSysInfo(dstring("���͡�")<<l_inviter->m_chaname[l_inviter->m_currcha].c_str()<<"���Ѿ��Ǻ����ˡ�");
			char l_buf[256];
			sprintf(l_buf,RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00007),l_inviter->m_chaname[l_inviter->m_currcha].c_str());
			ply->SendSysInfo(l_buf);
		}else
		{
			LogLine	l_line(g_LogFriend);
			/*
			l_line<<newln<<"���"<<ply->m_chaname[ply->m_currcha]<<"("<<ply->m_chaid[ply->m_currcha]
				<<")�����"<<l_inviter->m_chaname[l_inviter->m_currcha]<<"("<<l_inviter_chaid<<")��Ϊ�˺���"
				<<endln;
			*/
			l_line<<newln<<"player"<<ply->m_chaname[ply->m_currcha]<<"("<<ply->m_chaid[ply->m_currcha]
				<<")and player"<<l_inviter->m_chaname[l_inviter->m_currcha]<<"("<<l_inviter_chaid<<") make friends"
				<<endln;

			m_tblfriends->AddFriend(ply->m_chaid[ply->m_currcha],l_inviter.m_chaid);
			WPacket	l_wpk =GetWPacket();
			l_wpk.WriteCmd(CMD_PC_FRND_REFRESH);
			l_wpk.WriteChar(MSG_FRND_REFRESH_ADD);
			l_wpk.WriteString(gc_standard_group);
			WPacket l_wpk2=l_wpk;
			l_wpk.WriteLong(ply->m_chaid[ply->m_currcha]);
			l_wpk.WriteString(ply->m_chaname[ply->m_currcha].c_str());
			l_wpk.WriteString(ply->m_motto[ply->m_currcha].c_str());
			l_wpk.WriteShort(ply->m_icon[ply->m_currcha]);
			SendToClient(l_inviter.m_ply,l_wpk);
			l_wpk2.WriteLong(l_inviter->m_chaid[l_inviter->m_currcha]);
			l_wpk2.WriteString(l_inviter->m_chaname[l_inviter->m_currcha].c_str());
			l_wpk2.WriteString(l_inviter->m_motto[l_inviter->m_currcha].c_str());
			l_wpk2.WriteShort(l_inviter->m_icon[l_inviter->m_currcha]);
			SendToClient(ply,l_wpk2);
		}
	}
}
void GroupServerApp::CP_FRND_DELETE(Player *ply,DataSocket *datasock,RPacket &pk)
{
	uLong		l_deleted_chaid	=pk.ReadLong();
	MutexArmor l_lockDB(m_mtxDB);
	if(m_tblfriends->GetFriendsCount(ply->m_chaid[ply->m_currcha],l_deleted_chaid)<1)
	{
		//ply->SendSysInfo("������Ҫɾ������Ҳ��Ǻ��ѹ�ϵ������ϵ�ͷ�����");
		ply->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00018));
	}else
	{
		Player	*	l_deleted_ply;
		if((l_deleted_ply =(Player *)MakePointer(m_tblfriends->GetFriendAddr(ply->m_chaid[ply->m_currcha],l_deleted_chaid)))&& l_deleted_ply->m_currcha >=0)
		{
			WPacket	l_wpk =GetWPacket();
			l_wpk.WriteCmd(CMD_PC_FRND_REFRESH);
			l_wpk.WriteChar(MSG_FRND_REFRESH_DEL);
			WPacket l_wpk2=l_wpk;

			l_wpk.WriteLong(ply->m_chaid[ply->m_currcha]);
			SendToClient(l_deleted_ply,l_wpk);
			--(l_deleted_ply->m_CurrFriendNum);

			l_wpk2.WriteLong(l_deleted_chaid);
			SendToClient(ply,l_wpk2);
			--(ply->m_CurrFriendNum);
		}else
		{
			WPacket	l_wpk =GetWPacket();
			l_wpk.WriteCmd(CMD_PC_FRND_REFRESH);
			l_wpk.WriteChar(MSG_FRND_REFRESH_DEL);

			l_wpk.WriteLong(l_deleted_chaid);
			SendToClient(ply,l_wpk);
			--(ply->m_CurrFriendNum);
		}
		m_tblfriends->DelFriend(ply->m_chaid[ply->m_currcha],l_deleted_chaid);
		LogLine	l_line(g_LogFriend);
		/*
		l_line<<newln<<"���"<<ply->m_chaname[ply->m_currcha]<<"("<<ply->m_chaid[ply->m_currcha]
			<<")��("<<l_deleted_chaid<<")����˺��ѹ�ϵ";
	   */
		l_line<<newln<<"player"<<ply->m_chaname[ply->m_currcha]<<"("<<ply->m_chaid[ply->m_currcha]
			<<")and("<<l_deleted_chaid<<")free friends";
	}
}

void GroupServerApp::CP_FRND_CHANGE_GROUP(Player *ply,DataSocket *datasock,RPacket &pk)
{
	uLong l_id = (uLong)pk.ReadLong();
	cChar* l_grp = pk.ReadString((uShort *)&pk);

	if (!l_grp || (uShort)&pk > 16 || !IsValidName(l_grp, (uShort)&pk))
	{
		return;
	}
	MutexArmor l_lockDB(m_mtxDB);
	if (!strchr(l_grp, '\'') && m_tblfriends->GetFriendsCount(ply->m_chaid[ply->m_currcha], l_id) == 2)
	{
		int l_grpnum	= m_tblfriends->GetGroupCount(ply->m_chaid[ply->m_currcha]);
		if( l_grpnum	< 0 || l_grpnum > const_frnd.FriendGroupMax)
		{
			ply->SendSysInfo(RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00021));
		}
		else
		{
			if(m_tblfriends->UpdateGroup(ply->m_chaid[ply->m_currcha], l_id, l_grp))
			{
				WPacket l_wpk	=GetWPacket();
				l_wpk.WriteCmd(CMD_PC_FRND_CHANGE_GROUP);
				l_wpk.WriteLong(l_id);
				l_wpk.WriteString(l_grp);
				SendToClient(ply,l_wpk);
			}
		}
	}
}

void Player::FrndInvitedCheck(Invited	*invited)
{
	Player *l_inviter	=invited->m_ptinviter.m_ply;
	if(m_currcha <0)
	{
		FrndEndInvited(l_inviter);
	}else if(l_inviter->m_currcha <0 || l_inviter->m_chaid[l_inviter->m_currcha] !=invited->m_ptinviter.m_chaid)
	{
		WPacket l_wpk	=g_gpsvr->GetWPacket();
		l_wpk.WriteCmd(CMD_PC_FRND_CANCEL);
		l_wpk.WriteChar(MSG_FRND_CANCLE_OFFLINE);
		l_wpk.WriteLong(invited->m_ptinviter.m_chaid);
		g_gpsvr->SendToClient(this,l_wpk);
		FrndEndInvited(l_inviter);
	}else if(l_inviter->m_CurrFriendNum >=g_gpsvr->const_frnd.FriendMax)
	{
		WPacket l_wpk	=g_gpsvr->GetWPacket();
		l_wpk.WriteCmd(CMD_PC_FRND_CANCEL);
		l_wpk.WriteChar(MSG_FRND_CANCLE_INVITER_ISFULL);
		l_wpk.WriteLong(invited->m_ptinviter.m_chaid);
		g_gpsvr->SendToClient(this,l_wpk);
		FrndEndInvited(l_inviter);
	}else if(m_CurrFriendNum >=g_gpsvr->const_frnd.FriendMax)
	{
		WPacket l_wpk	=g_gpsvr->GetWPacket();
		l_wpk.WriteCmd(CMD_PC_FRND_CANCEL);
		l_wpk.WriteChar(MSG_FRND_CANCLE_SELF_ISFULL);
		l_wpk.WriteLong(invited->m_ptinviter.m_chaid);
		g_gpsvr->SendToClient(this,l_wpk);
		FrndEndInvited(l_inviter);
	}else if(g_gpsvr->GetCurrentTick() -invited->m_tick	>=g_gpsvr->const_frnd.PendTimeOut)
	{
		char l_buf[256];
		//sprintf(l_buf,"��ԡ�%s���ĺ��������ѳ���%d����û�л�Ӧ��ϵͳ�Զ�ȡ����������롣",m_chaname[m_currcha].c_str(),g_gpsvr->const_frnd.PendTimeOut/1000);
		sprintf(l_buf,RES_STRING(GP_GROUPSERVERAPPFRND_CPP_00022),m_chaname[m_currcha].c_str(),g_gpsvr->const_frnd.PendTimeOut/1000);
		l_inviter->SendSysInfo(l_buf);

		WPacket l_wpk	=g_gpsvr->GetWPacket();
		l_wpk.WriteCmd(CMD_PC_FRND_CANCEL);
		l_wpk.WriteChar(MSG_FRND_CANCLE_TIMEOUT);
		l_wpk.WriteLong(invited->m_ptinviter.m_chaid);
		g_gpsvr->SendToClient(this,l_wpk);
		FrndEndInvited(l_inviter);
	}
}
/*	�������ʱ��ȡ����������SQL���
cha_id-���߽�ɫID
select '' relation,count(*) addr,0 cha_id,'' cha_name,0 icon,'' motto from ( 
select distinct friends.relation relation from character INNER JOIN 
friends ON character.cha_id = friends.cha_id2 where friends.cha_id1 = 240 
) cc 

union select '' cha_name,0 addr, -1 cha_id,friends.relation relation,0 icon,'' motto from friends 
where friends.cha_id1 = 240 and friends.cha_id2 = -1

union select friends.relation relation,count(character.mem_addr) addr,0 
cha_id,'' cha_name,1 icon,'' motto from character INNER JOIN friends ON 
character.cha_id = friends.cha_id2 where friends.cha_id1 = 240 group by relation 
union select friends.relation relation,character.mem_addr addr,character.cha_id 
cha_id,character.cha_name cha_name,character.icon icon,character.motto motto 
from character INNER JOIN friends ON character.cha_id = friends.cha_id2 
where friends.cha_id1 = 240 order by relation,cha_id,icon  

*/
void GroupServerApp::PC_GM_INIT(Player *ply){
	friend_dat l_farray[255];
	int l_num = 255;
	m_tblX1->get_gm_dat(l_farray, l_num);

	WPacket	l_toSelf = GetWPacket();
	l_toSelf.WriteCmd(CMD_PC_GM_INFO);
	l_toSelf.WriteChar(MSG_FRND_REFRESH_START);
	l_toSelf.WriteChar(l_num);

	Player	*	l_ply1;
	for (int i = 0; i < l_num; i++){
		l_toSelf.WriteLong(l_farray[i].cha_id);
		l_toSelf.WriteString(l_farray[i].cha_name.c_str());
		l_toSelf.WriteString(l_farray[i].motto.c_str());
		l_toSelf.WriteShort(l_farray[i].icon_id);
		if (l_ply1 = (Player*)MakePointer(l_farray[i].memaddr)){
			l_toSelf.WriteChar(1);
		}
		else{
			l_toSelf.WriteChar(0);
		}
	}
	SendToClient(ply, l_toSelf);
	
	if (ply->m_gm > 0){
		WPacket	l_toAll = GetWPacket();
		l_toAll.WriteCmd(CMD_PC_GM_INFO);
		/*
		l_toAll.WriteChar(MSG_FRND_REFRESH_ONLINE);
		l_toAll.WriteLong(ply->m_chaid[ply->m_currcha]);
		*/
		l_toAll.WriteChar(MSG_FRND_REFRESH_ADD);
		l_toAll.WriteString("GM");
		l_toAll.WriteLong(ply->m_chaid[ply->m_currcha]);
		l_toAll.WriteString(ply->m_chaname[ply->m_currcha].c_str());
		l_toAll.WriteString(ply->m_motto[ply->m_currcha].c_str());
		l_toAll.WriteShort(ply->m_icon[ply->m_currcha]);

		Player *l_plylst[10240];
		short	l_plynum = 0;

		Player	*	l_ply1 = 0; char	l_currcha = 0;
		RunChainGetArmor<Player> l(g_gpsvr->m_plylst);
		while (l_ply1 = g_gpsvr->m_plylst.GetNextItem())
		{
			if ((l_currcha = l_ply1->m_currcha) >= 0)
			{
				l_plylst[l_plynum] = l_ply1;
				l_plynum++;
			}
		}
		l.unlock();
		g_gpsvr->SendToClient(l_plylst, l_plynum, l_toAll);
	}
}

void GroupServerApp::PC_FRND_INIT(Player *ply)
{
	friend_dat l_farray[200];
	int l_num = 200;
	m_tblX1->get_friend_dat(l_farray,l_num,ply->m_chaid[ply->m_currcha]);

	WPacket	l_toFrnd =GetWPacket();
	l_toFrnd.WriteCmd(CMD_PC_FRND_REFRESH);
	l_toFrnd.WriteChar(MSG_FRND_REFRESH_ONLINE);
	l_toFrnd.WriteLong(ply->m_chaid[ply->m_currcha]);

	WPacket	l_toSelf =GetWPacket();
	l_toSelf.WriteCmd(CMD_PC_FRND_REFRESH);
	l_toSelf.WriteChar(MSG_FRND_REFRESH_START);

	l_toSelf.WriteLong(ply->m_chaid[ply->m_currcha]);
	l_toSelf.WriteString(ply->m_chaname[ply->m_currcha].c_str());
	l_toSelf.WriteString(ply->m_motto[ply->m_currcha].c_str());
	l_toSelf.WriteShort(ply->m_icon[ply->m_currcha]);

	ply->m_CurrFriendNum	=0;

	Player *l_plylst[10240];
	short	l_plynum	=0;

	Player	*	l_ply1;
	char	l_currcha;
	for(int i=0;i<l_num;i++)
	{
		if(l_farray[i].cha_id == 0)
		{
			if(l_farray[i].icon_id == 0)
			{
				l_toSelf.WriteShort(uShort(l_farray[i].memaddr));
			}
			else
			{
				l_toSelf.WriteString(l_farray[i].relation.c_str());
				l_toSelf.WriteShort(uShort(l_farray[i].memaddr));
				ply->m_CurrFriendNum += l_farray[i].memaddr;
			}
		}
		else if((l_ply1 =(Player*)MakePointer(l_farray[i].memaddr))
			&&((l_currcha =l_ply1->m_currcha)>=0)
			&&(l_ply1->m_chaid[l_currcha] ==l_farray[i].cha_id))
		{
			l_plylst[l_plynum]	=l_ply1;
			l_plynum++;

			l_toSelf.WriteLong(l_farray[i].cha_id);
			l_toSelf.WriteString(l_farray[i].cha_name.c_str());
			l_toSelf.WriteString(l_farray[i].motto.c_str());
			l_toSelf.WriteShort(l_farray[i].icon_id);
			l_toSelf.WriteChar(1);
		}
		else
		{
			l_toSelf.WriteLong(l_farray[i].cha_id);
			l_toSelf.WriteString(l_farray[i].cha_name.c_str());
			l_toSelf.WriteString(l_farray[i].motto.c_str());
			l_toSelf.WriteShort(l_farray[i].icon_id);
			l_toSelf.WriteChar(0);
		}
	}
	SendToClient(ply,l_toSelf);
	LogLine	l_line(g_LogFriend);
	//l_line<<newln<<"����֪ͨ�ĺ�������"<<l_plynum<<endln;
	l_line<<newln<<"online friends num: "<<l_plynum<<endln;
	SendToClient(l_plylst,l_plynum,l_toFrnd);
}
