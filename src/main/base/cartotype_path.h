/*
CARTOTYPE_PATH.H
Copyright (C) 2013-2018 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_PATH_H__
#define CARTOTYPE_PATH_H__

#include <cartotype_types.h>
#include <cartotype_stream.h>

namespace CartoType
{

class CContour;
class TContour;
class COutline;
class CEngine;
class CMapObject;
class CMarker;
class TCircularPen;
class CProjection;

enum class TClipType
    {
    Inside,
    MayIntersectAndHasNoCurves,
    MayIntersectAndHasCurves
    };

enum class TClipOperation // must be semantically the same as ClipperLib::ClipType
    {
    Intersection,
    Union,
    Difference,
    Xor
    };

/**
Path objects, which are sequences of contours, must implement the
MPath interface class.
*/
class MPath
    {
    public:
    /**
    A virtual destructor: needed in case paths returned by ClippedPath
    are not the same as the path passed in and must therefore be deleted by the caller.
    */
    virtual ~MPath() { }

    /** Return the number of contours. */
    virtual size_t Contours() const = 0;

    /** Return the contour indexed by aIndex. */
    virtual void GetContour(size_t aIndex,TContour& aContour) const = 0;

    /**
    Indicate whether the path may have curves.
    General paths may have curves, but some path types are guaranteed not to have them,
    and knowing this saves time when clipping.
    */
    virtual bool MayHaveCurves() const { return true; }

    template<typename MPathTraverser> void Traverse(MPathTraverser& aTraverser,const TRect& aClip) const;
    template<typename MPathTraverser> void Traverse(MPathTraverser& aTraverser,const TRect* aClip = nullptr) const;

    TContour Contour(size_t aIndex) const;
    TRect CBox() const;
    bool CBoxBiggerThan(int32_t aSize) const;
    bool IsContainedIn(const TRect& aRect) const;
    bool Contains(double aX,double aY) const;
    bool Contains(const TPoint& aPoint) const { return Contains(aPoint.iX,aPoint.iY); }
    bool Contains(const TPointFP& aPoint) const { return Contains(aPoint.iX,aPoint.iY); }
    bool MayIntersect(const TRect& aRect) const;
    bool MayIntersect(const TRect& aRect,int32_t aBorder) const;
    bool Intersects(const TRect& aRect) const;
    bool Intersects(const MPath& aPath,const TRect* aBounds = nullptr) const;
    int32_t MaxDistanceFromOrigin() const;
    double DistanceFrom(const MPath& aOther,TPointFP* aNearest1 = nullptr,TPointFP* aNearest2 = nullptr) const;
    double DistanceFromPoint(const TPointFP& aPoint,TPointFP* aNearest = nullptr,size_t* aContourIndex = nullptr,size_t* aLineIndex = nullptr,double* aFractionaLineIndex = nullptr) const;
    bool IsClippingNeeded(const TRect& aClip) const;
    COutline Copy() const;
    COutline ClippedPath(const TRect& aClip) const;
    COutline ClippedPath(const MPath& aClip) const;
    COutline Clip(TClipOperation aClipOperation,const MPath* aPathArray,size_t aPathCount) const;
    COutline Envelope(double aOffset) const;
    bool IsSmoothingNeeded() const;
    COutline SmoothPath() const;
    COutline FlatPath(double aMaxDistance) const;
    COutline TruncatedPath(double aStart,double aEnd) const;
    COutline OffsetPath(double aOffset) const;
    TResult GetHorizontalPath(int32_t aDesiredWidth,TLine& aLine,const TRect* aBounds = nullptr,const TRect* aClip = nullptr) const;
    TPointFP CenterOfGravity() const;
    void GetCenterOfGravity(TPoint& aCenter) const;
    double Length() const;
    double Area() const;
    TPointFP Point(double aPos) const;
    const TPoint* End() const;
    TResult Write(TDataOutputStream& aOutput) const;
    bool IsEmpty() const;
    bool IsPoint() const;
    bool IsGridOrientedRectangle() const;

    TClipType ClipType(const TRect& aRect) const;

    private:
    double LengthHelper(double aPos,TPointFP* aPoint) const;
    bool SmoothPathHelper(COutline* aNewPath) const;
    };

