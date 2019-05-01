/*
CARTOTYPE_TREE.H
Copyright (C) 2012-2017 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_TREE_H__
#define CARTOTYPE_TREE_H__

#include <cartotype_base.h>
#include <cartotype_errors.h>

namespace CartoType
{

/**
A templated binary tree class for pointers to objects of type T,
which must have members iLeft, iRight and iParent, all pointers to T,
and must have a key function Key() returning type K,
and a static CompareKeys function returning negative, zero and positive results
according to the order of the comparison.

CPointerTree owns the objects in it if aOwnData is true on construction.
*/
template<class T,class K> class CPointerTree
    {
    public:
    CPointerTree(bool aOwnData):
        iOwnData(aOwnData)
        {
        }

    ~CPointerTree()
        {
        if (!iOwnData)
            return;

        DeleteAll(iRoot);
        }

    void Clear()
        {
        if (iOwnData)
            DeleteAll(iRoot);
        iRoot = nullptr;
        iCount = 0;
        }

    size_t Count() const
        {
        return iCount;
        }

    /**
    Insert a new item into the tree. If aForceInsert is true,
    insert the item even if its key is a duplicate of an existing key.
    Return the item inserted, or found if aForceInsert is false and the new item
    is a duplicate.
    */
    T* Insert(T* aItem,bool aForceInsert = true)
        {
        if (iRoot == nullptr)
            {
            assert(iCount == 0);
            iRoot = aItem;
            aItem->iLeft = aItem->iRight = aItem->iParent = nullptr;
            iCount = 1;
            return aItem;
            }

        T* parent_node = nullptr;
        T* cur_node = iRoot;
        K key = aItem->Key();
        for (;;)
            {
            parent_node = cur_node;
            int order = T::CompareKeys(key,cur_node->Key());
            if (order < 0)
                {
                cur_node = cur_node->iLeft;
                if (cur_node == nullptr)
                    {
                    parent_node->iLeft = aItem;
                    aItem->iParent = parent_node;
                    aItem->iLeft = aItem->iRight = nullptr;
                    iCount++;
                    return aItem;
                    }
                }
            else
                {
                if (order == 0 && !aForceInsert)
                    return cur_node;

                cur_node = cur_node->iRight;
                if (cur_node == nullptr)
                    {
                    parent_node->iRight = aItem;
                    aItem->iParent = parent_node;
                    aItem->iLeft = aItem->iRight = nullptr;
                    iCount++;
                    return aItem;
                    }
                }
            }
        }

    T* Find(K aKey) const
        {
        T* cur_node = iRoot;
        while (cur_node != nullptr)
            {
            int order = T::CompareKeys(aKey,cur_node->Key());
            if (order == 0)
                break;
            if (order < 0)
                cur_node = cur_node->iLeft;
            else
                cur_node = cur_node->iRight;
            }
        return cur_node;
        }

    T* FindGreaterThanOrEqual(K aKey) const
        {
        T* cur_node = iRoot;
        while (cur_node != nullptr)
            {
            int order = T::CompareKeys(aKey,cur_node->Key());
            if (order == 0)
                break;
            if (order < 0)
                {
                if (cur_node->iLeft == nullptr)
                    return cur_node;
                cur_node = cur_node->iLeft;
                }
            else
                {
                if (cur_node->iRight == nullptr)
                    return Next(cur_node);
                cur_node = cur_node->iRight;
                }
            }
        return cur_node;
        }

    T* Min()
        {
        if (!iRoot)
            return nullptr;
        return Min(iRoot);
        }

    const T* Min() const
        {
        if (!iRoot)
            return nullptr;
        return Min(iRoot);
        }

    T* Max()
        {
        if (!iRoot)
            return nullptr;
        return Max(iRoot);
        }

    const T* Max() const
        {
        if (!iRoot)
            return nullptr;
        return Max(iRoot);
        }

    void Delete(K aKey)
        {
        T* z = Find(aKey);
        if (!z)
            return;
        Delete(z);
        }

    void RemoveNodeWithZeroOrOneChild(T* aNode)
        {
        assert(aNode->iLeft == nullptr || aNode->iRight == nullptr);
        T* child = aNode->iLeft == nullptr ? aNode->iRight : aNode->iLeft;
        if (aNode->iParent)
            {
            if (aNode->iParent->iLeft == aNode)
                aNode->iParent->iLeft = child;
            else
                aNode->iParent->iRight = child;
            }
        else
            {
            assert(aNode == iRoot);
            iRoot = child;
            }

        if (child)
            child->iParent = aNode->iParent;
        }

    void Delete(T* aNode,bool aTakeOwnership = false)
        {
        if (!aNode)
            return;

        // No children or one child.
        if (aNode->iLeft == nullptr || aNode->iRight == nullptr)
            RemoveNodeWithZeroOrOneChild(aNode);

        // Two children.
        else
            {
            T* next = Next(aNode);
            RemoveNodeWithZeroOrOneChild(next);
            if (aNode->iParent)
                {
                if (aNode->iParent->iLeft == aNode)
                    aNode->iParent->iLeft = next;
                else
                    aNode->iParent->iRight = next;
                }
            next->iParent = aNode->iParent;
            next->iLeft = aNode->iLeft;
            if (next->iLeft)
                next->iLeft->iParent = next;
            next->iRight = aNode->iRight;
            if (next->iRight)
                next->iRight->iParent = next;
            if (next->iParent == nullptr)
                iRoot = next;
            }

        if (iOwnData && !aTakeOwnership)
            delete aNode;
        else
            aNode->iLeft = aNode->iRight = aNode->iParent = nullptr; // a safeguard against deleting already-deleted items

        assert(iCount > 0);
        iCount--;
        }

    static T* Prev(T* aNode)
        {
        if (aNode->iLeft)
            return Max(aNode->iLeft);
        T* x = aNode;
        T* y = aNode->iParent;
        while (y && x == y->iLeft)
            {
            x = y;
            y = y->iParent;
            }
        return y;
        }

    static T* Next(T* aNode)
        {
        if (aNode->iRight)
            return Min(aNode->iRight);
        T* x = aNode;
        T* y = aNode->iParent;
        while (y && x == y->iRight)
            {
            x = y;
            y = y->iParent;
            }
        return y;
        }

    static const T* Next(const T* aNode)
        {
        if (aNode->iRight)
            return Min(aNode->iRight);
        const T* x = aNode;
        const T* y = aNode->iParent;
        while (y && x == y->iRight)
            {
            x = y;
            y = y->iParent;
            }
        return y;
        }

    private:
    static void DeleteAll(T* aRoot)
        {
        if (aRoot)
            {
            DeleteAll(aRoot->iLeft);
            DeleteAll(aRoot->iRight);
            delete aRoot;
            }
        }

    static T* Min(T* aNode)
        {
        T* cur_node = aNode;
        while (cur_node && cur_node->iLeft)
            cur_node = cur_node->iLeft;
        return cur_node;
        }

    static T* Max(T* aNode)
        {
        T* cur_node = aNode;
        while (cur_node && cur_node->iRight)
            cur_node = cur_node->iRight;
        return cur_node;
        }

    T* iRoot = nullptr;
    bool iOwnData;
    size_t iCount = 0;
    };


