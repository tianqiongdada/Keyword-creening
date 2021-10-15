//

#pragma once

#include <typeinfo>
#include <memory>
#include <string>
#include <map>
#include <queue>

#define  SP_CHAR_NUM 7
namespace KeyWordMangaer
{
	static std::map<wchar_t, wchar_t> mpSpCh;
	static wchar_t cSp[SP_CHAR_NUM] = { L'\n', L' ', L'\r', L'\t', L'\"', L'��' }; //�ؼ�������ȥ��һЩ����Ҫ�ķ���
	static void initSpCh()
	{
		for (int n = 0; n < SP_CHAR_NUM; ++n)
		{
			wchar_t wch =cSp[n];
			mpSpCh[wch] = wch;
		}
	}

	static bool IsSpCh(wchar_t wch)
	{
		return mpSpCh.find(wch) != mpSpCh.end();
	}

	class wResult
	{
	public:
		CStringW m_strKeyWord;
		WORD m_wStart;
		WORD M_wEnd;
	public:
		wResult(CStringW strKey, WORD wStart, WORD wEnd) :
			m_strKeyWord(strKey), m_wStart(wStart), M_wEnd(wEnd){}

	};

	struct wNode
	{
		//ֻ��һ���ؼ��ʵ����һ���ڵ㱣�������Ĺؼ��� �����ж��Ƿ��ҵ��ؼ���
		CStringW strKeyWord;

		//map�е�key �����������е�һ���֣�����"1","��"��valueָ����һ�ڵ�
		std::map<wchar_t, std::unique_ptr<wNode>> child;
	};

	class Basic_Trie
	{
	private:
		//���ڵ�
		std::unique_ptr<wNode> _root;

	public:
		//��ʼ�����ڵ�
		Basic_Trie() :_root(new wNode()){}

		void ClearKeyTree()
		{
			if (_root == NULL)
				return;

			////�����һ���ڵ��µ������ӽڵ�
			//for (std::map<wchar_t, std::unique_ptr<wNode>>::iterator it = _root->child.begin(); it != _root->child.end(); ++it)
			//{
			//	wNode* node = it->second.get();
			//	ClearKeyNode(node);
			//}

			_root->child.clear(); //����ָ�룬ֱ������
		}

		void ClearKeyNode(wNode* root)
		{
			if (root == NULL)
				return;

			//�����һ���ڵ��µ������ӽڵ�
			for (std::map<wchar_t, std::unique_ptr<wNode>>::iterator it = root->child.begin(); it != root->child.end(); ++it)
			{
				wNode* node = it->second.get();
				ClearKeyNode(node);
			}

			root->child.clear();
			delete root;
			root = NULL;
			return;
		}

		//����ؼ��� 
		void insert(CStringW& keyWord)
		{
			wNode* root = _root.get();
			if (nullptr == root || keyWord.IsEmpty())
			{
				return;
			}

			int len = keyWord.GetLength();

			//һ�β����γ�һ����֧  �ٴβ���(����insert)�ؼ����γɶ�����ṹ
			for (auto i = 0; i < len; ++i)
			{
				wchar_t key = keyWord.GetAt(i);
				if (!root->child[key])
				{
					root->child[key] = std::unique_ptr<wNode>(new wNode());
				}

				root = root->child[key].get();
			}

			//һ���ؼ��������ն˽ڵ㱣�������Ĺؼ���
			root->strKeyWord = keyWord;
			return;
		}

		//��һ�ڵ㲻Ϊ�������ƥ�� key��Ϊ����˵��ƥ�䵽�ؼ���
		std::vector<wResult> search(CStringW& content)
		{
			wNode* node = _root.get();
			if (nullptr == node || content.IsEmpty())
				return std::vector<wResult>();

			int len = content.GetLength();
			std::vector<wResult> res;

			int i = 0;
			int skipNum = 0;
			int nCurStartIndex = -1;			//��¼���ο�ʼƥ���λ��
			bool bBegainKeyWord = false;
			while (i < len)
			{
				wchar_t key = content.GetAt(i);

				while ((i < len) && IsSpCh(key))
				{
					++i;
					key = content.GetAt(i);
					++skipNum;
				}

				if (3 == i)
				{
					int a = 0;
				}

				if (i >= len)
					break;

				if (node->child.find(key) != node->child.end())
				{
					if (!bBegainKeyWord) //����ƥ�䣬���Ҽ�¼
					{
						bBegainKeyWord = true;
						nCurStartIndex = i;
					}

					if (node == _root.get())
						skipNum = 0;

					wNode* temp = node->child[key].get();
					if (!temp->strKeyWord.IsEmpty())
					{
						res.emplace_back(wResult(temp->strKeyWord, i + 1 - skipNum - temp->strKeyWord.GetLength(), i));

						//�ҵ�֮�������һ�ֲ���
						node = _root.get();
						++i;
						skipNum = 0;
						bBegainKeyWord = false;
						continue;
					}

					node = node->child[key].get();
				}
				else
				{
					if (bBegainKeyWord)
					{
						i = nCurStartIndex;
						bBegainKeyWord = false;
					}

					skipNum = 0;
					node = _root.get();
				}

				++i;
			}

			return res;
		}