/**
Flags used for appending ellipses to determine the angle between
start and end point or start and end angle.
*/
enum class TEllipseAngleType
    {
    /** Select the shortest arc between start and end. */
    Shortest,
    /** Select the longest arc between start and end. */
    Longest,
    /** Select the arc from start to end in positive direction. */
    Positive,
    /** Select the arc from start to end in negative direction. */
    Negative
    };

/** An interface class for writable contour data. */
class MWritableContour
    {
    public:
    /** Returns a writable pointer to the point data. */
    virtual TOutlinePoint* Point() = 0;
    /** Returns the number of points. */
    virtual size_t Points() = 0;
    /** Reduces the number of points to aPoints. The address of the points must not change. */
    virtual void ReduceSizeTo(size_t aPoints) = 0;
    /** Sets the number of points to aPoints. The address of the points may change. */
    virtual void SetSize(size_t aPoints) = 0;
    /** Returns true if this contour is closed. */
    virtual bool Closed() = 0;
    /** Sets this contour's closed attribute. Does nothing if that is not possible. */
    virtual void SetClosed(bool aClosed) = 0;
    /** Appends a point. */
    virtual void AppendPoint(const TOutlinePoint& aPoint) = 0;

    /** Offset all the points by (aDx,aDy). */
    void Offset(int32_t aDx,int32_t aDy)
        {
        TOutlinePoint* p = Point();
        TOutlinePoint* q = p + Points();
        while (p < q)
            {
            p->iX += aDx;
            p->iY += aDy;
            p++;
            }
        }

    void Simplify(double aResolutionArea);
    void AppendCircularArc(const TPoint& aCenter,const TPoint& aStart,
                           const TPoint& aEnd,TEllipseAngleType aAngleType = TEllipseAngleType::Shortest,bool aAppendStart = false);
    void AppendHalfCircle(const TPoint& aCenter,const TPoint& aStart,
                          const TPoint& aEnd,TEllipseAngleType aAngleType = TEllipseAngleType::Shortest,bool aAppendStart = false);
    void AppendHalfCircle(double aCx,double aCy,double aSx,double aSy,double aEx,double aEy,
                          double aRadius,bool aAppendStart,bool aIsExactHalfCircle,bool aClockwise);
    void AppendQuadrant(double aCx,double aCy,double aSx,double aSy,double aEx,double aEy,double aRadius,
                        bool aAppendStart,bool aIsExactQuadrant,bool aClockwise);

    TOutlinePoint* begin() { return Point(); }
    TOutlinePoint* end() { return Point() + Points(); }
    };

/**
The data for a contour. The simplest implementation of
writable contour data.
*/
template<bool aClosed> class TSimpleContourData: public MWritableContour
    {
    public:
    TOutlinePoint* Point() override { return iPoint; }
    size_t Points() override { return iPoints; }
    void ReduceSizeTo(size_t aPoints) override { iPoints = aPoints; }
    void SetSize(size_t aPoints) override
        {
        assert(aPoints <= iPoints);
        if (aPoints < iPoints)
            iPoints = aPoints;
        }
    bool Closed() override { return aClosed; }
    void SetClosed(bool /*aClosed*/) override { } // not possible
    void AppendPoint(const TOutlinePoint&) override {  } // not possible

    TOutlinePoint* iPoint = nullptr;
    size_t iPoints = 0;
    };

