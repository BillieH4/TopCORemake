
#include "gateserver.h"


ToGameServer::ToGameServer(char const* fname, ThreadPool* proc, ThreadPool* comm)
: TcpServerApp(this, proc, comm), RPCMGR(this), _mut_game(),
_game_heap(1, 20), _game_list(NULL), _map_game()
{
	_mut_game.Create(false); _game_num = 0;

	// ��ʼ����
	IniFile inf(fname);
	IniSection& is = inf["ToGameServer"];
	cChar* ip = is["IP"]; uShort port = atoi(is["Port"]);

	// ���� PING �߳�

	SetPKParse(0, 2, 64 * 1024, 400); BeginWork(atoi(is["EnablePing"]));
	if (OpenListenSocket(port, ip) != 0)
		THROW_EXCP(excp, "ToGameServer listen failed");
}

ToGameServer::~ToGameServer() {ShutDown(12 * 1000);}

void ToGameServer::_add_game(GameServer* game)
{
	game->next = _game_list;
	_game_list = game;
	++ _game_num;
}

bool ToGameServer::_exist_game(char const* game)
{
	GameServer* curr = _game_list;
	bool exist = false;

	while (curr)
	{
		if (curr->gamename == game) {exist = true; break;}
		curr = curr->next;
	}

	return exist;
}

void ToGameServer::_del_game(GameServer* game)
{
	GameServer* curr = _game_list;
	GameServer* prev = 0;
	while (curr)
	{
		if (curr == game) break;
		prev = curr; curr = curr->next;
	}

	if (curr)
	{
		if (prev)
			prev->next = curr->next;
		else
			_game_list = curr->next;
		-- _game_num;
	}
}

bool ToGameServer::OnConnect(DataSocket* datasock) // ����ֵ:true-��������,false-����������
{
	datasock->SetPointer(0);
	datasock->SetRecvBuf(64 * 1024);
	datasock->SetSendBuf(64 * 1024);
	LogLine l_line(g_gatelog);
	//l_line<<newln<<"GameServer= ["<<datasock->GetPeerIP()<<"] ����,Socket��Ŀ= "<<GetSockTotal()+1;
	l_line<<newln<<"GameServer= ["<<datasock->GetPeerIP()<<"] come,Socket num= "<<GetSockTotal()+1;
	return true;
}

void ToGameServer::OnDisconnect(DataSocket* datasock, int reason) // reasonֵ:0-���س��������˳���-3-���类�Է��رգ�-1-Socket����;-5-�����ȳ������ơ�
{
	LogLine l_line(g_gatelog);
	//l_line<<newln<<"GameServer= ["<<datasock->GetPeerIP()<<"] ����,Socket��Ŀ= "<<GetSockTotal()+1<<",reason= "<<GetDisconnectErrText(reason).c_str();
	l_line<<newln<<"GameServer= ["<<datasock->GetPeerIP()<<"] gone,Socket num= "<<GetSockTotal()+1<<",reason= "<<GetDisconnectErrText(reason).c_str();
	l_line<<endln;
	if(reason ==DS_SHUTDOWN || reason ==DS_DISCONN) return;
	GameServer* l_game = (GameServer *)(datasock->GetPointer());
	bool already_delete = false;
	if (l_game == NULL)
		return;

	// ��������ɾ���� GameServer
	_mut_game.lock();
	try
	{
		l_game = (GameServer *)(datasock->GetPointer());
		if (l_game != NULL)
		{
			//l_line<<newln<<"ɾ���� ["<<l_game->gamename.c_str()<<"]"<<endln;
			l_line<<newln<<" delete ["<<l_game->gamename.c_str()<<"]"<<endln;
			_del_game(l_game);
			for (int i = 0; i < l_game->mapcnt; ++ i)
			{
				//l_line<<newln<<"ɾ����ͼ ["<<l_game->maplist[i].c_str()<<"]"<<endln;
				l_line<<newln<<"delete map ["<<l_game->maplist[i].c_str()<<"]"<<endln;
				_map_game.erase(l_game->maplist[i]);
			}
			l_game->mapcnt = 0;
			l_game->Free();
			datasock->SetPointer(NULL);
		}else
		{
			already_delete = true;
		}
	}catch (...)
	{
		//l_line<<newln<<"Exception raised from OnDisconnect{��������ɾ���� GameServer}"<<endln;
		l_line<<newln<<"Exception raised from OnDisconnect{delete GameServer from list}"<<endln;
	}
	_mut_game.unlock();

	if (already_delete) return;

	// ֪ͨͨ����GateServer���ϴ�GameServer�������û�����ͼ����������
	RPacket retpk = g_gtsvr->gp_conn->get_playerlist();
	uShort ply_cnt = retpk.ReverseReadShort(); // ��GateServer��������Ҹ���

	Player* ply_addr; uLong db_id;
	Player** ply_array = new Player*[ply_cnt];
	uShort	l_notcount	=0;
	for (uShort i = 0; i < ply_cnt; ++ i)
	{
		ply_addr = (Player *)MakePointer(retpk.ReadLong());
		db_id = (uLong)retpk.ReadLong();
		if(l_game ==ply_addr->game)
		{
			ply_array[i -l_notcount] = ply_addr;
		}else
		{
			l_notcount	++;
			continue;
		}

		try
		{ // ����ȷ��
			uLong tmp_id = ply_addr->m_dbid;
			if (tmp_id != db_id) // �˽�ɫ������
				continue;
		}catch (...)		// �����쳣����ʶ�˽�ɫͬ���Ѳ�����
		{
			continue;
		}

		try
		{
			g_gtsvr->cli_conn->post_mapcrash_msg(ply_addr); // ���û���Ȼ���ߣ����͵�ͼ������������Ϣ
		}catch (...)
		{
			continue;
		}

		continue;
	}

	ply_cnt	-=l_notcount;
	//l_line<<newln<<"����GameServer ���ϣ�Ҫ֪ͨ "<<ply_cnt<<" ���û�����"<<endln;
	l_line<<newln<<"becaulse GameServer trouble, notice "<<ply_cnt<<" user offline"<<endln;
	for (i = 0; i < ply_cnt; ++ i)
	{
		try			//�����ϵ���������
		{
			//l_line<<newln<<"����GameServer ���ϣ������Ͽ��� ["<<ply_array[i]->m_datasock->GetPeerIP()<<"] ������"<<endln;
			l_line<<newln<<"becaulse GameServer trouble, disconnect ["<<ply_array[i]->m_datasock->GetPeerIP()<<"] "<<endln;
			g_gtsvr->cli_conn->Disconnect(ply_array[i]->m_datasock,100,-29);
		}catch (...)
		{
		}
	}

	delete ply_array;
}