		//��ȫƥ�䣬�����ص����֣�����ؼ��ʴ���(���������ԡ������ҡ�������)�� ������䣺�б��¶��������԰�����ƥ����������ؼ��ʲ���¼�������
		std::vector<wResult> serachAll(CStringW& content)
		{
			wNode* node = _root.get();
			if (nullptr == node || content.IsEmpty())
				return std::vector<wResult>();

			int len = content.GetLength();
			std::vector<wResult> res;

			int i = 0;
			int skipNum = 0;
			bool bBegainKeyWord = false;				//�����ж��Ƿ���child
			int nCurStartIndex = -1;					//��¼���ο�ʼƥ���λ��
			while (i < len)
			{
				wchar_t key = content.GetAt(i);
				while ((i < len) && IsSpCh(key))
				{
					++i;
					key = content.GetAt(i);
					++skipNum;
				}

				if (i >= len)
					break;

				if (node->child.find(key) != node->child.end())
				{
					wNode* temp = node->child[key].get();

					if (!bBegainKeyWord) //����ƥ�䣬���Ҽ�¼λ��
					{
						bBegainKeyWord = true;
						nCurStartIndex = i;
					}

					if (node == _root.get())
						skipNum = 0;

					if (!temp->strKeyWord.IsEmpty()) //����key�����
					{
						res.emplace_back(wResult(temp->strKeyWord, i + 1 - skipNum - temp->strKeyWord.GetLength(), i));
					}

					node = node->child[key].get();//��ʹ�ҵ�֮�����������ң������Ƿ����ص���һ����������Ĺؼ���
				}
				else
				{
					if (bBegainKeyWord) //���������ƥ��Ĳ��������жϵģ�Ӧ�ôӿ�ʼƥ�����һ���ֿ�ʼ��ɾѡ����һ��Ĺؼ����ˣ�����ؼ���1,12,123
					{
						bBegainKeyWord = false;
						i = nCurStartIndex;
					}

					skipNum = 0;
					node = _root.get();
				}

				++i;
				if (i >= len && bBegainKeyWord) //�����ƥ������г���Դ���ˣ���ô�ӿ�ʼƥ�����һ����������ֹ�ص��ؼ��ʱ�Խ��
				{
					bBegainKeyWord = false;
					i = nCurStartIndex + 1;
					skipNum = 0;
					node = _root.get();
				}
			}

			return res;
		}

		//ֻ��һ��ƥ�� 
		BOOL IsFindKeyWord(CStringW& content)
		{
			wNode* node = _root.get();
			if (nullptr == node || content.IsEmpty())
				return FALSE;

			int len = content.GetLength();

			int i = 0;
			BOOL bFindKeyWord = FALSE;
			int nCurStartIndex = -1;			//��¼���ο�ʼƥ���λ��
			bool bBegainKeyWord = false; 
			while (i < len)
			{
				wchar_t key = content.GetAt(i);
				while ((i < len) && IsSpCh(key))
				{
					++i;
					key = content.GetAt(i);
				}

				if (i >= len)
					break;

				if (node->child.find(key) != node->child.end())
				{
					if (!bBegainKeyWord) //����ƥ�䣬���Ҽ�¼
					{
						bBegainKeyWord = true;
						nCurStartIndex = i;
					}

					wNode* temp = node->child[key].get();
					if (!temp->strKeyWord.IsEmpty())
					{
						bFindKeyWord = TRUE;
						break;
					}

					node = node->child[key].get();
				}
				else
				{
					if (bBegainKeyWord) //���ϴο�ʼƥ���λ����һ����ʼ�ұ������ص��Ĺؼ���
					{
						bBegainKeyWord = false;
						i = nCurStartIndex;
					}
					node = _root.get();
				}

				++i;
			}

			return bFindKeyWord;
		}
	};
}
