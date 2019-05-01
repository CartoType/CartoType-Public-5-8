/*
CARTOTYPE_STRING.H
Copyright (C) 2004-2018 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_STRING_H__
#define CARTOTYPE_STRING_H__

#include <cartotype_errors.h>
#include <cartotype_iter.h>
#include <cartotype_char.h>
#include <cartotype_base.h>
#include <cartotype_bidi.h>
#include <cartotype_array.h>
#include <cartotype_arithmetic.h>

#include <locale.h>
#include <string>
#include <map>

namespace CartoType
{

class TText;
class TFixed;

/**
A constant used to mean 'to the end of the string', or 'unknown string length: must be measured'.
It has the same value and definition as std::string::npos.
*/
static const size_t npos = size_t(-1);

class CString;
using CStringDictionary = std::map<CString,CString>;
using CStringTypeDictionary = std::map<CString,int>;

/** A buffer which can create UTF16 text from UTF8. */
class TUtf16Buffer
    {
    public:
    TUtf16Buffer(const uint8_t* aText,size_t aLength,size_t aExtraLength = 0);

    TStackArray<uint16_t,128> iText;
    size_t iLength;
    TResult iError = 0;
    };

/** Flags and constants to tell text searching how to match search terms with found strings. */
namespace TStringMatchMethodFlag
    {
    /** A flag to match all strings for which the search term is a perfect match or a prefix. */
    constexpr uint32_t Prefix = 1;
    /** A flag to ignore all characters that are not letters or digits when matching. */
    constexpr uint32_t IgnoreNonAlphanumerics = 2;
    /** A flag to match accented and unaccented letters. */
    constexpr uint32_t FoldAccents = 4;
    /** A flag to allow imperfect matches with a small number of omitted, incorrect or extra characters. */
    constexpr uint32_t Fuzzy = 8;
    /** A flag to fold letter case. */
    constexpr uint32_t FoldCase = 16;
    /** A flag to enable fast searching: just get the first matches, not the most relevant ones. */
    constexpr uint32_t Fast = 32;
    };

enum class TStringMatchMethod
    {
    /** Strings must match exactly. */
    Exact = 0,
    /** Fold (ignore) letter case when matching strings. */
    FoldCase = TStringMatchMethodFlag::FoldCase,
    /** Fold (ignore) accents when matching strings. */
    FoldAccents = TStringMatchMethodFlag::FoldAccents,
    /** Fold (ignore) case and accents when matching strings. */
    Fold = TStringMatchMethodFlag::FoldCase | TStringMatchMethodFlag::FoldAccents,
    /** The search term must be an exact match or a prefix of the found string. */
    Prefix = TStringMatchMethodFlag::Prefix,
    /** Loose matching: ignore non-alphanumerics and fold accents and case. */
    Loose = TStringMatchMethodFlag::IgnoreNonAlphanumerics |
            TStringMatchMethodFlag::FoldAccents |
            TStringMatchMethodFlag::FoldCase,
    /** Allow fuzzy matches: ignore non-alphanumerics, fold accents and allow imperfect matches. */
    Fuzzy = TStringMatchMethodFlag::IgnoreNonAlphanumerics |
            TStringMatchMethodFlag::FoldAccents |
            TStringMatchMethodFlag::Fuzzy |
            TStringMatchMethodFlag::FoldCase
    };

/** A TStringMatchMethod value for backward compatibility. */
constexpr TStringMatchMethod EStringMatchExact = TStringMatchMethod::Exact;
/** A TStringMatchMethod value for backward compatibility. */
constexpr TStringMatchMethod EStringMatchFoldCase = TStringMatchMethod::FoldCase;
/** A TStringMatchMethod value for backward compatibility. */
constexpr TStringMatchMethod EStringMatchPrefix = TStringMatchMethod::Prefix;
/** A TStringMatchMethod value for backward compatibility. */
constexpr TStringMatchMethod EStringMatchFuzzy = TStringMatchMethod::Fuzzy;