WPacket ToGameServer::OnServeCall(DataSocket* datasock, RPacket &in_para)
{
	/*
	GameServer* l_game = (GameServer *)(datasock->GetPointer());
	WPacket l_retpk = GetWPacket();
	uShort l_cmd = in_para.ReadCmd();

	return l_retpk;
	*/

	return NULL;
}

void ToGameServer::OnProcessData(DataSocket* datasock, RPacket &recvbuf)
{
	GameServer* l_game = (GameServer *)(datasock->GetPointer());

	uShort l_cmd = recvbuf.ReadCmd();
	//LG("ToGameServer", "-->l_cmd = %d\n", l_cmd);
	try
	{
		switch (l_cmd)
		{


		case CMD_MT_SPECTATEUPDATE:{
			//int userID = recvbuf.ReverseReadLong();
			//Player *l_ply = g_gtsvr->GetPlayerByID(userID);
			Player	*l_ply = (Player *)MakePointer(recvbuf.ReverseReadLong());
			if (l_ply){
				WPacket	l_wpk = WPacket(recvbuf).Duplicate();
				l_wpk.WriteCmd(CMD_MC_ENTERMAP);
				l_ply->m_datasock->SendData(l_wpk);
			}
			break;
		}
		case CMD_MT_SPECTATE:{
			//int userID = recvbuf.ReadLong();//user who is spectating
			//int spectatorID = recvbuf.ReadLong();//user being spectated
			bool hijack = recvbuf.ReadChar();
			bool lock = recvbuf.ReadChar();

			int specID = recvbuf.ReadLong();
			int targetID = recvbuf.ReadLong();

			Player	*l_ply = (Player *)MakePointer(specID);
			Player	*l_ply2 = (Player *)MakePointer(targetID);

			if (!l_ply){
				break;
			}

			if (!l_ply2 || l_ply == l_ply2){
				l_ply->StopSpectating();
			}else{
				l_ply->SetSpectating(l_ply2,hijack,lock);
			}
			
			break;
		}

		case CMD_MP_GM1SAY:
		case CMD_MP_DISCORD2PM:{
			g_gtsvr->gp_conn->SendData(g_gtsvr->gp_conn->get_datasock(), WPacket(recvbuf));
			break;
		}
		case CMD_MT_DISCORDLOGS:{
			GameServer* discord = g_gtsvr->gm_conn->find("Discord");
			if (discord){
				discord->m_datasock->SendData(WPacket(recvbuf));
			}
			break;
		}

		case CMD_MT_SAYFROMDISCORD:{
			//send this to all ingame players.
			WPacket l_wpk = GetWPacket();
			l_wpk.WriteCmd(CMD_PC_SAY2DIS);
			l_wpk.WriteString(recvbuf.ReadString());
			l_wpk.WriteString(recvbuf.ReadString());
			RPacket retpk = g_gtsvr->gp_conn->get_playerlist();
			uShort ply_cnt = retpk.ReverseReadShort();
			Player* ply_addr;
			for (uShort i = 0; i < ply_cnt; ++i){
				ply_addr = (Player *)MakePointer(retpk.ReadLong());
				retpk.ReadLong();
				ply_addr->m_datasock->SendData(l_wpk);
			}
			break;
		}
		case CMD_MT_LOGIN:
			MT_LOGIN(datasock, recvbuf);        
			break;
		case CMD_MT_SWITCHMAP:
			{
				RPacket	l_rpk		=recvbuf;
				uShort	l_aimnum	=l_rpk.ReverseReadShort();		//l_aimnum��Զ����1

				Player *l_ply	=(Player *)MakePointer(l_rpk.ReverseReadLong());
				if(l_ply->m_dbid	!=l_rpk.ReverseReadLong())					//chaid
				{
					return;
				}
				uChar	l_return	=l_rpk.ReverseReadChar();
				recvbuf.DiscardLast(sizeof(uChar) +sizeof(uLong)*2*l_aimnum + sizeof(uShort));

				cChar	*l_srcmap	=l_rpk.ReadString();
				Long	lSrcMapCopyNO = l_rpk.ReadLong();
				//...
				uLong	l_srcx	=l_rpk.ReadLong();	//����
				uLong	l_srcy	=l_rpk.ReadLong();	//����

				cChar	*l_map	=l_rpk.ReadString();
				Long	lMapCopyNO = l_rpk.ReadLong();
				//...
				uLong	l_x	=l_rpk.ReadLong();	//����
				uLong	l_y	=l_rpk.ReadLong();	//����
				GameServer *l_game	=g_gtsvr->gm_conn->find(l_map);

				LogLine l_line(g_gatelog);
				if (l_game)
				{
					l_ply->game->m_plynum--;
					l_ply->game		=0;
					l_ply->gm_addr	=0;
					//l_line<<newln<<"�ͻ���: "<<l_ply->m_datasock->GetPeerIP()<<":"<<l_ply->m_datasock->GetPeerPort()
					l_line<<newln<<"clinet: "<<l_ply->m_datasock->GetPeerIP()<<":"<<l_ply->m_datasock->GetPeerPort()
						//<<"	Switch����ͼ,Gate��["<<l_game->m_datasock->GetPeerIP()<<"]������EnterMap����,dbid:"<<l_ply->m_dbid
						<<"	Switch to map,to Gate["<<l_game->m_datasock->GetPeerIP()<<"]send EnterMap command,dbid:"<<l_ply->m_dbid
						//<<uppercase<<hex<<",����Gate��ַ:"<<MakeULong(l_ply)<<dec<<nouppercase<<endln;
						<<uppercase<<hex<<",Gate address:"<<MakeULong(l_ply)<<dec<<nouppercase<<endln;
                    l_game->EnterMap(l_ply, l_ply->m_loginID,l_ply->m_dbid,l_ply->m_worldid, l_map, lMapCopyNO, l_x, l_y, 1,l_ply->m_sGarnerWiner);	//���ݵ�ͼ����GameServer��Ȼ������GameServer�Խ��������ͼ��
					l_game->m_plynum++;
				}else if(!l_return) //Ŀ���ͼ���ɴ���½���Դ��ͼ
				{
					WPacket	l_wpk	=datasock->GetWPacket();
					l_wpk.WriteCmd(CMD_MC_SYSINFO);
					//l_wpk.WriteString(dstring("[")<<l_map<<"]��ǰ���ɵ�����Ժ����ԣ�");
					l_wpk.WriteString(dstring("[")<<l_map<<"] can't reach, pealse retry later!");
					l_ply->m_datasock->SendData(l_wpk);

					//l_line<<newln<<"�ͻ���: "<<l_ply->m_datasock->GetPeerIP()<<":"<<l_ply->m_datasock->GetPeerPort()
					l_line<<newln<<"clinet: "<<l_ply->m_datasock->GetPeerIP()<<":"<<l_ply->m_datasock->GetPeerPort()
						//<<"	Switch�ص�ͼ,Gate��["<<l_ply->game->m_datasock->GetPeerIP()<<"]������EnterMap����,dbid:"<<l_ply->m_dbid
						<<"	Switch back map,to Gate["<<l_ply->game->m_datasock->GetPeerIP()<<"]send EnterMap command,dbid:"<<l_ply->m_dbid
						//<<uppercase<<hex<<",����Gate��ַ:"<<MakeULong(l_ply)<<dec<<nouppercase<<endln;
						<<uppercase<<hex<<",Gate address:"<<MakeULong(l_ply)<<dec<<nouppercase<<endln;
					l_ply->game->EnterMap(l_ply, l_ply->m_loginID,l_ply->m_dbid,l_ply->m_worldid, l_srcmap, lSrcMapCopyNO, l_srcx, l_srcy, 1,l_ply->m_sGarnerWiner);	//���ݵ�ͼ����GameServer��Ȼ������GameServer�Խ��������ͼ��
				}else
				{
					g_gtsvr->cli_conn->Disconnect(l_ply->m_datasock,0,-24);
				}

				//if(!l_game)						//Ŀ���ͼ���ɴ�
				//{
				//	//char l_tmp[256];
				//	WPacket	l_wpk	=datasock->GetWPacket();
				//	l_wpk.WriteCmd(CMD_MC_SYSINFO);
				//	char l_sysinfo[256];
				//	strcpy(l_sysinfo,"[");
				//	strcat(l_sysinfo,l_map);
				//	strcat(l_sysinfo,"]��ǰ���ɵ�����Ժ����ԣ�");

				//	l_wpk.WriteString(l_sysinfo);
				//	l_ply->m_datasock->SendData(l_wpk);
				//}else if(l_game !=l_ply->game && l_game->m_plynum >1500)	//��������
				//{
				//	//char l_tmp[256];
				//	WPacket	l_wpk	=datasock->GetWPacket();
				//	l_wpk.WriteCmd(CMD_MC_SYSINFO);
				//	char l_sysinfo[256];
				//	strcpy(l_sysinfo,"[");
				//	strcat(l_sysinfo,l_map);
				//	strcat(l_sysinfo,"]���ڷ�������ǰ�������࣬���Ժ����ԣ�");

				//	l_wpk.WriteString(l_sysinfo);
				//	l_ply->m_datasock->SendData(l_wpk);
				//}else
				//{
				//	//char l_tmp[256];
				//	SwitchMap	*l_switch	=g_swmap.Get();
				//	l_switch->Init(l_ply,l_game,l_map,l_x,l_y);
				//	g_gtsvr->m_proc->AddTask(l_switch);
				//}
				break;
			}
		case CMD_MC_ENTERMAP:
			{
				RPacket	l_rpk		=recvbuf;
				uShort	l_aimnum	=l_rpk.ReverseReadShort();					//l_aimnum��Զ����1

				Player	*l_ply	=(Player *)MakePointer(l_rpk.ReverseReadLong());
				uLong		l_dbid	=l_rpk.ReverseReadLong();
				LogLine l_line(g_gatelog);
				if(l_ply->m_dbid	!=l_dbid)					//chaid
				{
					//l_line<<newln<<"�յ�һ������["<<datasock->GetPeerIP()<<"]��EnterMap����,���ͱ��ص�DBID��һ�£�����["<<l_ply->m_dbid<<"],Զ��["<<l_dbid<<"]"
					l_line<<newln<<"recieve from ["<<datasock->GetPeerIP()<<"] EnterMap command ,can't match DBID:locale ["<<l_ply->m_dbid<<"],far["<<l_dbid<<"]"
						//<<uppercase<<hex<<",����Gate��ַ:"<<MakeULong(l_ply)<<endln;
						<<uppercase<<hex<<",Gate address:"<<MakeULong(l_ply)<<endln;
					return;
				}
				uShort	l_retcode	=l_rpk.ReadShort();
				if( l_retcode==ERR_SUCCESS)
				{
					l_ply->game			=l_game;
					l_ply->gm_addr		=l_rpk.ReverseReadLong();
					l_game->m_plynum	=l_rpk.ReverseReadLong();
					char	l_isSwitch	=l_rpk.ReverseReadChar();

					//l_line<<newln<<"�ͻ���: "<<l_ply->m_datasock->GetPeerIP()<<":"<<l_ply->m_datasock->GetPeerPort()
					l_line<<newln<<"clinet: "<<l_ply->m_datasock->GetPeerIP()<<":"<<l_ply->m_datasock->GetPeerPort()
						//<<"	Gate�յ�������["<<datasock->GetPeerIP()<<"]�ɹ�EnterMap����,������Game��ַ:"
						<<"	recieve Gate  from ["<<datasock->GetPeerIP()<<"]success EnterMap command,Game address:"
						//<<uppercase<<hex<<l_ply->gm_addr<<",����Gate��ַ:"<<MakeULong(l_ply)<<dec<<nouppercase<<endln;
						<<uppercase<<hex<<l_ply->gm_addr<<",Gate address:"<<MakeULong(l_ply)<<dec<<nouppercase<<endln;

					recvbuf.DiscardLast(sizeof(uShort) + sizeof(uLong)*2*l_aimnum + sizeof(uLong)*2 +sizeof(uChar));

					l_ply->SendPacketToClient(recvbuf);

					/*
					if (l_ply->spectatorID == 0){
						g_gtsvr->cli_conn->SendData(l_ply->m_datasock, recvbuf);
						g_gtsvr->SendSpectatorPacket(l_ply, recvbuf);
					}*/
					{
						WPacket l_wpk	=GetWPacket();
						l_wpk.WriteCmd(CMD_MP_ENTERMAP);
						l_wpk.WriteChar(l_isSwitch);
						l_wpk.WriteLong(MakeULong(l_ply));
						l_wpk.WriteLong(l_ply->gp_addr);
						g_gtsvr->gp_conn->SendData(g_gtsvr->gp_conn->get_datasock(),l_wpk);

#ifdef CHAEXIT_ONTIME
						if( l_ply->m_exit != 0 )
						{
							WPacket l_wpk = datasock->GetWPacket();
							l_wpk.WriteCmd(CMD_TM_GOOUTMAP);
							l_wpk.WriteChar(1);
							l_wpk.WriteLong(MakeULong(l_ply));
							l_wpk.WriteLong(l_ply->gm_addr);			//����GameServer�ϵĵ�ַ
							g_gtsvr->gm_conn->SendData(l_game->m_datasock,l_wpk);
						}
#endif
					}
				}else
				{
					l_ply->m_status	=1;
					l_game->m_plynum--;
					//l_line<<newln<<"�ͻ���: "<<l_ply->m_datasock->GetPeerIP()<<":"<<l_ply->m_datasock->GetPeerPort()
					l_line<<newln<<"clinet: "<<l_ply->m_datasock->GetPeerIP()<<":"<<l_ply->m_datasock->GetPeerPort()
						//<<"	Gate�յ�������["<<datasock->GetPeerIP()<<"]ʧ��EnterMap����,������:"
						<<"	Gate recieve from ["<<datasock->GetPeerIP()<<"]failed EnterMap command ,Error:"
						<<l_retcode<<endln;

					recvbuf.DiscardLast(sizeof(uShort) + sizeof(uLong)*2*l_aimnum);
					//g_gtsvr->cli_conn->SendData(l_ply->m_datasock,recvbuf);
					g_gtsvr->cli_conn->Disconnect(l_ply->m_datasock, 10, -33 );
				}
				break;
			}
#ifdef CHAEXIT_ONTIME
		case CMD_MC_STARTEXIT:
			{
				RPacket	l_rpk		=recvbuf;
				uShort	l_aimnum	=l_rpk.ReverseReadShort();					//l_aimnum��Զ����1
				Player	*l_ply	=(Player *)MakePointer(l_rpk.ReverseReadLong());
				if( l_ply )
				{
					g_gtsvr->cli_conn->SendData(l_ply->m_datasock,recvbuf);
					//printf( "StartExit: id = %d\n", l_ply->m_actid );
				}
			}
			break;
		case CMD_MC_CANCELEXIT:
			{
				RPacket	l_rpk		=recvbuf;
				uShort	l_aimnum	=l_rpk.ReverseReadShort();					//l_aimnum��Զ����1
				Player	*l_ply	=(Player *)MakePointer(l_rpk.ReverseReadLong());
				if( l_ply )
				{
					MutexArmor l_lockStat(l_ply->m_mtxstat);
					if( !l_ply->m_datasock->IsFree() && l_ply->m_exit != 3 )
					{
						l_ply->m_exit = 0;
						g_gtsvr->cli_conn->SendData(l_ply->m_datasock,recvbuf);
						//printf( "MC:CancelExit: id = %d\n", l_ply->m_actid );
						l_lockStat.unlock();
					}
				}
			}
			break;		
		case CMD_MT_PALYEREXIT:
			{
				uShort	l_aimnum	=recvbuf.ReverseReadShort();
				for(uShort i=0;i<l_aimnum;i++)
				{
					Player *l_ply	=(Player *)MakePointer(recvbuf.ReverseReadLong());
					if(l_ply && l_ply->m_dbid ==recvbuf.ReverseReadLong())
					{
						MutexArmor l_lockStat(l_ply->m_mtxstat);
						//printf( "PlayerExit id = %d, status = %d\n", l_ply->m_actid, l_ply->m_status );
						uLong	l_ulMilliseconds	=30*1000;
						if( l_ply->m_status == 2 && l_ply->m_exit == 1 ) // ѡ���ɫ
						{
							WPacket l_wpk = g_gtsvr->gp_conn->GetWPacket();
							l_wpk.WriteCmd(CMD_TP_ENDPLAY);
							l_wpk.WriteLong(MakeULong(l_ply));
							l_wpk.WriteLong(l_ply->gp_addr);
							l_wpk	=g_gtsvr->gp_conn->SyncCall(g_gtsvr->gp_conn->get_datasock(),l_wpk,l_ulMilliseconds);
							if(!l_wpk.HasData())
							{
								l_wpk	=datasock->GetWPacket();
								l_wpk.WriteCmd(CMD_MC_ENDPLAY);
								l_wpk.WriteShort(ERR_MC_NETEXCP);
								//SendData(datasock,l_wpk);
								g_gtsvr->cli_conn->SendData(l_ply->m_datasock,l_wpk);
							}else
							{
								//���ظ�Client
								l_wpk.WriteCmd(CMD_MC_ENDPLAY);
								//SendData(datasock,l_wpk);
								g_gtsvr->cli_conn->SendData(l_ply->m_datasock,l_wpk);
								l_ply->m_status	=1;//����ѡ��ɫ����״̬
								l_ply->m_exit   =0;
								l_ply->m_dbid	=0;
								l_ply->m_worldid=0;
								if(RPacket(l_wpk).ReadShort() ==ERR_PT_KICKUSER)
								{
									Disconnect(l_ply->m_datasock,100,-25);
								}
							}
						}
						else if( l_ply->m_status == 2 && l_ply->m_exit == 2 ) // �ǳ��ͷ���Դ
						{
							/*
							WPacket l_wpk = g_gtsvr->gp_conn->GetWPacket();
							l_wpk.WriteCmd( CMD_TP_PLAYEREXIT );
							l_wpk.WriteLong( l_ply->m_dbid );
							g_gtsvr->gp_conn->SendData( g_gtsvr->gp_conn->get_datasock(),l_wpk);
							*/
							WPacket l_wpk	=g_gtsvr->gp_conn->get_datasock()->GetWPacket();
							l_wpk.WriteCmd(CMD_TP_DISC);
							l_wpk.WriteLong(l_ply->m_actid);
							l_wpk.WriteLong(inet_addr(datasock->GetPeerIP()));
							l_wpk.WriteString(GetDisconnectErrText(datasock->GetDisconnectReason()?datasock->GetDisconnectReason():-27).c_str());
							g_gtsvr->gp_conn->SendData(g_gtsvr->gp_conn->get_datasock(),l_wpk);

							l_wpk	=g_gtsvr->gp_conn->get_datasock()->GetWPacket();
							l_wpk.WriteCmd(CMD_TP_USER_LOGOUT);
							l_wpk.WriteLong(MakeULong(l_ply));
							l_wpk.WriteLong(l_ply->gp_addr);
							l_ply->gp_addr	=0;
							RPacket l_retpk	=g_gtsvr->gp_conn->SyncCall(g_gtsvr->gp_conn->get_datasock(),l_wpk,l_ulMilliseconds);
							//printf( "PlayerExit �ͷŽ�ɫid = %d, status = %d\n", l_ply->m_actid, l_ply->m_status );
							g_gtsvr->cli_conn->Disconnect(l_ply->m_datasock,0,-23);	//��23�������ʾGameServerҪ���ߵ�ĳ��
							l_ply->Free();
						}
						l_lockStat.unlock();
					}
				}
				break;
			}
#endif
		case CMD_MT_KICKUSER:
			{
				uShort l_aimnum = recvbuf.ReverseReadShort();
				Player *l_ply = (Player *)MakePointer(recvbuf.ReverseReadLong());
				uLong b = recvbuf.ReverseReadLong();
				if(l_ply && l_ply->m_dbid == b)
				{
					g_gtsvr->cli_conn->Disconnect(l_ply->m_datasock,0,-23);
				}
				break;
			}
		case CMD_MT_MAPENTRY:
			{
				WPacket l_wpk0 = WPacket( recvbuf ).Duplicate();
				WPacket l_wpk = l_wpk0;

				RPacket	l_rpk = recvbuf;
				cChar	*l_map = l_rpk.ReadString();
				GameServer *l_game = g_gtsvr->gm_conn->find(l_map);			
				if( l_game )
				{
					l_wpk.WriteCmd( CMD_TM_MAPENTRY );
					g_gtsvr->gm_conn->SendData(l_game->m_datasock,l_wpk);
				}
				else
				{
					l_wpk.WriteCmd( CMD_TM_MAPENTRY_NOMAP );
					g_gtsvr->gm_conn->SendData(datasock,l_wpk);
				}
			}
			break;
		default:		// ȱʡת��
			{
				if(l_cmd/500 == CMD_MC_BASE/500)
				{
					RPacket	l_rpk		=recvbuf;
					uShort	l_aimnum	=l_rpk.ReverseReadShort();
					recvbuf.DiscardLast(sizeof(uLong)*2*l_aimnum + sizeof(uShort));
					for(uShort i=0;i<l_aimnum;i++)
					{
						Player *l_ply	=(Player *)MakePointer(l_rpk.ReverseReadLong());
						if(l_ply->m_dbid ==l_rpk.ReverseReadLong())
						{
							l_ply->SendPacketToClient(recvbuf);
						//	l_ply->SendPacketToClient(l_rpk);
							/*
							if (l_ply->spectatorID == 0){
								g_gtsvr->cli_conn->SendData(l_ply->m_datasock, recvbuf);
								g_gtsvr->SendSpectatorPacket(l_ply, recvbuf);
							}
							*/
							


						}
					}
				}else if(l_cmd/500 == CMD_MP_BASE/500)
				{
					RPacket l_rpk		=recvbuf;
					uShort	l_aimnum	=l_rpk.ReverseReadShort();
					recvbuf.DiscardLast(sizeof(uLong)*2*l_aimnum + sizeof(uShort));
					if( l_aimnum > 0 )
					{
						WPacket l_wpk,l_wpk0 =WPacket(recvbuf).Duplicate();
						for(uShort i=0;i<l_aimnum;i++)
						{
							Player *l_ply	=(Player *)MakePointer(l_rpk.ReverseReadLong());
							if(l_ply->m_dbid ==l_rpk.ReverseReadLong())
							{
								l_wpk			=l_wpk0;
								l_wpk.WriteLong(MakeULong(l_ply));
								l_wpk.WriteLong(l_ply->gp_addr);
								g_gtsvr->gp_conn->SendData(g_gtsvr->gp_conn->get_datasock(),l_wpk);
							}
						}
					}
					else
					{
						WPacket l_wpk =WPacket(recvbuf).Duplicate();
						g_gtsvr->gp_conn->SendData(g_gtsvr->gp_conn->get_datasock(),l_wpk);
					}
				}else if(l_cmd/500 ==CMD_MM_BASE/500)
				{
					for(GameServer *l_game =_game_list;l_game;l_game =l_game ->next)
					{
						g_gtsvr->gm_conn->SendData(l_game->m_datasock,recvbuf);
					}
				}
				break;
			}
		}
	}
	catch(...)
	{
		LG("ToGameServerError", "l_cmd = %d\n", l_cmd);
	}
	//LG("ToGameServer", "<--l_cmd = %d\n", l_cmd);
}

