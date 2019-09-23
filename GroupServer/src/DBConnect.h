#ifndef DBCONNECT_H
#define DBCONNECT_H
#include "util.h"
#include "ChaAttrType.h"
/*
class TBLSystem : public cfl_rs
{
public:
	TBLSystem(cfl_db* db):cfl_rs(db,"system1",SYS_MAXCOL)
	{}
	int Increment();
	void Decrement();
private:
	enum{SYS_MAXCOL	=3};
	string m_buf[SYS_MAXCOL];
};*/

struct stPersonInfo;
struct stQueryPersonInfo;
struct stQueryResoultPersonInfo;

#define MAXORDERNUM 5
class TBLAccounts : public  cfl_rs
{
public:
	TBLAccounts(cfl_db* db):cfl_rs(db,"account",ACT_MAXCOL)
	{}
	void AddStatLog(long login,long play,long wgplay=0);

	bool IsReady();
	bool SetDiscInfo(int actid,const char *cli_ip,const char *reason);
	bool InsertRow(int act_id,const char *act_name,const char *cha_ids);
	bool UpdateRow(int act_id,const char *cha_ids);
	bool UpdatePassword( int act_id, const char szPassword[] );
	int FetchRowByActID(int act_id);
	int FetchRowByActName(const char szAccount[]);
	const char *GetActName(){return m_buf[0].c_str();}
	int GetActID() {return atoi(m_buf[0].c_str());} 
	char		GetGM()		 {return atoi(m_buf[1].c_str());}
	const char *GetChaIDs() {return m_buf[2].c_str();}
	const char *GetPassword() { return m_buf[3].c_str(); }
	const char *GetLast_IP() {return m_buf[4].c_str();}
	const char *GetDisc_Reason(){return m_buf[5].c_str();}
	const char *GetLast_Leave()	{return m_buf[6].c_str();}
private:
	enum{ACT_MAXCOL	=10};
	string m_buf[ACT_MAXCOL];
};
class TBLCharacters : public cfl_rs
{
public:
	TBLCharacters(cfl_db* db):cfl_rs(db,"character",CHA_MAXCOL)
	{}
	bool ZeroAddr();
	bool SetAddr(long cha_id,long addr);
	bool InsertRow(const char *cha_name,int act_id,const char *birth,const char * map,const char *look);
	bool UpdateInfo(unsigned long cha_id,unsigned short icon,const char * motto);
	//For������ɫ
	int	FetchRowByChaName(const char *cha_name);
	int			Getcha_id()			{return atoi(m_buf[0].c_str());}
	const char *GetMottonByName()	{return		 m_buf[1].c_str() ;}
	short		GetIconByName()		{return atoi(m_buf[2].c_str());}
	//For��ͨ��¼
	int	FetchRowByChaID(int cha_id);
	int FetchChaIDByCharName(cChar* cha_name);
	int FetchActIDByCharName(cChar* cha_name);
	const char *GetChaName()	{return		 m_buf[0].c_str(); }	//��ɫ��
	const char *GetMotto()		{return		 m_buf[1].c_str(); }
	short		GetIcon()		{return	atoi(m_buf[2].c_str());}	//Сͼ��
	int			GetGuildID()	{return atoi(m_buf[3].c_str());}	//��������ID
	const char *GetGuildName()	{return		 m_buf[4].c_str(); }	//����������
	const char *GetJob()		{return		 m_buf[5].c_str(); }	//ְҵ
	short		GetDegree()		{return atoi(m_buf[6].c_str());}	//�ȼ�
	const char *GetMap()		{return		 m_buf[7].c_str(); }	//��ͼ��
	int			GetMap_X()		{return atoi(m_buf[8].c_str());}
	int			GetMap_Y()		{return atoi(m_buf[9].c_str());}
	const char *GetLook()		{return		 m_buf[10].c_str(); }
	int			GetStr()		{return	atoi(m_buf[11].c_str());}
	int			GetDex()		{return atoi(m_buf[12].c_str());}
	int			GetAgi()		{return atoi(m_buf[13].c_str());}
	int			GetCon()		{return	atoi(m_buf[14].c_str());}
	int			GetSta()		{return atoi(m_buf[15].c_str());}
	int			GetLuk()		{return atoi(m_buf[16].c_str());}
	int			GetGuildPermission()	{ return atoi(m_buf[17].c_str()); }
	int			GetChatColour()	{ return atoi(m_buf[18].c_str()); }
	bool	BackupRow(int cha_id);
	bool		IsEstop(int char_id);
	bool		Estop( const char *cha_name, uLong lTimes );
	bool		DelEstop( const char *cha_name );
	bool		StartEstopTime( int cha_id );
	bool		EndEstopTime( int cha_id );
	bool		AddMoney( int cha_id, DWORD dwMoney );
	bool		FetchAccidByChaName(const char *cha_name, int& cha_accid );

private:
	enum{CHA_MAXCOL	=24};

