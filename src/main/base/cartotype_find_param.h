/*
CARTOTYPE_FIND_PARAM.H
Copyright (C) 2013-2018 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_FIND_PARAM_H__
#define CARTOTYPE_FIND_PARAM_H__

#include <cartotype_path.h>

namespace CartoType
{

/** Types of place to search for in the 'find nearby' parameters. */
enum class TPointOfInterestType
    {
    None,
    Airport,
    Bar,
    Beach,
    BusStation,
    Cafe,
    Camping,
    FastFood,
    Fuel,
    GolfCourse,
    Hospital,
    Hotel,
    Pharmacy,
    Police,
    Restaurant,
    Shops,
    SportsCenter,
    Supermarket,
    SwimmingPool,
    Tourism,
    TrainStation
    };

/** Parameters for finding nearby places. */
class TFindNearbyParam
    {
    public:
    /** The type of place to search for. The value None causes iName to be analyzed to narrow the search. */
    TPointOfInterestType iType;
    /** The name, full or partial, of the place. */
    CString iText;
    /** The location of interest. */
    CGeometry iLocation;
    };

/** Parameters for the general find function. */
class TFindParam
    {
    public:
    TFindParam() = default;
    TFindParam(const TFindNearbyParam& aFindNearbyParam);

    /** The maximum number of objects to return; default = SIZE_MAX. */
    size_t iMaxObjectCount = SIZE_MAX;
    /** The clip path; no clipping is done if iClip is empty. */
    CGeometry iClip;
    /** The current location. If it is non-empty, objects in or near this region are preferred. It may be a single point. */
    CGeometry iLocation;
    /** A list of layer names separated by spaces or commas. If it is empty all layers are searched. Layer names may contain the wild cards * and ?. */
    CString iLayers;
    /**
    iAttributes is used in text searching (if iText is non-null). If iAttributes
    is empty, search all attributes, otherwise iAttributes is a list of
    attributes separated by spaces or commas; use "$" to indicate the label (the
    unnamed attribute).
    */
    CString iAttributes;
    /**
    iText, if not empty, restricts the search to objects containing a string
    in one of their string attributes.
    */
    CString iText;
    /** The string matching method used for text searching; default = Exact. */
    TStringMatchMethod iStringMatchMethod = TStringMatchMethod::Exact;
    /**
    iCondition, if not empty, is a style sheet condition (e.g., "Type==2")
    which must be fulfilled by all the objects. 
    */
    CString iCondition;
    /** If iMerge is true (the default), adjoining objects with the same name and attributes may be merged into a single object. */
    bool iMerge = true;
    };


}

#endif
