/**
*	\file cCommonUBTree.h
*	\author 
*	\version 
*	\date  
*	\version 
*	\date 
*	\version 
*	\date 
*	\brief 
*/

#ifndef __cCommonUBTree_h__
#define __cCommonUBTree_h__

#include <float.h>
#include <mutex>
//#include "dstruct/paged/ubtree/cInsertBuffers.h"
#include "dstruct/paged/core/cQuickDB.h"
#include "dstruct/paged/core/cPagedTree.h"
#include "dstruct/paged/ubtree/cUBTreeHeader.h"
#include "dstruct/paged/ubtree/cUBTreeConst.h"
#include "common/datatype/tuple/cSpaceDescriptor.h"
#include "dstruct/paged/core/cPagedTree.h"
#include "dstruct/paged/core/cNodeItem.h"
#include "dstruct/paged/core/cTreeItemStream.h"

#include "common/memorystructures/cHashTable.h"
#include "common/datatype/cCharString.h"

using namespace common::datatype::tuple;
using namespace dstruct::paged::core;
using namespace dstruct::paged::ubtree;
using namespace common::datatype;

namespace dstruct {
	namespace paged {
		namespace ubtree {

template <class TNode, class TLeafNode, class TKey>
class cCommonUBTree: public cPagedTree<TKey, TNode, TLeafNode>
{
protected:
	typedef cPagedTree<TKey, TNode, TLeafNode> parent;

	bool mDebug;

	// Variables used during the sequential search
	tNodeIndex mActualNode;
	unsigned int mActualOrder;
	TLeafNode *mCurrentLeafNode;
	char* mSubtRlvM;
	cHashTable<TKey, cCharString> *mInMemCache;
	cMemoryBlock* mInMemCacheMemBlock;
	bool mInMemCacheEnabled;
	cSpaceDescriptor* mDataDTDesc;

	//cRangeQueryProcessor<TKey, TNode, TLeafNode> *mRQProcessor;

  	bool GetNode(unsigned int level, float alpha, TNode &node);
	virtual TNode *SearchForRightOrderInInnerNode(const tNodeIndex parentNodeIndex, const tNodeIndex childNodeIndex, unsigned int &counter, int &order, tNodeIndex* currentPath, int* orderCurrentPath);
	virtual void SetParentLinkToEveryChild(TNode *node);
	virtual void CreateRootNode(const tNodeIndex &linkToFirstNode, char* itemOfFirstNode, const tNodeIndex &linkToSecondNode, char* itemOfSecondNode);
	inline virtual void SplitLeafNode(TLeafNode *currentLeafNode, char* firstItem, char* secondItem, tNodeIndex& insNodeIndex, cNodeBuffers<TKey>* buffers = NULL);
	void FullTitle();
	
	//void Insert_pre(cInsertBuffers<TKey>* insertBuffers);
	//void Insert_post(cInsertBuffers<TKey>* insertBuffers);
	void RangeQuery_pre(cNodeBuffers<TKey>* nodeBuffers);
	void RangeQuery_post(cNodeBuffers<TKey>* nodeBuffers);
	void GetMaxKeyValue(TKey **key);
	void InitInMemCache(cUBTreeHeader<TKey> *header);

public:
	static const unsigned int SEEK_START = 0;

	cCommonUBTree();
	~cCommonUBTree();

	bool Create(cUBTreeHeader<TKey> *header, cQuickDB* quickDB);
	bool Open(cUBTreeHeader<TKey> *header, cQuickDB* quickDB, bool state = false);
	bool Clear();

	//bool Find(const TKey  &item);
	
	TLeafNode* ReadNewLeafNode();
	void UnlockLeafNode(TLeafNode* leafnode);
	TNode* ReadNewNode();
	void UnlockNode(TNode* node);

	//tNodeIndex InsertLeafNode(TKey **items, unsigned int &count, tNodeIndex previousNode);
	//tNodeIndex InsertInnerNode(TNodeItem *items, tNodeIndex *links, unsigned int& from, unsigned int count);
	//TNodeItem* GetNodeKey(tNodeIndex nodeIndex);
	
	//tNodeIndex LocateLeaf(const TKey &item);
	//bool Insert(const TKey  &item, char* data, unsigned int insertMode = cUBTreeConst::INSERT_MODE_INSERT_ONLY);
	bool Insert(const TKey &item, char* data);
	bool InsertOrIncrement(const TKey  &item, char* data);
	//bool UpdateOrInsert(const TKey &item, bool pIncrementFlag = false);

	void Delete(const TKey  &item);

	// methods for sequential search
	void Seek(int position);
	void GetNext(char* item, cNodeBuffers<TKey>* buffers = NULL);
	bool IsNext();

	// methods for sequential search, where the actual B-tree node is locked
	void LockedSeek(int position);
	TKey *LockedGetNext();
	bool LockedIsNext();
	void CurrentNodeModified();

	void CheckInnerNode(tNodeIndex index);
	void Print(int mode);
	void PrintInfo();
	void PrintNode(unsigned int index);
	void PrintDimDistribution();

	inline const cDTDescriptor* GetKeyDescriptor();

