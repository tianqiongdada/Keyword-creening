#pragma once
#include "stdafx.h"
#include "CommonDefs.h"
#include "IGameServer.h"
#include "IMongoDBEngine.h"
//�ֽڶ�������
#pragma pack(push)
#pragma pack(1)


#define  MAX_PLAYER_COUNT 6 //����������
#define  DBOID_SUBMIT_CARDRECORD 190 //�ύ�ƾּ�¼
#define  PLAYER_ACTION_DATA_LEN 128	//�����˶�������
#define	 MAX_GAME_DATA_LEN 1024	//��Ϸ������󳤶ȣ���ʱ��Ϊ1024

//�������Ͷ�Ӧ�洢Ϊ����Ϊjson�������ͣ��ַ��������֡�����{}������[]��������true,false��
//�����ṹ��������
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

	//��������
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

	int nChair;												//���������Ӻ�
	DWORD dwUserID;											//������ID
	DWORD dwActionDelay;									//�ϼ�ִ�ж����������ִ�ж�����ʱ����	
	BOOL bActionOverTime;									//�Ƿ�ʱ�Զ�ִ�еĶ���	
	int nRetain;											//�����ֶ�
	int nActionType;										//�����������磬���ƣ����壬�����ӵ�
	int nActionDataLen;
	int nGameDataLen;
	int nRetainDataLen;
	char cRetainData[MAX_GAME_DATA_LEN];					//�����ֶ�����
	char cActionData[PLAYER_ACTION_DATA_LEN];				//�����˶�������,json��ʽ��ֵ ��������:"",����[],����{}
	char cGameData[MAX_GAME_DATA_LEN];						//��Ϸ���ݣ������������ݣ�������Ƶȣ����Բ���¼��������Ҷ����������ӵȣ���json��ʽ��ֵ
};

//CGCCardRecordģ����������Ϸ�߼�ģ��ʹ�õĽӿ�
struct ICardRecord
{
	//������ҳ�ʱ������Ϣ
	/*
	���� :
	nChair:���Ӻ�
	dwUserID:���ID
	nOverTimeCount: ��ʱ����
	bOverTimeKickOut: �Ƿ�ʱ����
	*/
	virtual BOOL  __cdecl SetPlayerOverTimeInfo(int nChair, WORD wOverTimeCount, BOOL bOverTimeKickOut = FALSE) = NULL;

	//���õ�ǰ�ƾּ�¼��ע�⣬���ֻ�������һ�ζ����ļ�¼, �������ݽṹCardRecordBase
	virtual BOOL  __cdecl SetCardRecord(LPVOID* lpCardData) = NULL;
};

struct ICardRecordBase
{
	virtual void __cdecl Release() = NULL;
};

//CGCCardRecordģ�������˿��ʹ�õĽӿ�
struct ICardRecordHelper : public ICardRecordBase
{
	//������Ϸ��¼
	virtual BOOL __cdecl ClearRecordData() = NULL;

	//֪ͨ��Ϸ��ʼ
	virtual BOOL __cdecl NotifyGameStart() = NULL;

	//������Ϸ���
	virtual BOOL __cdecl SetPlayer(int nChair, DWORD dwUserID, char* szName, BYTE btSex) = NULL;

	//֪ͨ��Ϸ����
	virtual BOOL  __cdecl NotifyGameEnd() = NULL;

	//�����������
	virtual BOOL  __cdecl SetFleePlayer(int nChair = -1, BOOL bFlee = FALSE) = NULL;

	//���ö������
	virtual BOOL  __cdecl SetOffLinePlayer(int nChair = -1, BOOL bOffLine = FALSE) = NULL;

	//�����ƾֱ��
	virtual BOOL __cdecl SetCardRecordID(char szBatchID[]) = NULL;

	//�ύ��¼,��Ҫ���ݶ�Ӧ�������ͽ�������
	virtual BOOL __cdecl SubmitCardRecord(int nChair = -1, DWORD dwUserDBID = 0xFFFFFFFF) = NULL;

	//��ȡ�ƾּ�¼ָ��
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
				// ��һ�ε���ʱδ����dll��������м���dll
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

			// ������dll��������������ӿڶ���Ĵ�������
			CreateCR proc = NULL;
			proc = (CreateCR)::GetProcAddress(m_hDll, "CreateCardRecord");
			if (proc == NULL)
			{
				char cMes[1024] = "";
				sprintf(cMes, "Can't GetProcAddress('CreateCardRecord'),LastErr:%d", GetLastError());
				throw cMes;
			}

			// �������dll������������������ӿڶ���
			if (!proc(&m_pCardRecordHelper, pTrace, pGameServer, pMongoDBEngine, dwGameID, nTableID, nPlayerCount, pServerOption))
				throw "Error: CCardRecordHelper::Create() , CreateInterface() error!";

			m_bIsValid = TRUE;

			return TRUE;		// ���ˣ��ӿڶ��󴴽��ɹ���
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

