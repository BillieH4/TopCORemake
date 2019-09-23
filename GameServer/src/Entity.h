/* Generated by Together */

#ifndef ENTITY_H
#define ENTITY_H

#include "GameAppNet.h"
#include "LogFile.h"
#include "Event.h"
#include "CompCommand.h"

class CFightAble;
class CItem;
class Monster;
class CAttachable;
class CEventEntity;
class CStateCellNode;
class CStateCell;
class CChaListNode;
class CEyeshotCell;
struct SSkillStateUnit;

//using	namespace	GAME;

namespace mission
{
	class CNpc;
	class CEventEntity;
};

#define defENTITY_NAME_LEN	32

/**
 * @alias ʵ��
 */
class Entity
{
	friend class SubMap;
	friend class CMgrUnit;
	friend class GameServerApp;
	friend class CCharacter;
	friend class CMoveAble;
	friend class CAction;
	friend class CFightAble;
	friend class CGameApp;
	friend class CChaSpawn;
	friend class CMapSwitchEntitySpawn;

public:
	void	Free();
	void	Initially();
	void	Finally();

	void			SetInitShape(const Square &shape){	m_shape	=shape;}
	const Square&	GetShape() const {return m_shape;}
	void			SetPos(const Point &pos) { m_shape.centre = pos;}
	void			SetPos(dbc::Long lPosX, dbc::Long lPosY) {m_shape.centre.x = lPosX; m_shape.centre.y = lPosY;}
	const Point&	GetPos()const {return m_shape.centre;}
	void			SetRadius(const long &lRadius) {m_shape.radius = lRadius;}
	const long		&GetRadius() const {return m_shape.radius;}

	dbc::uLong		GetID()const			{return m_ID;}
	short			GetCat()const			{return m_cat;}
	void			SetID(dbc::uLong ulID)	{m_ID = ulID;}
	void			SetCat(short sCat)		{m_cat = sCat;}
	void			SetHandle(dbc::Long lHandle) {m_lHandle = lHandle;}
	dbc::Long		GetHandle(void) {return m_lHandle;}
	void			SetHoldID(dbc::Long lID) {m_lHoldID = lID;}
	dbc::Long		GetHoldID(void) {return m_lHoldID;}
	SubMap		*	GetSubMap()const{return m_submap;}
	SubMap*			GetSubMapFar();
	void			SetSubMap(SubMap *pCMap) {m_submap = pCMap;}
	short			GetAngle()const	{return m_sAngle;}
	void			SetAngle(short sAngle)		{m_sAngle = sAngle;}
	const Circle	&GetTerritory() {return m_STerritory;}
	void			SetTerritory(Circle &STerritory) {m_STerritory = STerritory;}
	void			NotiChgToEyeshot(WPACKET chginf, bool bIncludeOwn = true);
	void			SetLogEnable(bool bEnable = true){m_CLog.SetEnable(bEnable);}
	void			SetName(dbc::cChar *cszName) {strncpy(m_name, cszName, defENTITY_NAME_LEN - 1); m_name[defENTITY_NAME_LEN -1] = '\0';}
	const char*     GetName() { return m_name; }
	const char*     GetLogName() { return m_CLog.GetLogName(); }
	void			SetBirthCity(dbc::cChar *cszName) {strncpy(m_szBirthCity, cszName, MAX_MAPNAME_LENGTH - 1); m_szBirthCity[MAX_MAPNAME_LENGTH -1] = '\0';}
	const char*     GetBirthCity() { return m_szBirthCity; }
	void			SetBirthMap(dbc::cChar *cszName) {strncpy(m_szBirthMap, cszName, MAX_MAPNAME_LENGTH - 1); m_szBirthMap[MAX_MAPNAME_LENGTH -1] = '\0';}
	const char*     GetBirthMap() { return m_szBirthMap; }
	void			SetWitherTime(dbc::Long lWitherTime) {m_SExistCtrl.lWitherTime = lWitherTime;}
	void			SetResumeTime(dbc::Long lResumeTime) {m_SExistCtrl.lResumeTime = lResumeTime;}
	void			SetEvent(CEvent &CEvt) {m_CEvent = CEvt;}
	CEvent&			GetEvent(void) {return m_CEvent;}

	virtual void	Run(dbc::uLong ulCurTick) {};
	Entity*			SearchByIDInEyeshot(dbc::cuLong culID);

	dbc::Short		GetExistState(void) {return m_SExistCtrl.sState;}
	void			SetExistState(dbc::Short sState) {m_SExistCtrl.sState = sState;}
	bool			IsLiveing(void);
	dbc::Short		GetStopState(void) {return m_SExistCtrl.sStopState;}
	void			SetStopState(dbc::Short sState) {m_SExistCtrl.sStopState = sState;}
	bool			GetEyeshotAbility(void) {return m_bActiveEyeshot;}
	void			SetEyeshotAbility(bool bEyeshot) {m_bActiveEyeshot = bEyeshot;}
	void			ActiveEyeshot(bool bActive = true);
	bool			IsValid(void) {return m_bValid;}
	void			SetValid(bool bValid = true) {m_bValid = bValid;}
	dbc::uShort		GetAreaAttr(void) {return m_usAreaAttr[1];}
	void			SetAreaAttr(dbc::uShort usAreaAttr) {m_usAreaAttr[0] = m_usAreaAttr[1]; m_usAreaAttr[1] = usAreaAttr;}
	dbc::uChar		GetIslandID(void) {return m_ucIslandID[1];}
	void			SetIslandID(dbc::uChar uchIsland) {m_ucIslandID[0] = m_ucIslandID[1]; m_ucIslandID[1] = uchIsland;}
	bool			IsInSafeArea(void) {return GetAreaAttr() & enumAREA_TYPE_NOT_FIGHT ? true : false;}