	void SetDebug(bool value);
	TKey *GetMaxKeyValue();
};  

template <class TNode, class TLeafNode, class TKey> 
cCommonUBTree<TNode, TLeafNode, TKey>::cCommonUBTree()
	:cPagedTree<TKey, TNode, TLeafNode>(), mInMemCache(NULL), mInMemCacheEnabled(false)
{
//	FullTitle();
	mDebug = false;
	mRQProcessor = new cRangeQueryProcessor<TKey, TNode, TLeafNode>(this);
}

template <class TNode, class TLeafNode, class TKey> 
cCommonUBTree<TNode, TLeafNode, TKey>::~cCommonUBTree()
{
	if (mInMemCache != NULL)
	{
		delete mInMemCache;
		parent::mQuickDB->GetMemoryManager()->ReleaseMem(mInMemCacheMemBlock);
	}
	if (mRQProcessor != NULL)
	{
		delete mRQProcessor;
		mRQProcessor = NULL;
	}
}

template <class TNode, class TLeafNode, class TKey> 
inline const cDTDescriptor* cCommonUBTree<TNode, TLeafNode, TKey>::GetKeyDescriptor() 
{
	return parent::mHeader->GetNodeHeader(cTreeHeader::HEADER_LEAFNODE)->GetKeyDescriptor();
}

/**
  * Set Full title of B+tree according to node types
  */
template <class TNode, class TLeafNode, class TKey> 
void cCommonUBTree<TNode, TLeafNode, TKey>::FullTitle() 
{
	// mk: delete?
	/*char title[cTreeHeader::TITLE_SIZE];
	sprintf(title, "%s%c_%c%u:%c_%c%u%c", parent::mHeader->GetTitle(), TNodeItem::CODE, TNodeItem::BT::CODE, TNodeItem::BT::SER_SIZE, TKey::CODE, TKey::BT::CODE, TKey::BT::SER_SIZE, '\0');
	parent::mHeader->SetTitle(title);*/
}

/**
 * \param dsName Name of the data structure.
 * Create Empty Tree. Child of root node will be empty node.
 **/
template <class TNode, class TLeafNode, class TKey> 
bool cCommonUBTree<TNode, TLeafNode, TKey>::Create(cUBTreeHeader<TKey> *header, cQuickDB* quickDB)
{
	if (!cPagedTree<TKey ,TNode,TLeafNode>::Create(header, quickDB))
	{
		return false;
	}

	mRQProcessor->SetQuickDB(quickDB);
	mRQProcessor->SetTreeHeader(header);

	// create empty root node
	TLeafNode* leafnode = parent::ReadNewLeafNode();
	
	leafnode->SetItemCount(0);
	leafnode->SetLeaf(true);
	leafnode->Init();
	//leafnode->SetExtraLink(0, TNode::EMPTY_LINK);
	//leafnode->SetExtraLink(1, TNode::EMPTY_LINK);
	parent::mHeader = header;
	parent::mHeader->ResetNodeCount();
	parent::mHeader->ResetItemCount();

	parent::mHeader->SetHeight(0);
	parent::mHeader->SetLeafNodeCount(1);
	parent::mHeader->SetRootIndex(leafnode->GetLeafNodeIndex(leafnode->GetIndex()));    // 3.6.2013: nebylo GetLeafNodeIndex, nemohlo to fungovat
	parent::mSharedCache->UnlockW(leafnode);

	mActualNode = TNode::EMPTY_LINK;

	InitInMemCache(header);

	return true;
}

template <class TNode, class TLeafNode, class TKey> 
bool cCommonUBTree<TNode, TLeafNode, TKey>::Open(cUBTreeHeader<TKey> *header, cQuickDB* quickDB, bool state)
{
	if (!cPagedTree<TKey, TNode, TLeafNode>::Open(header, quickDB, state))
	{
		return false;
	}

	cUBTreeHeader<TKey> **p = &header;

	mRQProcessor->SetQuickDB(quickDB);
	mRQProcessor->SetTreeHeader(header);

	InitInMemCache(header);

	return true;
}

template <class TNode, class TLeafNode, class TKey> 
void cCommonUBTree<TNode, TLeafNode, TKey>::InitInMemCache(cUBTreeHeader<TKey> *header)
{
	// Create iff InMemCacheSize != 0 && variable length key or data are not used
	if (header->GetInMemCacheSize() != 0 && 
		(TKey::LengthType != cDataType::LENGTH_VARLEN && !parent::mHeader->GetNodeHeader(cTreeHeader::HEADER_LEAFNODE)->VariableLenDataEnabled()))
	{
		mInMemCacheEnabled = (header->GetInMemCacheSize() > 0) ? true : false;
		mDataDTDesc = new cSpaceDescriptor(header->GetLeafDataSize(), new cTuple(), new cChar());
		mInMemCacheMemBlock = parent::mQuickDB->GetMemoryManager()->GetMem(header->GetInMemCacheSize() * 
			parent::mHeader->GetNodeHeader(cTreeHeader::HEADER_LEAFNODE)->GetItemSize());
		mInMemCache = new cHashTable<TKey, cCharString>(header->GetInMemCacheSize(), mInMemCacheMemBlock,
			parent::mHeader->GetNodeHeader(cTreeHeader::HEADER_LEAFNODE)->GetKeyDescriptor(), mDataDTDesc);
		mInMemCache->Clear();
	}
	else
	{
		mInMemCacheEnabled = false;
		mInMemCache = NULL;
		mInMemCacheMemBlock = NULL;
	}
}


template <class TNode, class TLeafNode, class TKey> 
bool cCommonUBTree<TNode, TLeafNode, TKey>::Clear()
{
	if (!cPagedTree<TKey,TNode,TLeafNode>::Clear())
	{
		return false;
	}

	// create empty root node
	TLeafNode* leafnode = parent::ReadNewLeafNode();

	leafnode->SetItemCount(0);
	leafnode->SetLeaf(true);
	leafnode->Init();
	leafnode->SetExtraLink(0, TNode::EMPTY_LINK);
	leafnode->SetExtraLink(1, TNode::EMPTY_LINK);
	parent::mHeader->ResetNodeCount();
	parent::mHeader->ResetItemCount();

	parent::mHeader->SetHeight(0);
	parent::mHeader->SetLeafNodeCount(1);
	parent::mHeader->SetRootIndex(leafnode->GetLeafNodeIndex(leafnode->GetIndex()));    // 3.6.2013: nebylo GetLeafNodeIndex, nemohlo to fungovat
	parent::mSharedCache->UnlockW(leafnode);

	mActualNode = TNode::EMPTY_LINK;
	return true;
}

/**
*	Set the pointer for the sequential search
*/
template <class TNode, class TLeafNode, class TKey> 
void cCommonUBTree<TNode, TLeafNode, TKey>::Seek(int position)
{
	mReadWriteMutex.lock();

	if (parent::mReadOnly)
	{
		//if (mDebug)
		//{
		//	printf("Seek:\n");
		//	parent::mSharedCache->PrintLocks();
		//}


		for ( ; ; )
		{
			if (TNode::IsLeaf(nodeIndex) == true || (parent::mHeader->GetHeight()==0 && parent::mHeader->GetItemCount() != 0))
			{
				mActualNode = nodeIndex;
				mActualOrder = 0;
				break;
			}
			else if (nodeIndex == parent::mHeader->GetRootIndex() && (parent::mHeader->GetHeight()==0))
			{
				printf("error: You cannot prune empty B-tree!\n");
				break;
			}
			else
			{
				currentNode = parent::ReadInnerNodeR(nodeIndex);
				nodeIndex = currentNode->GetLink(0);
				parent::mSharedCache->UnlockR(currentNode);			
			}
		}
	}
}

/**
*	Get next item in a tree
*/
template <class TNode, class TLeafNode, class TKey> 
void cCommonUBTree<TNode, TLeafNode, TKey>::GetNext(char* item, cNodeBuffers<TKey>* buffers)
{
	assert(mActualNode != TLeafNode::EMPTY_LINK);
	TLeafNode *currentLeafNode = parent::ReadLeafNodeR(mActualNode);
	assert(mActualOrder < currentLeafNode->GetItemCount());
	// Warning: It is not necessary to copy!
	cNodeItem::Copy(item, currentLeafNode->GetCItem(mActualOrder++, &buffers->itemBuffer), parent::mHeader->GetLeafNodeItemSize());

	//if(mDebug)
	//{
	//	currentLeafNode->Print(buffer);
	//}

	if (mActualOrder >= currentLeafNode->GetItemCount())
	{
		mActualNode = currentLeafNode->GetExtraLink(1);
		mActualOrder = 0;
		
	}
	parent::mSharedCache->UnlockR(currentLeafNode);
}

/**
*	Test the end of the sequential search
*	\return false if we are at the end of tree
*/
template <class TNode, class TLeafNode, class TKey> 
bool cCommonUBTree<TNode, TLeafNode, TKey>::IsNext()
{
	if (mActualNode == TNode::EMPTY_LINK)
	{
		return false;
	} else
	{
		return true;
	}
}

/**
* Set the pointer for the sequential search. The leaf node with pointer is locked.
* \param position SEEK_START - set the pointer to the begining of the B-tree
*/
template <class TNode, class TLeafNode, class TKey> 
void cCommonUBTree<TNode, TLeafNode, TKey>::LockedSeek(int position)
{
	tNodeIndex nodeIndex = parent::mHeader->GetRootIndex();
	TNode *currentNode = NULL;

	if (position == SEEK_START)
	{
		//if (mDebug)
		//{
		//	printf("Seek:\n");
		//	parent::mSharedCache->PrintLocks();
		//}


		for ( ; ; )
		{
			if (TNode::IsLeaf(nodeIndex) == true || (parent::mHeader->GetHeight() == 0 && parent::mHeader->GetItemCount() != 0))
			{
				mCurrentLeafNode = parent::ReadLeafNodeR(nodeIndex);
				mActualNode = nodeIndex;
				mActualOrder = 0;
				break;
			}
			else if (nodeIndex == parent::mHeader->GetRootIndex() && (parent::mHeader->GetHeight()==0))
			{
				printf("error: You cannot prune empty B-tree!\n");
				break;
			}
			else
			{
				currentNode = parent::ReadInnerNodeR(nodeIndex);
				nodeIndex = currentNode->GetLink(0);
				parent::mSharedCache->UnlockR(currentNode);			
			}
		}
	}
}

/**
* Get next item in a tree and keep the leaf node locked. 
* This method require that the inital call is LockedSeek() (not only Seek()).
*/
template <class TNode, class TLeafNode, class TKey> 
TKey *cCommonUBTree<TNode, TLeafNode, TKey>::LockedGetNext()
{
	assert(mActualNode != TLeafNode::EMPTY_LINK);
	assert(IsNext());

	if (mActualOrder >= mCurrentLeafNode->GetItemCount())
	{
		parent::mSharedCache->UnlockR(mCurrentLeafNode->GetMemoryBlock());
		mCurrentLeafNode = parent::ReadLeafNodeR(mActualNode);
		mActualOrder = 0;
	}

	TKey *item = mCurrentLeafNode->GetKey(mActualOrder++);

	if (mActualOrder >= mCurrentLeafNode->GetItemCount())
	{
		mActualNode = mCurrentLeafNode->GetExtraLink(1);
	}

	return item;
}


/**
*	Test the end of the sequential search.
*	\return false if we are at the end of tree. It also unlock the last node.
*/
template <class TNode, class TLeafNode, class TKey> 
bool cCommonUBTree<TNode, TLeafNode, TKey>::LockedIsNext()
{
	if (mActualNode == TNode::EMPTY_LINK)
	{
		return false;
		parent::mSharedCache->UnlockR(mCurrentLeafNode->GetMemoryBlock());
	} else
	{
		return true;
	}
}

/**
* Set the modified flag of the actual leaf node. Can be used only together with locked methods of the sequential search.
* Otherwise it can lead to an error.
*/
template <class TNode, class TLeafNode, class TKey> 
void cCommonUBTree<TNode, TLeafNode, TKey>::CurrentNodeModified()
{
	mCurrentLeafNode->SetModified(true);
}

/// Read every child node and set the parent link
template <class TNode, class TLeafNode, class TKey> 
void cCommonUBTree<TNode, TLeafNode, TKey>::SetParentLinkToEveryChild(TNode *node)
{
	TNode *currentNode = NULL;
	TLeafNode *currentLeafNode = NULL;	

	for (unsigned int i = 0; i < node->GetItemCount(); i++)
	{
		if (TNode::IsLeaf(node->GetLink(i)))
		{
			currentLeafNode = parent::ReadLeafNodeW(node->GetLink(i));
			currentLeafNode->SetExtraLink(2, node->GetIndex());
			parent::mSharedCache->UnlockW(currentLeafNode);
		} else
		{
			currentNode = parent::ReadInnerNodeW(node->GetLink(i));
			currentNode->SetExtraLink(0, node->GetIndex());
			parent::mSharedCache->UnlockW(currentNode);
		}
	}
}

template <class TNode, class TLeafNode, class TKey>
TLeafNode* cCommonUBTree<TNode, TLeafNode, TKey>::ReadNewLeafNode()
{
	TLeafNode *leafNode = parent::ReadNewLeafNode();
	parent::mHeader->IncrementLeafNodeCount();
	return leafNode;
}


template <class TNode, class TLeafNode, class TKey>
void cCommonUBTree<TNode, TLeafNode, TKey>::UnlockLeafNode(TLeafNode* leafNode)
{
	parent::mSharedCache->UnlockW(leafNode);
}


template <class TNode, class TLeafNode, class TKey>
TNode* cCommonUBTree<TNode, TLeafNode, TKey>::ReadNewNode()
{
	TNode *node = parent::ReadNewInnerNode();
	parent::mHeader->IncrementInnerNodeCount();
	return node;
}


template <class TNode, class TLeafNode, class TKey>
void cCommonUBTree<TNode, TLeafNode, TKey>::UnlockNode(TNode* node)
{
	parent::mSharedCache->UnlockW(node);
}

/**
 * Insert item into B+tree.
 * \param
 *   insertMode: insert only (default) | insert or update (see cB+TreeConst.h)
 * \return 
 *		- true if the key and data was correctly inserted.
 *		- false if the key already exists in the B-tree.
 **/

template <class TNode, class TLeafNode, class TKey>
bool cCommonUBTree<TNode, TLeafNode, TKey>::Insert(const TKey &item, char* data)
{
	cSpaceDescriptor *sd = (cSpaceDescriptor*)parent::mHeader->GetNodeHeader(cTreeHeader::HEADER_LEAFNODE)->GetKeyDescriptor();
	unsigned int counter = 0;     // counter of acrossing pages
	cUBTreeHeader<TKey> * header = (cUBTreeHeader<TKey>*)parent::mHeader;
	tNodeIndex nodeIndex = parent::mHeader->GetRootIndex(), insNodeIndex = 0, previousNodeIndex = 0, parentNodeIndex = 0;
	bool leaf, ret = true;
	int state = 0;                // 0 ... go down, 1 ... go up
	int retInsert, chld, order;
	TNode *currentNode = NULL;
	TLeafNode *currentLeafNode = NULL;
	
	// item count = 0;
	if(header->GetItemCount()==0)
	{
		TKey::Copy(header->GetTreeMBR()->GetLoTuple()->GetData(), item.GetData(), sd);
		TKey::Copy(header->GetTreeMBR()->GetHiTuple()->GetData(), item.GetData(), sd);
	}
	

	for(;;)
	{
		if(leaf= TNode::IsLeaf(nodeIndex))
		{
			currentLeafNode = parent::ReadLeafNodeW(nodeIndex);
		}
		else
		{

		}
			
	}
	
	
	/*
	if (mInMemCacheEnabled)
	{
		if (!mInMemCache->Add(item, data))
		{
			printf("Warning: HashTable in B-tree is full!\n");
		}
	}

	unsigned int counter = 0;     // counter of acrossing pages
	cUBTreeHeader<TKey> * header = (cUBTreeHeader<TKey>*)parent::mHeader;
	tNodeIndex nodeIndex = parent::mHeader->GetRootIndex(), insNodeIndex = 0, previousNodeIndex = 0, parentNodeIndex = 0;
	bool leaf, ret = true;
	int state = 0;                // 0 ... go down, 1 ... go up
	int retInsert, chld, order;
	TNode *currentNode = NULL;
	TLeafNode *currentLeafNode = NULL;
	//cInsertBuffers<TKey> insertBuffers;
	//cNodeBuffers<TKey>* nodeBuffers = &insertBuffers.nodeBuffer;
	//mDebug = true;


	
	if (mDebug)
	{
		item.Print("\n", sd);
	}

	if (header->IsHistogramEnabled() && (TKey::CODE == cTuple::CODE))
	{
		if (header->GetItemCount() == 0)
		{
			TKey::Copy(header->GetTreeMBR()->GetLoTuple()->GetData(), item.GetData(), sd);
			TKey::Copy(header->GetTreeMBR()->GetHiTuple()->GetData(), item.GetData(), sd);
		}
		else
		{
			header->GetTreeMBR()->ModifyMbr(item, sd);
		}
	}

	parent::Insert(item);
	Insert_pre(&insertBuffers);

	
	
	/*
	for ( ; ; )
	{
		if (state == 0)          // go down
		{
			if ((leaf = TNode::IsLeaf(nodeIndex)))
			{
				currentLeafNode = parent::ReadLeafNodeW(nodeIndex);

				if (mDebug) { currentLeafNode->Print(&nodeBuffers->itemBuffer); }
			}
			else
			{
				currentNode = parent::ReadInnerNodeR(nodeIndex);
				insertBuffers.currentPath[counter++] = nodeIndex;

				if (mDebug) { currentNode->Print(); }
			}

			if (leaf)
			{
				retInsert = TNode::INSERT_NOSPACE;
				unsigned int itemOrder = TNode::FIND_NOTEXIST;

				// First, solve the mode: Insert or Increment, try to find the item
				if (insertMode == cUBTreeConst::INSERT_MODE_INSERT_OR_INC)
				{
					itemOrder = currentLeafNode->FindOrder(item, TNode::FIND_E, &nodeBuffers->itemBuffer);
				}

				// Test free space if: operation is in the insert only mode or it is in the inser or increment mode
				// and the item was not found
				if (itemOrder == TNode::FIND_NOTEXIST && currentLeafNode->HasLeafFreeSpace(item, data))
				{
					// If node contains enough space
					retInsert = currentLeafNode->InsertLeafItem(item, data, parent::mHeader->DuplicatesAllowed(), nodeBuffers);
				}

				if (retInsert != TNode::INSERT_NOSPACE)
				{
					if (retInsert == TNode::INSERT_EXIST)
					{
						ret = false; 
						parent::mSharedCache->UnlockW(currentLeafNode);
						break;
					}
					else if (retInsert == TNode::INSERT_AT_THE_END)
					{
						if (parent::mHeader->GetHeight() != 0)
						{
							currentLeafNode->CopyKeyTo(insertBuffers.firstItem, currentLeafNode->GetItemCount() - 1, &nodeBuffers->itemBuffer);
							parent::mSharedCache->UnlockW(currentLeafNode);

							state = 2;
						} else
						{
							parent::mSharedCache->UnlockW(currentLeafNode);
							break;
						}
					} else
					{
						parent::mSharedCache->UnlockW(currentLeafNode);
						break;	
					}
				} else if (itemOrder != TNode::FIND_NOTEXIST)
				{   // Operation is in the Insert or Increment mode and the item has been found
					ret = false; 
					assert(!parent::mHeader->DuplicatesAllowed());
					
					char* dataInNode = currentLeafNode->GetData(itemOrder, &nodeBuffers->itemBuffer);
					// increment data, it means, suppose the 4 first bytes as unsigned int, and copy it into the data parameter
					unsigned int value = cUInt::GetValue(dataInNode) + 1;  // read the first int and increment
					cUInt::SetValue(dataInNode, value);  // write the first int
					// copy the data
					cNodeItem::Copy(data, dataInNode, parent::mHeader->GetLeafDataSize());
		
					parent::mSharedCache->UnlockW(currentLeafNode);
					break;
				}
				else 
				{  
					// Operation is in the Insert only mode and there is enough space for the item
					// Split and insert
					SplitLeafNode(currentLeafNode, insertBuffers.firstItem, insertBuffers.secondItem, insNodeIndex, nodeBuffers);

					if (item.Compare(insertBuffers.firstItem, currentLeafNode->GetNodeHeader()->GetKeyDescriptor()) <= 0)
					{
						// insert into first node
						retInsert = currentLeafNode->InsertLeafItem(item, data, parent::mHeader->DuplicatesAllowed(), nodeBuffers);
					} else
					{
						// insert into second node
						parent::mSharedCache->UnlockW(currentLeafNode);
						currentLeafNode = parent::ReadLeafNodeW(insNodeIndex);
						retInsert = currentLeafNode->InsertLeafItem(item, data, parent::mHeader->DuplicatesAllowed(), &insertBuffers.nodeBuffer);

						if (retInsert == TNode::INSERT_AT_THE_END)
						{
							// update secondItem
							currentLeafNode->CopyKeyTo(insertBuffers.secondItem, currentLeafNode->GetItemCount() - 1, &nodeBuffers->itemBuffer);
							retInsert = TNode::INSERT_YES;
						}
					}

					if (retInsert == TNode::INSERT_EXIST)
					{
						ret = false; 
					}
					parent::mSharedCache->UnlockW(currentLeafNode);

					if (nodeIndex == parent::mHeader->GetRootIndex())
					{
						CreateRootNode(TLeafNode::GetLeafNodeIndex(parent::mHeader->GetRootIndex()), insertBuffers.firstItem, 
							insNodeIndex, insertBuffers.secondItem);
						break;
					}
					state = 1;
				}
				assert(retInsert != TNode::INSERT_NOSPACE);

				previousNodeIndex = TNode::GetLeafNodeIndex(currentLeafNode->GetIndex());
				parentNodeIndex = currentLeafNode->GetExtraLink(2);
			}
			else // Inner node; go down
			{
				// mk: 7.6.2013
				if ((chld = currentNode->FindOrderInsert(item, TNode::FIND_SBE)) == TNode::FIND_NOTEXIST)
				{
					chld = currentNode->GetItemCount() - 1;
				}

				if (chld >= currentNode->GetItemCount())
				{
					printf("Critical Error: A correct child is not found!\n");
					item.Print("\n", sd);
					currentNode->Print();
				}

				insertBuffers.orderCurrentPath[counter - 1] = chld;
				nodeIndex = currentNode->GetLink(chld);
				parent::mSharedCache->UnlockR(currentNode);
			}
		} else  if (state == 1)// Inner node; go up
		{
			currentNode = SearchForRightOrderInInnerNode(parentNodeIndex, previousNodeIndex, counter, order, insertBuffers.currentPath, insertBuffers.orderCurrentPath);
			if (currentNode->HasFreeSpace(insertBuffers.secondItem)) // is in the node enough space?
			{
				currentNode->SetKey(order, insertBuffers.firstItem);
				currentNode->InsertItem(order + 1, insertBuffers.secondItem, insNodeIndex);

				if (order + 2 == currentNode->GetItemCount())
				{
					if (counter != 0)
					{
						currentNode->CopyKeyTo(insertBuffers.firstItem, currentNode->GetItemCount() - 1, &nodeBuffers->itemBuffer);

						parent::mSharedCache->UnlockW(currentNode);
						state = 2;
					} else
					{
						parent::mSharedCache->UnlockW(currentNode);
						break;
					}
				} else {
					// write if don't spTKey
					parent::mSharedCache->UnlockW(currentNode);
					break;
				}
			} else
			{
				// split the inner node first and then process insert
				TNode* newNode = parent::ReadNewInnerNode();

				char* TNode_mem = parent::mMemoryPool->GetMem(parent::mHeader->GetNodeInMemSize());
				TNode tmpNode(newNode, TNode_mem);

				currentNode->SetKey(order, insertBuffers.firstItem);
				currentNode->Split(*newNode, tmpNode, nodeBuffers);
				newNode->SetExtraLink(0, currentNode->GetExtraLink(0));

				if (item.Compare(currentNode->GetCKey(currentNode->GetItemCount() - 1, &nodeBuffers->itemBuffer), GetKeyDescriptor()) <= 0)
				{
					retInsert = currentNode->InsertItem(insertBuffers.secondItem, insNodeIndex, parent::mHeader->DuplicatesAllowed());
				} else
				{
					retInsert = newNode->InsertItem(insertBuffers.secondItem, insNodeIndex, parent::mHeader->DuplicatesAllowed());
				}

				// delete the temporary node
				tmpNode.SetData(NULL);
				parent::mMemoryPool->FreeMem(TNode_mem);

				currentNode->CopyKeyTo(insertBuffers.firstItem, currentNode->GetItemCount() - 1, &nodeBuffers->itemBuffer);
				newNode->CopyKeyTo(insertBuffers.secondItem, newNode->GetItemCount() - 1, &nodeBuffers->itemBuffer);

				SetParentLinkToEveryChild(newNode);
				insNodeIndex = newNode->GetIndex();

				parent::mSharedCache->UnlockW(currentNode);
				parent::mSharedCache->UnlockW(newNode);

				/*** it is necessary create new root node? ***/
				/*if (counter == 0)
				{
					CreateRootNode(parent::mHeader->GetRootIndex(), insertBuffers.firstItem, insNodeIndex, insertBuffers.secondItem);
					break;
				}
				state = 1;
			}

			previousNodeIndex = currentNode->GetIndex();
			parentNodeIndex = currentNode->GetExtraLink(0);
		} 
		else if (state == 2) // modify the key in the inner node
		{			
			currentNode = SearchForRightOrderInInnerNode(parentNodeIndex, previousNodeIndex, counter, order, insertBuffers.currentPath, insertBuffers.orderCurrentPath);
			currentNode->SetKey(order, insertBuffers.firstItem);

			if (counter != 0 && (unsigned int)order + 1 == currentNode->GetItemCount())
			{
				previousNodeIndex = currentNode->GetIndex();
				parentNodeIndex = currentNode->GetExtraLink(0);
				currentNode->CopyKeyTo(insertBuffers.firstItem, currentNode->GetItemCount() - 1, &nodeBuffers->itemBuffer);
				parent::mSharedCache->UnlockW(currentNode);
			} else
			{
				parent::mSharedCache->UnlockW(currentNode);
				break;
			}
		} else
		{
			printf("BpTree::Insert - unknown state in B-tree insert algorithm! Damaged heap?\n");
		}
	}

	Insert_post(&insertBuffers);

	if (parent::mMemoryPool->IsMemUsed())
	{
		printf("Error\n");
	}
	*/
	return ret;
}

/*template <class TNode, class TLeafNode, class TKey> 
void cCommonUBTree<TNode, TLeafNode, TKey>::Insert_pre(cInsertBuffers<TKey>* insertBuffers)
{
	// buffer sometimes is not necessary
	unsigned int memSize = 2 * parent::mHeader->GetNodeItemSize() + (parent::mHeader->GetHeight() + 2) * sizeof(tNodeIndex) +
		(parent::mHeader->GetHeight() + 2) * sizeof(int) + (2 * parent::mHeader->GetTmpBufferSize());

	cMemoryBlock* bufferMemBlock = parent::mQuickDB->GetMemoryManager()->GetMem(memSize);
	insertBuffers->bufferMemBlock = bufferMemBlock;
	char* mem = bufferMemBlock->GetMem();

	//*memBlock = parent::mQuickDB->GetMemoryManager()->GetMem(memSize);
	//char* mem = (*memBlock)->GetMem();
	insertBuffers->firstItem = mem;
	mem += parent::mHeader->GetNodeItemSize();
	insertBuffers->secondItem = mem;

	mem += parent::mHeader->GetNodeItemSize();
	insertBuffers->currentPath = (tNodeIndex*) mem;

	mem += (parent::mHeader->GetHeight() + 2) * sizeof(tNodeIndex); // the number of nodes + 1 - the height may be increased
	insertBuffers->orderCurrentPath = (int*) mem;

	if (parent::mHeader->GetDStructMode() != cDStructConst::DSMODE_DEFAULT)
	{
		mem += (parent::mHeader->GetHeight() + 2) * sizeof(int) ;
	}

	if (parent::mHeader->GetDStructMode() == cDStructConst::DSMODE_RI)
	{
		insertBuffers->nodeBuffer.itemBuffer.riBuffer = mem;
	}
	else if (parent::mHeader->GetDStructMode() == cDStructConst::DSMODE_CODING)
	{
		insertBuffers->nodeBuffer.itemBuffer.codingBuffer = mem;
	}
	else  if (parent::mHeader->GetDStructMode() == cDStructConst::DSMODE_RICODING)
	{
		insertBuffers->nodeBuffer.itemBuffer.riBuffer = mem;
		insertBuffers->nodeBuffer.itemBuffer.codingBuffer = mem + (parent::mHeader->GetTmpBufferSize() / 2);
	}

	if (parent::mHeader->GetDStructMode() != cDStructConst::DSMODE_DEFAULT)
	{
		mem += parent::mHeader->GetTmpBufferSize();
	}

	if (parent::mHeader->GetDStructMode() == cDStructConst::DSMODE_RI)
	{
		insertBuffers->nodeBuffer.itemBuffer2.riBuffer = mem;
	}
	else if (parent::mHeader->GetDStructMode() == cDStructConst::DSMODE_CODING)
	{
		insertBuffers->nodeBuffer.itemBuffer2.codingBuffer = mem;
	}
	else  if (parent::mHeader->GetDStructMode() == cDStructConst::DSMODE_RICODING)
	{
		insertBuffers->nodeBuffer.itemBuffer2.riBuffer = mem;
		insertBuffers->nodeBuffer.itemBuffer2.codingBuffer = mem + (parent::mHeader->GetTmpBufferSize() / 2);
	}
}

template <class TNode, class TLeafNode, class TKey> 
void cCommonUBTree<TNode, TLeafNode, TKey>::Insert_post(cInsertBuffers<TKey>* insertBuffers)
{
	parent::mQuickDB->GetMemoryManager()->ReleaseMem(insertBuffers->bufferMemBlock);
}

template <class TNode, class TLeafNode, class TKey>
void cCommonUBTree<TNode, TLeafNode, TKey>::RangeQuery_pre(cNodeBuffers<TKey>* nodeBuffers)
{
	unsigned int memSize = 0;

	// create a temporary buffer if necessary
	if (parent::mHeader->GetDStructMode() == cDStructConst::DSMODE_RI || parent::mHeader->GetDStructMode() == cDStructConst::DSMODE_RICODING)
	{
		memSize = parent::mHeader->GetTmpBufferSize();
	}
	else if (parent::mHeader->GetDStructMode() == cDStructConst::DSMODE_CODING)
	{
		memSize = ((parent::mHeader->GetKeySize() * 2) + parent::mHeader->GetLeafDataSize()) * 2;
	}

	if (memSize > 0)
	{
		nodeBuffers->bufferMemBlock = parent::mQuickDB->GetMemoryManager()->GetMem(memSize);
		char* mem = (nodeBuffers->bufferMemBlock)->GetMem();

		if (parent::mHeader->GetDStructMode() == cDStructConst::DSMODE_RI)
		{
			nodeBuffers->itemBuffer.riBuffer = mem;
		}
		else if (parent::mHeader->GetDStructMode() == cDStructConst::DSMODE_CODING)
		{
			nodeBuffers->itemBuffer.codingBuffer = mem;
		}
		else  if (parent::mHeader->GetDStructMode() == cDStructConst::DSMODE_RICODING)
		{
			nodeBuffers->itemBuffer.riBuffer = mem;
			nodeBuffers->itemBuffer.codingBuffer = mem + (parent::mHeader->GetTmpBufferSize() / 2);
		}
	}
 else
 {
	 nodeBuffers->bufferMemBlock = NULL;
 }
}


template <class TNode, class TLeafNode, class TKey>
void cCommonUBTree<TNode, TLeafNode, TKey>::RangeQuery_post(cNodeBuffers<TKey>* nodeBuffers)
{
	if (nodeBuffers->bufferMemBlock != NULL)
	{
		parent::mQuickDB->GetMemoryManager()->ReleaseMem(nodeBuffers->bufferMemBlock);
	}	
}
*/
/// Look for a proper order of a child node in this node
template <class TNode, class TLeafNode, class TKey> 
TNode *cCommonUBTree<TNode, TLeafNode, TKey>::SearchForRightOrderInInnerNode(const tNodeIndex parentNodeIndex, const tNodeIndex childNodeIndex, unsigned int &counter, int &order, tNodeIndex* currentPath, int* orderCurrentPath)
{
	UNUSED(childNodeIndex);
	counter--; 
	tNodeIndex nodeIndex = currentPath[counter];
	TNode *currentNode = NULL;

	if (nodeIndex != parentNodeIndex) // if the node should be another else than was readed from stack
	{
		printf("%d, %d", nodeIndex, parentNodeIndex);
		printf("cCommonUBTree::SearchForRightOrderInInnerNode - error during up phase. It should never happen in B-tree with unsorted leaves!\n");
	} else
	{
		currentNode = parent::ReadInnerNodeW(nodeIndex);
		order = orderCurrentPath[counter];
	}

	if (order == -1)
	{
		printf("BpTree::SearchForRightOrderInInnerNode - the child node index wasn't found in this node. Wrong parent link?\n");
	}

	return currentNode;
}


/// Create new root node in the tree
template <class TNode, class TLeafNode, class TKey> 
void cCommonUBTree<TNode, TLeafNode, TKey>
	::CreateRootNode(const tNodeIndex &linkToFirstNode, char* itemOfFirstNode, const tNodeIndex &linkToSecondNode, char* itemOfSecondNode)
{
	TNode *currentNode = parent::ReadNewInnerNode();

	currentNode->SetLeaf(false);
	currentNode->SetItemCount(0);
	currentNode->SetFreeSize(currentNode->GetNodeHeader()->GetNodeItemsSpaceSize());

	currentNode->AddItem(itemOfFirstNode, linkToFirstNode, true);
	currentNode->AddItem(itemOfSecondNode, linkToSecondNode, true);
	currentNode->SetExtraLink(0, 0);

	//TKey::Print(itemOfFirstNode, "\n", currentNode->GetNodeHeader()->GetKeyDescriptor());
	//TKey::Print(itemOfSecondNode, "\n", currentNode->GetNodeHeader()->GetKeyDescriptor());
	//currentNode->Print();

	parent::mHeader->SetRootIndex(currentNode->GetIndex());
	parent::mHeader->IncrementHeight();
	parent::mHeader->IncrementInnerNodeCount();
	
	SetParentLinkToEveryChild(currentNode);

	parent::mSharedCache->UnlockW(currentNode);
}

/**
* Split the leaf node and return the keys of new nodes
* \param currentLeafNode Node that has to be split.
* \param firstItem Parameter by reference. Key of the currentLeafNode.
* \param secondItem Parameter by reference. Key of the new node which is created during the split.
* \param insNodeIndex Parameter by reference. Index of the newly created node (which is created during the split).
*/
template <class TNode, class TLeafNode, class TKey> 
void cCommonUBTree<TNode, TLeafNode, TKey>
::SplitLeafNode(TLeafNode *currentLeafNode, char* firstItem, char* secondItem, tNodeIndex& insNodeIndex, cNodeBuffers<TKey>* buffers)
{
	char* TLeafNode_mem = parent::mMemoryPool->GetMem(parent::mHeader->GetLeafNodeInMemSize());

	TLeafNode *nextLeafNode = NULL;
	TLeafNode *newLeafNode = parent::ReadNewLeafNode();
	//TLeafNode *newLeafNode = parent::ReadNewLeafNode((currentLeafNode->GetItemCount() + 1) / 2);

	TLeafNode tmpNode(newLeafNode, TLeafNode_mem);

	newLeafNode->SetLeaf(true);
	newLeafNode->SetExtraLink(0,TLeafNode::EMPTY_LINK);
	newLeafNode->SetExtraLink(1,TLeafNode::EMPTY_LINK);

	currentLeafNode->Split(*newLeafNode, tmpNode, buffers);

	currentLeafNode->CopyKeyTo(firstItem, currentLeafNode->GetItemCount()-1, &buffers->itemBuffer);
	newLeafNode->CopyKeyTo(secondItem, newLeafNode->GetItemCount()-1, &buffers->itemBuffer);

	newLeafNode->SetExtraLink(1, currentLeafNode->GetExtraLink(1)); 										//  new  -> R
	newLeafNode->SetExtraLink(0, currentLeafNode->GetLeafNodeIndex(currentLeafNode->GetIndex()));			//  this -> new
	currentLeafNode->SetExtraLink(1, currentLeafNode->GetLeafNodeIndex(newLeafNode->GetIndex()));			//  this <- new
	newLeafNode->SetExtraLink(2, currentLeafNode->GetExtraLink(2));											// copy parent node 
	
	insNodeIndex = TLeafNode::GetLeafNodeIndex(newLeafNode->GetIndex());

	// correct the prev link of the next node, which follow the spTKeyed node
	if (newLeafNode->GetExtraLink(1) != TLeafNode::EMPTY_LINK)
	{
		nextLeafNode = parent::ReadLeafNodeW(newLeafNode->GetExtraLink(1));
		nextLeafNode->SetExtraLink(0, insNodeIndex);
		parent::mSharedCache->UnlockW(nextLeafNode);
	}

	// free both tuples
	tmpNode.SetData(NULL);
	// free temporary memory
	parent::mMemoryPool->FreeMem(TLeafNode_mem);

	parent::mSharedCache->UnlockW(newLeafNode);
}


template <class TNode, class TLeafNode, class TKey> 
bool cCommonUBTree<TNode, TLeafNode, TKey>::InsertOrIncrement(const TKey  &item, char* data)
{
	return Insert(item, data, cUBTreeConst::INSERT_MODE_INSERT_OR_INC);

	//unsigned int counter = 0;     // counter of acrossing pages
	//tNodeIndex nodeIndex = parent::mHeader->GetRootIndex(), insNodeIndex = 0, previousNodeIndex = 0, parentNodeIndex = 0;
 //   char *firstItem = parent::mMemoryPool->GetMem(parent::mHeader->GetNodeItemSize());
 //   char *secondItem = parent::mMemoryPool->GetMem(parent::mHeader->GetNodeItemSize());
	//tNodeIndex* currentPath = (tNodeIndex*)parent::mMemoryPool->GetMem((parent::mHeader->GetHeight() + 2) * sizeof(tNodeIndex)); // the number of nodes + 1 - the height may be increased
	//int* orderCurrentPath = (int*)parent::mMemoryPool->GetMem((parent::mHeader->GetHeight() + 2) * sizeof(int));
	//bool leaf, ret = true;
	//int state = 0;                // 0 ... go down, 1 ... go up
	//int retInsert, chld, order;
	//TNode *currentNode = NULL;
	//TLeafNode *currentLeafNode = NULL;
	////mDebug = true;

	//if (mDebug && parent::mSharedCache->CheckLocks())
	//{
	//	printf("Insert:\n");
	//	parent::mSharedCache->PrintLocks();
	//}

	//for ( ; ; )
	//{
	//	if (state == 0)          // go down
	//	{
	//		leaf = TNode::IsLeaf(nodeIndex);
	//		if (leaf)
	//		{
	//			currentLeafNode = ReadLeafNodeW(nodeIndex);
	//			if (mDebug) { currentLeafNode->Print(); }
	//		}
	//		else if (nodeIndex == parent::mHeader->GetRootIndex() && parent::mHeader->GetHeight()==0)
	//		{
	//			leaf = true;
	//			currentLeafNode = ReadLeafNodeW(nodeIndex);
	//			if (mDebug) 
	//			{ 
	//				currentLeafNode->Print(); 
	//			}
	//		}
	//		else
	//		{
	//			currentNode = parent::ReadInnerNodeR(nodeIndex);
	//			// old: mTreePool->SetNodeIndex(nodeIndex, counter++);
	//			currentPath[counter++] = nodeIndex;

	//			if (mDebug) { currentNode->Print(); }
	//		}

	//		if (leaf)
	//		{
	//			//if (mDebug)
	//			//{
	//			//	item.Print("\n", GetKeyDescriptor());
	//			//	currentLeafNode->Print();
	//			//}

	//			if ((currentLeafNode->GetItemCount() + 1) == currentLeafNode->GetNodeHeader()->GetNodeCapacity() && currentLeafNode->GetCurrentCacheRow() < (parent::mSharedCache->GetCacheRows() - 1))
	//			{
	//				ChangeLeafNodePossitionInTheCache(&currentLeafNode);
	//			}

	//			if (currentLeafNode->HasFreeSpace(item, data))
	//			{
	//				// If node contains enough space
	//				retInsert = currentLeafNode->InsertInLeafNode(item, data,  parent::mHeader->DuplicatesAllowed());

	//				if (retInsert == TNode::INSERT_EXIST)
	//				{
	//					ret = false; 
	//					unsigned int itemOrder = currentLeafNode->FindOrder(item, TNode::FIND_SBE);
	//					assert(itemOrder != TNode::FIND_NOTEXIST && !parent::mHeader->DuplicatesAllowed());

	//					// increment data, it means, suppose the 4 first bytes as unsigned int, and copy it into the data parameter
	//					unsigned int value = cUInt::GetValue(currentLeafNode->GetData(itemOrder)) + 1;  // read the first int and increment
	//					cUInt::SetValue(currentLeafNode->GetData(itemOrder), value);  // write the first int
	//					// copy the data
	//					cNodeItem::Copy(data, currentLeafNode->GetData(itemOrder), parent::mHeader->GetLeafDataSize());

	//					parent::mSharedCache->UnlockW(currentLeafNode);
	//					break;
	//				}
	//				else if (retInsert == TNode::INSERT_AT_THE_END)
	//				{
	//					if (parent::mHeader->GetHeight() != 0)
	//					{
	//						currentLeafNode->CopyKeyTo(firstItem, currentLeafNode->GetItemCount()-1);
	//						parent::mSharedCache->UnlockW(currentLeafNode);

	//						state = 2;
	//					} else
	//					{
	//						parent::mSharedCache->UnlockW(currentLeafNode);
	//						break;
	//					}
	//				} else
	//				{
	//					parent::mSharedCache->UnlockW(currentLeafNode);
	//					break;	
	//				}
	//			} else
	//			{
	//				// Split and insert
	//				SplitLeafNode(currentLeafNode, firstItem, secondItem, insNodeIndex);
	//				ChangeLeafNodePossitionInTheCache(&currentLeafNode);

	//				if (item.Compare(firstItem, currentLeafNode->GetNodeHeader()->GetKeyDescriptor()) <= 0)
	//				{
	//					// insert into first node
	//					retInsert = currentLeafNode->InsertInLeafNode(item, data, parent::mHeader->DuplicatesAllowed());
	//				} else
	//				{
	//					// insert into second node
	//					parent::mSharedCache->UnlockW(currentLeafNode);
	//					currentLeafNode = ReadLeafNodeW(insNodeIndex);
	//					retInsert = currentLeafNode->InsertInLeafNode(item, data, parent::mHeader->DuplicatesAllowed());
	//					if (mDebug)
	//					{
	//						item.Print("\n", GetKeyDescriptor());
	//						currentLeafNode->Print();
	//					}
	//					if (retInsert == TNode::INSERT_AT_THE_END)
	//					{
	//						// update secondItem
	//						currentLeafNode->CopyKeyTo(secondItem, currentLeafNode->GetItemCount()-1);
	//						retInsert = TNode::INSERT_YES;
	//					}
	//				}

	//				if (retInsert == TNode::INSERT_EXIST)
	//				{
	//					unsigned int itemOrder = currentLeafNode->FindOrder(item, TNode::FIND_SBE);
	//					assert(itemOrder != TNode::FIND_NOTEXIST && !parent::mHeader->DuplicatesAllowed());
	//					//((unsigned int)*(currentLeafNode->GetData(itemOrder)))++;
	//					*((unsigned int*)currentLeafNode->GetData(itemOrder)) = *((unsigned int*)currentLeafNode->GetData(itemOrder)) + 1;


	//					ret = false; 
	//					//parent::mSharedCache->UnlockW(currentLeafNode);
	//					//break;
	//				}
	//				parent::mSharedCache->UnlockW(currentLeafNode);

	//				if (nodeIndex == parent::mHeader->GetRootIndex())
	//				{
	//					CreateRootNode(TLeafNode::GetLeafNodeIndex(parent::mHeader->GetRootIndex()), firstItem, insNodeIndex, secondItem);
	//					break;
	//				}
	//				state = 1;
	//			}
	//			assert(retInsert != TLeafNode::INSERT_NOSPACE);

	//			previousNodeIndex = TNode::GetLeafNodeIndex(currentLeafNode->GetIndex());
	//			parentNodeIndex = currentLeafNode->GetExtraLink(2);
	//		}
	//		else // Inner node; go down
	//		{
	//			if ((chld = currentNode->FindOrderInsert(item, TNode::FIND_SBE)) != TNode::FIND_NOTEXIST)
	//			{
	//				if (parent::mHeader->DuplicatesAllowed())
	//				{
	//					while (chld >= 0 && 
	//						item.Compare(currentNode->GetCKey(chld), GetKeyDescriptor()) >= 0)
	//					{
	//						if (chld-- == 0) {break;}
	//					}
	//					chld++;
	//				}
	//				orderCurrentPath[counter-1] = chld;
	//				nodeIndex = currentNode->GetLink(chld);
	//				parent::mSharedCache->UnlockR(currentNode);
	//			}
	//			else
	//			{
	//				chld = currentNode->GetItemCount() - 1;
	//				orderCurrentPath[counter-1] = chld;
	//				nodeIndex = currentNode->GetLink(chld);
	//				parent::mSharedCache->UnlockR(currentNode);
	//			}
	//		}

	//	} else  if (state == 1)// Inner node; go up
	//	{
	//		currentNode = SearchForRightOrderInInnerNode(parentNodeIndex, previousNodeIndex, counter, order, currentPath, orderCurrentPath);
	//		if (currentNode->HasFreeSpace(secondItem)) // is in the node enough space?
	//		{
	//			// normal insert into inner node (there is enough space)
	//		
	//			if ((currentNode->GetItemCount() + 1) == currentNode->GetNodeHeader()->GetNodeCapacity() && currentNode->GetCurrentCacheRow() < (parent::mSharedCache->GetCacheRows() - 1))
	//			{
	//				ChangeInnerNodePossitionInTheCache(&currentNode);
	//			}
	//		
	//			currentNode->SetKey(order, firstItem);
	//			currentNode->Insert(order + 1, secondItem, NULL, insNodeIndex);

	//			if (order + 2 == currentNode->GetItemCount())
	//			{
	//				if (counter != 0)
	//				{
	//					currentNode->CopyKeyTo(firstItem, currentNode->GetItemCount()-1);
	//					//memcpy(firstItem, currentNode->GetLastItem(), parent::mHeader->GetNodeHeader(cTreeHeader::HEADER_NODE)->GetItemSize());
	//					parent::mSharedCache->UnlockW(currentNode);
	//					state = 2;
	//				} else
	//				{
	//					parent::mSharedCache->UnlockW(currentNode);
	//					break;
	//				}
	//			} else {
	//				// write if don't spTKey
	//				parent::mSharedCache->UnlockW(currentNode);
	//				break;
	//			}

	//		} else
	//		{
	//			// split the inner node first and then process insert
	//			TNode* newNode = parent::ReadNewInnerNode(parent::mSharedCache->GetNodeHeaderForRow(currentNode->GetCurrentCacheRow(), currentNode->GetHeaderId())->GetNodeInMemSize());

	//			char* TNode_mem = parent::mMemoryPool->GetMem(parent::mHeader->GetNodeInMemSize());
	//			TNode tmpNode(newNode, TNode_mem);

	//			currentNode->SetKey(order, firstItem);
	//			currentNode->Split(*newNode, tmpNode);

	//			parent::mHeader->IncrementInnerNodeCount();
	//			if (item.Compare(currentNode->GetCKey(currentNode->GetItemCount()-1), GetKeyDescriptor()) <= 0)
	//			{
	//				retInsert = currentNode->Insert(secondItem, insNodeIndex, parent::mHeader->DuplicatesAllowed());
	//			} else
	//			{
	//				retInsert = newNode->Insert(secondItem, insNodeIndex, parent::mHeader->DuplicatesAllowed());
	//			}


	//			ChangeInnerNodePossitionInTheCache(&newNode); 
	//			ChangeInnerNodePossitionInTheCache(&currentNode);

	//			// delete the temporary node
	//			tmpNode.SetData(NULL);
	//			parent::mMemoryPool->FreeMem(TNode_mem);

	//			//memcpy(firstItem, currentNode->GetLastItem(), parent::mHeader->GetNodeHeader(cTreeHeader::HEADER_NODE)->GetItemSize());
	//			//memcpy(secondItem, newNode->GetLastItem(), parent::mHeader->GetNodeHeader(cTreeHeader::HEADER_NODE)->GetItemSize());
	//			currentNode->CopyKeyTo(firstItem, currentNode->GetItemCount()-1);
	//			newNode->CopyKeyTo(secondItem, newNode->GetItemCount()-1);
	//			SetParentLinkToEveryChild(newNode);
	//			insNodeIndex = newNode->GetIndex();

	//			parent::mSharedCache->UnlockW(currentNode);
	//			parent::mSharedCache->UnlockW(newNode);

	//			//if (currentNode->GetIndex() == 3 && parent::mHeader->GetNodeCount() >= 36)
	//			//{
	//			//	currentNode->Print(0);
	//			//	newNode->Print(0);
	//			//	TKey::Print(firstItem, "\n");
	//			//	TKey::Print(secondItem, "\n");
	//			//	//mDebug = true;
	//			//}

	//			/*** it is necessary create new root node? ***/
	//			if (counter == 0)
	//			{
	//				CreateRootNode(parent::mHeader->GetRootIndex(), firstItem, insNodeIndex, secondItem);
	//				break;
	//			}
	//			state = 1;
	//		}

	//		previousNodeIndex = currentNode->GetIndex();
	//		parentNodeIndex = currentNode->GetExtraLink(0);
	//	} 
	//	else if (state == 2) // modify the key in the inner node
	//	{			
	//		currentNode = SearchForRightOrderInInnerNode(parentNodeIndex, previousNodeIndex, counter, order, currentPath, orderCurrentPath);
	//		currentNode->SetKey(order, firstItem);

	//		if (counter != 0 && (unsigned int)order + 1 == currentNode->GetItemCount())
	//		{
	//			previousNodeIndex = currentNode->GetIndex();
	//			parentNodeIndex = currentNode->GetExtraLink(0);
	//			currentNode->CopyKeyTo(firstItem, currentNode->GetItemCount()-1);
	//			//memcpy(firstItem, currentNode->GetLastItem(), parent::mHeader->GetNodeHeader(cTreeHeader::HEADER_NODE)->GetItemSize());
	//			parent::mSharedCache->UnlockW(currentNode);
	//		} else
	//		{
	//			parent::mSharedCache->UnlockW(currentNode);
	//			break;
	//		}
	//	} else
	//	{
	//		printf("BpTree::Insert - unknown state in B-tree insert algorithm! Damaged heap?\n");
	//	}
	//}

	//// free temporary memory
	//parent::mMemoryPool->FreeMem(firstItem);
	//parent::mMemoryPool->FreeMem(secondItem);
	//parent::mMemoryPool->FreeMem((char*)currentPath);
	//parent::mMemoryPool->FreeMem((char*)orderCurrentPath);

	////if (parent::mHeader->GetNodeCount() >= 410)
	////{
	////	currentNode = parent::ReadInnerNodeR(0x19A);
	////	if (!currentNode->CheckNode())
	////	{
	////		currentNode->Print();
	////	}
	////	parent::mSharedCache->UnlockW(currentNode);

	////	//currentLeafNode = parent::ReadLeafNodeR(36);
	////	//currentLeafNode->Print();
	////	//parent::mSharedCache->UnlockW(currentLeafNode);
	////}

	//if (mDebug && parent::mSharedCache->CheckLocks())
	//{
	//	printf("Insert:\n");
	//	parent::mSharedCache->PrintLocks();
	//}

	//return ret;

}

template <class TNode, class TLeafNode, class TKey> 
void cCommonUBTree<TNode, TLeafNode, TKey>::GetMaxKeyValue(TKey** key)
{
	tNodeIndex nodeIndex = parent::mHeader->GetRootIndex();
	bool leaf;
	TNode *currentNode = NULL;
	TLeafNode *currentLeafNode = NULL;

	if ((leaf = TNode::IsLeaf(nodeIndex)) == true)
	{
		currentLeafNode = parent::ReadLeafNodeR(nodeIndex);
		//if (mDebug) {currentLeafNode->Print(cObject::MODE_BIN);}
	}
	else if (nodeIndex == parent::mHeader->GetRootIndex() && parent::mHeader->GetHeight()==0)
	{
		leaf = true;
		currentLeafNode = parent::ReadLeafNodeR(nodeIndex);
	}
	else
	{
		currentNode = parent::ReadInnerNodeR(nodeIndex);
		//if (mDebug) {currentNode->Print(cObject::MODE_BIN);}
	}

	if (leaf)
	{
		if (currentLeafNode->GetItemCount() == 0)
		{
			*key = NULL;
		} else
		{
			*key = (unsigned int*)&(currentLeafNode->GetLastItem()->GetKey());
		}
	} else
	{
		*key = (unsigned int*)&(currentNode->GetLastItem()->GetKey());
	}
}

/**
* Range query on the btree. Search for keys which lie within interval <il, ih>.
* \param il Lower key of the interval.
* \param ih Higher key of the interval.
* \param finishResultSize If the size of the result is reached, the range query is finished
* \param resultSet The result set you can pass through
* \param pData In some cases (point query without duplicated keys) you can not utilize the result set, data found are copied into this parameter
* \param findf When pData is used, findf detects if there is/is not any result.
* \return Result set with (key,data) pairs
*/


template <class TNode, class TLeafNode, class TKey> 
bool cCommonUBTree<TNode, TLeafNode, TKey>::GetNode(unsigned int level, float alpha, TNode &node)
{
	tNodeIndex nodeIndex = parent::mHeader->GetRootIndex();
	unsigned int levelcount = 0;
	bool ret = true;

	for ( ; ; )
	{
		levelcount++;
		node = *parent::ReadInnerNodeR(nodeIndex);
		if (node.IsLeaf())
		{
			ret = false;
			break;
		}
		else 
		{
			for (unsigned int i = 0 ; i < node.GetItemCount() ; i++)
			{
				if (alpha <= node.GetItem(i)->GetItem()) 
				{
					nodeIndex = node.GetLink(i);
					break;
				}
			}
		}
		if (levelcount == level)
			break;
	}

	return ret;
}

template <class TNode, class TLeafNode, class TKey> 
TKey* cCommonUBTree<TNode, TLeafNode, TKey>::GetMaxKeyValue()
{
	TKey* key; 
	GetMaxKeyValue(&key); 
	return key; 
}

/**
* Method read the inner node and control that highest item of every child is equal to the key of inner node.
*/
template <class TNode, class TLeafNode, class TKey> 
void cCommonUBTree<TNode, TLeafNode, TKey>::CheckInnerNode(tNodeIndex index)
{
	TNode *currentNode = parent::ReadInnerNodeR(index);
	TLeafNode *currentLeafNode = NULL;

	for (unsigned int i = 0; i < currentNode->GetItemCount(); i++)
	{
		currentLeafNode = parent::ReadLeafNodeR(currentNode->GetLink(i));
		if (TKey::Equal(currentLeafNode->GetLastItem(), currentNode->GetCItem(i)) != 0)
		{
			currentNode->Print(0);
			currentLeafNode->Print(0);
		}
		parent::mSharedCache->UnlockR(currentLeafNode);
	}
	parent::mSharedCache->UnlockR(currentNode);
}

/**
 * Method, which print contents of B+-tree.
 **/
template <class TNode, class TLeafNode, class TKey> 
void cCommonUBTree<TNode, TLeafNode, TKey>::Print(int mode)
{
	bool leaf, flag = true;
	int indW, indR = indW = 0;  // references into buffer
	tNodeIndex nodeIndex, * indexes = new tNodeIndex[100000];  // !!! 
	int level = 1;              // level of tree
	int numA, numT = 1;
	unsigned int count;
	char string[30];
	TNode *currentNode = NULL;
	TLeafNode *currentLeafNode = NULL;	
	char * ptr_s;
	cNodeBuffers<TKey> nodeBuffers;

	RangeQuery_pre(&nodeBuffers);

	if (mode == cObject::MODE_DEC) 
	{
		ptr_s = "%u";
	} else 
	{
		ptr_s = "0x%08x";
	}

	indexes[indW++] = parent::mHeader->GetRootIndex();

	printf("\nContent of B+-tree:\n");
	printf("***********************************************************");

	while (flag)
	{
		printf("\nLevel: %d:\n", level++);
		printf("-----------------------------------------------------------\n");
		numA = numT;
		numT = 0;

		for (int j = 0 ; j < numA ; j++) 
		{
			printf("|| ");
			printf(ptr_s, indexes[indR]);
			printf(" ||");
			if (indexes[indR] == 1)
			{
				printf("");
			}

			nodeIndex = indexes[indR++];

			if (TNode::IsLeaf(nodeIndex) == true || 
				(nodeIndex == parent::mHeader->GetRootIndex() && (parent::mHeader->GetHeight()==0)))
			{
				leaf = true;
				currentLeafNode = parent::ReadLeafNodeR(nodeIndex); // read node from into currentLeafNode
				printf(" (leaf/%s) ||", (currentLeafNode->IsLeaf()?"leaf":"inner"));

				if (flag)
				{
					flag = false;
				}
			}
			else
			{
				leaf = false;
				currentNode = parent::ReadInnerNodeR(nodeIndex);
				printf(" (inner/%s) ||", (currentNode->IsLeaf()?"leaf":"inner"));
				count = currentNode->GetItemCount();
				numT += count;
			}


			if (leaf)
			{
				currentLeafNode->Print();
				parent::mSharedCache->UnlockR(currentLeafNode);
			}
			else
			{
				currentNode->Print();
				for (unsigned int i = 0; i < count ; i++)
				{
					indexes[indW++] = currentNode->GetLink(i);
				}
				parent::mSharedCache->UnlockR(currentNode);
			}
		}
	}

	RangeQuery_post(&nodeBuffers);

	printf("***********************************************************\n");
	delete []indexes;
}

template <class TNode, class TLeafNode, class TKey>
void cCommonUBTree<TNode, TLeafNode, TKey>::PrintDimDistribution()
{
	cUBTreeHeader<TKey> * header = (cUBTreeHeader<TKey>*)parent::mHeader;

	char* minValues = header->GetTreeMBR()->GetLoTuple()->GetData();
	char* maxValues = header->GetTreeMBR()->GetHiTuple()->GetData();
	parent::PrintDimDistribution(minValues, maxValues);
}

template <class TNode, class TLeafNode, class TKey> 
void cCommonUBTree<TNode, TLeafNode, TKey>::PrintInfo()
{
	cPagedTree<TKey,TNode,TLeafNode>::PrintInfo();
	if (mInMemCacheEnabled)
	{
		mInMemCache->PrintInfo();
	}
}

template <class TNode, class TLeafNode, class TKey> 
void cCommonUBTree<TNode, TLeafNode, TKey>::SetDebug(bool value) { mDebug = value; };

template <class TNode, class TLeafNode, class TKey> 
void cCommonUBTree<TNode, TLeafNode, TKey>
	::PrintNode(unsigned int index)
{
	TNode *currentNode = NULL;
	TLeafNode *currentLeafNode = NULL;

	if (TNode::IsLeaf(index) == true)
	{
		currentLeafNode = parent::ReadLeafNodeR(index);
		currentLeafNode->Print(cObject::MODE_DEC);
		parent::mSharedCache->UnlockR(currentLeafNode);
	}
	else if (index == parent::mHeader->GetRootIndex() && (parent::mHeader->GetHeight()==0))
	{
		currentLeafNode = parent::ReadLeafNodeR(index);
		currentLeafNode->Print(cObject::MODE_DEC);
		parent::mSharedCache->UnlockR(currentLeafNode);
	}
	else
	{
		currentNode = parent::ReadInnerNodeR(index);
		currentNode->Print(cObject::MODE_DEC);
		parent::mSharedCache->UnlockR(currentNode);
	}

}
}}}
#endif