/** A constant for the title case dictionary passed to MString::SetCase. No special title case treatment. This constant must be zero. */
constexpr int KStandardTitleCase = 0;
/**
A constant for the title case dictionary passed to MString::SetCase.
When setting text to title case, do not capitalize the first letter unless the word is initial.
Used for particles like 'on', 'of', etc. and their upper-case variants.
*/
constexpr int KLowerTitleCase = 1;
/**
A constant for the title case dictionary passed to MString::SetCase.
When setting text to title case, leave these strings as they are (they are already upper-case).
Used for acronyms like US, roman numerals like II, etc.
*/
constexpr int KUpperTitleCase = 2;

/**
The string interface class. All strings implement this interface.
The private part of the interface allows completely general compare, append,
insert, delete and replace functions to be implemented in MString.

Although the M prefix of MString implies that this class is a pure mix-in with
no data, it has one data item, the length, which was moved to the base
class for efficiency reasons after profiling indicated a bottleneck accessing
the various virtual Length functions of derived classes.
*/
class MString
    {
    public:
    /**
    A virtual destructor. Provided so that templated collection classes can be created using
    MString as the template class, while still being able to own the strings.
    */
    virtual ~MString() { }

    template<typename T> MString& operator=(const T& aText)
        {
        Set(aText);
        return *this;
        }
    template<typename T> MString& operator+=(const T& aText)
        {
        Append(aText);
        return *this;
        }

    /** Return the length of the text. */
    size_t Length() const { return iLength; }

    /** Return a constant pointer to Unicode text stored in UTF16 format. */
    virtual const uint16_t* Text() const = 0;

    static int32_t Compare(MIter<int32_t>& aIter1,MIter<int32_t>& aIter2,TStringMatchMethod aStringMatchMethod) noexcept;
    /** Delete the text in the range aStart...aEnd. */
    void Delete(size_t aStart,size_t aEnd) { Replace(aStart,aEnd,(const uint16_t*)0,0); }
    /** Delete all the text. */
    void Clear()
        {
        if (!Writable())
            {
            assert(false);
            return;
            }
        ResizeBuffer(0);
        }
    /** Set the length of the string to zero without discarding reserved memory if any. */
    void SetLengthToZero() { iLength = 0; }

    std::basic_string<uint16_t> CreateUtf16String() const;
    std::string CreateUtf8String() const;

    /** A conversion operator to convert a string to a UTF-8 string. */
    operator std::string() const { return CreateUtf8String(); }

    /** Get the character at the position aIndex. */
    uint16_t operator[](size_t aIndex) const { assert(aIndex < (size_t)iLength); return Text()[aIndex]; }

    TText First(size_t aLength) const;
    TText Last(size_t aLength) const;

    void DeletePrefix(const MString& aPrefix);
    void DeleteSuffix(const MString& aSuffix);

    void SetCase(TLetterCase aCase,const CStringTypeDictionary* aTitleCaseDictionary = 0);
    void SetSentenceCase();

    // functions taking an iterator
    int32_t Compare(MIter<int32_t>& aIter,bool aFoldCase) const noexcept;
    TResult Replace(size_t aStart,size_t aEnd,MIter<int32_t>& aText,size_t aMaxLength);

    // functions taking an MString
    int32_t Compare(const MString& aString) const
        {
        auto n = iLength;
        auto m = aString.iLength;
        const uint16_t* p = Text();
        const uint16_t* q = aString.Text();
        while (n != 0 && m != 0)
            {
            if (*p < *q)
                return -2;
            if (*p > *q)
                return 2;
            p++;
            q++;
            n--;
            m--;
            }
        if (m)
            return -1;
        if (n)
            return 1;
        return 0;
        }
    int32_t Compare(const MString& aString,bool aFoldCase) const noexcept;
    int32_t Compare(const MString& aString,TStringMatchMethod aStringMatchMethod) const noexcept;
    size_t Find(const MString& aString,TStringMatchMethod aStringMatchMethod = TStringMatchMethod::Exact,size_t* aEndPos = nullptr) const noexcept;
    /** Replace the text aStart...aEnd with aString. */
    void Replace(size_t aStart,size_t aEnd,const MString& aString) { Replace(aStart,aEnd,aString.Text(),aString.Length()); }
    /** Insert aString at aIndex. */
    void Insert(size_t aIndex,const MString& aString) { Replace(aIndex,aIndex,aString); }

    /** Append aString. */
    void Append(const MString& aString)
        {
        if (!Writable())
            {
            assert(false);
            return;
            }
        size_t old_length = iLength;
        ResizeBuffer(iLength + aString.iLength);
        memcpy(WritableText() + old_length,aString.Text(),(iLength - old_length) * 2);
        }

    void AppendCommaSeparated(const CString& aLabel,const CString& aText,size_t& aItems,size_t aMaxItems = SIZE_MAX);

    /** Set the string to aString. */
    void Set(const MString& aString)
        {
        if (!Writable())
            {
            assert(false);
            return;
            }
        ResizeBuffer(aString.iLength);
        memcpy(WritableText(),aString.Text(),iLength * 2);
        }

    // functions taking sixteen-bit text
    int32_t Compare(const uint16_t* aText) const
        {
        auto n = iLength;
        const uint16_t* p = Text();
        const uint16_t* q = aText;
        while (n-- != 0)
            {
            if (*q == 0)
                return 1;
            if (*p < *q)
                return -2;
            if (*p > *q)
                return 2;
            p++;
            q++;
            }
        if (*q == 0)
            return 0;
        return -1;
        }
    int32_t Compare(const uint16_t* aText,size_t aLength) const
        {
        if (aLength == npos)
            return Compare(aText);

        auto n = iLength;
        auto m = aLength;
        const uint16_t* p = Text();
        const uint16_t* q = aText;
        while (n != 0 && m != 0)
            {
            if (*p < *q)
                return -2;
            if (*p > *q)
                return 2;
            p++;
            q++;
            n--;
            m--;
            }
        if (m)
            return -1;
        if (n)
            return 1;
        return 0;
        }
    int32_t Compare(const char16_t* aText,size_t aLength = npos) const { return Compare((const uint16_t*)aText,aLength); }
    int32_t Compare(const uint16_t* aText,size_t aLength,bool aFoldCase) const noexcept;
    void Replace(size_t aStart,size_t aEnd,const uint16_t* aText,size_t aLength = npos);

    /** Insert aText at aIndex. If aLength is npos the text must be null-terminated, otherwise the length is aLength. */
    void Insert(size_t aIndex,const uint16_t* aText,size_t aLength = npos)
        { Replace(aIndex,aIndex,aText,aLength); }

    /** Append aText. If aLength is npos the text must be null-terminated, otherwise the length is aLength. */
    void Append(const uint16_t* aText,size_t aLength = npos)
        {
        if (!Writable())
            {
            assert(false);
            return;
            }
        if (aLength == npos)
            {
            const uint16_t* p = aText;
            aLength = 0;
            while (*p++)
                aLength++;
            }
        size_t old_length = iLength;
        ResizeBuffer(iLength + aLength);
        memcpy(WritableText() + old_length,aText,(iLength - old_length) * 2);
        }

    /** Set the string to aText. If aLength is npos the text must be null-terminated, otherwise the length is aLength. */
    void Set(const uint16_t* aText,size_t aLength = npos)
        {
        if (!Writable())
            {
            assert(false);
            return;
            }
        if (aLength == npos)
            {
            const uint16_t* p = aText;
            aLength = 0;
            while (*p++)
                aLength++;
            }

        ResizeBuffer(aLength);
        memcpy(WritableText(),aText,iLength * 2);
        }

    // functions taking eight-bit text
    int32_t Compare(const char* aText,size_t aLength = npos,bool aFoldCase = false) const noexcept;
    void Replace(size_t aStart,size_t aEnd,const char* aText,size_t aLength = npos,int32_t aCodePage = 0);
    bool operator==(const char* aText) const noexcept;
    bool operator%(const char* aText) const noexcept;
    /** The inequality operator for comparing MString objects with null-terminated UTF8 strings */
    bool operator!=(const char* aText) const { return !(*this == aText); }

    /**
    Insert the eight-bit string aText at aIndex. If aLength is -1 the text must be null-terminated, otherwise the length is aLength.
    A code page of 0 (the default) implies UTF-8.
    */
    void Insert(size_t aIndex,const char* aText,size_t aLength = npos,int32_t aCodePage = 0)
        { Replace(aIndex,aIndex,aText,aLength,aCodePage); }
    /**
    Append the eight-bit string aText. If aLength is npos the text must be null-terminated, otherwise the length is aLength.
    A code page of 0 (the default) implies UTF-8.
    */
    void Append(const char* aText,size_t aLength = npos,int32_t aCodePage = 0)
        { Insert(Length(),aText,aLength,aCodePage); }

    /** Appends a standard string. */
    template<typename T> void Append(const std::basic_string<T> aString) { Append(aString.data(),aString.length()); }

    /** Sets the text to a standard string. */
    template<typename T> void Set(const std::basic_string<T> aString) { Set(aString.data(),aString.length()); }

    /**
    Set the string to the UTF-8 string aText. If aLength is -1 the text must be null-terminated, otherwise the length is aLength.
    */
    void Set(const char* aText,size_t aLength = npos)
        {
        if (!Writable())
            {
            assert(false);
            return;
            }
        TUtf16Buffer buffer((const unsigned char*)aText,aLength);
        ResizeBuffer(buffer.iLength);
        memcpy(WritableText(),buffer.iText.Data(),iLength * 2);
        }

    /**
    Set the string to the eight-bit string aText. If aLength is -1 the text must be null-terminated, otherwise the length is aLength.
    A code page of 0 implies UTF-8.
    */
    void Set(const char* aText,size_t aLength,int32_t aCodePage)
        { Replace(0,Length(),aText,aLength,aCodePage); }

    // functions taking a character
    /** Replace the range aStart...aEnd with the single character aChar. */
    void Replace(size_t aStart,size_t aEnd,uint16_t aChar) { Replace(aStart,aEnd,&aChar,1); }
    /** Insert aChar at aIndex. */
    void Insert(size_t aIndex,uint16_t aChar) { Replace(aIndex,aIndex,aChar); }
    /** Append aChar. */
    void Append(uint16_t aChar) { Append(&aChar,1); }
    /** Set the entire text to aChar. */
    void Set(uint16_t aChar) { Replace(0,Length(),aChar); }
    void Trim();

    /** The equality operator. */
    bool operator==(const MString& aString) const
        {
        return aString.iLength == iLength && !memcmp(aString.Text(),Text(),iLength * sizeof(uint16_t));
        }
    /** The equality operator for comparing MString objects with null-terminated UTF16 strings. */
    bool operator==(const uint16_t* aText) const
        {
        const uint16_t* p = Text();
        const uint16_t* q = p + iLength;
        while (p < q && *aText && *p == *aText)
            {
            p++;
            aText++;
            }
        return p == q && *aText == 0;
        }
    /** The equality operator for comparing MString objects with null-terminated UTF16 strings. */
    bool operator==(const char16_t* aText) const
        {
        const char16_t* p = (char16_t*)Text();
        const char16_t* q = p + iLength;
        while (p < q && *aText && *p == *aText)
            {
            p++;
            aText++;
            }
        return p == q && *aText == 0;
        }
    /** The inequality operator. */
    bool operator!=(const MString& aString) const { return !(*this == aString); }
    /** The inequality operator for comparing MString objects with null-terminated UTF16 strings */
    bool operator!=(const uint16_t* aText) const { return !(*this == aText); }
    /** The inequality operator for comparing MString objects with null-terminated UTF16 strings */
    bool operator!=(const char16_t* aText) const { return !(*this == aText); }

    /** The equality operator, ignoring case. */
    bool operator%(const MString& aString) const { return Compare(aString,true) == 0; }
    /** The equality operator, ignoring case, for comparing MString objects with null-terminated UTF16 strings */
    bool operator%(const uint16_t* aText) const { return Compare(aText,true) == 0; }

    // wild-card comparison
    bool WildMatch(const MString& aWildText) const noexcept;
    bool WildMatch(const char* aWildText) const noexcept;
    bool LayerMatch(const MString& aWildText) const;
    bool LayerMatch(const char* aWildText) const noexcept;

    // fuzzy comparison
    /** Maximum edit distance allowed by fuzzy matching: the maximum value for aMaxDistance in MString::FuzzyMatch(). */
    static constexpr int KMaxFuzzyDistance = 4;
    static bool FuzzyMatch(MIter<int32_t>& aIter1,MIter<int32_t>& aIter2,int32_t aMaxDistance) noexcept;
    static bool FuzzyMatch(const char* aText1,const char* aText2,int32_t aMaxDistance) noexcept;

    // comparison operators
    template<class T> bool operator<(const T& aOther) const { return Compare(aOther) < 0; }
    template<class T> bool operator<=(const T& aOther) const { return Compare(aOther) <= 0; }
    template<class T> bool operator>(const T& aOther) const { return Compare(aOther) > 0; }
    template<class T> bool operator>=(const T& aOther) const { return Compare(aOther) >= 0; }

    // conversion functions
    static void ToInt32(const uint8_t* aText,size_t aLength,int32_t& aValue,size_t& aLengthUsed,int32_t aBase = 10) noexcept;
    void ToInt32(int32_t& aValue,size_t& aLengthUsed,int32_t aBase = 10) const noexcept;
    void ToInt64(int64_t& aValue,size_t& aLengthUsed,int32_t aBase = 10) const noexcept;
    void ToUint32(uint32_t& aValue,size_t& aLengthUsed,int32_t aBase = 10) const noexcept;
    TResult ToFixed(TFixed& aValue,size_t& aLengthUsed) const noexcept;
    TResult ToDouble(double& aValue,size_t& aLengthUsed) const noexcept;
    TResult ToDimension(double& aValue,size_t& aLengthUsed) const noexcept;

    // conversion to presentation form: bidirectional reordering, mirroring and contextual shaping
    TResult Shape(TBidiParDir aParDir,CBidiEngine* aBidiEngine,bool aParStart,bool aReorderFontSelectors);

    void Abbreviate(const CStringDictionary& aDictionary);
    void Transliterate(const CStringTypeDictionary* aTitleCaseDictionary = nullptr,const char* aLocale = nullptr);

    // line breaking
    bool IsLineBreak(size_t aPos) const noexcept;
    size_t LineBreakBefore(size_t aPos) const noexcept;
    size_t LineBreakAfter(size_t aPos) const noexcept;

    // treating a string as a set of string attributes used by a map object
    void SetAttribute(const MString& aKey,const MString& aValue);
    void SetAttribute(const CString& aKey,const CString& aValue);
    TText GetAttribute(const MString& aKey) const noexcept;
    TText GetAttribute(const CString& aKey) const noexcept;
    bool NextAttribute(size_t& aPos,TText& aKey,TText& aValue) const noexcept;

    protected:
    MString(): iLength(0) { }
    MString(size_t aLength): iLength(aLength) { }

    /** The length of the text in 16-bit UTF16 values. */
    size_t iLength;

    private:
    /** Return true if the string is writable. */
    virtual bool Writable() const = 0;

    /**
    Return the maximum writable length of the text: the length of the currently reserved buffer.
    Return 0 if no buffer is reserved or if the text is not writable.
    */
    virtual size_t MaxWritableLength() const = 0;

    /**
    Return a non-constant pointer to the text.
    Return null if the text is not writable.
    */
    virtual uint16_t* WritableText() = 0;

    /**
    Adjust the text buffer to hold up to aNewLength characters;
    the actual new size may be less. Set iLength to the actual new size.
    */
    virtual void ResizeBuffer(size_t aNewLength) = 0;

    void ReplaceAndConvert(size_t aStart,size_t aEnd,const char* aText,size_t aLength,int32_t aCodePage);
    void FindStringAttrib(const MString& aKey,size_t& aKeyStart,size_t& aValueStart,size_t& aValueEnd) const noexcept;
    };

