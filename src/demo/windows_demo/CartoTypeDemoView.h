
// CartoTypeDemoView.h : interface of the CCartoTypeDemoView class
//

#pragma once

namespace CartoType
    {
    class CLegend;
    }

// An extended bitmap info class with entries for bitfield specifiers.
class BitmapInfoX
    {
    public:
        BitmapInfoX()
            {
            memset(this,0,sizeof(*this));
            }

        BITMAPV4HEADER bmiHeader;
    };

class CMyImageServerHelper : public CartoType::MImageServerHelper
    {
    public:
        CartoType::TResult DrawBitmap(void* aDeviceContext,const CartoType::TBitmap* aBitmap,const void* /*aImageData*/,const CartoType::TTransformFP& aTransform)
            {
            ASSERT(aBitmap != NULL);

            SetGraphicsMode((HDC)aDeviceContext,GM_ADVANCED);
            SetMapMode((HDC)aDeviceContext,MM_TEXT);

            // Transform the bitmap's bounds to window coordinates.
            XFORM xform;
            xform.eM11 = (FLOAT)aTransform.A();
            xform.eM12 = (FLOAT)aTransform.B();
            xform.eM21 = (FLOAT)aTransform.C();
            xform.eM22 = (FLOAT)aTransform.D();
            xform.eDx = (FLOAT)aTransform.Tx();
            xform.eDy = (FLOAT)aTransform.Ty();
            SetWorldTransform((HDC)aDeviceContext,&xform);

            CartoType::TRect dest_rect(0,0,aBitmap->Width(),aBitmap->Height());

            BitmapInfoX bm;
            bm.bmiHeader.bV4Size = sizeof(BITMAPV4HEADER);
            bm.bmiHeader.bV4Width = aBitmap->Width();
            bm.bmiHeader.bV4Height = -aBitmap->Height();
            bm.bmiHeader.bV4Planes = 1;
            bm.bmiHeader.bV4BitCount = 32;
            bm.bmiHeader.bV4V4Compression = BI_BITFIELDS;
            bm.bmiHeader.bV4SizeImage = 0;
            bm.bmiHeader.bV4XPelsPerMeter = 1000;
            bm.bmiHeader.bV4YPelsPerMeter = 1000;
            bm.bmiHeader.bV4ClrUsed = 0;
            bm.bmiHeader.bV4ClrImportant = 0;
            bm.bmiHeader.bV4RedMask = 0xFF000000;
            bm.bmiHeader.bV4GreenMask = 0xFF0000;
            bm.bmiHeader.bV4BlueMask = 0xFF00;
            bm.bmiHeader.bV4AlphaMask = 0xFF;
            const unsigned char* map_data = (const unsigned char*)(aBitmap->Data());

            ::StretchDIBits((HDC)aDeviceContext,
                            dest_rect.iTopLeft.iX,dest_rect.iTopLeft.iY,dest_rect.Width(),dest_rect.Height(),
                            0,0,aBitmap->Width(),aBitmap->Height(),
                            map_data,(BITMAPINFO*)&bm,DIB_RGB_COLORS,SRCCOPY);

            memset(&xform,0,sizeof(xform));
            xform.eM11 = xform.eM22 = 1;
            SetWorldTransform((HDC)aDeviceContext,&xform);

            return CartoType::KErrorNone;
            }
    };

