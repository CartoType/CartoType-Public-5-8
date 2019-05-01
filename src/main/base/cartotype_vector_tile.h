/*
cartotype_vector_tile.h
Copyright (C) 2015-2019 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_VECTOR_TILE_H__
#define	CARTOTYPE_VECTOR_TILE_H__

#include <cartotype_framework.h>
#include <queue>
#include <set>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <memory>
#include <atomic>

namespace CartoType
{

class CMapLevelStore;
class CVectorTileServer;
class CDrawTileTask;
class CDrawLabelTask;
class CStackAllocator;
class CTransformingGc;
class CIcon;

template<typename T> class TTaskOutputQueue
    {
    public:
    TTaskOutputQueue() = default;

    void Add(T aObject)
        {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& p : m_queue)
            {
            if (p == aObject)
                return;
            }
        m_queue.push_back(aObject);
        m_condition.notify_one();
        }

    T Remove()
        {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty())
            m_condition.wait(lock);
        T object = m_queue.front();
        m_queue.pop_front();
        return object;
        }

    T RemoveWithoutWaiting()
        {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty())
            return T();
        T object = m_queue.front();
        m_queue.pop_front();
        return object;
        }

    bool Empty() const
        {
        return m_queue.empty();
        }

    TTaskOutputQueue(const TTaskOutputQueue&) = delete;
    TTaskOutputQueue& operator=(const TTaskOutputQueue&) = delete;

    protected:
    std::deque<T> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_condition;
    };

template<typename T> class TTaskQueue
    {
    public:
    TTaskQueue() = default;

    void Add(T aRequest)
        {
        std::lock_guard<std::mutex> lock(m_mutex);

        // Remove requests superseded by this one.
        auto remove_iter = std::remove_if(m_queue.begin(),m_queue.end(),[&aRequest](const T& aP)->bool { return aRequest.Supersedes(aP); });
        m_queue.erase(remove_iter,m_queue.end());

        for (const auto& p : m_queue)
            if (p == aRequest)
                return;

        auto pending_iter = m_pending.find(aRequest);
        if (pending_iter != m_pending.end())
            return;

        m_queue.push_back(aRequest);
        m_condition.notify_one();
        }

    T StartTask()
        {
        std::unique_lock<std::mutex> lock(m_mutex);

        // Loop until a task is found that's not already being handled.
        for (;;)
            {
            while (m_queue.empty())
                m_condition.wait(lock);
            T object = m_queue.back(); // get the most recently added item; this is a LIFO queue
            m_queue.pop_back();
            if (m_pending.insert(object).second) // the second element of the return value is true if the object was inserted, and not already there
                return object;
            }
        }

    void EndTask(T aRequest)
        {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_pending.erase(aRequest);
        }

    bool Empty() const
        {
        return m_queue.empty();
        }

    TTaskQueue(const TTaskQueue&) = delete;
    TTaskQueue& operator=(const TTaskQueue&) = delete;

    protected:
    std::deque<T> m_queue;  // tasks not yet started
    std::set<T> m_pending;  // tasks currently being handled
    mutable std::mutex m_mutex;
    std::condition_variable m_condition;
    };

enum class TVectorDataType: int
    {
    Static,
    Dynamic
    };

class TTileSpec
    {
    auto Tuple() const { return std::forward_as_tuple(m_zoom,m_x,m_y,m_type,m_generation); }
    auto TupleWithoutGeneration() const { return std::forward_as_tuple(m_zoom,m_x,m_y,m_type); }

    public:
    bool operator==(const TTileSpec& aOther) const
        {
        return Tuple() == aOther.Tuple();
        }
    bool operator<(const TTileSpec& aOther) const
        {
        return Tuple() < aOther.Tuple();
        }
    bool operator%(const TTileSpec& aOther) const
        {
        return TupleWithoutGeneration() == aOther.TupleWithoutGeneration();
        }

    bool Supersedes(const TTileSpec& aOther) const
        {
        return m_zoom != aOther.m_zoom || m_generation > aOther.m_generation; // tile requests supersede requests for other zoom levels or previous generations
        }

    bool Contains(const TTileSpec& aOther) const
        {
        int shift = aOther.m_zoom - m_zoom;
        if (shift <= 0)
            return false;
        return (aOther.m_x >= (m_x << shift)) &&
               (aOther.m_x < ((m_x + 1) << shift)) &&
               (aOther.m_y >= (m_y << shift)) &&
               (aOther.m_y < ((m_y + 1) << shift));
        }

    int32_t m_zoom = 0;
    int32_t m_x = 0;
    int32_t m_y = 0;
    TVectorDataType m_type = TVectorDataType::Static;
    uint32_t m_generation = 0;
    };

class TLabelSetSpec: public TViewState
    {
    public:
    TLabelSetSpec() = default;
    TLabelSetSpec(const TViewState& aViewState)
        {
        dynamic_cast<TViewState&>(*this) = aViewState;
        }

    bool Supersedes(const TLabelSetSpec& aOther) const
        {
        return *this != aOther; // any label set request supersedes a different one already in the task queue
        }
    };

class TVectorObjectStyle
    {
    public:
    TVectorObjectStyle(): m_draw_height(false),m_repeated_symbol_is_one_way_arrow(false) { }

    /** The fill color for lines and polygons. */
    TColor m_color;
    /** The line width; not used for polygons. */
    float m_line_width = 0;
    /** The offset of the line from the centre line of the geometry - for highlights; not used for polygons. */
    float m_line_offset = 0;
    /** The line cap type */
    TLineCap m_line_cap = TLineCap::Round;
    /** The color of the border. */
    TColor m_border_color;
    /** The border width; for lines this is the width of an outer line to be drawn before the main line. */
    float m_border_width = 0;
    /** If non-null, an icon to be drawn at intervals of m_interval along the line. */
    std::shared_ptr<CIcon> m_repeated_symbol;
    /** The interval between repeated symbols (if <= 0, draw one symbol only), or between strokes of a hachure used to fill polygons. */
    float m_interval = 0;
    /** If not empty, an array of dashes and gaps to be used for lines, and for polygon borders. */
    std::vector<float> m_dash_array;
    /** If positive, the stroke width of a hachure used to fill polygons. */
    float m_hachure_stroke_width = 0;
    /** The stroke angle of a hachure used to fill polygons; ignored if m_hachure_stroke_width is not positive. */
    float m_hachure_angle = 0;
    /** If true, draw polygons in three dimensions using their height and other attributes. */
    bool m_draw_height : 1;
    /** If true, the repeated symbol is a one-way arrow, and the objects drawn using this style are roads, and so the symbol is drawn only if the road is one-way. */
    bool m_repeated_symbol_is_one_way_arrow : 1;
    };