/**
A contour that doesn't own its data.
Contours are sequences of lines and curves.
*/
class TContour: public MPath
    {
    public:
    /** Create an empty contour. */
    TContour(): iPoint(nullptr), iPoints(0), iClosed(false), iMayHaveCurves(true) { }
    /** Create a contour from a supplied array of points. */
    TContour(const TOutlinePoint* aPoint,size_t aPoints,bool aClosed,bool aMayHaveCurves = true):
        iPoint(aPoint),
        iPoints(aPoints),
        iClosed(aClosed),
        iMayHaveCurves(aMayHaveCurves)
        {
        }

    // Virtual functions from MPath.
    size_t Contours() const override { return 1;}
    void GetContour(size_t /*aIndex*/,TContour& aContour) const override { aContour = *this; }
    bool MayHaveCurves() const override { return iMayHaveCurves; }

    /** Return a constant pointer to the start of the array of points. */
    const TOutlinePoint* Point() const { return iPoint; }
    /** Return a point selected by its index. */
    const TOutlinePoint& Point(size_t aIndex) const
        {
        assert(aIndex < iPoints);
        return iPoint[aIndex];
        }
    TOutlinePoint FractionalPoint(double aIndex);
    /** Return the number of points in the contour. */
    size_t Points() const { return iPoints; }
    /** Return true if the contour is closed. */
    bool Closed() const { return iClosed; }
    bool IsGridOrientedRectangle() const;

    /** Set whether the contour may have curves. */
    void SetMayHaveCurves(bool aValue) { iMayHaveCurves = aValue; }

    bool Anticlockwise() const;
    bool Contains(double aX,double aY) const;
    bool Contains(const TPoint& aPoint) const { return Contains(aPoint.iX,aPoint.iY); }
    COutline ClippedContour(const TRect& aClip) const;
    void AppendClippedContour(COutline& aDest,const TRect& aClip) const;
    size_t AppendSplitContour(COutline& aDest,const TPointFP& aLineStart,const TPointFP& aLineVector);
    CContour TruncatedContour(double aStart,double aEnd) const;
    CContour TransformedContour(const TTransform& aTransform) const;
    CContour CentralPath(TResult& aError,std::shared_ptr<CEngine> aEngine,const TRect& aClip,bool aFractionalPixels,
                         TLine& aFallbackLine,bool aFallbackMustBeHorizontal) const;
    CContour Smooth(double aRadius);
    bool MayIntersect(const TRect& aRect) const;
    bool Intersects(const TRect& aRect) const;
    double DistanceFrom(const TContour& aOther,TPointFP* aNearest1 = nullptr,TPointFP* aNearest2 = nullptr) const;
    double DistanceFromPoint(const TPointFP& aPoint,TPointFP* aNearest = nullptr,
                             double* aNearestLength = nullptr,bool* aLeft = nullptr,size_t* aLineIndex = nullptr,
                             double* aFractionalLineIndex = nullptr) const;
    TPointFP PointAtLength(double aLength,double aOffset = 0,int32_t* aLineIndex = nullptr) const;
    void GetOrientation(const TPoint& aCenter,TPoint& aOrientation) const;
    void GetPrincipalAxis(TPointFP& aCenter,TPointFP& aVector) const;
    TResult Write(TDataOutputStream& aOutput) const;
    void GetAngles(double aDistance,double& aStartAngle,double& aEndAngle);
    template<typename MTraverser> void Traverse(MTraverser& aTraverser,const TRect* aClip = nullptr) const;

    const TOutlinePoint* begin() const { return iPoint; }
    const TOutlinePoint* end() const { return iPoint + iPoints; }

    private:
    void GetLongestLineIntersection(const TPoint& aLineStart,const TPoint& aLineEnd,TPoint& aStart,TPoint& aEnd,const TRect& aClip) const;
    void AppendClippedNonCurvedContour(COutline& aDest,const TRect& aClip) const;
    void AppendClippedCurvedContour(COutline& aDest,const TRect& aClip) const;

    const TOutlinePoint* iPoint;
    size_t iPoints;
    bool iClosed;
    bool iMayHaveCurves;
    };