/** An unmodifiable string that doesn't own its text. */
class TText: public MString
    {
    public:
    /** Construct an empty TText object. */
    TText(): iText(nullptr) { }
    /** Construct a TText object from a null-terminated string. */
    TText(const uint16_t* aText): iText(aText)
        {
        iLength = 0;
        if (iText)
            while (*aText++)
                iLength++;
        }
    /** Construct a TText object from a pointer and a length. */
    TText(const uint16_t* aText,size_t aLength): MString(aLength), iText(aText) { }
    /** Construct a TText object from a pointer and a length. */
    TText(const char16_t* aText,size_t aLength): MString(aLength), iText((const uint16_t*)aText) { }
    /** Construct a TText object from an MString object. */
    TText(const MString& aString): MString(aString.Length()), iText(aString.Text())  { }

    // virtual functions from MString
    const uint16_t* Text() const override { return iText; }

    private:
    // virtual functions from MString
    bool Writable() const override { return false; }
    size_t MaxWritableLength() const override { return 0; }
    uint16_t* WritableText() override { return nullptr; }
    void ResizeBuffer(size_t /*aNewLength*/) override { assert(false); }

    const uint16_t* iText;
    };

/** A writable string that doesn't own its text. */
class TWritableText: public MString
    {
    public:
    TWritableText(): iText(nullptr), iMaxLength(0) { }
    TWritableText(uint16_t* aText,size_t aLength):
        MString(aLength),
        iText(aText),
        iMaxLength(aLength)
        {
        }

    // virtual functions from MString
    const uint16_t* Text() const { return iText; }

    private:
    // virtual functions from MString
    bool Writable() const { return true; }
    size_t MaxWritableLength() const { return iMaxLength; }
    uint16_t* WritableText() { return iText; }
    void ResizeBuffer(size_t aNewLength) { iLength = Arithmetic::Min(aNewLength,iMaxLength);  }

    uint16_t* iText;
    size_t iMaxLength;
    };