class TVectorObjectGroup
    {
    public:
    int32_t m_layer_group = 0;
    uint32_t m_priority = 0;
    TVectorObjectStyle m_style;
    TMapObjectType m_type = TMapObjectType::None;
    bool m_color_texture = false; // true if array map objects in this group use colored bitmaps as opposed to alpha bitmaps
    const CMapObjectArray* m_object_array = nullptr;
    size_t m_object_array_start = 0;    // first object in m_object_array belonging to the group
    size_t m_object_array_end = 0;      // first object in m_object_array not belonging to the group
    };

class CDrawDataBase
    {
    public:
    virtual ~CDrawDataBase() { }

    /**
    This virtual function is called by the drawing thread to initialise
    any data such as OpenGL vertex buffers and textures that can be accessed only in that thread;
    e.g., in the case of OpenGL ES, the thread that owns the EGL context.

    The return value is true for success, false for failure.
    */
    virtual bool Init() = 0;
    };

/**
Data used when drawing the vector tile.
This data is owned by the graphics acceleration system
and will normally consist of vertex buffers and associated data.
*/
class CTileDrawData: public CDrawDataBase
    {
    public:
    /** The transform from map object coordinates to the OpenGL coordinate system; set by the vector tile server just before drawing. */
    TTransform3FP m_transform;
    /** Flags stating which layers are enabled; set by the vector tile server just before drawing. */
    std::shared_ptr<std::vector<bool>> m_enabled_layers;
    /** The style used to create the draw data. */
    std::shared_ptr<CMapStyle> m_style;
    };

