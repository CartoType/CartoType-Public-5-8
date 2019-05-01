/*
cartotype_style_sheet_data.h
Copyright (C) 2016-2019 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_STYLE_SHEET_DATA_H__
#define CARTOTYPE_STYLE_SHEET_DATA_H__

#include <cartotype_stream.h>
#include <string>

namespace CartoType
{

/** Style sheet data stored as text or in a file. */
class CStyleSheetData
    {
    public:
    CStyleSheetData() = default;
    CStyleSheetData(const CStyleSheetData& aOther) = default;
    CStyleSheetData(CStyleSheetData&& aOther) = default;
    CStyleSheetData& operator=(const CStyleSheetData& aOther) = default;
    CStyleSheetData& operator=(CStyleSheetData&& aOther) = default;
    CStyleSheetData(const uint8_t* aData,size_t aLength);
    CStyleSheetData(TResult& aError,const char* aFileName);
    CStyleSheetData(TResult& aError,const uint8_t* aData,size_t aLength);
    TResult Reload();
    TMemoryInputStream Stream() const { return TMemoryInputStream((const uint8_t*)iText.data(),iText.length()); }
    const std::string& FileName() const { return iFileName; }
    const std::string& Text() const { return iText; }

    private:
    std::string iText;
    std::string iFileName;
    };

/** A set of style sheet data that may consist of more than one style sheet. */
using CStyleSheetDataArray = std::vector<CStyleSheetData>;

}

#endif // CARTOTYPE_STYLE_SHEET_DATA_H__