void ToGameServer::MT_LOGIN(DataSocket* datasock, RPacket& rpk)
{
	cChar* gms_name = rpk.ReadString();
	cChar* map_list = rpk.ReadString();
	GameServer* gms = _game_heap.Get();
	WPacket retpk = GetWPacket();
	bool valid = true;
	int i;

	retpk.WriteCmd(CMD_TM_LOGIN_ACK);
	int cnt = Util_ResolveTextLine(map_list, gms->maplist, MAX_MAP, ';', 0);
	LogLine l_line(g_gatelog);
	//l_line<<newln<<"�յ�GameServer ["<<gms_name<<"] ��ͼ��["<<map_list<<"] ��["<<cnt<<"]��"<<endln;
	l_line<<newln<<"recieve GameServer ["<<gms_name<<"] map string ["<<map_list<<"] total ["<<cnt<<"]"<<endln;
	if (cnt <= 0)
	{ // MAP���﷨�д�
		//l_line<<newln<<"��ͼ�� ["<<map_list<<"] �����﷨��������';'�ָ�"<<endln;
		l_line<<newln<<"map string ["<<map_list<<"] has syntax mistake, please use ';'compart"<<endln;
		retpk.WriteShort(ERR_TM_MAPERR);
		datasock->SetPointer(NULL);
		gms->Free();
	}else
	{
		gms->gamename = gms_name;
		gms->mapcnt = cnt;

		_mut_game.lock();
		try
		{
			do
			{ // ���ȼ�� GameServer �����Ƿ���ע��
				if (_exist_game(gms_name))
				{
					//l_line<<newln<<"����ͬ����GameServer: "<<gms_name<<endln;
					l_line<<newln<<"the same name GameServer exsit: "<<gms_name<<endln;
					retpk.WriteShort(ERR_TM_OVERNAME);
					datasock->SetPointer(NULL);
					valid = false; break;
				}

				// ��μ���ͼ���Ƿ�����ظ���
				for (i = 0; i < cnt; ++ i)
				{
					if (find(gms->maplist[i].c_str()) != NULL)
					{
						//l_line<<newln<<"����ͬ����MAP: "<<gms->maplist[i].c_str()<<endln;
						l_line<<newln<<"the same name MAP exsit: "<<gms->maplist[i].c_str()<<endln;
						retpk.WriteShort(ERR_TM_OVERMAP);
						datasock->SetPointer(NULL);
						valid = false;
						break;
					}
				}
			} while (false);

			if (valid)
			{ // �Ϸ��� GameServer�� ���뵽����
				_add_game(gms); // ��ӵ�������
				//l_line<<newln<<"���GameServer ["<<gms_name<<"] �ɹ�"<<endln;
				l_line<<newln<<"add GameServer ["<<gms_name<<"] ok"<<endln;
				for (i = 0; i < cnt; ++ i) // ��ӵ� map ��
				{
					//l_line<<newln<<"��� ["<<gms_name<<"] �ϵ� ["<<gms->maplist[i].c_str()<<"] ��ͼ�ɹ�"<<endln;
					l_line<<newln<<"add ["<<gms_name<<"]  ["<<gms->maplist[i].c_str()<<"] map ok"<<endln;
					_map_game[gms->maplist[i]] = gms;
				}

				datasock->SetPointer(gms);
				gms->m_datasock	= datasock;
				retpk.WriteShort(ERR_SUCCESS);
				retpk.WriteString(g_gtsvr->gp_conn->_myself.c_str());
			}else
			{ // �Ƿ��� GateServer
				gms->Free();
			}
		}
		catch (...)
		{
			//l_line<<newln<<"Exception raised from MT_LOGIN{��ӵ�ͼ}"<<endln;
			l_line<<newln<<"Exception raised from MT_LOGIN{add map}"<<endln;
		}
		_mut_game.unlock();

		//if (!valid) Disconnect(datasock);
	}

	SendData(datasock, retpk);
}