class CCartoTypeDemoView : public CView
{
protected: // create from serialization only
    CCartoTypeDemoView();
    DECLARE_DYNCREATE(CCartoTypeDemoView)

// Attributes
public:
    CCartoTypeDemoDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
    virtual void OnDraw(CDC* pDC);  // overridden to draw this view
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnInitialUpdate();

protected:

// Implementation
public:
    virtual ~CCartoTypeDemoView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnLButtonDown(UINT nFlags,CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags,CPoint point);
    afx_msg void OnMouseLeave();
    afx_msg void OnMouseMove(UINT nFlags,CPoint point);
    afx_msg BOOL OnMouseWheel(UINT nFlags,short zDelta,CPoint pt);
    afx_msg void OnRButtonUp(UINT nFlags,CPoint point);
    afx_msg void OnSize(UINT nType,int cx,int cy);
    afx_msg void OnFileOpenInCurrentMap();
    afx_msg void OnFileClose();
    afx_msg void OnUpdateFileClose(CCmdUI *pCmdUI);
    afx_msg void OnFileSaveAsPng();
    afx_msg void OnFileSaveAsCtsql();
    afx_msg void OnFind();
    afx_msg void OnUpdateFind(CCmdUI *pCmdUI);
    afx_msg void OnFindAddress();
    afx_msg void OnUpdateFindAddress(CCmdUI *pCmdUI);
    afx_msg void OnFindNext();
    afx_msg void OnUpdateFindNext(CCmdUI *pCmdUI);
    afx_msg void OnViewReloadStyleSheet();
    afx_msg void OnUpdateViewReloadStyleSheet(CCmdUI *pCmdUI);
    afx_msg void OnViewZoomIn();
    afx_msg void OnViewZoomOut();
    afx_msg void OnUpdateViewZoomIn(CCmdUI *pCmdUI);
    afx_msg void OnUpdateViewZoomOut(CCmdUI *pCmdUI);
    afx_msg void OnViewRotate();
    afx_msg void OnUpdateViewRotate(CCmdUI *pCmdUI);
    afx_msg void OnViewNorthup();
    afx_msg void OnUpdateViewNorthup(CCmdUI *pCmdUI);
    afx_msg void OnViewLatlong();
    afx_msg void OnUpdateViewLatlong(CCmdUI *pCmdUI);
    afx_msg void OnViewStylesheet();
    afx_msg void OnUpdateViewStylesheet(CCmdUI *pCmdUI);
    afx_msg void OnViewLayers();
    afx_msg void OnUpdateViewLayers(CCmdUI *pCmdUI);
    afx_msg void OnViewLegend();
    afx_msg void OnUpdateViewLegend(CCmdUI *pCmdUI);
    afx_msg void OnViewSetScale();
    afx_msg void OnUpdateViewSetScale(CCmdUI *pCmdUI);
    afx_msg void OnViewPerspective();
    afx_msg void OnUpdateViewPerspective(CCmdUI *pCmdUI);
    afx_msg void OnViewMetricUnits();
    afx_msg void OnUpdateViewMetricUnits(CCmdUI *pCmdUI);
    afx_msg void OnViewTurnInstructions();
    afx_msg void OnUpdateViewTurnInstructions(CCmdUI *pCmdUI);
    afx_msg void OnViewFlyThrough();
    afx_msg void OnUpdateViewFlyThrough(CCmdUI *pCmdUI);
    afx_msg void OnGoAlongRoute();
    afx_msg void OnUpdateGoAlongRoute(CCmdUI *pCmdUI);
    afx_msg void OnDeleteRoute();
    afx_msg void OnUpdateDeleteRoute(CCmdUI *pCmdUI);
    afx_msg void OnSetDriveProfile();
    afx_msg void OnUpdateSetDriveProfile(CCmdUI *pCmdUI);
    afx_msg void OnSetCycleProfile();
    afx_msg void OnUpdateSetCycleProfile(CCmdUI *pCmdUI);
    afx_msg void OnSetWalkProfile();
    afx_msg void OnUpdateSetWalkProfile(CCmdUI *pCmdUI);
    afx_msg void OnSetHikeProfile();
    afx_msg void OnUpdateSetHikeProfile(CCmdUI *pCmdUI);
    afx_msg void OnShortestRoute();
    afx_msg void OnUpdateShortestRoute(CCmdUI *pCmdUI);
    afx_msg void OnUseGradients();
    afx_msg void OnUpdateUseGradients(CCmdUI *pCmdUI);
    afx_msg void OnSaveRouteInstructions();
    afx_msg void OnUpdateSaveRouteInstructions(CCmdUI *pCmdUI);
    afx_msg void OnSaveRouteXml();
    afx_msg void OnUpdateSaveRouteXml(CCmdUI *pCmdUI);
    afx_msg void OnSaveRouteGpx();
    afx_msg void OnUpdateSaveRouteGpx(CCmdUI *pCmdUI);
    afx_msg void OnLoadRouteXml();
    afx_msg void OnUpdateLoadRouteXml(CCmdUI *pCmdUI);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnEditLoadKml();
    afx_msg void OnUpdateEditLoadKml(CCmdUI *pCmdUI);
    afx_msg void OnEditSaveKml();
    afx_msg void OnUpdateEditSaveKml(CCmdUI *pCmdUI);
    afx_msg void OnEditLoadCtms();
    afx_msg void OnUpdateEditLoadCtms(CCmdUI *pCmdUI);
    afx_msg void OnEditSaveCtms();
    afx_msg void OnUpdateEditSaveCtms(CCmdUI *pCmdUI);
    afx_msg void OnEditLoadCtsql();
    afx_msg void OnUpdateEditLoadCtsql(CCmdUI *pCmdUI);
    afx_msg void OnEditSaveCtsql();
    afx_msg void OnUpdateEditSaveCtsql(CCmdUI *pCmdUI);
    afx_msg void OnEditLoadGpx();
    afx_msg void OnUpdateEditLoadGpx(CCmdUI *pCmdUI);
    afx_msg void OnDestroy();
    DECLARE_MESSAGE_MAP()

private:
    void TestCode();
    bool SetCentre(CPoint& aPoint);
    void Update() { Invalidate(false); }
    void PanToDraggedPosition();
    void StopDragging();
    const CartoType::TBitmap* MapBitmap(CartoType::TResult& aError,const CartoType::TRect& aMapClientArea,bool& aRedrawNeeded);
    void CreateLegend();
    void ShowNextFoundObject();
    CartoType::TResult LoadStyleSheet(const ::CString& aPath);
    void SimulateNavigationFix(bool aGoToNextTurn,bool& aAtEnd);
    void StopFlyThrough();
    void CalculateAndDisplayRoute();
    void SetProfile(CartoType::TRouteProfileType aProfileType);