	string m_buf[CHA_MAXCOL];
};

class TBLFriends : public cfl_rs
{
public:
	TBLFriends(cfl_db* db):cfl_rs(db,"friends",FRD_MAXCOL)
	{}

	int GetFriendsCount(long cha_id1,long cha_id2);

	int GetGroupCount(long cha_id1);
	bool UpdateGroup(long cha_id1,const char *oldgroup,const char *newgroup);
	bool UpdateGroup(long cha_id1, long cha_id2, const char *newgroup);

	unsigned long GetFriendAddr(long cha_id1,long cha_id2);
	bool AddFriend(long cha_id1,long cha_id2);
	bool DelFriend(long cha_id1,long cha_id2);
private:
	enum{FRD_MAXCOL	=3};

	string m_buf[FRD_MAXCOL];
};

class TBLMaster : public cfl_rs
{
public:
	struct master_dat
	{
		unsigned long memaddr; // VA in GameServer
		unsigned int cha_id; // id of character
		string relation; // relationship
		string cha_name; // name of character
		unsigned int icon_id;
		string motto;
	};

	TBLMaster(cfl_db* db):cfl_rs(db,"master",MASTER_MAXCOL)
	{}
	int GetMasterCount(long cha_id);
	int GetPrenticeCount(long cha_id);
	int HasMaster(long cha_id1,long cha_id2);

	bool AddMaster(long cha_id1,long cha_id2);
	bool DelMaster(long cha_id1,long cha_id2);
	bool GetMasterData(master_dat* farray, int& array_num, unsigned int cha_id);
	bool GetPrenticeData(master_dat* farray, int& array_num, unsigned int cha_id);
	bool FinishMaster(long cha_id);
	bool InitMasterRelation(map<uLong, uLong> &mapMasterRelation);

private:
	enum{MASTER_MAXCOL	=6};

	string m_buf[MASTER_MAXCOL];
};

class TBLGuilds : public cfl_rs
{
public:
	TBLGuilds(cfl_db* db):cfl_rs(db,"guild",GLD_MAXCOL)
	{}
	bool IsReady();
	int FetchRowByName(const char *guild_name);
	int GetGuildStatPoint(int guildID);
	int GetGuildAttr(int guildID,int attr);
	bool IncrementGuildAttr(int guildID, int attr, int upgCost);
	bool SaveGuildPoints(int guildID, int money, int lv, int exp);
	bool SetGuildCircleColour(int guildID, int colour,int icon, bool skipCheck = false);
	int  Getguild_id(){return atoi(m_buf[0].c_str());}
	bool InitAllGuilds(char disband_days);
	bool InitGuildMember(Player *ply,uLong chaid,uLong gldid,int mode);
	bool Disband(uLong gldid);

private:
	bool SendGuildInfo(Player *ply);
	enum{GLD_MAXCOL	=50};

	string m_buf[GLD_MAXCOL];
};

struct ORDERINFO
{
	long nid;
	long nfightpoint;
	char strname[20];
	long nlev;
	char strjob[100];
	ORDERINFO():nid(-1),nfightpoint(0),nlev(-1)
	{};
};


class TBLParam : public cfl_rs
{
private:
	enum{PARAM_MAXCOL=7};
	ORDERINFO m_nOrder[MAXORDERNUM];

public:
	TBLParam(cfl_db* db):cfl_rs(db,"Param",PARAM_MAXCOL)
	{};

	bool InitParam(void);

	bool SaveParam(void);
	ORDERINFO * GetOrderData(int index){return &(m_nOrder[index]);};
	void UpdateOrder(ORDERINFO &Order);
	bool IsReady();
protected:
	string m_buf[PARAM_MAXCOL];
};
///

inline bool getGuildAttrCol(int attr, char attrCol[32]){
	switch (attr){
	case ATTR_PDEF:
		strcpy(attrCol, "PDEF");
		break;
	case ATTR_MSPD:
		strcpy(attrCol, "MSPD");
		break;
	case ATTR_ASPD:
		strcpy(attrCol, "ASPD");
		break;
	case ATTR_DEF:
		strcpy(attrCol, "DEF");
		break;
	case ATTR_HIT:
		strcpy(attrCol, "HIT");
		break;
	case ATTR_FLEE:
		strcpy(attrCol, "FLEE");
		break;
	case ATTR_MXATK:
		strcpy(attrCol, "MXATK");
		break;
	case ATTR_HREC:
		strcpy(attrCol, "HREC");
		break;
	case ATTR_SREC:
		strcpy(attrCol, "SREC");
		break;
	case ATTR_MXHP:
		strcpy(attrCol, "MXHP");
		break;
	case ATTR_MXSP:
		strcpy(attrCol, "MXSP");
		break;
	default:
		return false;
	}
	return true;
}
#endif