/*
Neither CPriorityTree nor CPriorityQueue is used for route calculation because benchmarks showed them to be slower
than CPointerTree on long routes.

The test was of 10 iterations of a route from -3E,51N (near Taunton, Somerset) to -4,58N (near Golspie, Sutherland),
which is 622 miles.

Results: CPointerTree: 33.77s
         CPriorityTree: 37.29s
         CPriorityQueue: 40.542s
*/

/**
A priority queue implemented as an intrusive binary tree.
Objects of type T must have members iLeft, iRight and iParent, all pointers to T,
and a function Key() returning a priority of type K.
*/
template<class T,class K> class CPriorityTree
    {
    public:
    void Clear()
        {
        iRoot = nullptr;
        iCount = 0;
        iTopBit = 0;
        }

    void Insert(T& aItem)
        {
        aItem.iLeft = aItem.iRight = aItem.iParent = nullptr;
        iCount++;
        if (!(iTopBit & iCount))
            iTopBit = iCount;
        if (!iRoot)
            {
            iRoot = &aItem;
            return;
            }
        
        T* cur_parent = iRoot;
        size_t bit = iTopBit >> 1;
        while (bit > 1)
            {
            if (bit & iCount)
                cur_parent = cur_parent->iRight;
            else
                cur_parent = cur_parent->iLeft;
            bit >>= 1;
            }
        if (bit & iCount)
            cur_parent->iRight = &aItem;
        else
            cur_parent->iLeft = &aItem;
        aItem.iParent = cur_parent;
        SiftUp(&aItem);
        }

    T* ExtractMin()
        {
        if (!iRoot)
            return nullptr;
        if (iCount == 1)
            {
            T* min_item = iRoot;
            iRoot = nullptr;
            iCount = 0;
            iTopBit = 0;
            return min_item;
            }

        // move the void down to the last row
        T* min_item = iRoot;
        T* p = iRoot;
        while (p->iLeft)
            {
            if (p->iRight && p->iRight->Key() < p->iLeft->Key())
                SwapWithParent(p->iRight);
            else
                SwapWithParent(p->iLeft);
            }

        // find the last item
        T** last_item_parent_ptr = &iRoot;
        T* last_item = iRoot;
        size_t bit = iTopBit >> 1;
        while (bit)
            {
            if (bit & iCount)
                last_item_parent_ptr = &last_item->iRight;
            else
                last_item_parent_ptr = &last_item->iLeft;
            last_item = *last_item_parent_ptr;
            bit >>= 1;
            }

        iCount--;
        if (!(iTopBit & iCount))
            iTopBit >>= 1;

        // swap the void with the last item
        if (p != last_item)
            Swap(p,last_item);
        *last_item_parent_ptr = nullptr;

        // correct the position of the last item
        if (p != last_item)
            SiftUp(last_item);

        return min_item;
        }

    void DecreaseKey(T& aItem)
        {
        // this function assumes that aItem's key has in fact been decreased
        SiftUp(&aItem);
        }

    private:
    void Swap(T* aP,T* aQ)
        {
        T* new_p_parent = aQ->iParent;
        T* new_p_left = aQ->iLeft;
        T* new_p_right = aQ->iRight;
        T** new_p_link = &iRoot;
        if (aQ->iParent)
            new_p_link = aQ->iParent->iLeft == aQ ? &aQ->iParent->iLeft : &aQ->iParent->iRight;
 
        T* new_q_parent = aP->iParent;
        T* new_q_left = aP->iLeft;
        T* new_q_right = aP->iRight;
        T** new_q_link = &iRoot;
        if (aP->iParent)
            new_q_link = aP->iParent->iLeft == aP ? &aP->iParent->iLeft : &aP->iParent->iRight;

        if (aP->iParent == aQ)
            {
            new_q_parent = aP;
            new_q_link = nullptr;
            if (aQ->iLeft == aP)
                new_p_left = aQ;
            else
                new_p_right = aQ;
            }
        else if (aQ->iParent == aP)
            {
            new_p_parent = aQ;
            new_p_link = nullptr;
            if (aP->iLeft == aQ)
                new_q_left = aP;
            else
                new_q_right = aP;
            }

        aP->iParent = new_p_parent;
        aP->iLeft = new_p_left;
        if (aP->iLeft)
            aP->iLeft->iParent = aP;
        aP->iRight = new_p_right;
        if (aP->iRight)
            aP->iRight->iParent = aP;
        if (new_p_link)
            *new_p_link = aP;

        aQ->iParent = new_q_parent;
        aQ->iLeft = new_q_left;
        if (aQ->iLeft)
            aQ->iLeft->iParent = aQ;
        aQ->iRight = new_q_right;
        if (aQ->iRight)
            aQ->iRight->iParent = aQ;
        if (new_q_link)
            *new_q_link = aQ;
        }

    void SwapWithParent(T* aQ)
        {
        T* p = aQ->iParent;
        T* new_q_parent = p->iParent;
        T** new_q_link;
        if (p->iParent)
            new_q_link = p->iParent->iLeft == p ? &p->iParent->iLeft : &p->iParent->iRight;
        else
            new_q_link = &iRoot;

        T* new_q_left;
        T* new_q_right;
        if (p->iLeft == aQ)
            {
            new_q_left = p;
            new_q_right = p->iRight;
            }
        else
            {
            new_q_left = p->iLeft;
            new_q_right = p;
            }

        p->iParent = aQ;
        p->iLeft = aQ->iLeft;
        if (p->iLeft)
            p->iLeft->iParent = p;
        p->iRight = aQ->iRight;
        if (p->iRight)
            p->iRight->iParent = p;

        aQ->iParent = new_q_parent;
        aQ->iLeft = new_q_left;
        if (aQ->iLeft)
            aQ->iLeft->iParent = aQ;
        aQ->iRight = new_q_right;
        if (aQ->iRight)
            aQ->iRight->iParent = aQ;
        if (new_q_link)
            *new_q_link = aQ;
        }

    void SiftUp(T* aItem)
        {
        while (aItem->iParent && aItem->Key() < aItem->iParent->Key())
            SwapWithParent(aItem);
        }

    T* iRoot = nullptr;
    size_t iCount = 0;
    size_t iTopBit = 0;
    };

