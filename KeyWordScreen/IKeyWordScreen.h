#pragma once
#include "stdafx.h"
#include "CommonDefs.h"
#include "IGameServer.h"
#include "IMongoDBEngine.h"
//字节对齐命令
#pragma pack(push)
#pragma pack(1)


#define  MAX_PLAYER_COUNT 6 //最大玩家数量
#define  DBOID_SUBMIT_CARDRECORD 190 //提交牌局记录
#define  PLAYER_ACTION_DATA_LEN 128	//出牌人动作长度
#define	 MAX_GAME_DATA_LEN 1024	//游戏数据最大长度，暂时定为1024

//数据类型都应存储为基础为json数据类型，字符串、数字、对象{}、数组[]、布尔（true,false）
//基础结构数据类型
struct CardRecordBase
{
	CardRecordBase()
	{
		Clear();
	}

	~CardRecordBase()
	{
		Clear();
	}

	//拷贝构造
	CardRecordBase(const CardRecordBase & otherCard)
	{
		Clear();
		memcpy(this, &otherCard, sizeof(CardRecordBase));
	}

	void operator = (const CardRecordBase& otherCard)
	{
		Clear();
		memcpy(this, &otherCard, sizeof(CardRecordBase));
	}

	void Clear()
	{
		nChair = -1;
		dwUserID = 0xFFFFFFFF;
		dwActionDelay = 0;
		bActionOverTime = FALSE;
		nRetain = 0;
		nActionType = 0;
		nRetainDataLen = 0;
		nActionDataLen = 0;
		nGameDataLen = 0;
		memset(cActionData, 0, PLAYER_ACTION_DATA_LEN);
		memset(cGameData, 0, MAX_GAME_DATA_LEN);
		memset(cRetainData, 0, MAX_GAME_DATA_LEN);
	}

	BOOL FillPlayerActionData(char* cData, int nLen)
	{
		if (nActionDataLen + nLen > PLAYER_ACTION_DATA_LEN)
			return FALSE;

		memcpy(cActionData + nActionDataLen, cData, nLen);
		nActionDataLen += nLen;
		return TRUE;
	}

	BOOL FillGameData(char* cData, int nLen)
	{
		if (nGameDataLen + nLen > MAX_GAME_DATA_LEN)
			return FALSE;

		memcpy(cGameData + nGameDataLen, cData, nLen);
		nGameDataLen += nLen;
		return FALSE;
	}

	BOOL FillRetainData(char* cData, int nLen)
	{
		if (nRetainDataLen + nLen > MAX_GAME_DATA_LEN)
			return FALSE;

		memcpy(cRetainData + nRetainDataLen, cData, nLen);
		nRetainDataLen += nLen;
		return FALSE;
	}

	int nChair;												//出牌人椅子号
	DWORD dwUserID;											//出牌人ID
	DWORD dwActionDelay;									//上家执行动作到该玩家执行动作的时间间隔	
	BOOL bActionOverTime;									//是否超时自动执行的动作	
	int nRetain;											//保留字段
	int nActionType;										//动作类型例如，出牌，走棋，掷骰子等
	int nActionDataLen;
	int nGameDataLen;
	int nRetainDataLen;
	char cRetainData[MAX_GAME_DATA_LEN];					//保留字段数据
	char cActionData[PLAYER_ACTION_DATA_LEN];				//出牌人动作数据,json格式的值 基本类型:"",数组[],对象{}
	char cGameData[MAX_GAME_DATA_LEN];						//游戏数据，例如棋盘数据，玩家手牌等（可以不记录，例如玩家动作是掷骰子等），json格式的值
};

//CGCCardRecord模块给服务端游戏逻辑模块使用的接口
struct ICardRecord
{
	//设置玩家超时数据信息
	/*
	参数 :
	nChair:椅子号
	dwUserID:玩家ID
	nOverTimeCount: 超时次数
	bOverTimeKickOut: 是否超时被踢
	*/
	virtual BOOL  __cdecl SetPlayerOverTimeInfo(int nChair, WORD wOverTimeCount, BOOL bOverTimeKickOut = FALSE) = NULL;

	//设置当前牌局记录。注意，这个只设置玩家一次动作的记录, 基本数据结构CardRecordBase
	virtual BOOL  __cdecl SetCardRecord(LPVOID* lpCardData) = NULL;
};

struct ICardRecordBase
{
	virtual void __cdecl Release() = NULL;
};

//CGCCardRecord模块给服务端框架使用的接口
struct ICardRecordHelper : public ICardRecordBase
{
	//清理游戏记录
	virtual BOOL __cdecl ClearRecordData() = NULL;