/** A contour that owns its data. */
class CContour: public MPath, public MWritableContour
    {
    public:
    // Virtual functions from MPath.
    size_t Contours() const override { return 1; }
    void GetContour(size_t /*aIndex*/,TContour& aContour) const override { aContour = *this; }
    
    // Virtual functions from MWritableContour.
    TOutlinePoint* Point() override { return iPoint.data(); }
    size_t Points() override { return iPoint.size(); }
    void ReduceSizeTo(size_t aPoints) override
        {
        assert(aPoints <= iPoint.size());
        iPoint.resize(aPoints);
        }
    bool Closed() override { return iClosed; }
    void SetClosed(bool aClosed) override { iClosed = aClosed; }
    
    /** Append a point to the contour, but only if it differs from the previous point or is a control point. */
    void AppendPoint(const TOutlinePoint& aPoint) override
        {
        if (iPoint.size() && aPoint.iType == TPointType::OnCurve && aPoint == iPoint.back())
            return;
        iPoint.push_back(aPoint);
        }
    /** Append a point to the contour whether or not it differs from the previous point. */
    void AppendPointEvenIfSame(const TOutlinePoint& aPoint)
        {
        iPoint.push_back(aPoint);
        }
    /** Append some points to the contour. */
    void AppendPoints(const TOutlinePoint* aPoint,size_t aPoints) { iPoint.insert(iPoint.end(),aPoint,aPoint + aPoints); }
    /** Insert a point at the specified index. */
    void InsertPoint(const TOutlinePoint& aPoint,size_t aIndex) { iPoint.insert(iPoint.begin() + aIndex,aPoint); }
    /** Insert some points at the specified index. */
    void InsertPoints(const TOutlinePoint* aPoint,size_t aPoints,size_t aIndex) { iPoint.insert(iPoint.begin() + aIndex,aPoint,aPoint + aPoints); }
    /** Set the path to its newly constructed state: empty and open. */
    void Clear() { iPoint.clear(); iClosed = false; }
    /** Reduce the number of points to zero. */
    void SetSizeToZero() { iPoint.clear(); }
    /** Set the number of points to a specified size. */
    void SetSize(size_t aSize) override { iPoint.resize(aSize); }
    /** Replace a point selected by an index. */
    void ReplacePoint(size_t aIndex,const TOutlinePoint& aPoint) { iPoint[aIndex] = aPoint; }
    /** Replace last point. */
    void ReplaceLastPoint(const TOutlinePoint& aPoint) { iPoint.back() = aPoint; }
    /** Remove a point specified by an index. */
    void RemovePoint(size_t aIndex) { iPoint.erase(iPoint.begin() + aIndex); }
    /** Remove a series of aCount points starting at aIndex. */
    void RemovePoints(size_t aIndex,size_t aCount) { iPoint.erase(iPoint.begin() + aIndex, iPoint.begin() + aIndex + aCount); }
    void AppendTransformedPointFixed(const TTransform& aTransform,
                                     const TPointFixed& aPoint,TPointType aPointType);
    /** Convert point from fixed point to point in 64ths and append. */
    void AppendPointFixed(const TPointFixed& aPoint,TPointType aPointType)
        {
        iPoint.emplace_back(aPoint.iX.Rounded64ths(),aPoint.iY.Rounded64ths(),aPointType);
        }
    /** Convert point from double precision floating point to point in 64ths and append. */
    void AppendPointDouble(double aX,double aY,TPointType aPointType)
        {
        iPoint.emplace_back(Arithmetic::Round(aX * 64.0),Arithmetic::Round(aY * 64.0),aPointType);
        }
    /**
    Pre-allocate enough space to hold at least aCount points. This function has no effect on behaviour
    but may increase speed.
    */
    void ReservePoints(size_t aCount) { iPoint.reserve(aCount); }
    /** Return a constant pointer to the array of points. */
    const TOutlinePoint* Point() const { return iPoint.data(); }
    /** Return a point selected by its index. */
    const TOutlinePoint& Point(size_t aIndex) const { return iPoint[aIndex]; }
    /** Return the number of points in the contour. */
    size_t Points() const { return iPoint.size(); }
    /** Return a TContour object representing the data owned by this CContour object. */
    operator TContour() const { return TContour(iPoint.data(),iPoint.size(),iClosed); }
    /** Reverse the order of the points in the contour. */
    void Reverse() { std::reverse(iPoint.begin(),iPoint.end()); }
    
    static CContour Ellipse(const TPoint& aCenter,TFixed aRX,TFixed aRY,TFixedSmall aEllipseAngle);
    static CContour Circle(const TPoint& aCenter,TFixed aRadius);
    void MakeCircle(double aX,double aY,double aRadius);
    void MakePolygon(double aX,double aY,double aRadius,int32_t aSides);
    void MakeRoundedRectangle(double aX1,double aY1,double aX2,double aY2,double aWidth,double aRadius);
    static CContour RoundedRectangle(const TPoint& aTopLeft,
                                     TFixed aWidth,TFixed aHeight,TFixed aRX,TFixed aRY);
    static CContour Rectangle(const TPoint& aTopLeft,
                              TFixed aWidth,TFixed aHeight);
    void AppendEllipticalArc(const TPoint& aCenter,TFixed aRX,
                             TFixed aRY,TFixedSmall aEllipseAngle,
                             TFixedSmall aStartAngle,TFixedSmall aEndAngle,
                             TEllipseAngleType aAngleType = TEllipseAngleType::Shortest,bool aAppendStart = false);
    void AppendEllipticalArc(const TPoint& aStart,TFixed aRX,
                             TFixed aRY,TFixedSmall aEllipseAngle,bool aLargeArc,
                             bool aSweep,const TPoint& aEnd,bool aAppendStart = false);
    /** Return true if the contour is closed. */
    bool Closed() const { return iClosed; }
    void Transform(const TTransform& aTransform);
    void Offset(int32_t aDx,int32_t aDy);
    static CContour Read(TResult& aError,TDataInputStream& aInput);
    
    std::vector<TOutlinePoint>::iterator begin() { return iPoint.begin(); }
    std::vector<TOutlinePoint>::iterator end() { return iPoint.end(); }
    std::vector<TOutlinePoint>::const_iterator begin() const { return iPoint.begin(); }
    std::vector<TOutlinePoint>::const_iterator end() const { return iPoint.end(); }
    
    private:
    TResult CircleEndPointToCenterParameters(TPointFixed& aCenter,
                                             const TPointFixed& aPoint1,const TPointFixed& aPoint2,TFixed aRadius,
                                             bool aPositiveDirection,TFixedSmall& aStartAngle,TFixedSmall& aEndAngle);
    TResult EllipseEndPointToCenterParameters(TPointFixed& aCenter,
                                              const TPointFixed& aPoint1,const TPointFixed& aPoint2,TFixed aRX,
                                              TFixed aRY,TFixedSmall aEllipseAngle,bool aPositiveDirection,
                                              TFixedSmall& aStartAngle,TFixedSmall& aEndAngle);
    
    std::vector<TOutlinePoint> iPoint;
    bool iClosed = false;
    };

