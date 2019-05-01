/*
CARTOTYPE_CACHE.H
Copyright (C) 2011-2017 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_CACHE_H__
#define CARTOTYPE_CACHE_H__

#include "cartotype_list.h"

namespace CartoType
{

/**
A cache for objects of type TCached with a key of type TKey.
Objects must have a Key function to return the key,
and keys must be comparable for equality. They must also have a Size function
to return their size in arbitrary units, so that a maximum size can be enforced.
*/
template<class TCached,class TKey> class CCache
    {
    public:
    explicit CCache(size_t aMaxSize):
        iMaxSize(aMaxSize)
        {
        }

    TCached* Find(TKey aKey)
        {
        typename CPointerList<TCached>::TIter iter = iList.First();
        for (;;)
            {
            TCached* p = iter;
            if (!p)
                break;
            if (p->Key() == aKey)
                {
                iter.MoveCurrentToStart();
                return p;
                }
            iter.Next();
            }
        return nullptr;
        };

    void Add(TCached* aItem)
        {
        Trim(); // trim before adding; the rule is that the cache must be able to hold at least one item

        iList.Prefix(aItem);
        assert(aItem->Size() >= 0);
        iSize += aItem->Size();
        }
    
    void Clear()
        {
        iList.Clear();
        iSize = 0;
        }

    void SetMaxSize(int32_t aMaxSize)
        {
        if (aMaxSize < 0)
            aMaxSize = 0;
        iMaxSize = aMaxSize;
        Trim();
        }

    private:
    void Trim()
        {
        while (iSize > iMaxSize)
            {
            typename CPointerList<TCached>::TIter iter = iList.Last();
            TCached* p = iter;
            assert(p->Size() > 0);
            iSize -= p->Size();
            iList.Delete(iter);
            }
        }

    CPointerList<TCached> iList;
    size_t iSize { 0 };
    size_t iMaxSize { 0 };
    };

}

#endif
