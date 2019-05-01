/*
CARTOTYPE_ADDRESS.H
Copyright (C) 2013-2018 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_ADDRESS_H__
#define CARTOTYPE_ADDRESS_H__

#include <cartotype_string.h>
#include <cartotype_path.h>

namespace CartoType
{

class CMapObject;
class CStreetAddressSearchStateImplementation;

/** A structured address for use with FindAddress. */
class CAddress
    {
    public:
    void Clear();
    CString ToString(bool aFull = true,const TPointFP* aLocation = nullptr) const;
    CString ToStringWithLabels() const;
    
    /** The name or number of the building. */
    CString iBuilding;
    /** The name of a feature or place of interest. */
    CString iFeature;
    /** The street, road or other highway. */
    CString iStreet;
    /** The suburb, neighborhood, quarter or other subdivision of the locality. */
    CString iSubLocality;
    /** The village, town or city. */
    CString iLocality;
    /** The name of an island. */
    CString iIsland;
    /**
    The subsidiary administrative area: county, district, etc.
    By preference this is a level-6 area in the OpenStreetMap classification.
    Levels 7, 8 and 5 are used in that order if no level-6 area is found.
    */
    CString iSubAdminArea;
    /**
    The administrative area: state, province, etc.
    By preference this is a level-4 area in the OpenStreetMap classification.
    Level 3 is used if no level-4 area is found.
    */
    CString iAdminArea;
    /** The country. */
    CString iCountry;
    /** The postal code. */
    CString iPostCode;
    };

/**
The state of a street address search.
It is used to iterate through all the objects found satisfying the search parameters.
Each iteration returns all the streets in a certain combination of country, admin area and locality.
*/
using CStreetAddressSearchState = std::unique_ptr<CStreetAddressSearchStateImplementation>;

/** Address part codes used when searching for address parts separately. */
enum class TAddressPart
    {
    /** Building names or numbers. */
    Building,
    /** Features or points of interest. */
    Feature,
    /** Streets or roads. */
    Street,
    /** Suburbs and neighborhoods. */
    SubLocality,
    /** Cities, towns and villages. */
    Locality,
    /** Islands. */
    Island,
    /** Lower-level administrative areas like counties. */
    SubAdminArea,
    /** Higher-level administrative areas like states. */
    AdminArea,
    /** Countries. */
    Country,
    /** Postal codes. */
    PostCode
    };

/**
A type used in addresses obtained by reverse geocoding.
It gives a coarse notion of what an object is.
Codes are ordered roughly from small to large.
*/
enum class TGeoCodeType
    {
    None,
    Position,
    Address,
    
    Building,
    Farm,

    Footpath,
    Cycleway,
    SkiRoute,
    WalkingRoute,

    FerryRoute,
    Railway,

    PedestrianRoad,
    Track,
    ServiceRoad,
    ResidentialRoad,
    UnclassifiedRoad,
    TertiaryRoad,
    SecondaryRoad,
    PrimaryRoad,
    TrunkRoad,
    Motorway,

    HikingRoute,
    CyclingRoute,

    LandFeature,
    WaterFeature,

    Locality,
    Hamlet,

    PostCode,

    AdminArea10,
    AdminArea9,
    Neighborhood,
    Village,
    Suburb,

    Island,

    AdminArea8,
    Town,

    AdminArea7,
    City,
    
    AdminArea6,
    AdminArea5,
    AdminArea4,
    AdminArea3,
    AdminArea2,
    AdminArea1,
    Country,
    Continent
    };

/** A geocode item describes a single map object. */
class CGeoCodeItem
    {
    public:
    TGeoCodeType iGeoCodeType = TGeoCodeType::None;

    /**
    The name of the object in the locale used when requesting a geocode.
    For buildings, this may be a building number.
    */
    CString iName;
    /** The postal code if any. */
    CString iPostCode;
    };

}

#endif