/**
Data used when drawing the label set.
This data is owned by the graphics acceleration system
and will normally consist of a texture and associated data.
*/
class CLabelDrawData: public CDrawDataBase
    {
    };

/**
A vector tile map store owns the map objects for
a tile, and their styles. Temporary objects of this class are created
as part of the process of creating a vector tile.
CVectorTileDrawData objects can keep the shared pointers returned by MapStore() and MapStyle() but must not
keep pointers or references to anything in ObjectGroupArray().
*/
class CVectorTileMapStore
    {
    public:
    CVectorTileMapStore(CFramework& aFramework,CVectorTileServer& aVectorTileServer,const TTileSpec& aTileSpec,std::shared_ptr<CMapStyle> aStyle,bool aForGraphicsAcceleration);
    const std::shared_ptr<CMapStore> MapStore() const { return m_map_store; }
    const std::shared_ptr<CMapStyle> MapStyle() const { return m_style; }
    const std::vector<TVectorObjectGroup>& ObjectGroupArray() const { return m_object_group_array; }
    const TTileSpec& TileSpec() const { return m_tile_spec; }
    bool Empty() const { return m_object_group_array.empty(); }
    double UnitsPerPixel() const { return m_units_per_pixel; }
    double UnitsPerMeter() const { return m_units_per_meter; }
    double Orientation() const { return m_orientation; }

    private:
    void GetGroups();
    void GetGroupsForLayers(size_t aStartLayer,size_t aEndLayer,int aPhase);
    void GetGroupsForLayerGroup(const CMapLevelStore& aStore,size_t aStart,size_t aEnd,int aPhase);

    std::shared_ptr<CMapStore> m_map_store; // the map store, projected to the rectangle (0,0,32768,32768)
    std::shared_ptr<CMapStyle> m_style;
    std::vector<TVectorObjectGroup> m_object_group_array;
    TTileSpec m_tile_spec;
    double m_units_per_pixel;   // the number of map object units to a pixel
    double m_units_per_meter;   // the number of map object units to a meter
    double m_orientation;       // the orientation of the data as an angle in degrees: north is 0 and angles increase clockwise
    };

class CVectorTile
    {
    public:
    CVectorTile(const TTileSpec& aRequest,std::unique_ptr<CTileDrawData> aDrawData);
    const TTileSpec Request() const { return m_request; }
    CTileDrawData& DrawData() const { return *m_draw_data; }

    private:
    TTileSpec m_request;
    std::unique_ptr<CTileDrawData> m_draw_data;
    };

class CLabelSet
    {
    public:
    CLabelSet(const TLabelSetSpec& aRequest,std::unique_ptr<CLabelDrawData> aDrawData);
    const TLabelSetSpec Request() const { return m_request; }
    CLabelDrawData& DrawData() const { return *m_draw_data; }

    private:
    TLabelSetSpec m_request;
    std::unique_ptr<CLabelDrawData> m_draw_data;
    };

class CPositionedLabel
    {
    public:
    CPositionedLabel() { }
    CPositionedLabel(uint32_t aLayer,const TBitmap& aBitmap,const TPoint& aTopLeft,const TPoint& aHotspot):
        m_layer(aLayer),
        m_bitmap(aBitmap),
        m_top_left(aTopLeft),
        m_hotspot(aHotspot)
        {
        }

    /** The label layer. */
    uint32_t m_layer = 0;
    /** A bitmap containing the label. */
    CBitmap m_bitmap;
    /** The top left of the bitmap in pixels. */
    TPoint m_top_left;
    /** The hotspot in pixels. */
    TPoint m_hotspot;
    };

