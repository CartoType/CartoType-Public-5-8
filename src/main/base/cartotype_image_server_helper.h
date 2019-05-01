/*
CARTOTYPE_IMAGE_SERVER_HELPER.H
Copyright (C) 2013-2018 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_IMAGE_SERVER_HELPER_H__
#define CARTOTYPE_IMAGE_SERVER_HELPER_H__

#include <cartotype_base.h>

namespace CartoType
{

/**
Image data consisting either of a bitmap (not owned),
or arbitrary data in a format known to the user, or both.
*/
class TImageData
    {
    public:
    const TBitmap* iBitmap = nullptr;
    void* iImageData = nullptr;
    int32_t iImageDataSize = 0;
    };

/** Create an object of a class derived from this class to use CImageServer. */
class MImageServerHelper
    {
    public:
    virtual ~MImageServerHelper() { }

    /**
    This function is called when a bitmap is first loaded.
    The user can optionally create data representing it (e.g., a texture)
    and return a unique reference to it. If a non-null value is returned
    the original bitmap is discarded.

    The default implementation returns null.
    */
    virtual void* CreateImageData(const TBitmap& aBitmap);

    /**
    This function is called when an image is removed from the cache.
    The user should release any resources allocated by CreateImageData.
    The pointer originally returned by that function is supplied.
    The default implementation does nothing.
    */
    virtual void ReleaseImageData(void* aImageData);

    /**
    This function is called to obtain the size in bytes
    of an image data object, for the purpose of maintaining the cache.
    If image data is cached separately from main memory, zero should be returned.
    The default implementation returns zero.
    */
    virtual int32_t ImageDataSize(void* aImageData);
    
    /**
    This function adds image data to an implementation-provided cache, for example a cache on disk.
    If there is no cache, or the data cannot be cached, it should return KErrorNone, which is what
    the default implementation does.
    
    The identifier aId is used as a key when obtaining data from the cache.
    */
    virtual TResult AddImageDataToCache(const MString& aId,const TImageData& aImageData);
    
    /**
    This function obtains image data from an implementation-provided cache, for example a cache on disk.
    If there is no cache it should return KErrorNotFound, which is what the default implementation does.
    */
    virtual TResult GetImageDataFromCache(const MString& aId,TImageData& aImageData);

    /** This function is called to draw the background of the entire display in the specified paint. */
    virtual TResult DrawBackground(const TPaint& aPaint);

    /**
    This function is called to draw a bitmap, transforming it using the supplied transform.
    The transform scales, rotates and translates the image to the correct place on the display,
    converting pixel coordinates to display coordinates.

    Either the user's image data is supplied or the CartoType bitmap. If the user returned
    a non-null value for CreateImageData that value is supplied in aImageData and aBitmap is null.
    If not, the bitmap is supplied in aBitmap and aImageData is null.
    */
    virtual TResult DrawBitmap(void* aDeviceContext,const TBitmap* aBitmap,const void* aImageData,const TTransformFP& aTransform) = 0;
    };

}

#endif