	CStateCellNode	*EnterStateCell(CStateCell *pStateCell, CChaListNode *pEntiNode, bool bIsIn = false);
	void			OutMgrUnit(CStateCellNode *pCMgrNode);
	void			SetCenterMgrNode(CStateCellNode *pCMgrNode);
	SSkillStateUnit	*GetAreaState(dbc::uChar uchStateID);
	void			RefreshArea(void);
	void			RefreshArea(Point *pSrcPos);
	dbc::Short		GetEyeshotWidth(void);
	dbc::Short		GetEyeshotHeight(void) {return GetEyeshotWidth();}
	bool			IsInEyeshot(Entity *pCTarEnti);

	// ʵ���л���ͼ��Ԫ�����¼���������
	virtual	void	GotoMapUnit() {};

	virtual CFightAble *IsFightAble(){return 0;}
	virtual CMoveAble *IsMoveAble(){return 0;}
	virtual CItem *IsItem(){return 0;}
	virtual Monster *IsMonster(){return 0;}
	virtual CCharacter *IsCharacter(){return 0;}
	virtual CAttachable *IsAttachable() {return 0;}
	virtual mission::CNpc* IsNpc() { return 0; }
	virtual mission::CEventEntity* IsEvent() { return 0; }

	void	BeginSee(Entity *obj){OnBeginSee(obj);}
	void	EndSee(Entity *obj){OnEndSee(obj);}

	void	WriteEventInfo(WPACKET &pk);
	void	SynEventInfo(void);

	CLogFile		m_CLog;
	dbc::uLong		m_ID;

protected:
	Entity();

	virtual	void	WritePK(WPACKET& wpk);			//д����ұ����������и��ӽṹ(���ٻ��޵�)����������
	virtual	void	ReadPK(RPACKET& rpk);			//�ع���ұ����������и��ӽṹ(���ٻ��޵�)

	virtual void ReflectINFof(Entity *srcent, WPACKET chginf){}

	virtual bool overlap(long& xdist,long& ydist);
private:
	virtual const Square & GetLapChkShape(){return m_shape;}
	virtual void OnBeginSee(Entity *){}
	virtual void OnEndSee(Entity *){}
	virtual void OnBeginSeen(CCharacter *pCCha) =0;		//ToDo:�����Լ�������Ϣ�Է�ӳ���ͻ���
	virtual void OnEndSeen(CCharacter *pCCha) =0;			//ToDo:�ӿͻ���ɾ���Լ�
	virtual void AreaChange(void){}

	bool	EdgeOverlap(long& xdist,long& ydist);
	bool	ObstacleOverlap(long& xdist,long& ydist);
public:
	bool		m_bValid;

	struct // ��Ӧʵ���������������CEntityAlloc�������ã�
	{
		dbc::Long	m_lHandle;
		dbc::Long	m_lHoldID;
	};

	short		m_cat;
	
	Point		m_lastpos;
	Square		m_shape;
	Circle		m_STerritory; // ���
	short		m_sAngle;
	/**
	 * @label ��ǰ���ڵĵ�ͼ 
	 * @labelDirection forward*/
	SubMap		*m_submap;
	CStateCellNode	*m_pCStateCellHead, *m_pCStateCellTail;	// ��¼���ڵĹ�����Ԫ���׽ڵ������ĵ����ڵĹ�����Ԫ
	struct
	{
		Entity	*m_pCEyeshotCellNext;
		Entity	*m_pCEyeshotCellLast;
		CEyeshotCell	*m_pCEyeshotHost;
	};

	dbc::Char	m_szBirthMap[MAX_MAPNAME_LENGTH];
	dbc::Char	m_szBirthCity[MAX_MAPNAME_LENGTH];

	CEvent		m_CEvent;

	struct SExistCtrl
	{
		dbc::Short	sState;			// ����״̬���μ�CompCommand.h �е�EExistState����
		dbc::Short	sStopState;		// ����ֹͣ���״̬��EExistState ��enumEXISTS_WAITING��enumEXISTS_SLEEPING��

		// ���������͸���
		dbc::Long	lWitherTime;	// ������Ҫ��ʱ�䣨���룩
		dbc::Long	lResumeTime;	// ������Ҫ��ʱ�䣨���룩
		dbc::uLong	ulTick;
		//
	};

	char			m_name[defENTITY_NAME_LEN];
	SExistCtrl		m_SExistCtrl;
	bool			m_bActiveEyeshot;

	dbc::uShort		m_usAreaAttr[2];	// ʵ�����ڵر����������ԣ�0��ʾ֮ǰ���������ԣ�1��ʾ���ڵģ��μ�CompCommand.h��EAreaMask
	dbc::uChar		m_ucIslandID[2];	// ʵ�����ڵر��ĵ�����Ϣ��0��ʾ֮ǰ�ĵ�����Ϣ��1��ʾ���ڵ�

};

extern void	NotiPkToWorld(WPACKET chginf);

#endif //OBJECT_H