class TMapState
    {
    public:
    TMapState() :
        m_location_valid(false),
        m_heading_valid(false)
        {
        }

    void Set(CFramework& aFramework);
    TMapState Interpolate(CFramework& aFramework,const TMapState& aOther,double aTime);

    TViewState m_view_state;
    TPointFP m_view_center_in_map_coords;
    TFixedSizeContourFP<4,true> m_view_in_map_coords;
    TTransform3FP m_map_transform;
    TTransformFP m_map_transform_2D;
    TTransform3FP m_perspective_transform;
    TPointFP m_location_in_map_coords;
    double m_heading_in_radians = 0;
    bool m_location_valid : 1;
    bool m_heading_valid : 1;
    };

/**
A class to store the current state of the master framework in a thread-safe way.
It is set by the map app thread when the client app changes pans, zooms, or
rotates the map.
*/
class TThreadSafeMapState
    {
    public:
    void Set(CFramework& aFramework);
    void Get(CFramework& aFramework,TMapState& aMapState);

    private:
    std::mutex m_mutex;
    TMapState m_start_state;
    TMapState m_end_state;
    TMapState m_cur_state;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time;
    double m_time = 1.0;    // varies between 0 and 1 as the state passes from start to end
    };

class TStyleSheetData
    {
    public:
    CStyleSheetDataArray m_style_sheet_data_array;
    CVariableDictionary m_style_sheet_variables;
    };

class TThreadSafeStyleSheetData: private TStyleSheetData
    {
    public:
    void Set(CFramework& aFramework);
    void Get(TStyleSheetData& aStyleSheetData);

    private:
    std::mutex m_mutex;
    };

class TThreadSafeDrawParam
    {
    public:
    bool Set(CFramework& aFramework);
    std::set<CString> DisabledLayers();
    std::unique_ptr<std::vector<bool>> CreateEnabledLayerArray(const CMapStyle& aStyle);

    private:
    std::mutex m_mutex;
    std::set<CString> m_disabled_layer;
    };

/**
Create a class derived from CVectorTileHelper to use the vector tile system,
implementing the pure virtual functions.
*/
class CVectorTileHelper
    {
    public:
    virtual ~CVectorTileHelper() { }

    /**
    This function is called by one of the vector tile creation worker threads to
    create all the data needed to draw the tile using the Draw function.
    It is called once for each tile, when the tile is created. It must use the data obtained
    via CVectorTileMapStore::ObjectGroupArray, copying it rather than making references to it, because the
    object group data is destroyed after the call to CreateDrawData.

    The coordinates in the map objects are integers representing 64ths of pixels, referring to a notional 512 x 512 tile,
    and are thus in the range 0...32768.

    The dimensions in the styles are floating-point numbers representing full pixels.
    */
    virtual std::unique_ptr<CTileDrawData> CreateDrawData(const CVectorTileMapStore& aVectorTileMapStore) = 0;

    /**
    This function is called by the label bitmap worker thread to
    create the data needed to draw the labels and notices.
    It may return null if labels or notices are not used.
    The framework is supplied so that label points can be converted to map coordinates if necessary.
    */
    virtual std::unique_ptr<CLabelDrawData> CreateLabelDrawData(CFramework& /*aFramework*/,const std::vector<CPositionedLabel>& /*aLabelArray*/,
                                                                CPositionedBitmap /*aNoticeBitmap*/,const TViewState& /*aViewState*/) { return nullptr; }

    /**
    This function is called when the icon used to display the user's location changes.
    The implementation should store the icon in a texture so that it can be drawn at the correct position and orientation.
    */
    virtual void OnLocationIconChange(const CBitmap& aLocationIcon,const TRectFP& aBounds) = 0;
    
    /**
    This function is called just before drawing a frame, whether by a call to DrawFrame or multiple calls to Draw.
    It can be used to set any parameters affecting the whole frame, like the viewport size in pixels.
    It also supplies any background tiles to be drawn as placeholders for tiles that could not be loaded.
    */
    virtual void OnStartDrawing(const TMapState& /*aMapState*/,const std::vector<TRectFP>& /*aBackgroundRectArray*/) { }

    /**
    This function is called once per frame. It allows the data in a frame to be drawn
    in the proper order across all tiles: for example, all road outlines before all road cores.
    The transform for each frame is in the draw data's m_transform member.

    If label data is available it is supplied in aLabelDrawData, which may be null.
    */
    virtual void DrawFrame(const std::vector<CTileDrawData*>& /*aTileDrawDataArray*/,const CLabelDrawData* /*aLabelDrawData*/,bool /*aDraw3DBuildings*/) { }

    /**
    This data member tells the CVectorTileServer what type of information to put in the CVectorTileMapStore objects.
    If it is true, CVectorTileMapStore objects contain object groups suitable for use by graphics-accelerated drawing.
    If not, the object groups are not created; they are not necessary when using software drawing.
    */
    bool m_for_graphics_acceleration = true;

    /** This data member tells the CVectorTileServer whether to draw labels using label sets. */
    bool m_use_label_sets = true;
    };

