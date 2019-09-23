//=============================================================================
// FileName: Attachable.cpp
// Creater: ZhangXuedong
// Date: 2004.10.19
// Comment: CAttachable class
//=============================================================================

#include "Attachable.h"
#include <iostream>
#include <fstream>
#include <ostream>
// #include <fstream>
#include <stdio.h>
#include "io.h"
#include <string>
#include "windows.h"
#include "TryUtil.h"

CAttachable::CAttachable()
{T_B
	m_pCConjureLast = 0;
	m_pCConjureNext = 0;

	m_pCPassengerLast = 0;
	m_pCPassengerNext = 0;

	m_pCPlayer = 0;
	m_pCShipMaster = 0;
	m_pCShip = 0;
T_E}

void CAttachable::Initially()
{T_B
	Entity::Initially();

	m_pCConjureLast = 0;
	m_pCConjureNext = 0;

	m_pCPassengerLast = 0;
	m_pCPassengerNext = 0;

	m_pCPlayer = 0;
	m_pCShipMaster = 0;
	m_pCShip = 0;
T_E}

void CAttachable::Finally()
{T_B
	Entity::Finally();
	m_pCPlayer = 0;
T_E}
