//  [10/14/2005]	��AccountServer2�ļ������ģ����,�������ģ�� - by Arcol
#pragma once

#ifndef _MYTHREAD_H_
#define _MYTHREAD_H_

#include "Comm.h"

// �̻߳���
class MyThread
{
public:
	MyThread();
	virtual ~MyThread();

	bool Launch();
	void NotifyToExit();
	int WaitForExit(int ms); // block calling thread

protected:
	static DWORD WINAPI ThreadProc(LPVOID);
	virtual int Run();
	bool GetExitFlag() const {return m_bExitFlag;}
	void ExitThread() {m_bExitThrd = true;}

private:
	bool volatile m_bExitFlag;
	bool volatile m_bExitThrd;
	DWORD m_dwThreadId;
	HANDLE m_hThread;
};

#endif