/** The standard path class. */
class COutline: public MPath
    {
    public:
    COutline() = default;
    COutline(const MPath& aPath);
    COutline(const TRect& aRect);
    COutline& operator=(const MPath& aPath);
    COutline& operator=(const TRect& aRect);
    
    // virtual functions from MPath
    size_t Contours() const override { return iContour.size(); }
    
    void GetContour(size_t aIndex,TContour& aContour) const override
        {
        aContour = iContour[aIndex];
        }
    
    /** Append a contour. */
    void AppendContour(CContour&& aContour)
        {
        iContour.push_back(std::move(aContour));
        }
    
    /** Clear the outline by removing all contours. */
    void Clear() { iContour.clear(); }
    
    /** Append a new empty contour to the outline and return it. */
    CContour& AppendContour()
        {
        iContour.emplace_back();
        return iContour.back();
        }

    void AppendContour(const TContour& aContour)
        {
        auto& c = AppendContour();
        c.SetClosed(aContour.Closed());
        c.AppendPoints(aContour.Point(),aContour.Points());
        }
    
    /** Return a constant reference to a contour, selected by its index. */
    const CContour& Contour(size_t aIndex) const { return iContour[aIndex]; }
    /** Return a non-constant reference to a contour, selected by its index. */
    CContour& Contour(size_t aIndex) { return iContour[aIndex]; }
    
    static COutline* Markers(TResult& aError,const MPath& aPath,
                             const CMarker* const aMarkerStart,const CMarker* const aMarkerMid,
                             const CMarker* const aMarkerEnd,const TRect& aClip);
    void Copy(const MPath& aPath);
    TResult MapCoordinatesToLatLong(const CProjection& aProjection,int32_t aLatLongFractionalBits = 16);
    TResult LatLongToMapCoordinates(const CProjection& aProjection,int32_t aLatLongFractionalBits = 16);
    void RemoveData(std::vector<CContour>& aDest)
        {
        aDest = std::move(iContour);
        iContour.clear();
        }
    static COutline Read(TResult& aError,TDataInputStream& aInput);
    
    std::vector<CContour>::iterator begin() { return iContour.begin(); }
    std::vector<CContour>::iterator end() { return iContour.end(); }
    std::vector<CContour>::const_iterator begin() const { return iContour.begin(); }
    std::vector<CContour>::const_iterator end() const { return iContour.end(); }
    
    private:
    void AppendMarkers(const MPath& aPath,const CMarker* const aMarkerStart,const CMarker* const aMarkerMid,const CMarker* const aMarkerEnd,const TRect& aClip);
    static int32_t MaximumMarkerExtent(const CMarker* aMarker);
    
    std::vector<CContour> iContour;
    };

