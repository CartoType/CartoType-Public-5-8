/*
CARTOTYPE_LIST.H
Copyright (C) 2004-2018 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_LIST_H__
#define CARTOTYPE_LIST_H__

#include <cartotype_types.h>
#include <cartotype_errors.h>

namespace CartoType
{

/** The base class for list items in the CList and CPointerList template classes. */
class TListLink
    {
    public:
    /**
    A pointer to the previous item in the list, or, if this is the first item,
    the last item in the list.
    */
    TListLink* iPrev = nullptr;

    /**
    A pointer to the next item in the list, or, if this is the last item,
    the first item in the list.
    */
    TListLink* iNext = nullptr;
    };

/** The base class for the CList and CPointerList template classes. */
class CListBase
    {
    public:
    CListBase() = default;
    void Link(TListLink* aLink,TListLink* aNext);
    void Insert(TListLink* aLink,TListLink* aNext);
    void Delete(TListLink* aLink);
    /** Move aLink to the start of the list. */
    void MoveToStart(TListLink* aLink)
        {
        if (aLink && aLink != iStart)
            {
            aLink->iPrev->iNext = aLink->iNext;
            aLink->iNext->iPrev = aLink->iPrev;
            aLink->iNext = iStart;
            aLink->iPrev = iStart->iPrev;
            iStart->iPrev->iNext = aLink;
            iStart->iPrev = aLink;
            iStart = aLink;
            }
        }

    /** Move aLink to the position before aNext. */
    void MoveTo(TListLink* aLink,TListLink* aNext)
        {
        assert(aLink && aNext);
        if (aLink != aNext && aLink->iNext != aNext)
            {
            aLink->iPrev->iNext = aLink->iNext;
            aLink->iNext->iPrev = aLink->iPrev;
            aLink->iNext = aNext;
            aLink->iPrev = aNext->iPrev;
            aNext->iPrev->iNext = aLink;
            aNext->iPrev = aLink;
            }
        if (aNext == iStart)
            iStart = aLink;
        }

    /** Return the first item in the list, or null if the list is empty. */
    const TListLink* Start() const { return iStart; }
    /** Return a non-const pointer to the first item in the list, or null if the list is empty. */
    TListLink* Start() { return iStart; }

    protected:
    TListLink* iStart = nullptr;
    int32_t iElements = 0;

    private:
    CListBase(const CListBase&) = delete;
    CListBase(CListBase&&) = delete;
    void operator=(const CListBase&) = delete;
    void operator=(CListBase&&) = delete;
    };

/** A list class designed to hold small objects directly. */
template<class T> class CList: private CListBase
    {
    public:
    ~CList()
        {
        Clear();
        }
    void Clear()
        {
        TLink* p = (TLink*)iStart;
        while (iElements > 0)
            {
            TLink* next = (TLink*)(p->iNext);
            delete p;
            p = next;
            iElements--;
            }
        iStart = nullptr;
        }

    private:
    class TLink: public TListLink
        {
        public:
        TLink(const T& aT): iT(aT) { }
        T iT;
        };

    public:
    /**
    A non-constant iterator for traversing lists.
    The iterator's 'operator TYPE*' function returns a pointer to the iterator's current element,
    or null if the list is empty or the iterator has reached the end or start of the list.
    */
    class TIter
        {
        public:
        TIter(CListBase& aList,TListLink* aLink): iList(&aList), iCur((TLink*)aLink) { }
        operator T*() { return iCur ? &iCur->iT : 0 ; }
        void Next()
            {
            if (iCur)
                {
                iCur = (TLink*)(iCur->iNext);
                if (iCur == iList->Start())
                    iCur = nullptr;
                }
            }
        void Prev()
            {
            if (iCur)
                {
                if (iCur == iList->Start())
                    iCur = nullptr;
                else
                    iCur = (TLink*)(iCur->iPrev);
                }
            }
        TLink* Cur() { return iCur; }
        void MoveCurrentToStart() { iList->MoveToStart(iCur); }
        TResult Insert(const T& aT) { return iList->Insert(new TLink(aT),iCur); }
        void MoveToCurrentPosition(TListLink* aLink) { iList->MoveTo(aLink,iCur); }
        bool AtStart() const { return iCur == iList->Start(); }

        private:
        CListBase* iList;
        TLink* iCur;
        };
    /**
    A constant iterator for traversing lists.
    The iterator's 'operator const TYPE*' function returns a const pointer to the iterator's current element,
    or null if the list is empty or the iterator has reached the end or start of the list.
    */
    class TConstIter
        {
        public:
        TConstIter(const CListBase& aList,const TListLink* aLink): iList(&aList), iCur((TLink*)aLink) { }
        operator const T*() const { return iCur ? &iCur->iT : 0 ; }
        void Next()
            {
            if (iCur)
                {
                iCur = (TLink*)(iCur->iNext);
                if (iCur == iList->Start())
                    iCur = nullptr;
                }
            }
        void Prev()
            {
            if (iCur)
                {
                if (iCur == iList->Start())
                    iCur = nullptr;
                else
                    iCur = (TLink*)(iCur->iPrev);
                }
            }
        bool AtStart() const { return iCur == iList->Start(); }

        private:
        const CListBase* iList;
        const TLink* iCur;
        };

    /** Return the number of elements in the list. */
    int32_t Count() const { return iElements; }
    /** Insert a new element at the start of the list. */
    void Prefix(const T& aT) { CListBase::Insert(new TLink(aT),iStart); }
    /** Insert a new element at the end of the list. */
    void Append(const T& aT) { CListBase::Link(new TLink(aT),iStart); }
    /** Delete the element referred to by the iterator aIter. */
    void Delete(TIter& aIter)
        {
        TListLink* p = aIter.Cur(); assert(p); aIter.Next(); CListBase::Delete(p); delete p;
        }
    /** Return a non-constant iterator referring to the first element of the list. */
    TIter First() { return TIter(*this,iStart); }
    /** Return a non-constant iterator referring to the last element of the list. */
    TIter Last() { return TIter(*this,iStart ? iStart->iPrev : 0); }
    /** Return a constant iterator referring to the first element of the list. */
    TConstIter First() const { return TConstIter(*this,iStart); }
    /** Return a constant iterator referring to the last element of the list. */
    TConstIter Last() const { return TConstIter(*this,iStart ? iStart->iPrev : 0); }
    /** Return a non-constant iterator referring to a specified link. */
    TIter Iter(TListLink* aLink) { return TIter(*this,aLink); }
    /** Return a constant iterator referring to a specified link. */
    TConstIter Iter(const TListLink* aLink) { return TConstIter(*this,aLink); }
    };