/** A string that owns its own text, which has a maximum length fixed at compile time. */
template<size_t aMaxLength> class TTextBuffer: public MString
    {
    public:
    /** Construct an empty TTextBuffer object. */
    TTextBuffer() { }

    /** Construct a TTextBuffer object from an 8-bit ASCII string. */
    TTextBuffer(const char* aText)
        {
        iLength = 0;
        while (*aText && iLength < aMaxLength)
            iText[iLength++] = *aText++;
        }

    // virtual functions from MString
    const uint16_t* Text() const { return iText; }

    private:
    // virtual functions from MString
    bool Writable() const { return true; }
    size_t MaxWritableLength() const { return aMaxLength; }
    uint16_t* WritableText() { return iText; }
    void ResizeBuffer(size_t aNewLength) { iLength = Arithmetic::Min(aNewLength,aMaxLength);  }

    uint16_t iText[aMaxLength];
    };

/** A macro to define the literal TText object aName from the UTF16 string aText. Example: TTextLiteral(mytext,u"hello"). */
#define TTextLiteral(aName,aText) TText static const aName(aText,sizeof(aText) / sizeof(char16_t) - 1);

/** A string that owns its own text, which is freely resizable. */
class CString: public MString
    {
    public:
    /** Construct an empty CString object. */
    CString():
        iText(iOwnText),
        iReserved(KOwnTextLength)
        {
        }
    CString(std::nullptr_t):
        iText(iOwnText),
        iReserved(KOwnTextLength)
        {
        }
    CString(const CString& aOther);
    CString(const MString& aOther);
    CString(CString&& aOther);
    CString& operator=(const CString& aOther);
    CString& operator=(const MString& aOther);
    CString& operator=(CString&& aOther);
    CString(const char* aText,size_t aLength = npos);
    CString(const uint16_t* aText,size_t aLength = npos);
    CString(const char16_t* aText,size_t aLength = npos);
    CString(const std::string& aText);

    ~CString()
        {
        if (iText != iOwnText)
            delete [] iText;
        }

    using MString::operator[];
    uint16_t& operator[](size_t aIndex) { assert(aIndex < (size_t)iLength); return iText[aIndex]; }

    // virtual functions from MString
    const uint16_t* Text() const override { return iText; }

    private:

    // virtual functions from MString
    bool Writable() const override { return true; }
    size_t MaxWritableLength() const override { return iReserved; }
    uint16_t* WritableText() override { return iText; }
    void ResizeBuffer(size_t aNewLength) override;

    static constexpr size_t KOwnTextLength = 32;
    uint16_t iOwnText[KOwnTextLength];
    uint16_t* iText;
    size_t iReserved;
    };

