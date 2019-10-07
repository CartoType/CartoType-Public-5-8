/*
cartotype_framework.h
Copyright (C) 2012-2019 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_FRAMEWORK_H__
#define CARTOTYPE_FRAMEWORK_H__

#include <cartotype_address.h>
#include <cartotype_bitmap.h>
#include <cartotype_find_param.h>
#include <cartotype_navigation.h>
#include <cartotype_stream.h>
#include <cartotype_string.h>
#include <cartotype_tile_param.h>
#include <cartotype_map_object.h>
#include <cartotype_graphics_context.h>
#include <cartotype_image_server_helper.h>
#include <cartotype_legend.h>
#include <cartotype_style_sheet_data.h>
#include <cartotype_expression.h>

#include <memory>
#include <set>

namespace CartoType
{

/**
\mainpage CartoType

\section introduction Introduction

CartoType is a portable C++ library for drawing maps, calculating routes
and providing navigation instructions. It uses map files created using the
makemap tool supplied with the package.

\section highlevelapi The Framework API

The CFramework class is the main API for CartoType and allows
you to build CartoType into your application.

You create a single CFramework
object and keep it in existence while your program is running.
It provides access to everything you need, including
map drawing, adding your own data to the map, handling user
interaction, searching, route calculation and turn-by-turn navigation.

The classes CFrameworkEngine and CFrameworkMapDataSet, in conjunction
with CFramework, allow you to use the model-view-controller pattern.
Usually, however, CFramework is all you need.
*/

class CEngine;
class CImageServer;
class CGcImageServerHelper;
class CMapDataAccessor;
class CPerspectiveGraphicsContext;
class MInternetAccessor;
class CWebMapServiceClient;
class CMap;
class CMapDrawParam;
class CMapDataBase;
class CMapStore;
class CMapStyle;
class CMemoryMapDataBase;
class CNavigator;
class CTileDataAccessor;
class CGraphicsContext;
class C32BitColorBitmapGraphicsContext;
class CStackAllocator;
class CTileServer;
class TMapTransform;
class CMapRendererImplementation;

/** A flag to make the center of the map follow the user's location. */
constexpr uint32_t KFollowFlagLocation = 1;
/** A flag to rotate the map to the user's heading. */
constexpr uint32_t KFollowFlagHeading = 2;
/** A flag to set the map to a suitable zoom level for the user's speed. */
constexpr uint32_t KFollowFlagZoom = 4;

/** Flags controlling the way the map follows the user location and heading and automatically zooms. */
enum class TFollowMode
    {
    /** The map does not follow the user's location or heading. */
    None = 0,
    /** The map is centred on the user's location. */
    Location = KFollowFlagLocation,
    /** The map is centred on the user's location and rotated to the user's heading. */
    LocationHeading = KFollowFlagLocation | KFollowFlagHeading,
    /** The map is centred on the user's location and zoomed to a suitable level for the user's speed. */
    LocationZoom = KFollowFlagLocation | KFollowFlagZoom,
    /** The map is centred on the user's location, rotated to the user's heading, and zoomed to a suitable level for the user's speed. */
    LocationHeadingZoom = KFollowFlagLocation | KFollowFlagHeading | KFollowFlagZoom
    };

/**
CFrameworkEngine holds a CTM1 data accessor and a standard font.
Together with a CFrameworkMapDataSet object it makes up the 'model' part of the model-view-controller pattern.
*/
class CFrameworkEngine
    {
    public:
    static std::unique_ptr<CFrameworkEngine> New(TResult& aError,const CString& aFontFileName,int32_t aFileBufferSizeInBytes = 0,int32_t aMaxFileBufferCount = 0,int32_t aTextIndexLevels = 0);
    static std::unique_ptr<CFrameworkEngine> New(TResult& aError,const std::vector<TTypefaceData>& aTypefaceDataArray,int32_t aFileBufferSizeInBytes = 0,int32_t aMaxFileBufferCount = 0,int32_t aTextIndexLevels = 0);
    ~CFrameworkEngine();
    TResult Configure(const CString& aFilename);
    TResult LoadFont(const CString& aFontFileName);
    TResult LoadFont(const uint8_t* aData,size_t aLength,bool aCopyData);
    std::unique_ptr<CFrameworkEngine> Copy(TResult& aError) const;
    void CancelDrawing();

    // internal use only
    std::shared_ptr<CEngine> Engine() const { return iEngine; }

    private:
    CFrameworkEngine();
    CFrameworkEngine(const CFrameworkEngine&) = delete;
    CFrameworkEngine(CFrameworkEngine&&) = delete;
    CFrameworkEngine& operator=(const CFrameworkEngine&) = delete;
    CFrameworkEngine& operator=(CFrameworkEngine&&) = delete;

    std::shared_ptr<CEngine> iEngine;
    CString iConfigFileName;
    std::vector<TTypefaceData> iTypefaceDataArray;
    int32_t iFileBufferSizeInBytes;
    int32_t iMaxFileBufferCount;
    int32_t iTextIndexLevels;
    };