/**
Traverse a contour, extracting lines and curves and calling
the following functions in aTraverser to process them:

// Start a new contour.
// Move to aPoint without drawing a line and set the current point to aPoint.
void MoveTo(const TPoint& aPoint);

// Draw a line from the current point to aPoint and set the current point to aPoint.
void LineTo(const TPoint& aPoint);

// Draw a quadratic spline from the current point to aPoint2, using aPoint1 as the off-curve control point,
// and set the current point to aPoint2.
void QuadraticTo(const TPoint& aPoint1,const TPoint& aPoint2);

// Draw a cubic spline from the current point to aPoint3, using aPoint1 and aPoint2 as the
// off-curve control points, and set the current point to aPoint3.
void CubicTo(const TPoint& aPoint1,const TPoint& aPoint2,const TPoint& aPoint3);
*/
template<typename MTraverser> void TContour::Traverse(MTraverser& aTraverser,const TRect* aClip) const
    {
    if (iPoints < 2)
        return;
    
    // A contour cannot start with a cubic control point.
    assert(iPoint->iType != TPointType::Cubic);
    
    if (aClip && !aClip->IsMaximal() && ClipType(*aClip) != TClipType::Inside)
        {
        COutline clipped_contour = ClippedContour(*aClip);
        clipped_contour.Traverse(aTraverser);
        return;
        }
    
    size_t last = iPoints - 1;
    const TOutlinePoint* limit = iPoint + last;
    TPoint v_start = iPoint[0];
    TPoint v_last = iPoint[last];
    const TOutlinePoint* point = iPoint;
    
    /* Check first point to determine origin. */
    if (point->iType == TPointType::Quadratic)
        {
        /* First point is conic control. Yes, this happens. */
        if (iPoint[last].iType == TPointType::OnCurve)
            {
            /* start at last point if it is on the curve */
            v_start = v_last;
            limit--;
            }
        else
            {
            /*
             If both first and last points are conic,
             start at their middle and record its position
             for closure.
             */
            v_start.iX = (v_start.iX + v_last.iX) / 2;
            v_start.iY = (v_start.iY + v_last.iY) / 2;
            v_last = v_start;
            }
        point--;
        }
    
    aTraverser.MoveTo(v_start);
    
    while (point < limit)
        {
        point++;
        switch (point->iType)
            {
            case TPointType::OnCurve:
                aTraverser.LineTo(*point);
                continue;
            case TPointType::Quadratic:
                {
                const TPoint* v_control = point;
                while (point < limit)
                    {
                    point++;
                    const TOutlinePoint* cur_point = point;
                    if (point->iType == TPointType::OnCurve)
                        {
                        aTraverser.QuadraticTo(*v_control,*cur_point);
                        continue;
                        }
                    if (point->iType != TPointType::Quadratic)
                        return; // invalid outline
                    TPoint v_middle((v_control->iX + cur_point->iX) / 2,(v_control->iY + cur_point->iY) / 2);
                    aTraverser.QuadraticTo(*v_control,v_middle);
                    v_control = cur_point;
                    }
                aTraverser.QuadraticTo(*v_control,v_start);
                return;
                }
            default: // cubic
                {
                if (point + 1 > limit || point->iType != TPointType::Cubic)
                    return; // invalid outline
                const TPoint* vec1 = point++;
                const TPoint* vec2 = point++;
                if (point <= limit)
                    {
                    aTraverser.CubicTo(*vec1,*vec2,*point);
                    continue;
                    }
                aTraverser.CubicTo(*vec1,*vec2,v_start);
                return;
                }
            }
        }
    
    // Close the contour with a line segment.
    if (iClosed && v_last != v_start)
        aTraverser.LineTo(v_start);
    }

