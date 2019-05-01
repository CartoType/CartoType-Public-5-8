/*
CARTOTYPE_STRING_TOKENIZER.H
Copyright (C) 2004-2018 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_STRING_TOKENIZER_H__
#define CARTOTYPE_STRING_TOKENIZER_H__

#include <cartotype_string.h>

namespace CartoType
{

/**
A class to find all the tokens in a string.
A token is a sequence of characters that contains no separators,
or a single character from a specified list, or optionally a string delimited by single or double quotes.

Tokens are separated by optional separators, which also may precede the first token and
follow the last. If no separators are specified, only word spaces (U+0020) are allowed as
separators.

Only separator characters from the basic multilingual plane (U+0000...U+FFFF)
are supported.
*/
class TStringTokenizer
    {
    public:
    TStringTokenizer(const MString& aString,const MString* aSeparators = 0,const MString* aSingleCharacterTokens = 0,bool aHandleQuotedStrings = false);
    TStringTokenizer(const MString& aString,const uint16_t* aSeparators,const uint16_t* aSingleCharacterTokens,bool aHandleQuotedStrings = false);
    size_t TokensLeft() const;
    bool HasMoreTokens() const;
    TResult NextToken(TText& aToken);
    TResult AppendNextToken(std::vector<CString>& aTokenArray);
    std::vector<CString> Tokenize();
    TText TextRemaining() const { return TText(iPtr,iEnd - iPtr); }
    bool LastTokenWasQuotedString() const { return iWasQuotedString; }

    private:
    void Skip(bool aSkipSeparators);

    const uint16_t* iPtr = nullptr;
    const uint16_t* iEnd = nullptr;
    const uint16_t* iSeparator = nullptr;
    const uint16_t* iSeparatorEnd = nullptr;
    const uint16_t* iSingleCharacterToken = nullptr;
    const uint16_t* iSingleCharacterTokenEnd = nullptr;
    bool iHandleQuotedStrings = false;
    bool iWasQuotedString = false;
    CString iTempString;
    };

} // namespace CartoType

#endif