/**
CFrameworkMapDataSet owns a set of map data used to draw a map.
Together with a CFrameworkEngine object it makes up the 'model' part of the model-view-controller pattern.
*/
class CFrameworkMapDataSet
    {
    public:
    static std::unique_ptr<CFrameworkMapDataSet> New(TResult& aError,CFrameworkEngine& aEngine,const CString& aMapFileName,const std::string* aEncryptionKey = nullptr,bool aMapOverlaps = true);
    static std::unique_ptr<CFrameworkMapDataSet> New(TResult& aError,CFrameworkEngine& aEngine,std::unique_ptr<CMapDataBase> aDb);
    static std::unique_ptr<CFrameworkMapDataSet> New(TResult& aError,CFrameworkEngine& aEngine,const TTileParam& aTileParam);

    ~CFrameworkMapDataSet();

    std::unique_ptr<CFrameworkMapDataSet> Copy(TResult& aError,bool aFull = true) const;
    TResult LoadMapData(CFrameworkEngine& aEngine,const CString& aMapFileName,const std::string* aEncryptionKey,bool aMapOverlaps);
    TResult LoadMapData(CFrameworkEngine& aEngine,const TTileParam& aTileParam,bool aMapOverlaps);
    TResult LoadMapData(std::unique_ptr<CMapDataBase> aDb);
    TResult UnloadMapByHandle(uint32_t aHandle);
    uint32_t GetLastMapHandle() const;
    TResult CreateWritableMap(CFrameworkEngine& aEngine,TWritableMapType aType,CString aFileName = nullptr);
    TResult SaveMap(uint32_t aHandle,const CString& aFileName,TFileType aFileType);
    TResult ReadMap(uint32_t aHandle,const CString& aFileName,TFileType aFileType);
    bool MapIsEmpty(uint32_t aHandle);
    std::unique_ptr<CMap> CreateMap(CFrameworkEngine& aEngine,int32_t aViewWidth,int32_t aViewHeight);
    uint32_t GetMainMapHandle() const;
    uint32_t GetMemoryMapHandle() const;
    size_t MapCount() const;
    uint32_t MapHandle(size_t aIndex) const;
    bool MapIsWritable(size_t aIndex) const;
    std::vector<CString> LayerNames();
    
    TResult InsertMapObject(uint32_t aMapHandle,TMapObjectType aType,const CString& aLayerName,const MPath& aGeometry,
                            const CString& aStringAttributes,int32_t aIntAttribute,uint64_t& aId,bool aReplace);
    TResult InsertPointMapObject(uint32_t aMapHandle,const CString& aLayerName,TPoint aPoint,
                                 const CString& aStringAttributes,int32_t aIntAttribute,uint64_t& aId,bool aReplace);
    TResult InsertCircleMapObject(uint32_t aMapHandle,const CString& aLayerName,TPoint aCenter,int32_t aRadius,
                                  const CString& aStringAttributes,int32_t aIntAttribute,uint64_t& aId,bool aReplace);
    TResult InsertEnvelopeMapObject(uint32_t aMapHandle,const CString& aLayerName,const MPath& aGeometry,int32_t aRadius,
                                    const CString& aStringAttributes,int32_t aIntAttribute,uint64_t& aId,bool aReplace);
    TResult DeleteMapObjects(uint32_t aMapHandle,uint64_t aStartId,uint64_t aEndId,uint64_t& aDeletedCount,CString aCondition = nullptr);
    std::unique_ptr<CMapObject> LoadMapObject(TResult& aError,uint32_t aMapHandle,uint64_t aId);
    TResult ReadGpx(uint32_t aMapHandle,const CString& aFileName);
    std::string Proj4Param() const;

    // For internal use only.
    std::shared_ptr<CMapDataBaseArray> MapDataBaseArray() const { return iMapDataBaseArray; }
    CMapDataBase& MainDb() const { return *(*iMapDataBaseArray)[0]; }

    private:
    CFrameworkMapDataSet();
    CFrameworkMapDataSet(const CFrameworkMapDataSet&) = delete;
    CFrameworkMapDataSet(CFrameworkMapDataSet&&) = delete;
    CFrameworkMapDataSet& operator=(const CFrameworkMapDataSet&) = delete;
    CFrameworkMapDataSet& operator=(const CFrameworkMapDataSet&&) = delete;
    
    CMapDataBase* GetMapDb(uint32_t aHandle);
    void RecalculateOverlapPaths();

    std::shared_ptr<CMapDataBaseArray> iMapDataBaseArray;
    std::unique_ptr<CTileDataAccessor> iTileDataAccessor;
    uint32_t iLastMapHandle;
    uint32_t iMemoryMapHandle;
    };

/**
An observer interface which receives notifications
of changes to the framework state. It is intended for the use
of higher-level GUI objects which need to update their display
after framework state has been changed programmatically. For example,
if a route is created, dynamic map objects need to be redrawn.
*/
class MFrameworkObserver
    {
    public:
    virtual ~MFrameworkObserver() { }

    /**
    This virtual function is called when the map view changes,
    which can be caused by panning, zooming, rotation, moving to a
    new location, enabling or disabling a layer, or resizing the map.
    */
    virtual void OnViewChange() = 0;
    
    /**
    This virtual function is called when the map data changes,
    which can be caused by loading a new map, unloading a map,
    or enabling or disabling a map.
    */
    virtual void OnMainDataChange() = 0;
    
    /**
    This virtual function is called when the dynamic data changes,
    which can be caused by creating or deleting a route, or
    inserting or deleting a pushpin or other dynamic map object.
    */
    virtual void OnDynamicDataChange() = 0;

    /** This virtual function is called when the style sheet is changed. */
    virtual void OnStyleChange() = 0;

    /** This virtual function is called when layers are enabled or disabled, or the drawing of 3D buildings is enabled or disabled. */
    virtual void OnLayerChange() = 0;

    /** This virtual function is called when the notices such as the legend, scale bar and copyright notice are changed, enabled or disabled. */
    virtual void OnNoticeChange() = 0;
    };

/** Parameters giving detailed control of the perspective view. */
class TPerspectiveParam
    {
    public:
    TResult ReadFromXml(const char* aText);
    std::string ToXml() const;

    bool operator==(const TPerspectiveParam& aP) const
        {
        return std::forward_as_tuple(iPositionDegrees,iAutoPosition,iHeightInMeters,iAzimuthDegrees,iAutoAzimuth,iDeclinationDegrees,iRotationDegrees,iFieldOfViewDegrees) ==
               std::forward_as_tuple(aP.iPositionDegrees,aP.iAutoPosition,aP.iHeightInMeters,aP.iAzimuthDegrees,aP.iAutoAzimuth,aP.iDeclinationDegrees,aP.iRotationDegrees,aP.iFieldOfViewDegrees);
        }
    bool operator!=(const TPerspectiveParam& aP) const
        {
        return !(*this == aP);
        }
    bool operator<(const TPerspectiveParam& aP) const
        {
        return std::forward_as_tuple(iPositionDegrees,iAutoPosition,iHeightInMeters,iAzimuthDegrees,iAutoAzimuth,iDeclinationDegrees,iRotationDegrees,iFieldOfViewDegrees) <
               std::forward_as_tuple(aP.iPositionDegrees,aP.iAutoPosition,aP.iHeightInMeters,aP.iAzimuthDegrees,aP.iAutoAzimuth,aP.iDeclinationDegrees,aP.iRotationDegrees,aP.iFieldOfViewDegrees);
        }

    /** The position of the point on the terrain below the camera, in degrees longitude (x) and latitude (y). */
    TPointFP iPositionDegrees;
    /** If true, ignore iPosition, and set the camera position so that the location in the center of the display is shown. */
    bool iAutoPosition = true;
    /** The height of the camera above the terrain. The value 0 causes a default value to be used, which is the width of the display. */
    double iHeightInMeters = 0;
    /** The azimuth of the camera in degrees going clockwise, where 0 is N, 90 is E, etc. */
    double iAzimuthDegrees = 0;
    /** If true, ignore iAzimuthDegrees and use the current map orientation. */
    bool iAutoAzimuth = true;
    /** The declination of the camera downward from the horizontal plane. Values are clamped to the range -90...90. */
    double iDeclinationDegrees = 30;
    /** The amount by which the camera is rotated about its axis, after applying the declination, in degrees going clockwise. */
    double iRotationDegrees = 0;
    /** The camera's field of view in degrees. */
    double iFieldOfViewDegrees = 22.5;
    };

