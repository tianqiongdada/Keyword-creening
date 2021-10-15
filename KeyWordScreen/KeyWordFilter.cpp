#include "StdAfx.h"
#include "KeyWordFilter.h"
#include <fstream>


CKeyWordFilter::CKeyWordFilter()
{
	::InitializeCriticalSection(&m_cs);

	KeyWordMangaer::initSpCh();
}


CKeyWordFilter::~CKeyWordFilter()
{
	::DeleteCriticalSection(&m_cs);
}

BOOL __cdecl CKeyWordFilter::Release()
{
	delete this;
	return TRUE;
}

BOOL __cdecl CKeyWordFilter::InitKeyWord()
{
	try
	{
		DWORD time1 = GetTickCount();
		EnterCriticalSection(&m_cs);
		m_keyManager.ClearKeyTree();
		std::vector<CString> vcKeyWordFiles;
		_GetKeyWordFiles("./KeyWord", vcKeyWordFiles);
		for (int i = 0; i < vcKeyWordFiles.size(); ++i)
		{
			tPrintEventLn("---------开始关键词文件[%s]------------", vcKeyWordFiles[i]);
			int nCount = 0;
			CStdioFile read;
			if (!read.Open(vcKeyWordFiles[i], CFile::modeRead))
			{
				tPrintEventLn("------未找到文件:%s-----", vcKeyWordFiles[i]);
				read.Close();
				continue;
			}

			CString line;
			while (read.ReadString(line))
			{
				line = line.MakeLower();
				CStringW wString = _CStrA2CStrW(line);
				for (int n = 0; n < SP_CHAR_NUM; ++n)
				{
					wString.Replace(KeyWordMangaer::cSp[n], L''); //去掉指定的一些字符
				}

				m_keyManager.insert(wString); //加入关键词树
				++nCount;
			}
			read.Close();

			tPrintEventLn("---------读取关键词文件[%s]完成, 获取关键词数量:%d------------", vcKeyWordFiles[i], nCount);
		}
		LeaveCriticalSection(&m_cs);

		tPrintEventLn("-----重新加载所有关键词完成,耗时:%d ms----", GetTickCount() - time1);
	}
	catch (...)
	{
		LeaveCriticalSection(&m_cs);
	}
	
	return TRUE;
}

BOOL __cdecl CKeyWordFilter::KeyWordScreen(LPCSTR lpcstr)
{
	BOOL bFind = FALSE;
	try
	{
		EnterCriticalSection(&m_cs);
		CStringW strW = _CStrA2CStrW(lpcstr).MakeLower();//大小写不敏感
		bFind = m_keyManager.IsFindKeyWord(strW);
		LeaveCriticalSection(&m_cs);
	}
	catch (...)
	{
		LeaveCriticalSection(&m_cs);
	}

	return  bFind;
}

void __cdecl CKeyWordFilter::KeyWordReplace(char* cstr, int nMaxLen)
{
	try
	{
		EnterCriticalSection(&m_cs);
		CStringW strW = _CStrA2CStrW(cstr);
		CStringW strScreen = strW.MakeLower(); //大小写不敏感

		DWORD dwTime = GetTickCount();
		std::vector<wResult> vcRes = m_keyManager.serachAll(strScreen);
		tPrintEventLn("----匹配时长:%d ms----", GetTickCount() - dwTime);

		CString strMes = _CStrW2CStrA(_ReplaceKeyWord(strW, _GetNewPosArr(vcRes)));
		int nCopyLen = nMaxLen >= strMes.GetLength() ? strMes.GetLength() : nMaxLen;
		ZeroMemory(cstr, nMaxLen);
		memcpy(cstr, strMes.GetBuffer(0), nCopyLen);
		LeaveCriticalSection(&m_cs);

	}
	catch (...)
	{
		LeaveCriticalSection(&m_cs);
	}
}

void CKeyWordFilter::_GetKeyWordFiles(CString strDirPath, std::vector<CString>& vcKeyWordFiles)
{
	CString strFileName;
	CString strFullName;

	CFileFind find;
	BOOL IsFind = find.FindFile(strDirPath + _T("/*.*"));
	while (IsFind)
	{
		IsFind = find.FindNextFile();
		if (find.IsDots())
		{
			continue;
		}
		else
		{
			strFileName = find.GetFileName();
			strFullName = strDirPath + "/" + strFileName;
			vcKeyWordFiles.push_back(strFullName);
			if (PathIsDirectory(strFullName))
			{
				_GetKeyWordFiles(strFullName, vcKeyWordFiles); //
			}
		}
	}
}