/** A type for reference-counted strings, which are used for layer names in map objects. */
class CRefCountedString: public std::shared_ptr<CString>
    {
    public:
    CRefCountedString(): std::shared_ptr<CString>(new CString) { } // ensure that the string is not empty on default construction
    CRefCountedString(const CString& aText): std::shared_ptr<CString>(new CString(aText)) { }
    CRefCountedString(std::nullptr_t): std::shared_ptr<CString>() { }
    };

/** An iterator to convert UTF8 text to UTF32. */
class TUtf8ToUtf32: public MIter<int32_t>
    {
    public:
    TUtf8ToUtf32(const uint8_t* aText,size_t aLength = npos);
    TResult Next(int32_t& aValue);
    void Back();
    const uint8_t* Pos() const { return iPos; }

    private:
    const uint8_t* iStart;
    const uint8_t* iPos;
    const uint8_t* iEnd;
    bool iEof;
    };

/** An iterator to convert UTF16 text to UTF32. */
class TUtf16ToUtf32: public MIter<int32_t>
    {
    public:
    TUtf16ToUtf32(const uint16_t* aText,size_t aLength);
    TResult Next(int32_t& aValue);
    void Back();
    const uint16_t* Pos() const { return iPos; }

    private:
    const uint16_t* iStart;
    const uint16_t* iPos;
    const uint16_t* iEnd;
    bool iEof;
    };