/** The view state, which can be retrieved or set so that maps can be recreated. */
class TViewState
    {
    public:
    bool operator==(const TViewState& aOther) const;
    bool operator<(const TViewState& aOther) const;
    bool operator!=(const TViewState& aOther) const { return !(*this == aOther); };
    TResult ReadFromXml(const char* aText);
    std::string ToXml() const;

    /** The display width in pixels. */
    int32_t iWidthInPixels = 256;
    /** The display height in pixels. */
    int32_t iHeightInPixels = 256;
    /** The view center in degrees longitude (x) and latitude (y). */
    TPointFP iViewCenterDegrees;
    /** The denominator of the scale fraction; e.g., 50000 for 1:50000. */
    double iScaleDenominator = 0;
    /** The clockwise rotation of the view in degrees. */
    double iRotationDegrees = 0;
    /** True if perspective mode is on. */
    bool iPerspective = false;
    /** The parameters to be used for perspective mode. */
    TPerspectiveParam iPerspectiveParam;

    private:
    auto Tuple() const { return std::forward_as_tuple(iWidthInPixels,iHeightInPixels,iViewCenterDegrees,iScaleDenominator,iRotationDegrees,iPerspective,iPerspectiveParam); }
    };

/**
The CFramework class provides a high-level API for CartoType.
through which map data can be loaded, maps can be created and viewed,
and routing and navigation can be performed.

It owns or references a CFrameworkEngine and a CFrameworkMapDataSet.

It is the 'view' part of the model-view-controller pattern. If there is only
one model it owns the engine and map data set; otherwise it can share
them with other CFramework objects.
*/
class CFramework: public MNavigatorObserver
    {
    public:
    static std::unique_ptr<CFramework> New(TResult& aError,
                                           const CString& aMapFileName,
                                           const CString& aStyleSheetFileName,
                                           const CString& aFontFileName,
                                           int32_t aViewWidth,int32_t aViewHeight,
                                           const std::string* aEncryptionKey = nullptr);
    static std::unique_ptr<CFramework> New(TResult& aError,
                                           std::shared_ptr<TTileParam> aTileParam,
                                           const CString& aStyleSheetFileName,
                                           const CString& aFontFileName,
                                           int32_t aViewWidth,int32_t aViewHeight);
    static std::unique_ptr<CFramework> New(TResult& aError,
                                           std::shared_ptr<CFrameworkEngine> aSharedEngine,
                                           std::shared_ptr<CFrameworkMapDataSet> aSharedMapDataSet,
                                           const CString& aStyleSheetFileName,
                                           int32_t aViewWidth,int32_t aViewHeight,
                                           const std::string* aEncryptionKey = nullptr);

    /**
    Parameters for creating a framework when more detailed control is needed.
    For example, file buffer size and the maximum number of buffers can be set.
    */
    class TParam
        {
        public:
        /** The first map. Must not be empty. */
        CString iMapFileName;
        /** The first style sheet. If this string is empty, the style sheet must be supplied in iStyleSheetText. */
        CString iStyleSheetFileName;
        /** The style sheet text; used if iStyleSheetFileName is empty. */
        std::string iStyleSheetText;
        /** The first font file. If this is empty, a small built-in font is loaded containing the Roman script only. */
        CString iFontFileName;
        /** The width of the initial map in pixels. Must be greater than zero. */
        int32_t iViewWidth = 256;
        /** The height of the initial map in pixels. Must be greater than zero. */
        int32_t iViewHeight = 256;
        /** If not empty, an encryption key to be used when loading the first map. */
        std::string iEncryptionKey;
        /** The file buffer size in bytes. If it is less than four the default value is used. */
        int32_t iFileBufferSizeInBytes = 0;
        /** The maximum number of file buffers. If it is zero or less the default value is used. */
        int32_t iMaxFileBufferCount = 0;
        /**
        The number of levels of the text index to load into RAM.
        Use values from 2 to 5 to make text searches faster, at the cost of using much more RAM.
        The value 0 causes the default number of levels to be loaded, which is 1.
        The value -1 disables text index loading.
        */
        int32_t iTextIndexLevels = 0;
        /** If non-null, use this shared engine and do not use iMapFileName or iFontFileName. */
        std::shared_ptr<CFrameworkEngine> iSharedEngine;
        /** If non-null, use this shared dataset and do not use iMapFileName or iFontFileName. */
        std::shared_ptr<CFrameworkMapDataSet> iSharedMapDataSet;
        /** If non-null, use these tile parameters and do not use iMapFileName. */
        std::shared_ptr<TTileParam> iTileParam;
        /**
        If true, maps are allowed to overlap.
        If false, maps are clipped so that they do not overlap maps previously loaded.
        */
        bool iMapsOverlap = true;
        };
    static std::unique_ptr<CFramework> New(TResult& aError,const TParam& aParam);

    ~CFramework();

    std::unique_ptr<CFramework> Copy(TResult& aError,bool aFull = true) const;

    // setting the navigator observer so that it can receive notifications of view changes
    void SetFrameworkObserver(std::weak_ptr<MFrameworkObserver> aObserver);

    // general
    void License(const CString& aKey);
    void License(const uint8_t* aKey,size_t aKeyLength);
    std::string Licensee() const;
    std::string ExpiryDate() const;
    std::string AppBuildDate() const;
    uint8_t License() const;
    CString Copyright() const;
    void SetCopyrightNotice();
    void SetCopyrightNotice(const CString& aCopyright);
    void SetLegend(std::unique_ptr<CLegend> aLegend,double aWidth,const char* aUnit,const TExtendedNoticePosition& aPosition);
    void SetLegend(const CLegend& aLegend,double aWidth,const char* aUnit,const TExtendedNoticePosition& aPosition);
    void EnableLegend(bool aEnable);
    void SetScaleBar(bool aMetricUnits,double aWidth,const char* aUnit,const TExtendedNoticePosition& aPosition);
    void EnableScaleBar(bool aEnable);
    void SetTurnInstructions(bool aMetricUnits,bool aAbbreviate,double aWidth,const char* aUnit,const TExtendedNoticePosition& aPosition,double aTextSize = 7,const char* aTextSizeUnit = "pt");
    void EnableTurnInstructions(bool aEnable);
    void SetTurnInstructionText(const CString& aText);
    CString TurnInstructionText();
    void DrawNoticesAutomatically(bool aEnable);
    bool HasNotices() const;
    CPositionedBitmap GetNoticeBitmap();
    TResult Configure(const CString& aFilename);
    TResult LoadMap(const CString& aMapFileName,const std::string* aEncryptionKey = nullptr);
    TResult LoadMap(const TTileParam& aTileParam);
    bool SetMapsOverlap(bool aEnable);
    TResult CreateWritableMap(TWritableMapType aType,CString aFileName = nullptr);
    TResult SaveMap(uint32_t aHandle,const CString& aFileName,TFileType aFileType);
    TResult ReadMap(uint32_t aHandle,const CString& aFileName,TFileType aFileType);
    TResult WriteMapImage(const CString& aFileName,TFileType aFileType,bool aCompress = false);
    bool MapIsEmpty(uint32_t aHandle);
    size_t MapCount() const;
    uint32_t MapHandle(size_t aIndex) const;
    bool MapIsWritable(size_t aIndex) const;
    TResult UnloadMapByHandle(uint32_t aHandle);
    void EnableMapByHandle(uint32_t aHandle,bool aEnable);
    void EnableAllMaps();
    uint32_t GetLastMapHandle() const;
    uint32_t GetMainMapHandle() const;
    uint32_t GetMemoryMapHandle() const;
    TResult LoadFont(const CString& aFontFileName);
    TResult LoadFont(const uint8_t* aData,size_t aLength,bool aCopyData);
    TResult SetStyleSheet(const CString& aStyleSheetFileName,size_t aIndex = 0);
    TResult SetStyleSheet(const uint8_t* aData,size_t aLength,size_t aIndex = 0);
    TResult SetStyleSheet(const CStyleSheetData& aStyleSheetData,size_t aIndex = 0);
    TResult SetStyleSheet(const CStyleSheetDataArray& aStyleSheetDataArray,const CVariableDictionary* aStyleSheetVariables = nullptr);
    TResult ReloadStyleSheet(size_t aIndex = 0);
    void AppendStyleSheet(const CString& aStyleSheetFileName);
    void AppendStyleSheet(const uint8_t* aData,size_t aLength);
    TResult DeleteStyleSheet(size_t aIndex);
    std::string GetStyleSheetText(size_t aIndex) const;
    CStyleSheetData GetStyleSheetData(size_t aIndex) const;
    const CStyleSheetDataArray& GetStyleSheetDataArray() const;
    const CVariableDictionary& GetStyleSheetVariables() const;

    void Resize(int32_t aViewWidth,int32_t aViewHeight);
    void SetResolutionDpi(double aDpi);
    double ResolutionDpi() const;
    void SetScaleDenominator(double aScale);
    double ScaleDenominator() const;
    double MapUnitSize() const;
    void SetScaleDenominatorInView(double aScale);
    double GetScaleDenominatorInView() const;
    double GetDistanceInMeters(double aX1,double aY1,double aX2,double aY2,TCoordType aCoordType) const;
    double ScaleDenominatorFromZoomLevel(double aZoomLevel,int32_t aImageSizeInPixels = 256) const;
    double ZoomLevelFromScaleDenominator(double aScaleDenominator,int32_t aImageSizeInPixels = 256) const;
    
    void SetPerspective(bool aSet);
    void SetPerspective(const TPerspectiveParam& aParam);
    bool Perspective() const;
    TPerspectiveParam PerspectiveParam() const;
    void Zoom(double aZoomFactor);
    TResult ZoomAt(double aZoomFactor,double aX,double aY,TCoordType aCoordType);
    void Rotate(double aAngle);
    TResult RotateAt(double aAngle,double aX,double aY,TCoordType aCoordType);
    void SetRotation(double aAngle);
    TResult SetRotationAt(double aAngle,double aX,double aY,TCoordType aCoordType);
    double Rotation() const;
    TResult RotateAndZoom(double aAngle,double aZoomFactor,double aX,double aY,TCoordType aCoordType);
    TResult Pan(int32_t aDx,int32_t aDy);
    TResult Pan(const TPoint& aFrom,const TPoint& aTo);
    TResult Pan(double aFromX,double aFromY,TCoordType aFromCoordType,double aToX,double aToY,TCoordType aToCoordType);
    TResult SetViewCenter(double aX,double aY,TCoordType aCoordType);
    TResult SetView(double aMinX,double aMinY,double aMaxX,double aMaxY,TCoordType aCoordType,int32_t aMarginInPixels = 0,int32_t aMinScaleDenominator = 0);
    TResult SetView(const CMapObject * const * aMapObjectArray,size_t aMapObjectCount,int32_t aMarginInPixels,int32_t aMinScaleDenominator);
    TResult SetView(const CMapObjectArray& aMapObjectArray,int32_t aMarginInPixels,int32_t aMinScaleDenominator);
    TResult SetView(const CMapObject& aMapObject,int32_t aMarginInPixels,int32_t aMinScaleDenominator);
    TResult SetView(const CGeometry& aGeometry,int32_t aMarginInPixels,int32_t aMinScaleDenominator);
    TResult SetView(const TViewState& aViewState);
    TResult SetViewToRoute(size_t aRouteIndex,int32_t aMarginInPixels,int32_t aMinScaleDenominator);
    TResult SetViewToWholeMap();
    TResult GetView(double& aMinX,double& aMinY,double& aMaxX,double& aMaxY,TCoordType aCoordType) const;
    TResult GetView(TRectFP& aView,TCoordType aCoordType) const;
    TResult GetView(TFixedSizeContourFP<4,true>& aView,TCoordType aCoordType) const;
    TResult GetMapExtent(double& aMinX,double& aMinY,double& aMaxX,double& aMaxY,TCoordType aCoordType) const;
    TResult GetMapExtent(TRectFP& aMapExtent,TCoordType aCoordType) const;
    CString GetProjectionAsProj4Param() const; 
    TViewState ViewState() const;
    
    TResult InsertMapObject(uint32_t aMapHandle,TMapObjectType aType,const CString& aLayerName,const CGeometry& aGeometry,
                            const CString& aStringAttributes,int32_t aIntAttribute,uint64_t& aId,bool aReplace);
    TResult InsertPointMapObject(uint32_t aMapHandle,const CString& aLayerName,double aX,double aY,
                                 TCoordType aCoordType,const CString& aStringAttributes,int32_t aIntAttribute,uint64_t& aId,bool aReplace);
    TResult InsertCircleMapObject(uint32_t aMapHandle,const CString& aLayerName,
                                  double aCenterX,double aCenterY,TCoordType aCenterCoordType,double aRadius,TCoordType aRadiusCoordType,
                                  const CString& aStringAttributes,int32_t aIntAttribute,uint64_t& aId,bool aReplace);
    TResult InsertEnvelopeMapObject(uint32_t aMapHandle,const CString& aLayerName,const CGeometry& aGeometry,
                                    double aRadius,TCoordType aRadiusCoordType,
                                    const CString& aStringAttributes,int32_t aIntAttribute,uint64_t& aId,bool aReplace);
    TResult InsertPushPin(double aX,double aY,TCoordType aCoordType,const CString& aStringAttrib,const CString& aColor,int32_t aIconCharacter,uint64_t& aId);
    TResult InsertCopyOfMapObject(uint32_t aMapHandle,const CString& aLayerName,const CMapObject& aObject,double aEnvelopeRadius,TCoordType aRadiusCoordType,uint64_t& aId,bool aReplace);
    TResult DeleteMapObjects(uint32_t aMapHandle,uint64_t aStartId,uint64_t aEndId,uint64_t& aDeletedCount,CString aCondition = nullptr);
    std::unique_ptr<CMapObject> LoadMapObject(TResult& aError,uint32_t aMapHandle,uint64_t aId);
    TResult ReadGpx(uint32_t aMapHandle,const CString& aFileName);
    CGeometry Range(TResult& aError,const TRouteProfile* aProfile,double aX,double aY,TCoordType aCoordType,double aTimeOrDistance,bool aIsTime);

    void EnableLayer(const CString& aLayerName,bool aEnable);
    bool LayerIsEnabled(const CString& aLayerName) const;
    void SetDisabledLayers(const std::set<CString>& aLayerNames);
    std::vector<CString> LayerNames();

    TResult ConvertCoords(double* aCoordArray,size_t aCoordArraySize,TCoordType aFromCoordType,TCoordType aToCoordType) const;
    TResult ConvertCoords(const TWritableCoordSet& aCoordSet,TCoordType aFromCoordType,TCoordType aToCoordType) const;
    TResult ConvertPoint(double& aX,double& aY,TCoordType aFromCoordType,TCoordType aToCoordType) const;
    double PixelsToMeters(double aPixels) const;
    double MetersToPixels(double aMeters) const;
    CString DataSetName() const;

    // drawing the map
    const TBitmap* MapBitmap(TResult& aError,bool* aRedrawWasNeeded = nullptr);
    const TBitmap* LabelBitmap(TResult& aError,bool* aRedrawWasNeeded = nullptr);
    const TBitmap* MemoryDataBaseMapBitmap(TResult& aError,bool* aRedrawWasNeeded = nullptr);
    void DrawNotices(CGraphicsContext& aGc);
    void EnableDrawingMemoryDataBase(bool aEnable);
    size_t ObjectsDrawn() const;
    void ForceRedraw();
    void CancelDrawing();
    void UseImageServer(MImageServerHelper* aImageServerHelper,bool aDrawMemoryDataBase = false,int32_t aCacheSize = KDefaultImageCacheSize);
    void UseGcImageServer(bool aEnable,int32_t aCacheSize = KDefaultImageCacheSize);
    TResult DrawUsingImageServer(void* aDeviceContext);
    bool UsingImageServer() const;
    bool ClipBackgroundToMapBounds(bool aEnable);
    bool DrawBackground(bool aEnable);
    int32_t SetTileOverSizeZoomLevels(int32_t aLevels);
    TResult DrawLabelsToLabelHandler(MLabelHandler& aLabelHandler);
    bool ObjectWouldBeDrawn(TResult& aError,uint64_t aId,TMapObjectType aType,const CString& aLayer,int32_t aIntAttrib,const CString& aStringAttrib) const;
    bool SetDraw3DBuildings(bool aEnable);
    bool Draw3DBuildings() const;
    bool SetAnimateTransitions(bool aEnable);
    bool AnimateTransitions() const;

    // adding and removing style sheet icons loaded from files
    TResult LoadIcon(const CString& aFileName,const CString& aId,const TPoint& aHotSpot,const TPoint& aLabelPos,int32_t aLabelMaxLength);
    void UnloadIcon(const CString& aId);
    
    // using a web map service to get an underlay or overlay for the current map
    TResult UseWebMapService(MInternetAccessor& aInternetAccessor,const std::string& aUrl,const std::string& aUrlSuffix);
    const TBitmap* WebMapServiceBitmap(TResult& aError,bool* aRedrawWasNeeded = nullptr);
    
    // providing data for a web map service
    TResult WebMapServiceGetMap(const char* aWebMapServiceRequest,std::vector<uint8_t>& aData);

    // drawing Mercator, Plate Carree or arbitrary tiles
    CBitmap TileBitmap(TResult& aError,int32_t aTileSizeInPixels,int32_t aZoom,int32_t aX,int32_t aY,const TTileBitmapParam* aParam = nullptr);
    CBitmap TileBitmap(TResult& aError,int32_t aTileSizeInPixels,const CString& aQuadKey,const TTileBitmapParam* aParam = nullptr);
    CBitmap TileBitmap(TResult& aError,int32_t aTileWidth,int32_t aTileHeight,const TRectFP& aBounds,TCoordType aCoordType,const TTileBitmapParam* aParam = nullptr);

    // finding map objects
    TResult Find(CMapObjectArray& aObjectArray,const TFindParam& aFindParam) const;
    TResult Find(CMapObjectGroupArray& aObjectGroupArray,const TFindParam& aFindParam) const;
    TResult FindInDisplay(CMapObjectArray& aObjectArray,size_t aMaxObjectCount,double aX,double aY,double aRadius) const;
    TResult FindInLayer(CMapObjectArray& aObjectArray,size_t aMaxObjectCount,
                        const CString& aLayer,double aMinX,double aMinY,double aMaxX,double aMaxY,TCoordType aCoordType) const;
    TResult FindText(CMapObjectArray& aObjectArray,size_t aMaxObjectCount,const CString& aText,
                     TStringMatchMethod aMatchMethod,const CString& aLayers,const CString& aAttribs) const;
    TResult FindAddress(CMapObjectArray& aObjectArray,size_t aMaxObjectCount,const CAddress& aAddress,bool aFuzzy = false) const;
    TResult FindStreetAddresses(CMapObjectArray& aObjectArray,size_t aMaxObjectCount,const CAddress& aAddress,const CGeometry* aClip = nullptr) const;
    TResult FindAddressPart(CMapObjectArray& aObjectArray,size_t aMaxObjectCount,const CString& aText,TAddressPart aAddressPart,bool aFuzzy,bool aIncremental) const;
    TResult FindBuildingsNearStreet(CMapObjectArray& aObjectArray,const CMapObject& aStreet) const;
    TResult FindPolygonsContainingPath(CMapObjectArray& aObjectArray,const CGeometry& aPath,const TFindParam* aParam = nullptr) const;
    TResult FindPointsInPath(CMapObjectArray& aObjectArray,const CGeometry& aPath,const TFindParam* aParam = nullptr) const;

    // geocoding
    TResult GeoCodeSummary(CString& aSummary,const CMapObject& aMapObject) const;
    TResult GeoCodeSummary(CString& aSummary,double aX,double aY,TCoordType aCoordType) const;
    TResult GetAddress(CAddress& aAddress,const CMapObject& aMapObject) const;
    TResult GetAddressFast(CAddress& aAddress,const CMapObject& aMapObject) const;
    TResult GetAddress(CAddress& aAddress,double aX,double aY,TCoordType aCoordType,bool aFullAddress = true) const;

    // getting information about a map object, taking account of the framework's projection
    double GetMapObjectArea(TResult& aError,const CMapObject& aMapObject) const;
    double GetMapObjectLengthOrPerimeter(TResult& aError,const CMapObject& aMapObject) const;
    double GetMapObjectDistanceFromPoint(TResult& aError,const CMapObject& aMapObject,
                                         double aPointX,double aPointY,TCoordType aPointCoordType,
                                         double& aNearestPointX,double& aNearestPointY,TCoordType aNearestPointCoordType) const;

    // terrain heights
    TResult GetHeights(const TCoordSet& aCoordSet,TCoordType aCoordType,int32_t* aTerrainHeightArray) const;
    TResult GetHeightProfile(const double* aX,const double* aY,int32_t* aTerrainHeightArray,int32_t* aObstacleHeightArray,size_t aCoordCount,TCoordType aCoordType,
                             double aRadiusInMetres,CMapObjectArray* aObstacleArray) const;

    // style sheet variables
    void SetStyleSheetVariable(const CString& aVariableName,const CString& aValue);
    void SetStyleSheetVariable(const CString& aVariableName,int32_t aValue);
    
    // access to graphics
    std::unique_ptr<CGraphicsContext> CreateGraphicsContext(int32_t aWidth,int32_t aHeight);
    TFont Font(const TFontSpec& aFontSpec);
    CGraphicsContext& GetMapGraphicsContext();

    /** The default size of the cache used by the image server. */
    static constexpr size_t KDefaultImageCacheSize = 10 * 1024 * 1024;

    // navigation
    static constexpr size_t KMaxRoutesDisplayed = 16;    // allow a number of alternative routes well in excess of the expected maximum, which is probably 3
    static constexpr size_t KMaxWayPointsDisplayed = 1024 * 1024;    // allow up to a million waypoints; in practice there are likely to be no more than 100

    /** The first ID reserved for route-related map objects. */
    static constexpr uint64_t KRouteIdReservedStart = 0x40000000;

    /** A reserved map object ID for the first route. */
    static constexpr uint64_t KFirstRouteId = KRouteIdReservedStart;
    /** A reserved map object ID for the last route. */
    static constexpr uint64_t KLastRouteId = KRouteIdReservedStart + KMaxRoutesDisplayed - 1;
    /** A reserved map object ID for the start of the route. */
    static constexpr uint64_t KRouteStartId = KLastRouteId + 1;
    /** A reserved map object ID for the end of the route. */
    static constexpr uint64_t KRouteEndId = KLastRouteId + 2;
    /** A reserved map object ID for a point object at the current position on the route. */
    static constexpr uint64_t KRoutePositionId = KLastRouteId + 3;
    /** A reserved map object ID for a vector object centered on the current position on the route. */
    static constexpr uint64_t KRouteVectorId = KLastRouteId + 4;
    /** A reserved map object ID for the first waypoint. */
    static constexpr uint64_t KFirstWayPointId = KLastRouteId + 5;
    /** One more than the last ID reserved for route-related map objects. */
    static constexpr uint64_t KRouteIdReservedEnd = KFirstWayPointId + KMaxWayPointsDisplayed;
    
    void SetPreferredRouterType(TRouterType aRouterType);
    TRouterType PreferredRouterType() const;
    TRouterType ActualRouterType() const;
    TResult StartNavigation(double aStartX,double aStartY,TCoordType aStartCoordType,
                            double aEndX,double aEndY,TCoordType aEndCoordType);
    TResult StartNavigation(const TRouteCoordSet& aCoordSet);
    TResult StartNavigation(const TCoordSet& aCoordSet,TCoordType aCoordType);
    void EndNavigation();
    bool EnableNavigation(bool aEnable);
    bool NavigationEnabled() const;
    TResult LoadNavigationData();
    bool NavigationDataHasGradients() const;
    void SetMainProfile(const TRouteProfile& aProfile);
    size_t BuiltInProfileCount();
    const TRouteProfile* BuiltInProfile(size_t aIndex);
    TResult SetBuiltInProfile(size_t aIndex);
    void AddProfile(const TRouteProfile& aProfile);
    TResult ChooseRoute(size_t aRouteIndex);
    const TRouteProfile* Profile(size_t aIndex) const;
    bool Navigating() const;
    void AddNavigatorObserver(std::weak_ptr<MNavigatorObserver> aObserver);
    void RemoveNavigatorObserver(std::weak_ptr<MNavigatorObserver> aObserver);
    TPoint RouteStart();
    TPoint RouteEnd();
    TResult DisplayRoute(bool aEnable = true);
    const CRoute* Route() const; 
    const CRoute* Route(size_t aIndex) const;
    std::unique_ptr<CRoute> CreateRoute(TResult& aError,const TRouteProfile& aProfile,const TRouteCoordSet& aCoordSet);
    std::unique_ptr<CRoute> CreateRoute(TResult& aError,const TRouteProfile& aProfile,const TCoordSet& aCoordSet,TCoordType aCoordType);
    std::unique_ptr<CRoute> CreateBestRoute(TResult& aError,const TRouteProfile& aProfile,const TRouteCoordSet& aCoordSet,bool aStartFixed,bool aEndFixed,size_t aIterations);
    std::unique_ptr<CRoute> CreateBestRoute(TResult& aError,const TRouteProfile& aProfile,const TCoordSet& aCoordSet,TCoordType aCoordType,bool aStartFixed,bool aEndFixed,size_t aIterations);
    std::unique_ptr<CRoute> CreateRouteFromXml(TResult& aError,const TRouteProfile& aProfile,const CString& aFileNameOrData);
    CString RouteInstructions(const CRoute& aRoute) const;
    TResult UseRoute(const CRoute& aRoute,bool aReplace);
    TResult ReadRouteFromXml(const CString& aFileNameOrData,bool aReplace);
    TResult WriteRouteAsXml(const CRoute& aRoute,const CString& aFileName,TFileType aFileType = TFileType::CTROUTE) const;
    TResult WriteRouteAsXmlString(const CRoute& aRoute,std::string& aXmlString,TFileType aFileType = TFileType::CTROUTE) const;
    const CRouteSegment* CurrentRouteSegment() const;
    const CRouteSegment* NextRouteSegment() const;
    size_t RouteCount() const;
    TResult ReverseRoutes();
    TResult DeleteRoutes();
    TResult Navigate(const TNavigationData& aNavData);
    const TNavigatorTurn& FirstTurn() const;
    const TNavigatorTurn& SecondTurn() const;
    const TNavigatorTurn& ContinuationTurn() const;
    TNavigationState NavigationState() const;
    void SetNavigatorParam(const TNavigatorParam& aParam);
    TNavigatorParam NavigatorParam() const;
    void SetNavigationMinimumFixDistance(int32_t aMeters);
    void SetNavigationTimeOffRouteTolerance(int32_t aSeconds);
    void SetNavigationDistanceOffRouteTolerance(int32_t aMeters);
    void SetNavigationAutoReRoute(bool aAutoReRoute);
    TResult AddNearbyObjectWarning(const CString& aLayer,double aMaxDistanceToRoute,double aMaxDistanceAlongRoute);
    TResult DeleteNearbyObjectWarning(const CString& aLayer);
    TResult CopyNearbyObjects(const CString& aLayer,CMapObjectArray& aObjectArray,int32_t aMaxObjectCount);
    double DistanceToDestination();
    double EstimatedTimeToDestination();
    void UseSerializedNavigationData(bool aEnable);
    TResult FindNearestRoad(TNearestRoadInfo& aInfo,double aX,double aY,TCoordType aCoordType,double aHeadingInDegrees,const TLocationMatchParam& aParam,bool aDisplayPosition);
    TResult DisplayPositionOnNearestRoad(const TNavigationData& aNavData,TNearestRoadInfo* aInfo = nullptr,double aMaxDistanceInMeters = 100);
    TResult SetVehiclePosOffset(double aXOffset,double aYOffset);
    TResult SetFollowMode(TFollowMode aFollowMode);
    TFollowMode FollowMode() const;
    TResult GetNavigationPosition(TPointFP& aPos,TCoordType aCoordType) const;
    void GetNavigationData(TNavigationData& aData,double& aHeading) const;
    
    // locales
    void SetLocale(const char* aLocale);
    std::string Locale() const;

    // locale-dependent and configuration-dependent string handling
    void AppendDistance(MString& aString,double aDistanceInMeters,bool aMetricUnits,bool aAbbreviate = false);
    void AppendTime(MString& aString,double aTimeInSeconds);
    void SetCase(MString& aString,TLetterCase aCase);
    void AbbreviatePlacename(MString& aString);

    // traffic information and general location referencing
    TResult AddTrafficInfo(uint64_t& aId,const CTrafficInfo& aTrafficInfo,CLocationRef& aLocationRef,bool aDriveOnLeft);
    TResult AddPolygonSpeedLimit(uint64_t& aId,const CGeometry& aPolygon,double aSpeed,uint32_t aVehicleTypes);
    TResult AddLineSpeedLimit(uint64_t& aId,const CGeometry& aLine,double aSpeed,uint32_t aVehicleTypes,bool aDriveOnLeft);
    TResult AddClosedLineSpeedLimit(uint64_t& aId,const CGeometry& aLine,double aSpeed,uint32_t aVehicleTypes);
    TResult AddForbiddenArea(uint64_t& aId,const CGeometry& aPolygon);
    TResult DeleteTrafficInfo(uint64_t aId);
    void ClearTrafficInfo();
    TResult WriteTrafficMessageAsXml(MOutputStream& aOutput,const CTrafficInfo& aTrafficInfo,CLocationRef& aLocationRef);
    TResult WriteLineTrafficMessageAsXml(MOutputStream& aOutput,const CTrafficInfo& aTrafficInfo,const CString& aId,const CRoute& aRoute);
    TResult WriteClosedLineTrafficMessageAsXml(MOutputStream& aOutput,const CTrafficInfo& aTrafficInfo,const CString& aId,const CRoute& aRoute);
    bool EnableTrafficInfo(bool aEnable);

    // functions for internal use only
    TResult CompileStyleSheet(std::shared_ptr<CMapStyle>& aStyleSheet,double aScale);
    std::unique_ptr<CMapStore> NewMapStore(const CMapStyle& aStyleSheet,const TRect& aBounds,bool aUseFastAllocator);
    const CMapDataBase& MainDb() const { return iMapDataSet->MainDb(); }
    TTransform3FP MapTransform() const;
    TTransformFP MapTransform2D() const;
    TTransform3FP PerspectiveTransform() const;
    std::shared_ptr<CEngine> Engine() const { return iEngine->Engine(); }
    CMap& Map() const { return *iMap; }
    TColor OutlineColor() const;
    CGraphicsContext& Gc() const;
    std::unique_ptr<CFramework> CreateLegendFramework(TResult& aError) const;
    std::unique_ptr<CBitmap> CreateBitmapFromSvg(TResult& aError,MInputStream& aInputStream,TFileLocation& aErrorLocation,int32_t aForcedWidth = 0);
    void SetUserData(std::shared_ptr<MUserData> aUserData) { iUserData = aUserData; }
    void SetLabelUpAngleInRadians(double aAngle);
    void SetLabelUpVector(TPointFP aVector);
    TPointFP LabelUpVector() const;
    size_t RouteCalculationCost() const;
    CMapDrawParam& MapDrawParam() const { return *iMapDrawParam; }
    double PolygonArea(const TCoordSet& aCoordSet,TCoordType aCoordType);
    double Pixels(double aSize,const char* aUnit) const;

    private:
    CFramework();
    
    CFramework(const CFramework&) = delete;
    CFramework(CFramework&&) = delete;
    void operator=(const CFramework&) = delete;
    void operator=(CFramework&&) = delete;

    TResult Construct(const TParam& aParam);
    void HandleChangedMapData();
    void InvalidateMapBitmaps() { iMapBitmapType = TMapBitmapType::None; iWebMapServiceBitmap = nullptr; }
    void HandleChangedView() { InvalidateMapBitmaps(); ViewChanged(); }
    void HandleChangedLayer() { InvalidateMapBitmaps(); LayerChanged(); }
    TResult CreateTileServer(int32_t aTileWidthInPixels,int32_t aTileHeightInPixels);
    TResult SetRoutePositionAndVector(const TPoint& aPos,const TPoint& aVector);
    TResult ConvertCoordSetToMapCoords(TRouteCoordSet& aRouteCoordSet);
    TResult CreateNavigator();
    std::unique_ptr<CRoute> CreateRouteHelper(TResult& aError,bool aBest,const TRouteProfile& aProfile,const TRouteCoordSet& aCoordSet,bool aStartFixed,bool aEndFixed,size_t aIterations);
    void SetCameraParam(TCameraParam& aCameraParam,double aViewWidth,double aViewHeight);

    // Notifying the framework observer.
    void ViewChanged() { auto p = iFrameworkObserver.lock(); if (p) p->OnViewChange(); }
    void MainDataChanged() { auto p = iFrameworkObserver.lock(); if (p) p->OnMainDataChange(); }
    void DynamicDataChanged() { auto p = iFrameworkObserver.lock(); if (p) p->OnDynamicDataChange(); }
    void StyleChanged() { auto p = iFrameworkObserver.lock(); if (p) p->OnStyleChange(); }
    void LayerChanged() { auto p = iFrameworkObserver.lock(); if (p) p->OnLayerChange(); }
    void NoticeChanged() { auto p = iFrameworkObserver.lock(); if (p) p->OnNoticeChange(); }

    // virtual functions from MNavigatorObserver
    void OnRoute(const CRoute* aRoute) override;
    void OnTurn(const TNavigatorTurn& aFirstTurn,
                const TNavigatorTurn* aSecondTurn,
                const TNavigatorTurn* aContinuationTurn) override;
    void OnState(TNavigationState aState) override;
    void OnAddNearbyObject(const CMapObject* aObject,double aDistanceToRoute,double aDistanceAlongRoute) override;
    void OnUpdateNearbyObject(const CMapObject* aObject,double aDistanceToRoute,double aDistanceAlongRoute) override;
    void OnRemoveNearbyObject(const CMapObject* aObject) override;
    
    void DeleteNullNavigatorObservers();
    void ClearStyleSheet();
    void ClearTurns();

    std::shared_ptr<CFrameworkEngine> iEngine;
    std::shared_ptr<CFrameworkMapDataSet> iMapDataSet;
    std::vector<CStyleSheetData> iStyleSheetData;
    std::unique_ptr<CMap> iMap;
    std::unique_ptr<CMapDrawParam> iMapDrawParam;
    std::unique_ptr<C32BitColorBitmapGraphicsContext> iGc;
    std::unique_ptr<CPerspectiveGraphicsContext> iPerspectiveGc;
    std::weak_ptr<MFrameworkObserver> iFrameworkObserver;
    TPerspectiveParam iPerspectiveParam;

    enum class TMapBitmapType
        {
        None,   // the map bitmap is invalid
        Full,   // the map bitmap is valid
        Memory, // the map bitmap has memory map data only
        Label   // the map bitmap has labels only
        };

    TMapBitmapType iMapBitmapType = TMapBitmapType::None;
    bool iPerspective = false;
    bool iUseSerializedNavigationData = true;
    TRouterType iPreferredRouterType = TRouterType::Default;
    std::unique_ptr<CNavigator> iNavigator;
    std::vector<std::weak_ptr<MNavigatorObserver>> iNavigatorObservers;
    std::weak_ptr<CLegend> iTurnInstructionNotice;
    TNavigatorTurn iFirstTurn;
    TNavigatorTurn iSecondTurn;
    TNavigatorTurn iContinuationTurn;
    TNavigationState iNavigationState = TNavigationState::None;
    TNavigatorParam iNavigatorParam;
    std::vector<TRouteProfile> iRouteProfile;
    TPointFP iVehiclePosOffset;
    std::unique_ptr<CTileServer> iTileServer;
    int32_t iTileServerOverSizeZoomLevels = 1;
    std::unique_ptr<CImageServer> iImageServer;
    std::unique_ptr<CWebMapServiceClient> iWebMapServiceClient;
    std::unique_ptr<CBitmap> iWebMapServiceBitmap;
    std::unique_ptr<CGcImageServerHelper> iGcImageServerHelper;
    std::string iLocale;
    TFollowMode iFollowMode = TFollowMode::LocationHeadingZoom;
    std::shared_ptr<MUserData> iUserData;
    bool iMapsOverlap = true;
    };

