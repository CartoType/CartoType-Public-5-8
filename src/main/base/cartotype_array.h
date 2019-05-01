/*
CARTOTYPE_ARRAY.H
Copyright (C) 2004-2017 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_ARRAY_H__
#define CARTOTYPE_ARRAY_H__

#include <cartotype_types.h>
#include <cartotype_errors.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <array>
#include <memory>

namespace CartoType
{

/** An array that is kept on the stack if it has aMaxStackElements or fewer, otherwise on the heap. */
template<class T,size_t aMaxStackElements> class TStackArray
    {
    public:
    TStackArray(size_t aElements):
        iElements(aElements)
        {
        if (aElements > aMaxStackElements)
            iDataPtr = new T[aElements];
        else
            iDataPtr = iData;
        }
    ~TStackArray()
        {
        if (iElements > aMaxStackElements)
            delete [] iDataPtr;
        }

    /** Return the number of elements. */
    size_t Count() const { return iElements; }
    /** Return a writable pointer to the underlying C++ array of elements. */
    T* Data() { return iDataPtr; }
    /** Return a read-only pointer to the underlying C++ array of elements. */
    const T* Data() const { return (const T*)iDataPtr; }
    /** The writable array index operator, allowing the array to be treated like a C++ array. */
    T& operator[](size_t aIndex) { assert(aIndex < iElements); return iDataPtr[aIndex]; }
    /** The constant array index operator, allowing the array to be treated like a C++ array. */
    const T& operator[](size_t aIndex) const { assert(aIndex < iElements); return iDataPtr[aIndex]; }

    private:
    size_t iElements;
    T* iDataPtr;
    T iData[aMaxStackElements];
    };

/**
A limited-functionality variable-size array for small objects,
optimised for the cases of 0...N elements in the array.
*/
template<class T,size_t N> class CSmallArray
    {
    public:
    CSmallArray() { }
    
    ~CSmallArray()
        {
        if (m_vector_or_count && !(size_t(m_vector_or_count) & 1))
            delete m_vector_or_count;
        }

    CSmallArray(const CSmallArray& aOther)
        {
        if (this != &aOther)
            {
            auto p = aOther.begin();
            auto q = aOther.end();
            while (p < q)
                Append(*p++);
            }
        }
    
    CSmallArray(CSmallArray&& aOther)
        {
        if (this != &aOther)
            {
            m_vector_or_count = aOther.m_vector_or_count;
            for (size_t i = 0; i < N; i++)
                m_value[i] = std::move(aOther.m_value[i]);
            aOther.m_vector_or_count = nullptr;
            }
        }
    
    CSmallArray& operator=(const CSmallArray& aOther)
        {
        if (this != &aOther)
            {
            Clear();
            auto p = aOther.begin();
            auto q = aOther.end();
            while (p < q)
                Append(*p++);
            }
        return *this;
        }
    
    CSmallArray& operator=(CSmallArray&& aOther)
        {
        if (this != &aOther)
            {
            Clear();
            m_vector_or_count = aOther.m_vector_or_count;
            for (size_t i = 0; i < N; i++)
                m_value[i] = std::move(aOther.m_value[i]);
            aOther.m_vector_or_count = nullptr;
            }
        return *this;
        }

    /** Delete all the elements. */
    void clear()
        {
        if (m_vector_or_count && !(size_t(m_vector_or_count) & 1))
            delete m_vector_or_count;
        m_vector_or_count = nullptr;
        }
    /** Delete all the elements. */
    void Clear() { clear(); }

    /** Return the number of elements. */
    size_t size() const
        {
        if (!m_vector_or_count)
            return 0;
        if (size_t(m_vector_or_count) & 1)
            return size_t(m_vector_or_count) >> 1;
        return m_vector_or_count->size();
        }
    /** Return the number of elements. */
    size_t Count() const { return size(); }
    bool empty() const { return m_vector_or_count == nullptr; }
    /** Return a writable pointer to the underlying C++ array of elements. */
    T* Data()
        {
        if (!m_vector_or_count)
            return nullptr;
        if (size_t(m_vector_or_count) & 1)
            return m_value;
        return m_vector_or_count->data();
        }
    /** Return a read-only pointer to the underlying C++ array of elements. */
    const T* Data() const
        {
        if (!m_vector_or_count)
            return nullptr;
        if (size_t(m_vector_or_count) & 1)
            return m_value;
        return m_vector_or_count->data();
        }
    /** The writable array index operator, allowing the array to be treated like a C++ array. */
    T& operator[](size_t aIndex) { assert(aIndex < Count()); return Data()[aIndex]; }
    T& front() { return Data()[0]; }
    T& back() { return Data()[size() - 1]; }
    /** The constant array index operator, allowing the array to be treated like a C++ array. */
    const T& operator[](size_t aIndex) const { assert(aIndex < Count()); return Data()[aIndex]; }
    const T& front() const { return Data()[0]; }
    const T& back() const { return Data()[size() - 1]; }
    /** Append a single element. */
    void push_back(T aElement)
        {
        size_t n = Count();
        if (n < N)
            {
            m_value[n++] = aElement;
            m_vector_or_count = (std::vector<T>*)(n * 2 + 1);
            }
        else
            {
            if (n == N)
                {
                std::unique_ptr<std::vector<T>> v { new std::vector<T> };
                for (size_t i = 0; i < N; i++)
                    v->push_back(std::move(m_value[i]));
                m_vector_or_count = v.release();
                }

            m_vector_or_count->push_back(aElement);
            }
        }
    /** Append a single element. */
    void Append(T aElement) { push_back(aElement); }

    void emplace_back() { push_back(T()); }
    
    /** Delete a single element. */
    void Delete(size_t aIndex)
        {
        size_t n = Count();
        assert(aIndex < n);
        n--;
        if (n < N)
            {
            for (size_t i = aIndex; i < n; i++)
                m_value[i] = std::move(m_value[i + 1]);
            m_vector_or_count = (std::vector<T>*)(n * 2 + 1);
            return;
            }
        m_vector_or_count->erase(m_vector_or_count->begin() + aIndex,m_vector_or_count->begin() + aIndex + 1);
        if (n == N)
            {
            std::vector<T>* v = m_vector_or_count;
            for (size_t i = 0; i < N; i++)
                m_value[i] = std::move((*v)[i]);
            m_vector_or_count = (std::vector<T>*)(n * 2 + 1);
            delete v;
            }
        }

    T* begin() { return Data(); }
    const T* begin() const { return Data(); }
    T* end()
        {
        if (!m_vector_or_count)
            return nullptr;
        if (size_t(m_vector_or_count) & 1)
            return m_value + size_t(m_vector_or_count) / 2;
        return m_vector_or_count->data() + m_vector_or_count->size();
        }
    const T* end() const
        {
        if (!m_vector_or_count)
            return nullptr;
        if (size_t(m_vector_or_count) & 1)
            return m_value + size_t(m_vector_or_count) / 2;
        return m_vector_or_count->data() + m_vector_or_count->size();
        }

    private:
    T m_value[N];
    std::vector<T>* m_vector_or_count = nullptr;
    };

template<typename T> using CCompactArray = CSmallArray<T,1>;

template<typename T> void inline DeleteNulls(std::vector<std::unique_ptr<T>>& aVector)
    {
    auto iter = std::remove_if(aVector.begin(),aVector.end(),[](std::unique_ptr<T>& aObject) { return !aObject; });
    aVector.erase(iter,aVector.end());
    }

} // namespace CartoType

#endif