/**
A class to draw vector tiles using multiple threads.
To use it, create a CVectorTileServer, supplying a CVectorTileHelper object
with functions to convert vector tiles to any suitable format such as data
stored in graphics memory, and to draw the tile using that data.
*/
class CVectorTileServer: public MFrameworkObserver
    {
    public:
    CVectorTileServer(CFramework& aFramework,std::shared_ptr<CVectorTileHelper> aHelper,size_t aThreadCount,size_t aMaxZoomLevel = 32,size_t aMaxTileCacheItems = 128);
    ~CVectorTileServer();

    void Draw();
    void DeleteTilesOutsideView();
    void Invalidate(TVectorDataType aType);
    double ZoomLevelFromScaleDenominator(double aScaleDenominator) const;
    double ScaleDenominatorFromZoomLevel(double aZoomLevel) const;
    TTileSpec TileFromMapPoint(TPointFP aMapPoint,size_t aZoomLevel) const;
    TRectFP TileBounds(const TTileSpec& aTileSpec) const;
    TTileSpec StartTileTask() { return m_task_queue.StartTask(); }
    TLabelSetSpec StartLabelBitmapTask() { return m_label_set_task_queue.StartTask(); }
    void EndTileTask(const TTileSpec& aTask) { m_task_queue.EndTask(aTask); }
    void EndLabelBitmapTask(const TLabelSetSpec& aTask) { m_label_set_task_queue.EndTask(aTask); }
    void AddTileRequest(const TTileSpec& aRequest) { m_task_queue.Add(aRequest); }
    void AddLabelBitmapRequest(const TLabelSetSpec& aRequest) { m_label_set_task_queue.Add(aRequest); }
    void AddTile(std::shared_ptr<CVectorTile> aTile) { m_tile_queue.Add(aTile); }
    void AddLabelSet(std::shared_ptr<CLabelSet> aLabelSet) { m_label_set_queue.Add(aLabelSet); }
    std::shared_ptr<CMapStyle> GetStyleSheet(CFramework& aFramework,size_t aZoomLevel);
    std::unique_ptr<CTileDrawData> CreateDrawData(const CVectorTileMapStore& aVectorTileMapStore);
    std::unique_ptr<CLabelDrawData> CreateLabelDrawData(CFramework& aFramework,const std::vector<CPositionedLabel>& aLabelArray,
                                                        CPositionedBitmap aNoticeBitmap,const TViewState& aViewState);
    std::shared_ptr<CVectorTile> GetTile(const TTileSpec& aTileSpec,bool aTriggerTileCreation = true);
    std::shared_ptr<CLabelSet> GetLabelSet(const TLabelSetSpec& aBitmapSpec);
    bool ForGraphicsAcceleration() const { return m_helper->m_for_graphics_acceleration; }
    uint32_t StyleSheetGeneration() const { return m_style_sheet_generation; }
    void GetStyleSheetData(TStyleSheetData& aData) { m_thread_safe_style_sheet_data.Get(aData); }
    uint32_t EnabledLayerGeneration() const { return m_enabled_layer_generation; }
    std::set<CString> DisabledLayers() { return m_thread_safe_draw_param.DisabledLayers(); }

    static const int32_t KImageSizeInPixels = 512;

    private:
    CVectorTileServer(const CVectorTileServer&) = delete;
    CVectorTileServer(CVectorTileServer&&) = delete;
    CVectorTileServer& operator=(const CVectorTileServer&) = delete;
    CVectorTileServer& operator=(CVectorTileServer&&) = delete;

    // from MFrameworkObserver
    void OnViewChange() override;
    void OnMainDataChange() override;
    void OnDynamicDataChange() override;
    void OnStyleChange() override;
    void OnLayerChange() override;
    void OnNoticeChange() override;

    CFramework& m_framework;
    std::shared_ptr<CVectorTileHelper> m_helper;
    size_t m_max_zoom_level;
    size_t m_location_icon_zoom_level;
    TTaskQueue<TTileSpec> m_task_queue;
    TTaskOutputQueue<std::shared_ptr<CVectorTile>> m_tile_queue;
    size_t m_max_tile_cache_items = 128;
    std::vector<std::shared_ptr<CVectorTile>> m_tile_cache;
    std::vector<std::unique_ptr<CDrawTileTask>> m_task_array;
    std::vector<std::thread> m_thread_array;
    std::vector<std::shared_ptr<CMapStyle>> m_style_array;
    std::vector<std::shared_ptr<std::vector<bool>>> m_enabled_layer_array;
    std::mutex m_style_array_mutex;
    TRectFP m_level_0_tile_extent;
    double m_level_0_tile_width_in_metres;
    double m_pixel_size_in_metres;
    double m_map_unit_size;
    bool m_x_axis_right;
    bool m_y_axis_up;
    std::atomic<uint32_t> m_style_sheet_generation;
    std::atomic<uint32_t> m_enabled_layer_generation;
    std::atomic<uint32_t> m_static_data_generation;
    std::atomic<uint32_t> m_dynamic_data_generation;
    TMapState m_map_state;
    TThreadSafeMapState m_thread_safe_map_state;
    TThreadSafeStyleSheetData m_thread_safe_style_sheet_data;
    TThreadSafeDrawParam m_thread_safe_draw_param;

    TTaskQueue<TLabelSetSpec> m_label_set_task_queue;
    TTaskOutputQueue<std::shared_ptr<CLabelSet>> m_label_set_queue;
    std::shared_ptr<CLabelSet> m_cached_label_set;
    std::shared_ptr<CLabelSet> m_prev_cached_label_set;
    std::unique_ptr<CDrawLabelTask> m_label_task;
    std::thread m_label_thread;
    };

/**
Creates a CVectorTileServer using OpenGL ES 2.0.
Returns null if this feature is not available.
*/
std::shared_ptr<CVectorTileServer> CreateOpenGLESVectorTileServer(CFramework& aFramework);

class CEGLWindow
    {
    public:
    virtual ~CEGLWindow() { };
    virtual void MakeCurrent() = 0;
    virtual void Swap() = 0;
    };

/**
Creates an EGL window and attaches it to the native window.
Returns null if this feature is not available.
At present it is supported on Windows only.
*/
std::unique_ptr<CEGLWindow> CreateEGLWindow(const void* aNativeWindow);

}

#endif