/** An iterator that does no conversion but simply passes UTF32 text straight through. */
class TUtf32Iter: public MIter<int32_t>
    {
    public:
    TUtf32Iter(const int32_t* aText,size_t aLength);
    TResult Next(int32_t& aValue);
    void Back();
    const int32_t* Pos() const { return iPos; }

    private:
    const int32_t* iStart;
    const int32_t* iPos;
    const int32_t* iEnd;
    bool iEof;
    };

/** A class to set the C locale. The destructor restores the original locale. */
class CCLocale
    {
    public:
    CCLocale()
        {
#ifndef _WIN32_WCE
        const char* locale = setlocale(LC_ALL,"C");
        if (locale)
            iSavedLocale = locale;
#endif
        }
    ~CCLocale()
        {
#ifndef _WIN32_WCE
        setlocale(LC_ALL,iSavedLocale.c_str());
#endif
        }

    private:
    std::string iSavedLocale;
    };

class TAbbreviationInfo
    {
    public:
    /** The locale (e.g., "en" or "en_GB") for this data: null implies universal data. */
    const char* iLocale;
    /** An array of abbreviations: pairs of UTF-8 strings in the order long, short. */
    const char* const * iAbbreviation;
    /** The number of strings in iAbbreviation: twice the number of abbreviations. */
    int32_t iAbbreviationCount;
    /** An array of words to be put into lower case if the entire string is put into title case. */
    const char* const * iLowerTitleCaseException;
    /** The number of lower-case title-case exceptions. */
    int32_t iLowerTitleCaseExceptionCount;
    /** An array of words to be put into upper case if the entire string is put into title case. */
    const char* const * iUpperTitleCaseException;
    /** The number of upper-case title-case exceptions. */
    int32_t iUpperTitleCaseExceptionCount;
    };

const TAbbreviationInfo* AbbreviationInfo(const char* aLocale);
CString CountryToCode(const MString& aCountry);
CString CodeToCountry(const MString& aCode);

} // namespace CartoType

#endif
