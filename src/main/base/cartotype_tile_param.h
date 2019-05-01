/*
CARTOTYPE_TILE_PARAM.H
Copyright (C) 2013-2018 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_TILE_PARAM_H__
#define CARTOTYPE_TILE_PARAM_H__

#include <cartotype_string.h>
#include <cartotype_epsg.h>
#include <memory>

namespace CartoType
{

/** An interface class to provide tile filenames. */
class MTileNameProvider
    {
    public:
    /** A virtual destructor is needed in case the object is owned by CTileDataAccessor. */
    virtual ~MTileNameProvider() { }
    
    /**
    Provide the name of the file containing a certain tile. 
    It is acceptable always to return a reference to the same string object.
    Return nullptr to use the ordinary tile name format.
    
    The base implementation returns nullptr.
    */
    virtual const MString* TileName(TResult& aError,int32_t aZoom,int32_t aX,int32_t aY);
    
    /**
    Choose a zoom level (1-18) for tiles for a given map scale.
    Return -1 if the tile accessor should choose the zoom level.
    The number you return is passed to TileName().
    
    The base implementation returns -1.
    */
    virtual int32_t GetTileZoomLevel(double aScaleDenominator);
    };

/** Tile database parameters collected into a single object for convenience. */
class TTileParam
    {
    public:
    TTileParam():
        iResolution(0),
        iProjectionEpsg(KEpsgWebMercator),
        iLoadTextIndex(true)
        {
        }
    TTileParam(const CString& aTileNameFormat,const CString& aZoomLevels,
               std::shared_ptr<MTileNameProvider> aTileNameProvider = nullptr,
               const TRect* aExtent = nullptr,
               int32_t aProjectionEpsg = KEpsgWebMercator):
        iTileNameFormat(aTileNameFormat),
        iZoomLevels(aZoomLevels),
        iTileNameProvider(aTileNameProvider),
        iResolution(0),
        iProjectionEpsg(aProjectionEpsg),
        iLoadTextIndex(true)
        {
        if (aExtent)
            iExtent = *aExtent;
        }

    TResult SetExtent(int32_t aZoomLevel,int32_t aLeft,int32_t aTop,int32_t aRight,int32_t aBottom);

    /**
    The full path and filename for all tiles: the symbols %z, %x, and %y are
    replaced by the zoom level and the tile x and y coordinates. The symbols %z, %x and %y may appear
    more than once (so that a tilename at the end of a hierarchy of directories may itself be uniquely
    identified: e.g., /%z/%x/tile%z_%x_%y). The format must contain at least one each of %z, %x and %y.

    If a more general method of specifying tile filenames is required, in particular if different groups of files are stored
    in different trees, you can supply a file name provider object as iTileNameProvider. If it is present, it is used
    and iTileNameFormat is ignored; it can be null if iTileNameProvider is non-null.
    */
    CString iTileNameFormat;

    /**
    The parameter aZoomLevels is a string containing the space-separated zoom levels supported as CTM1 files;
    Zoom levels in the range 0...18 inclusive are legal.
    
    If a zoom level is marked with a trailing *, it is used as the smallest level to draw all maps where the largest dimension
    of the map (width or height) is no larger than the largest dimension of this zoom level. That facility prevents the use of
    over-large CTM1 files for small maps.
    */
    CString iZoomLevels;

    /** An optional object to provide tile filenames and zoom levels. */
    std::shared_ptr<MTileNameProvider> iTileNameProvider;

    /** The optional extent, in map coordinates, of the map (default = nullptr); if empty, a whole-world extent is used. */
    TRect iExtent;

    /** The resolution of the coordinates used in the map tiles: 0 = meters, 5 = 32nds of meters. */
    int32_t iResolution;

    /**
    The EPSG projection code of the CTM1 files (default = 3857: web mercator). Legal values are
    3857 (web mercator) and 4326 (plate carree).
    */
    int32_t iProjectionEpsg;
    
    /**
    If true (the default), load the text indexes of the individual CTM1 files.
    Set this to false to save memory if fast text searching is not needed.
    */
    bool iLoadTextIndex;
    };

}

#endif