GameServer* ToGameServer::find(cChar* mapname)
{
	map<string, GameServer*>::iterator it = _map_game.find(mapname);
	if (it == _map_game.end())
	{
		LogLine l_line(g_gatelog);
		//l_line<<newln<<"δ�ҵ� ["<<mapname<<"] ��ͼ������";
		l_line<<newln<<"not found ["<<mapname<<"] map!!!";
		return NULL;
	}else
		return (*it).second;
}

void GameServer::Initially()
{
	m_plynum	=0;
	gamename = ""; ip = ""; port = 0;
	m_datasock = NULL; next = NULL;
	mapcnt = 0;
}

void GameServer::Finally()
{
	m_plynum	=0;
	gamename = ""; ip = ""; port = 0;
	m_datasock = NULL; next = NULL;
	mapcnt = 0;
}

void GameServer::EnterMap(Player *ply,uLong actid, uLong dbid,uLong worldid,cChar *map, Long lMapCpyNO,uLong x,uLong y,char entertype,short swiner)
{
	WPacket l_wpk	=m_datasock->GetWPacket();
	l_wpk.WriteCmd(CMD_TM_ENTERMAP);
    l_wpk.WriteLong(actid);
	l_wpk.WriteString(ply->m_password);
	l_wpk.WriteLong(dbid);
	l_wpk.WriteLong(worldid);
	l_wpk.WriteString(map);
	l_wpk.WriteLong(lMapCpyNO);
	l_wpk.WriteLong(x);
	l_wpk.WriteLong(y);
	l_wpk.WriteChar(entertype);
	l_wpk.WriteLong(MakeULong(ply));		//��һ�θ������Լ��ĵ�ַ
	l_wpk.WriteShort(swiner);
	g_gtsvr->gm_conn->SendData(m_datasock,l_wpk);
	ply->SetMapName(map); // Chaos Blind
}