    std::unique_ptr<CartoType::CFramework> iFramework;
    CartoType::CMapObjectArray iFoundObject;
    CartoType::CString iFindText;
    bool iFindTextPrefix = false;
    bool iFindTextFuzzy = false;
    CartoType::CAddress iAddress;
    int iFoundObjectIndex = 0;
    uint64_t iFoundObjectId = 0;                            // map object ID of found object 
    bool iDrawLegend = false;							    // if true and if there is room, draw the legend
    bool iDrawTurnInstructions = true;				        // if true, draw turn instructions if any
    bool iMapDragEnabled = false;						    // true if dragging the map
    CartoType::TPoint iMapDragAnchor;
    CartoType::TPointFP iMapDragAnchorInMapCoords;
    CartoType::TPoint iMapDragOffset;
    UINT iFlyThroughTimer = 0;								// if non-zero, ID of timer used to fly through a route
    double iTimerIntervalInSeconds = 0.5;
    double iFlyThroughSpeedInKph = 5;
    std::unique_ptr<CartoType::TRouteIterator> iRouteIter;	// if non-null, an iterator used to traverse a route
    double iOnRouteTime = 0;                                // simulated time in seconds on the route

    class TRoutePoint 
        {
        public:
        CartoType::TRoutePoint iPoint;
        uint64_t iId = 0;
        };

    std::vector<TRoutePoint> iRoutePointArray;
    CartoType::TRouteProfileType iRouteProfileType = CartoType::TRouteProfileType::Car;
    bool iShortestRoute = false;
    bool iUseGradients = false;
    bool iMetricUnits = true;
    std::unique_ptr<CMyImageServerHelper> iImageServerHelper;
    bool iUsingImageServer = false;
    std::unique_ptr<CartoType::CGraphicsContext> iExtraGc;
    CartoType::uint32 iWritableMapHandle = 0;
    bool iWritableMapChanged = false;
    CartoType::CString iPushPinName;
    CartoType::CString iPushPinDesc;
    bool iBestRoute = false;                                // calculate the best route through the waypoints don't just take them in order
    std::vector<CartoType::uint32> iMapHandle;
    bool iLeftClickSimulatesNavigationFix = false;

    std::unique_ptr<CartoType::CMapRenderer> iMapRenderer;
};

#ifndef _DEBUG  // debug version in CartoTypeDemoView.cpp
inline CCartoTypeDemoDoc* CCartoTypeDemoView::GetDocument() const
   { return reinterpret_cast<CCartoTypeDemoDoc*>(m_pDocument); }
#endif