/** A framework for finding map objects in a map, when the ability to draw the map is not needed. */
class CFindFramework
    {
    public:
    static std::unique_ptr<CFindFramework> New(TResult& aError,const CString& aMapFileName);
    TResult Find(CMapObjectArray& aObjectArray,const TFindParam& aFindParam);
    TResult FindRoads(CMapObjectArray& aObjectArray,int32_t aMaxObjectCount,double aLong,double aLat,double aRadiusInMeters);

    private:
    CFindFramework() = default;
    TResult Construct(const CString& aMapFileName);

    std::unique_ptr<CMap> iMap;
    };

#ifndef CARTOTYPE_NO_GL
/** A map renderer using OpenGL ES graphics acceleration. */
class CMapRenderer
    {
    public:
    CMapRenderer(CFramework& aFramework,const void* aNativeWindow = nullptr);
    ~CMapRenderer();
    void Draw();
    bool Valid() const;

    private:
    std::unique_ptr<CMapRendererImplementation> m_implementation;
    };
#endif

CString UKGridReferenceFromMapPoint(const TPointFP& aPointInMapMeters,int32_t aDigits);
CString UKGridReferenceFromDegrees(const TPointFP& aPointInDegrees,int32_t aDigits);
TRectFP MapRectFromUKGridReference(const CString& aGridReference);
TPointFP MapPointFromUKGridReference(const CString& aGridReference);
TPointFP PointInDegreesFromUKGridReference(const CString& aGridReference);
CString ExpandStreetName(const MString& aText);

}

#endif