// function defined outside MPath because it uses the TContour class
template<typename MPathTraverser> inline void MPath::Traverse(MPathTraverser& aTraverser,const TRect& aClip) const
    {
    const TRect* clip = aClip.IsMaximal() ? nullptr : &aClip;
    TContour c;
    size_t n = Contours();
    for (size_t i = 0; i < n; i++)
        {
        GetContour(i,c);
        c.Traverse(aTraverser,clip);
        }
    }
    
// function defined outside MPath because it uses the TContour class
template<typename MPathTraverser> inline void MPath::Traverse(MPathTraverser& aTraverser,const TRect* aClip) const
    {
    if (aClip && aClip->IsMaximal())
        aClip = nullptr;
    TContour c;
    size_t n = Contours();
    for (size_t i = 0; i < n; i++)
        {
        GetContour(i,c);
        c.Traverse(aTraverser,aClip);
        }
    }

/**
An iterator to traverse a path.
Limitation: for the moment it works with straight lines only; it
treats all points as on-curve points.
*/
class TPathIterator
    {
    public:
    TPathIterator(const MPath& aPath);
    bool Forward(double aDistance);
    bool NextContour();
    void MoveToNearestPoint(const TPointFP& aPoint);
    size_t ContourIndex() const { return iContourIndex; }
    const TContour& Contour() const { return iContour; }
    size_t LineIndex() const { return iLineIndex; }
    double PositionOnLine() const { return iPositionOnLine; }
    const TPoint& Position() const { return iPosition; }
    double Direction() const { return iDirection; }

    private:
    void CalculatePosition();

    const MPath& iPath;
    size_t iContourIndex;
    TContour iContour;
    size_t iLineIndex;
    TPoint iPosition;
    double iPositionOnLine;
    double iLineLength;
    double iDirection;
    double iDx;
    double iDy;
    };

