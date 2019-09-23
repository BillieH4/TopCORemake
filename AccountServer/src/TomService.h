#pragma once


class CTomService
{
public:
	CTomService(void);
	~CTomService(void);

	bool InitService();													//��ʼ��Tom����ϵͳ
	bool VerifyLoginMember(const char* lpszUserName, const char* lpszMD5PSW);		//true:��ȷ false:����
	void ReportMemberCounts(int nCounts);								//��Tom���������������
	bool IsEnable();													//�����Ƿ�������Tom����ϵͳ
	//enum eState{eState_AllowLogin, eState_LoginLocked};
	//eState CheckMemberState(const char* lpszUserName);


private:
#ifndef NOAUTHDLL
	typedef void (*GameOnline)(int); 
	typedef bool (*UserAttest)(char* lpszUserName, char* lpszMD5PSW);
	UserAttest  pUserAttestFun;
	GameOnline  pGameOnlineFun;
#else
	bool pUserAttestFun(char* lpszUserName, char* lpszMD5PSW);
	void pGameOnlineFun(int);
#endif
	

private:
	HMODULE m_hTomMod;
	bool m_bEnableService;

	//key=�û�����data=����Tickʱ�̣�0�������ߣ�
	//map<const char*, DWORD> m_mapUserState;
};