CStringW CKeyWordFilter::_ReplaceKeyWord(CStringW strContent, std::vector<stPos> vcPos)
{
	if (vcPos.size() <= 0)
		return strContent;

	CStringW strRes;
	for (int i = 0; i < vcPos.size(); ++i)
	{
		int nStartPos = 0;
		int nMidPos1 = vcPos[i].nStart;
		int nMidPos2 = vcPos[i].nEnd + 1;
		int nEndPos = strContent.GetLength();
		if (0 != i)
		{
			nStartPos = vcPos[i - 1].nEnd + 1;
		}
		if (i + 1 < vcPos.size())
		{
			nEndPos = vcPos[i + 1].nStart;
		}

		strRes += strContent.Mid(nStartPos, nMidPos1 - nStartPos);

		//多少个字多少颗*
		for (int n = 0; n < vcPos[i].nEnd - vcPos[i].nStart + 1; ++n)
		{
			strRes += "*";
		}

		if (vcPos.size() - 1 == i) //把尾部加上
		{
			strRes += strContent.Mid(nMidPos2, nEndPos - nMidPos2);
		}
	}

	return strRes;
}

BOOL CKeyWordFilter::_HasCom(stPos Pos1, stPos Pos2)
{
	if ((Pos1.nStart > Pos2.nEnd) || (Pos1.nEnd < Pos2.nStart))
		return FALSE;

	return TRUE;
}

stPos CKeyWordFilter::_GetNewPos(stPos Pos1, stPos Pos2)
{
	stPos pos;
	pos.nStart = Pos1.nStart >= Pos2.nStart ? Pos2.nStart : Pos1.nStart;
	pos.nEnd = Pos1.nEnd >= Pos2.nEnd ? Pos1.nEnd : Pos2.nEnd;
	return pos;
}

std::vector<stPos> CKeyWordFilter::_GetNewPosArr(std::vector<wResult> vcRes)
{
	if (vcRes.size() <= 0)
	{
		return std::vector<stPos>();
	}

	std::vector<stPos> vcNewPosArr;
	std::vector<int> vcIndex; //存储计算过相同区间的索引
	for (int i = 0; i < vcRes.size(); ++i)
	{
		BOOL bFind = FALSE;
		for (int n = 0; n < vcIndex.size(); ++n)
		{
			if (vcIndex[n] == i)
			{
				bFind = TRUE;
				break;
			}
		}

		if (bFind)
			continue;

		stPos pos;
		pos.nStart = vcRes[i].m_wStart;
		pos.nEnd = vcRes[i].M_wEnd;
		for (int j = i + 1; j < vcRes.size(); ++j)
		{
			BOOL bFind = FALSE;
			for (int n = 0; n < vcIndex.size(); ++n)
			{
				if (vcIndex[n] == j)
				{
					bFind = TRUE;
					break;
				}
			}

			if (bFind)
				continue;

			stPos pos2;
			pos2.nStart = vcRes[j].m_wStart;
			pos2.nEnd = vcRes[j].M_wEnd;
			if (_HasCom(pos, pos2))
			{
				pos = _GetNewPos(pos, pos2);
				vcIndex.push_back(i);
				vcIndex.push_back(j);
			}
		}

		vcNewPosArr.push_back(pos);
	}

	return vcNewPosArr;
}

CStringW CKeyWordFilter::_CStrA2CStrW(const CString &cstrSrcA)
{
	int len = MultiByteToWideChar(CP_ACP, 0, LPCSTR(cstrSrcA), -1, NULL, 0);
	wchar_t *wstr = new wchar_t[len];
	memset(wstr, 0, len*sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, LPCSTR(cstrSrcA), -1, wstr, len);
	CStringW cstrDestW = wstr;
	delete[] wstr;

	return cstrDestW;
}

CString CKeyWordFilter::_CStrW2CStrA(const CStringW &cstrSrcW)
{
	int len = WideCharToMultiByte(CP_ACP, 0, LPCWSTR(cstrSrcW), -1, NULL, 0, NULL, NULL);
	char *str = new char[len];
	memset(str, 0, len);
	WideCharToMultiByte(CP_ACP, 0, LPCWSTR(cstrSrcW), -1, str, len, NULL, NULL);
	CStringA cstrDestA = str;
	delete[] str;

	return cstrDestA;
}