/** A contour with a fixed number of points. */
template<size_t aPointCount,bool aClosed> class TFixedSizeContour: public MPath, public std::array<TOutlinePoint,aPointCount>
    {
    public:
    size_t Contours() const override { return 1; }
    void GetContour(size_t /*aIndex*/,TContour& aContour) const override { aContour = TContour(std::array<TOutlinePoint,aPointCount>::data(),aPointCount,aClosed); }
    operator TContour() const { return TContour(std::array<TOutlinePoint,aPointCount>::data(),aPointCount,aClosed); }
    };

/** A class to hold geometry when creating map objects. */
class CGeometry
    {
    public:
    CGeometry() { }
    explicit CGeometry(TCoordType aCoordType,bool aClosed = false):
        iCoordType(aCoordType),
        iClosed(aClosed)
        {
        }
    explicit CGeometry(const CMapObject& aMapObject);
    CGeometry(const MPath& aPath,TCoordType aCoordType,bool aClosed);
    CGeometry(const TRectFP& aRect,TCoordType aCoordType);
    CGeometry(const TPointFP& aPoint,TCoordType aCoordType);
    void Clear() { iPoint.clear(); iContourStart.clear(); iCoordType = TCoordType::Map; iClosed = false; }

    TCoordType CoordType() const { return iCoordType; }
    size_t ContourCount() const;
    size_t PointCount(size_t aContourIndex) const;
    TPointFP Point(size_t aContourIndex, size_t aPointIndex) const;
    bool IsEmpty() const { return iPoint.empty(); }
    bool IsClosed() const { return iClosed; }
    void SetClosed(bool aClosed) { iClosed = aClosed; }
    TRectFP Bounds() const;

    void AppendPoint(double aX,double aY);
    void AppendPoint(const TPoint& aPoint);
    void AppendPoint(const TPointFP& aPoint);
    void BeginContour();
    explicit operator COutline() const;
    explicit operator TWritableCoordSet();
    explicit operator TCoordSet() const;
    void Reverse();

    private:
    std::vector<double> iPoint;
    std::vector<size_t> iContourStart;
    TCoordType iCoordType = TCoordType::Map;
    bool iClosed = false;
    };

class TContourFP
    {
    public:
    TContourFP() = default;
    TContourFP(const TPointFP* aPoint,size_t aPoints,bool aClosed) :
        m_point(aPoint),
        m_end(aPoint + aPoints),
        m_closed(aClosed)
        {
        }
    const TPointFP* Point() const noexcept { return m_point; }
    size_t Points() const noexcept { return m_end - m_point; }
    bool Closed() const noexcept { return m_closed; }
    TRectFP Bounds() const noexcept;
    bool Intersects(const TRectFP& aRect) const noexcept;
    bool Contains(const TPointFP& aPoint) const noexcept;

    private:
    const TPointFP* m_point = nullptr;
    const TPointFP* m_end = nullptr;
    bool m_closed = false;
    };

/** A contour with a fixed number of floating-point points. */
template<size_t aPointCount,bool aClosed> class TFixedSizeContourFP: public std::array<TPointFP,aPointCount>
    {
    public:
    operator TContourFP() const { return TContourFP(std::array<TPointFP,aPointCount>::data(),aPointCount,aClosed); }
    };

class CPolygonFP;
class CContourFP
    {
    public:
    CContourFP() { }
    explicit CContourFP(const TRectFP& aRect)
        {
        iPoint.resize(4);
        iPoint[0] = aRect.iTopLeft;
        iPoint[1] = aRect.BottomLeft();
        iPoint[2] = aRect.iBottomRight;
        iPoint[3] = aRect.TopRight();
        }    
    void AppendPoint(const TPointFP& aPoint) { iPoint.push_back(aPoint); }
    CPolygonFP Clip(const TRectFP& aClip);
    TRectFP Bounds() const
        { return TContourFP(iPoint.data(),iPoint.size(),false).Bounds(); }

    std::vector<TPointFP> iPoint;
    };

class CPolygonFP
    {
    public:
    TRectFP Bounds() const;

    std::vector<CContourFP> iContour;
    };

}

#endif