	//通知游戏开始
	virtual BOOL __cdecl NotifyGameStart() = NULL;

	//设置游戏玩家
	virtual BOOL __cdecl SetPlayer(int nChair, DWORD dwUserID, char* szName, BYTE btSex) = NULL;

	//通知游戏结束
	virtual BOOL  __cdecl NotifyGameEnd() = NULL;

	//设置逃跑玩家
	virtual BOOL  __cdecl SetFleePlayer(int nChair = -1, BOOL bFlee = FALSE) = NULL;

	//设置断线玩家
	virtual BOOL  __cdecl SetOffLinePlayer(int nChair = -1, BOOL bOffLine = FALSE) = NULL;

	//设置牌局编号
	virtual BOOL __cdecl SetCardRecordID(char szBatchID[]) = NULL;

	//提交记录,需要根据对应数据类型解析数据
	virtual BOOL __cdecl SubmitCardRecord(int nChair = -1, DWORD dwUserDBID = 0xFFFFFFFF) = NULL;

	//获取牌局记录指针
	virtual ICardRecord* __cdecl GetCardRecord() = NULL;
};

class CCardRecordHelper
{
public:
	typedef BOOL(*CreateCR)(ICardRecordHelper** ppCardRecordSubmit,
		ITrace *pTrace,
		IGameServer*pGameServer,
		IMongoDBEngine* pMongoDBEngine,
		DWORD dwGameID,
		int nTableID,
		int nPlayerCount,
		tagServerOption* pServerOption);

	CCardRecordHelper()
	{
		m_pCardRecordHelper = NULL;
		m_bIsValid = FALSE;
	}

	~CCardRecordHelper()
	{
		Close();
		m_bIsValid = FALSE;
	}

public:
	ICardRecordHelper* m_pCardRecordHelper;

private:
	BOOL m_bIsValid;
	HINSTANCE m_hDll;
public:
	BOOL IsValid() { return m_bIsValid; }
	ICardRecordHelper* operator->() { return m_pCardRecordHelper; }
	ICardRecordHelper* GetCardRecordSubmit() { return m_pCardRecordHelper; }
	BOOL Create(ITrace *pTrace, IGameServer*pGameServer, IMongoDBEngine* pMongoDBEngine, DWORD dwGameID, int nTableID, int nPlayerCount, tagServerOption* pServerOption)
	{
		try
		{
			if (!pServerOption)
				return FALSE;

			if (m_hDll == NULL)
			{
				// 第一次调用时未加载dll，下面进行加载dll
#ifdef _DEBUG
				m_hDll = ::LoadLibrary("CGCCardRecord.dll");
				if (m_hDll == NULL)
					throw "Error: CCardRecordHelper::Create() , Can't load CGCCardRecord.dll";
#else
				m_hDll = ::LoadLibrary("CGCCardRecord.dll");
				if (NULL == m_hDll)
				{
					char cMes[1024] = "";
					sprintf(cMes, "Error: CCardRecordHelper::Create() , Can't load CGCCardRecord.dll,LastErr:%d", GetLastError());
					throw cMes;
				}
#endif
			}

			// 下面获得dll的输出函数，即接口对象的创建函数
			CreateCR proc = NULL;
			proc = (CreateCR)::GetProcAddress(m_hDll, "CreateCardRecord");
			if (proc == NULL)
			{
				char cMes[1024] = "";
				sprintf(cMes, "Can't GetProcAddress('CreateCardRecord'),LastErr:%d", GetLastError());
				throw cMes;
			}

			// 下面调用dll的输出函数，来创建接口对象
			if (!proc(&m_pCardRecordHelper, pTrace, pGameServer, pMongoDBEngine, dwGameID, nTableID, nPlayerCount, pServerOption))
				throw "Error: CCardRecordHelper::Create() , CreateInterface() error!";

			m_bIsValid = TRUE;

			return TRUE;		// 到此，接口对象创建成功！
		}

		catch (LPCSTR szMsg)
		{
#ifdef _DEBUG
			OutputDebugString(szMsg); OutputDebugString("\r\n");
#else
			tPrintEventLn(szMsg);
#endif
			return FALSE;
		}
		catch (...)
		{
#ifdef _DEBUG
			OutputDebugString("Error : CCardRecordHelper::Create() , catched Unknown error!\r\n"); OutputDebugString("\r\n");
#endif
		}
	}

	void Close()
	{
		if (m_pCardRecordHelper)
		{
			m_pCardRecordHelper->Release();
			m_pCardRecordHelper = NULL;
		}

		if (m_hDll)
		{
			FreeLibrary(m_hDll);
			m_hDll = NULL;
		}
	}
};