/**
A list class for storing large objects via pointers.
The list takes ownership of the objects.
*/
template<class T> class CPointerList: private CListBase
    {
    public:
    CPointerList(bool aOwnData = true): iOwnData(aOwnData) { }
    ~CPointerList()
        {
        Clear();
        }
    void Clear()
        {
        TLink* p = (TLink*)iStart;
        while (iElements > 0)
            {
            TLink* next = (TLink*)(p->iNext);
            if (iOwnData)
                delete (T*)(p->iPtr);
            delete p;
            p = next;
            iElements--;
            }
        iStart = nullptr;
        }

    private:
    class TLink: public TListLink
        {
        public:
        TLink(const T* aPtr): iPtr(aPtr) { }
        const T* iPtr;
        };

    public:
    /**
    A non-constant iterator for traversing lists of pointers.
    There are both const and non-const 'operator TYPE*' functions to return the iterator's current element.
    They return null if the list is empty or the iterator has reached the end or start of the list.
    */
    class TIter
        {
        public:
        TIter(CListBase& aList,TListLink* aLink): iList(&aList), iCur((TLink*)aLink) { }
        operator const T*() { return iCur ? iCur->iPtr : 0 ; }
        operator T*() { return iCur ? (T*)(iCur->iPtr) : 0 ; }
        void Next()
            {
            if (iCur)
                {
                iCur = (TLink*)(iCur->iNext);
                if (iCur == iList->Start())
                    iCur = nullptr;
                }
            }
        void Prev()
            {
            if (iCur)
                {
                if (iCur == iList->Start())
                    iCur = nullptr;
                else
                    iCur = (TLink*)(iCur->iPrev);
                }
            }
        TLink* Cur() { return iCur; }
        void MoveCurrentToStart() { iList->MoveToStart(iCur); }
        TResult Insert(T* aPtr) { return iList->Insert(new TLink(aPtr),iCur); }
        void MoveToCurrentPosition(TListLink* aLink) { iList->MoveTo(aLink,iCur); }
        bool AtStart() const { return iCur == iList->Start(); }

        private:
        CListBase* iList;
        TLink* iCur;
        };
    /**
    A constant iterator for traversing lists of pointers.
    The iterator's 'operator const TYPE*' function returns a const pointer to the iterator's current element,
    or null if the list is empty or the iterator has reached the end or start of the list.
    */
    class TConstIter
        {
        public:
        TConstIter(const CListBase& aList,const TListLink* aLink): iList(&aList), iCur((TLink*)aLink) { }
        operator const T*() const { return iCur ? iCur->iPtr : 0 ; }
        void Next()
            {
            if (iCur)
                {
                iCur = (TLink*)(iCur->iNext);
                if (iCur == iList->Start())
                    iCur = nullptr;
                }
            }
        void Prev()
            {
            if (iCur)
                {
                if (iCur == iList->Start())
                    iCur = nullptr;
                else
                    iCur = (TLink*)(iCur->iPrev);
                }
            }
        bool AtStart() const { return iCur == iList->Start(); }
        private:
        const CListBase* iList;
        const TLink* iCur;
        };

    int32_t Count() const { return iElements; }
    void Prefix(const T* aPtr) { CListBase::Insert(new TLink(aPtr),iStart); }
    void Append(const T* aPtr) { CListBase::Link(new TLink(aPtr),iStart); }
    void Delete(TIter& aIter)
        {
        TLink* p = aIter.Cur();
        assert(p);
        aIter.Next();
        CListBase::Delete(p);
        if (iOwnData)
            delete (T*)(p->iPtr);
        delete p;
        }
    TIter First() { return TIter(*this,iStart); }
    TIter Last() { return TIter(*this,iStart ? iStart->iPrev : 0); }
    TConstIter First() const { return TConstIter(*this,iStart); }
    TConstIter Last() const { return TConstIter(*this,iStart ? iStart->iPrev : 0); }
    TIter Iter(TListLink* aLink) { return TIter(*this,aLink); }
    TConstIter Iter(const TListLink* aLink) { return TConstIter(*this,aLink); }

    private:
    bool iOwnData;
    };

} // namespace CartoType

#endif