/**
A priority queue implemented as a binary heap.
Objects of type T must have a function Key() returning a priority of type K,
and a member iQueueIndex of type size_t.

You need to supply an array of pointers to T.

Note: this is not used for route calculation because benchmarks showed it to be slower
than using CPointerTree on long routes.
*/
template<class T,class K> class CPriorityQueue
    {
    public:
    CPriorityQueue(T** aArray,size_t aArraySize):
        iArray(aArray),
        iArraySize(aArraySize)
        {
        }

    void Clear()
        {
        iCount = 0;
        }

    void Insert(T& aItem)
        {
        if (iCount >= iArraySize)
            {
            assert(false);
            return;
            }

        iArray[iCount] = &aItem;
        aItem.iQueueIndex = iCount;
        SiftUp(iCount++);
        }

    T* ExtractMin()
        {
        if (!iCount)
            return nullptr;
        T* min_item = iArray[0];
        if (iCount > 1)
            {
            size_t i = 0;
            for (;;)
                {
                size_t c = i + i + 1;
                if (c >= iCount)
                    break;
                if (c + 1 < iCount && iArray[c + 1]->Key() < iArray[c]->Key())
                    c++;
                iArray[i] = iArray[c];
                iArray[i]->iQueueIndex = i;
                i = c;
                }
            if (i < iCount - 1)
                {
                iArray[i] = iArray[iCount - 1];
                iArray[i]->iQueueIndex = i;
                SiftUp(i);
                }
            }
        iCount--;
        return min_item;
        }

    void DecreaseKey(T& aItem)
        {
        SiftUp(aItem.iQueueIndex);
        }

    private:
    void SiftUp(size_t aIndex)
        {
        while (aIndex)
            {
            size_t parent = (aIndex - 1) >> 1;
            if (iArray[parent]->Key() <= iArray[aIndex]->Key())
                break;
            T* temp = iArray[aIndex];
            iArray[aIndex] = iArray[parent];
            iArray[aIndex]->iQueueIndex = aIndex;
            iArray[parent] = temp;
            temp->iQueueIndex = parent;
            aIndex = parent;
            }
        }

    T** iArray = nullptr;
    size_t iArraySize = 0;
    size_t iCount = 0;
    };


}

#endif
