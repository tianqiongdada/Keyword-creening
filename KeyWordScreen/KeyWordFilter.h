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
	//获取关键词目录下的所有文件
	void _GetKeyWordFiles(CString strDirPath, std::vector<CString>& vcKeyWordFiles);

	CStringW _ReplaceKeyWord(CStringW strContent, std::vector<stPos> vcPos);
	BOOL _HasCom(stPos Pos1, stPos Pos2);
	stPos _GetNewPos(stPos Pos1, stPos Pos2);

	//计算重复区间，获取新的关键词区间
	std::vector<stPos> _GetNewPosArr(std::vector<wResult> vcRes);

	CStringW _CStrA2CStrW(const CString &cstrSrcA); //多字节转为宽字节
	CString _CStrW2CStrA(const CStringW &cstrSrcW); //宽字节转多字节

private:
	Basic_Trie m_keyManager;
	CRITICAL_SECTION	m_cs;		//

};

