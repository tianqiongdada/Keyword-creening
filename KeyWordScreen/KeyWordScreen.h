// KeyWordScreen.h : KeyWordScreen DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CKeyWordScreenApp
// �йش���ʵ�ֵ���Ϣ������� KeyWordScreen.cpp
//

class CKeyWordScreenApp : public CWinApp
{
public:
	CKeyWordScreenApp();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
