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
	static wchar_t cSp[SP_CHAR_NUM] = { L'\n', L' ', L'\r', L'\t', L'\"', L'？' }; //关键词里面去掉一些不必要的符号
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
		//只在一个关键词的最后一个节点保存完整的关键词 用于判断是否找到关键词
		CStringW strKeyWord;

		//map中的key 是中文意义中的一个字，例如"1","我"，value指向下一节点
		std::map<wchar_t, std::unique_ptr<wNode>> child;
	};

	class Basic_Trie
	{
	private:
		//根节点
		std::unique_ptr<wNode> _root;

	public:
		//初始化根节点
		Basic_Trie() :_root(new wNode()){}

		void ClearKeyTree()
		{
			if (_root == NULL)
				return;

			////清理掉一个节点下的所有子节点
			//for (std::map<wchar_t, std::unique_ptr<wNode>>::iterator it = _root->child.begin(); it != _root->child.end(); ++it)
			//{
			//	wNode* node = it->second.get();
			//	ClearKeyNode(node);
			//}

			_root->child.clear(); //智能指针，直接清理
		}

		void ClearKeyNode(wNode* root)
		{
			if (root == NULL)
				return;

			//清理掉一个节点下的所有子节点
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

		//插入关键字 
		void insert(CStringW& keyWord)
		{
			wNode* root = _root.get();
			if (nullptr == root || keyWord.IsEmpty())
			{
				return;
			}

			int len = keyWord.GetLength();

			//一次插入形成一个分支  再次插入(调用insert)关键词形成多叉树结构
			for (auto i = 0; i < len; ++i)
			{
				wchar_t key = keyWord.GetAt(i);
				if (!root->child[key])
				{
					root->child[key] = std::unique_ptr<wNode>(new wNode());
				}

				root = root->child[key].get();
			}

			//一个关键词链的终端节点保存完整的关键词
			root->strKeyWord = keyWord;
			return;
		}

		//下一节点不为空则继续匹配 key不为空则说明匹配到关键词
		std::vector<wResult> search(CStringW& content)
		{
			wNode* node = _root.get();
			if (nullptr == node || content.IsEmpty())
				return std::vector<wResult>();

			int len = content.GetLength();
			std::vector<wResult> res;

			int i = 0;
			int skipNum = 0;
			int nCurStartIndex = -1;			//记录本次开始匹配的位置
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
					if (!bBegainKeyWord) //进入匹配，并且记录
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

						//找到之后继续下一轮查找
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


		//完全匹配，包括重叠部分，例如关键词存在(动动我试试、动动我、我试试)， 输入语句：有本事动动我试试啊，将匹配出这三个关键词并记录到结果中
		std::vector<wResult> serachAll(CStringW& content)
		{
			wNode* node = _root.get();
			if (nullptr == node || content.IsEmpty())
				return std::vector<wResult>();

			int len = content.GetLength();
			std::vector<wResult> res;

			int i = 0;
			int skipNum = 0;
			bool bBegainKeyWord = false;				//用于判断是否有child
			int nCurStartIndex = -1;					//记录本次开始匹配的位置
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

					if (!bBegainKeyWord) //进入匹配，并且记录位置
					{
						bBegainKeyWord = true;
						nCurStartIndex = i;
					}

					if (node == _root.get())
						skipNum = 0;

					if (!temp->strKeyWord.IsEmpty()) //存在key的情况
					{
						res.emplace_back(wResult(temp->strKeyWord, i + 1 - skipNum - temp->strKeyWord.GetLength(), i));
					}

					node = node->child[key].get();//即使找到之后继续玩后面找，看下是否有重叠在一起符合条件的关键词
				}
				else
				{
					if (bBegainKeyWord) //如果是正在匹配的操作进行中断的，应该从开始匹配的下一个字开始，删选跌在一起的关键词了，比如关键词1,12,123
					{
						bBegainKeyWord = false;
						i = nCurStartIndex;
					}

					skipNum = 0;
					node = _root.get();
				}

				++i;
				if (i >= len && bBegainKeyWord) //如果是匹配过程中超出源串了，那么从开始匹配的下一个继续，防止重叠关键词被越过
				{
					bBegainKeyWord = false;
					i = nCurStartIndex + 1;
					skipNum = 0;
					node = _root.get();
				}
			}

			return res;
		}

		//只做一次匹配 
		BOOL IsFindKeyWord(CStringW& content)
		{
			wNode* node = _root.get();
			if (nullptr == node || content.IsEmpty())
				return FALSE;

			int len = content.GetLength();

			int i = 0;
			BOOL bFindKeyWord = FALSE;
			int nCurStartIndex = -1;			//记录本次开始匹配的位置
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
					if (!bBegainKeyWord) //进入匹配，并且记录
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
					if (bBegainKeyWord) //从上次开始匹配的位置下一个开始找避免错过重叠的关键词
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
