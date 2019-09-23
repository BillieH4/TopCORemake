//=============================================================================
// FileName: Parser.cpp
// Creater: ZhangXuedong
// Date: 2004.11.22
// Comment: scripts interface
//=============================================================================

#include "Parser.h"
#include "util.h"
//#include "lua_gamectrl.h"
#include <stdlib.h>
#include <list>
using namespace std;

#pragma comment (lib, "lua50.lib")
#pragma comment (lib, "lualib.lib")

CParser	g_CParser;
list<string> g_luaFNList;

// ʹ��lua pcall�Ĵ��󱨸溯��
inline void lua_callalert(lua_State* L, int status)
{
	if (status != 0)
	{
		lua_getglobal(L, "_ALERT");
		if (lua_isfunction(L, -1))
		{
			lua_insert(L, -2);
			lua_call(L, 1, 0);
		}
		else
		{ // no _ALERT function; print it on stderr
			LG("lua_err", "%s\n", lua_tostring(L, -2));
			lua_pop(L, 2);
		}
	}
}

void CParser::Init(lua_State *pLS)
{
	m_pSLua = pLS;
}

void CParser::Free()
{
}

int CParser::DoString(const char *csString, char chRetType, int nRetNum, ...)
{
	MPTimer t; t.Begin();
	lua_getglobal(m_pSLua, csString);
	if (!lua_isfunction(m_pSLua, -1)) // ���Ǻ�����
	{
		lua_pop(m_pSLua, 1);
		if (nRetNum == 1 && chRetType == enumSCRIPT_RETURN_NUMBER)
		{
			m_nDoStringRet[0] = atoi(csString);
			return 1;
		}
		//LG("lua_err", "û�ж����DoString(%s)\n", csString);
		LG("lua_err", "no define DoString(%s)\n", csString);
		return 0;
	}

	if (nRetNum > DOSTRING_RETURN_NUM)
	{
		//LG("lua_err", "msgDoString(%s) ����ֵ�������󣡣���\n", csString);
		LG("lua_err", "msgDoString(%s) return wrong num !!!\n", csString);

		lua_settop(m_pSLua, 0);
		return 0;
	}

	va_list list;
	va_start(list, nRetNum);
	int nParam, nParamNum = 0, nNum;
	while((nParam = va_arg(list, int)) != DOSTRING_PARAM_END)
	{
		switch (nParam)
		{
		case	enumSCRIPT_PARAM_NUMBER:
			nNum = va_arg(list, int);
			nParamNum += nNum;
			while (nNum-- > 0)
				lua_pushnumber(m_pSLua, va_arg(list, int));
			break;
		case	enumSCRIPT_PARAM_NUMBER_UNSIGNED:
			nNum = va_arg(list, int);
			nParamNum += nNum;
			while (nNum-- > 0)
				lua_pushnumber(m_pSLua, va_arg(list, unsigned int));
			break;
		case	enumSCRIPT_PARAM_LIGHTUSERDATA:
			{
			nNum = va_arg(list, int);
			nParamNum += nNum;
			void	*Pointer;
			while (nNum-- > 0)
			{
				Pointer = va_arg(list, void *);
				lua_pushlightuserdata(m_pSLua, Pointer);
			}
			break;
			}
		case	enumSCRIPT_PARAM_STRING:
			nNum = va_arg(list, int);
			nParamNum += nNum;
			while (nNum-- > 0)
				lua_pushstring(m_pSLua, va_arg(list, char *));
			break;
		default:
			//LG("lua_err", "msgDoString(%s) �������ʹ��󣡣���\n", csString);
			LG("lua_err", "msgDoString(%s) parameter type is wrong!!!\n", csString);
			lua_settop(m_pSLua, 0);
			return 0;
			break;
		}
		//luaL_checkstack(m_pSLua, 1, "too many arguments");
	}
	va_end( list );
	int nState = lua_pcall(m_pSLua, nParamNum, LUA_MULTRET, 0);
	if (nState != 0)
	{
		LG("lua_err", "DoString %s\n", csString);
		lua_callalert(m_pSLua, nState);

		lua_settop(m_pSLua, 0);
		return 0;
	}

	int	nRet = 1;
	int i = 0;
	for (; i < nRetNum; i++)
	{
		if (chRetType == enumSCRIPT_RETURN_NUMBER)
		{
			if (!lua_isnumber(m_pSLua, -1 - i))
			{
				//LG("lua����ֵ����", "���ýű� %s������%d��������ֵ%d���� ʱ���䷵��ֵ���Ͳ�ƥ�䣡\n", csString, nParamNum, nRetNum);
				LG("lua return", "call %s(parameter %d ��return %d), return type can't matched!\n", csString, nParamNum, nRetNum);
				nRet = 0;
				break;
			}
			m_nDoStringRet[nRetNum - 1 - i] = (int)lua_tonumber(m_pSLua, -1 - i);
		}
		else if (chRetType == enumSCRIPT_RETURN_STRING)
		{
			if (!lua_isstring(m_pSLua, -1 - i))
			{
				//LG("lua����ֵ����", "���ýű� %s������%d��������ֵ%d���� ʱ���䷵��ֵ���Ͳ�ƥ�䣡\n", csString, nParamNum, nRetNum);
				LG("lua return", "call %s(parameter %d ��return %d), return type can't matched!\n", csString, nParamNum, nRetNum);
				nRet = 0;
				break;
			}
			SetRetString(nRetNum - 1 - i, lua_tostring(m_pSLua, -1 - i));
		}
		else
		{
			//LG("lua_err", "msgDoString(%s) ����ֵ���ʹ��󣡣���\n", csString);
			LG("lua_err", "msgDoString(%s) eturn type is wrong!!!\n", csString);
			lua_settop(m_pSLua, 0);
			return 0;
		}
	}
	lua_settop(m_pSLua, 0);
	//lua_pop(m_pSLua, nRetNum);

	DWORD dwEndTime = t.End();
	if(dwEndTime > 20)
	{
		//LG("script_time", "�ű�[%s]����ʱ����� time = %d\n", csString, dwEndTime);
		LG("script_time", "script [%s] too long time = %d\n", csString, dwEndTime);
	}
	return nRet;
}
