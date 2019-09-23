#include "stdafx.h"
#define MAX_BIRTHPOINT 12

struct SBirthPoint // ����������
{
	char szMapName[16];  // ��ͼ��
	int  x;				 // ��ͼ����
	int  y;
};

struct SBirthplace // ������, �������������
{
	SBirthPoint PointList[MAX_BIRTHPOINT];
	int nCount;
	
	SBirthplace():nCount(0) {}
	
	void	Add(const char *pszMapName, int x, int y)
	{
		if(nCount>=MAX_BIRTHPOINT) return; // ��������������
		
		strcpy(PointList[nCount].szMapName, pszMapName);
		PointList[nCount].x = x;
		PointList[nCount].y = y;
		nCount++;
	}
};


class CBirthMgr // �����ع���
{
public:

	CBirthMgr::~CBirthMgr();
	void		 AddBirthPoint(const char *pszLocation, const char *pszMapName, int x, int y);
	SBirthPoint* GetRandBirthPoint(const char *pszLocation);
	void		 ClearAll();
	
protected:

	map<string, SBirthplace*>  _LocIdx;
};

// ��ӵ���������
inline void CBirthMgr::AddBirthPoint(const char *pszLocation, const char *pszMapName, int x, int y)
{
	SBirthplace *p = NULL;
	map<string, SBirthplace*>::iterator it = _LocIdx.find(pszLocation);
	if(it!=_LocIdx.end()) // �Ѵ��ڵĳ�����
	{
		p = (*it).second;
	}
	else
	{
		// �²����ĳ�����
		p = new SBirthplace;
		_LocIdx[pszLocation] = p;
	}
	
	p->Add(pszMapName, x, y);
}

inline SBirthPoint* CBirthMgr::GetRandBirthPoint(const char *pszLocation)
{
	map<string, SBirthplace*>::iterator it = _LocIdx.find(pszLocation);
	if(it!=_LocIdx.end())
	{
		SBirthplace *p = (*it).second;
		int nSel = rand()%(p->nCount);
		SBirthPoint *pPoint = &(p->PointList[nSel]);
		//LG("birth", "ѡ�������������[%s] %d %d\n", pPoint->szMapName, pPoint->x, pPoint->y);
		return pPoint;
	}
	return NULL;
}

extern CBirthMgr g_BirthMgr;

inline SBirthPoint* GetRandBirthPoint(const char *pszChaName, const char *pszLocation)
{
	SBirthPoint* pBirth = g_BirthMgr.GetRandBirthPoint(pszLocation);
	if(pBirth==NULL)
	{
		//LG("birth_error", "��Ч�ĳ�����[%s], Cha = [%s], ����ǿ������������\n", pszLocation, pszChaName);
		LG("birth_error", "invalid birth place[%s], Cha = [%s],will force to silver city\n", pszLocation, pszChaName);
		pBirth = g_BirthMgr.GetRandBirthPoint(RES_STRING(GM_BIRTHPLACE_H_00001));
	}
	return pBirth;
}
