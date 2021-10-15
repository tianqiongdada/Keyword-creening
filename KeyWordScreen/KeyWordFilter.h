#pragma once
#include "keyWordManger.h"
#include "../../include/IKeyWordScreen.h"

struct stPos
{
	int nStart;
	int nEnd;
	stPos()
	{
		nStart = 0;
		nEnd = 0;
	}
};

using namespace KeyWordMangaer;

class CKeyWordFilter : public IKeyWordScreen
{
public:
	CKeyWordFilter();
	~CKeyWordFilter();

	//IKeyWordScreen
public:
	virtual BOOL __cdecl Release();
	virtual BOOL  __cdecl InitKeyWord();
	virtual BOOL  __cdecl KeyWordScreen(LPCSTR lpcstr);
	virtual void  __cdecl KeyWordReplace(char* cstr, int nMaxLen);

public:
	//��ȡ�ؼ���Ŀ¼�µ������ļ�
	void _GetKeyWordFiles(CString strDirPath, std::vector<CString>& vcKeyWordFiles);

	CStringW _ReplaceKeyWord(CStringW strContent, std::vector<stPos> vcPos);
	BOOL _HasCom(stPos Pos1, stPos Pos2);
	stPos _GetNewPos(stPos Pos1, stPos Pos2);

	//�����ظ����䣬��ȡ�µĹؼ�������
	std::vector<stPos> _GetNewPosArr(std::vector<wResult> vcRes);

	CStringW _CStrA2CStrW(const CString &cstrSrcA); //���ֽ�תΪ���ֽ�
	CString _CStrW2CStrA(const CStringW &cstrSrcW); //���ֽ�ת���ֽ�

private:
	Basic_Trie m_keyManager;
	CRITICAL_SECTION	m_cs;		//

};

