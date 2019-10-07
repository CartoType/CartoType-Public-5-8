/*
cartotype_base.h
Copyright (C) 2004-2019 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_BASE_H__
#define CARTOTYPE_BASE_H__

#include <cartotype_errors.h>

#include <memory>
#include <vector>
#include <cmath>
#include <functional>
#include <array>

namespace CartoType
{

const char* Version();
const char* Build();

class TRectFP;

/** A point in two-dimensional space. */
class TPoint
    {
    public:
    /** Create a point with coordinates (0,0). */
    constexpr TPoint() noexcept { }
    /** Create a point, specifying coordinates. */
    constexpr TPoint(int32_t aX,int32_t aY) noexcept : iX(aX), iY(aY) { }
    /** The equality operator. */
    constexpr bool operator==(const TPoint& aPoint) const noexcept { return iX == aPoint.iX && iY == aPoint.iY; }
    /** The inequality operator. */
    constexpr bool operator!=(const TPoint& aPoint) const noexcept { return !(*this == aPoint); }
    /** Offset a point by another point, treated as a positive vector. */
    void operator+=(const TPoint& aPoint) noexcept { iX += aPoint.iX; iY += aPoint.iY; }
    /** Offset a point by another point, treated as a negative vector. */
    void operator-=(const TPoint& aPoint) noexcept { iX -= aPoint.iX; iY -= aPoint.iY; }
    /** The less-than operator, ordering points by x coordinate, then y coordinate. */
    constexpr bool operator<(const TPoint& aPoint) const noexcept
        {
        return (iX < aPoint.iX) ||
               ((iX == aPoint.iX) && iY < aPoint.iY);
        }
    /** The greater-than operator, ordering points by x coordinate, then y coordinate. */
    constexpr bool operator>(const TPoint& aPoint) const noexcept
        {
        return (iX > aPoint.iX) ||
               ((iX == aPoint.iX) && iY > aPoint.iY);
        }

    /** Return a single number which can be used to sort points into order. */
    constexpr uint64_t ComparisonValue() const noexcept { return (uint64(iX) << 32) + uint64(iY); }

    /** The x coordinate. */
    int32_t iX { 0 };
    /** The y coordinate. */
    int32_t iY { 0 };
    };

/**
A templated 2D point or vector class.
In the vector functions, left and right are defined using x increasing right and y increasing up.
*/
template<class T> class TPoint2
    {
    public:
    /** Create a point with coordinates (0,0). */
    TPoint2() noexcept { }
    /** Create a point, specifying coordinates. */
    TPoint2(T aX,T aY) noexcept : iX(aX), iY(aY) { }
    /** Create a point from a TPoint object. */
    TPoint2(const TPoint& aPoint) noexcept : iX(aPoint.iX), iY(aPoint.iY) { }
    /** The equality operator. */
    bool operator==(const TPoint2<T>& aPoint) const noexcept { return iX == aPoint.iX && iY == aPoint.iY; }
    /** The inequality operator. */
    bool operator!=(const TPoint2<T>& aPoint) const noexcept { return !(*this == aPoint); }
    /** Offset a point by another point, treated as a positive vector. */
    void operator+=(const TPoint2<T>& aPoint) noexcept { iX += aPoint.iX; iY += aPoint.iY; }
    /** Offset a point by another point, treated as a negative vector. */
    void operator-=(const TPoint2<T>& aPoint) noexcept { iX -= aPoint.iX; iY -= aPoint.iY; }
    /** Scale a point by a factor. */
    void operator*=(T aFactor) noexcept { iX *= aFactor; iY *= aFactor; }
    /** The less-than operator, ordering points by x coordinate, then y coordinate. */
    bool operator<(const TPoint2<T>& aPoint) const noexcept
        {
        if (iX < aPoint.iX)
            return true;
        if (iX > aPoint.iX)
            return false;
        if (iY < aPoint.iY)
            return true;
        return false;
        }
    /** The greater-than operator, ordering points by x coordinate, then y coordinate. */
    bool operator>(const TPoint2<T>& aPoint) const noexcept
        {
        if (iX > aPoint.iX)
            return true;
        if (iX < aPoint.iX)
            return false;
        if (iY > aPoint.iY)
            return true;
        return false;
        }
    T VectorLength() const noexcept
        {
        return sqrt(iX * iX + iY * iY);
        }
    T DistanceFrom(const TPoint2<T>& aOther) const noexcept
        {
        TPoint2<T> v { *this };
        v -= aOther;
        return v.VectorLength();
        }
    T CrossProduct(const TPoint2<T>& aOther) const noexcept
        {
        return iX * aOther.iY - iY * aOther.iX;
        }
    TPoint2 UnitVector() const noexcept
        {
        T length = VectorLength();
        if (length > 0)
            {
            TPoint2<T> p(*this);
            p.iX /= length;
            p.iY /= length;
            return p;
            }
        return TPoint2<T>(1,0);
        }

    /** Returns a unit vector pointing 90 degrees left of this vector. */
    TPoint2 LeftUnitVector() const noexcept
        {
        TPoint2<T> u { UnitVector() };
        return TPoint2<T>(-u.iY,u.iX);
        }
    /** Returns a unit vector pointing 90 degrees right of this vector. */
    TPoint2 RightUnitVector() const noexcept
        {
        TPoint2<T> u { UnitVector() };
        return TPoint2<T>(u.iY,-u.iX);
        }
    /** Returns true if aPoint is left of this vector. */
    bool LeftOfVector(const TPoint2<T>& aPoint) const noexcept
        {
        return CrossProduct(aPoint) > 0;
        }
    /** Returns true if aPoint is right of this vector. */
    bool RightOfVector(const TPoint2<T>& aPoint) const noexcept
        {
        return CrossProduct(aPoint) < 0;
        }
    TPoint Rounded() const noexcept
        {
        return TPoint((int32_t)(iX < 0.0 ? iX - 0.5 : iX + 0.5),(int32_t)(iY < 0.0 ? iY - 0.5 : iY + 0.5));
        }

    /** The x coordinate. */
    T iX { 0 };
    /** The y coordinate. */
    T iY { 0 };
    };

/** A templated 3D point class. */
template<class T> class TPoint3
    {
    public:
    /** Create a point with coordinates (0,0,0). */
    TPoint3() noexcept { }
    /** Create a point, specifying coordinates. */
    TPoint3(T aX,T aY,T aZ) noexcept : iX(aX), iY(aY), iZ(aZ) { }
    /** Create a 3D point from a 2D point. */
    TPoint3(TPoint2<T> aPoint) noexcept : iX(aPoint.iX), iY(aPoint.iY), iZ(0) { }
    /** The equality operator. */
    bool operator==(const TPoint3<T>& aPoint) const noexcept { return iX == aPoint.iX && iY == aPoint.iY && iZ == aPoint.iZ; }
    /** The inequality operator. */
    bool operator!=(const TPoint3<T>& aPoint) const noexcept { return !(*this == aPoint); }
    /** Offset a point by another point, treated as a positive vector. */
    void operator+=(const TPoint3<T>& aPoint) noexcept { iX += aPoint.iX; iY += aPoint.iY; iZ += aPoint.iZ; }
    /** Offset a point by another point, treated as a negative vector. */
    void operator-=(const TPoint3<T>& aPoint) noexcept { iX -= aPoint.iX; iY -= aPoint.iY; iZ -= aPoint.iZ; }
    /** Scale a point by a factor. */
    void operator*=(T aFactor) noexcept { iX *= aFactor; iY *= aFactor; iZ *= aFactor; }
    /** The less-than operator, ordering points by x coordinate, then y coordinate, then z coordinate. */
    bool operator<(const TPoint3<T>& aPoint) const noexcept
        {
        if (iX < aPoint.iX)
            return true;
        if (iX > aPoint.iX)
            return false;
        if (iY < aPoint.iY)
            return true;
        if (iY > aPoint.iY)
            return false;
        if (iZ < aPoint.iZ)
            return true;
        return false;
        }
    /** The greater-than operator, ordering points by x coordinate, then y coordinate. */
    bool operator>(const TPoint2<T>& aPoint) const noexcept
        {
        if (iX > aPoint.iX)
            return true;
        if (iX < aPoint.iX)
            return false;
        if (iY > aPoint.iY)
            return true;
        if (iY < aPoint.iY)
            return false;
        if (iZ > aPoint.iZ)
            return true;
        return false;
        }
    T VectorLength() const noexcept
        {
        return sqrt(iX * iX + iY * iY + iZ * iZ);
        }
    T DistanceFrom(const TPoint3<T>& aOther) const noexcept
        {
        TPoint3<T> v { *this };
        v -= aOther;
        return v.VectorLength();
        }
    TPoint3 UnitVector() const noexcept
        {
        T length = VectorLength();
        if (length > 0)
            {
            TPoint3 p(*this);
            p.iX /= length;
            p.iY /= length;
            p.iZ /= length;
            return p;
            }
        return TPoint3<T>(1,0,0);
        }

    /** Returns the surface normal as a unit vector. */
    static TPoint3<T> Normal(const TPoint3<T>& aA,const TPoint3<T>& aB,const TPoint3<T>& aC) noexcept
        {
        TPoint3<T> b { aB }; b -= aA;
        TPoint3<T> c { aC }; c -= aA;
        TPoint3<T> normal { b.iY * c.iZ - b.iZ * c.iY,
                            b.iZ * c.iX - b.iX * c.iZ,
                            b.iX * c.iY - b.iY * c.iX };
        return normal.UnitVector();
        }

    /** The x coordinate. */
    T iX { 0 };
    /** The y coordinate. */
    T iY { 0 };
    /** The z coordinate. */
    T iZ { 0 };
    };

/** A double-precision floating-point 2D point class. */
using TPointFP = TPoint2<double>;

/** A double-precision floating-point 3D point class. */
using TPoint3FP = TPoint3<double>;

/** A set of modifiable coordinate pairs. */
class TWritableCoordSet
    {
    public:
    constexpr TWritableCoordSet() noexcept { } // define an explicit constructor to prevent aggregate initialisation, which stops braced initialisation from CGeometry working

    /** A pointer to the first x coordinate. */
    double* iX { nullptr };
    /** A pointer to the first y coordinate. */
    double* iY { nullptr };
    /** The step by which to increment iX and iY. */
    size_t iStep { 1 };
    /** The number of coordinates. */
    size_t iCount { 0 };
    };

/** A set of coordinate pairs. */
class TCoordSet
    {
    public:
    /** Creates an empty coordinate set. */
    constexpr TCoordSet() noexcept { }

    /** Creates a coordinate set from a writable coordinate set. */
    constexpr TCoordSet(const TWritableCoordSet& aOther) noexcept :
        iX(aOther.iX),
        iY(aOther.iY),
        iStep(aOther.iStep),
        iCount(aOther.iCount)
        {
        }

    TCoordSet(const std::vector<TPointFP>& aPointArray) noexcept :
        iX(&aPointArray.front().iX),
        iY(&aPointArray.front().iY),
        iStep(2),
        iCount(aPointArray.size())
        {
        }

    double DistanceFromPoint(bool aIsPolygon,double aX,double aY,double& aNearestX,double& aNearestY) const noexcept;
    bool PolygonContains(double aX,double aY) const noexcept;
    double X(size_t aIndex) const noexcept;
    double Y(size_t aIndex) const noexcept;

    /** A pointer to the first x coordinate. */
    const double* iX { nullptr };
    /** A pointer to the first y coordinate. */
    const double* iY { nullptr };
    /** The step by which to increment iX and iY. */
    size_t iStep { 1 };
    /** The number of coordinates. */
    size_t iCount { 0 };
    };

/** A set of coordinates consisting of a single point. */
class TCoordPair: public TCoordSet
    {
    public:
    TCoordPair(double aX,double aY) noexcept :
        iOwnX(aX),
        iOwnY(aY)
        {
        iX = &iOwnX;
        iY = &iOwnY;
        iStep = 0;
        iCount = 1;
        }

    double iOwnX, iOwnY;
    };

/** A set of coordinates consising of two points. */
class TCoordSetOfTwoPoints: public TCoordSet
    {
    public:
    TCoordSetOfTwoPoints(double aX0,double aY0,double aX1,double aY1) noexcept
        {
        iOwnX[0] = aX0;
        iOwnX[1] = aX1;
        iOwnY[0] = aY0;
        iOwnY[1] = aY1;
        iX = iOwnX;
        iY = iOwnY;
        iStep = 1;
        iCount = 2;
        }

    double iOwnX[2] = { };
    double iOwnY[2] = { };
    };

/** Coordinate types. */
enum class TCoordType
    {
    /** Longitude (x) and latitude (y) in degrees. */
    Degree,
    /** Pixels on the display: x increases to the right and y increases downwards. */
    Display,
    /** A synonym for Display: pixels on the display: x increases to the right and y increases downwards.. */
    Screen = Display,
    /** Map coordinates: projected meters, 32nds of meters or 64ths of meters, depending on the map. */
    Map,
    /** Map meters: projected meters, not correcting for distortion introduced by the projection. */
    MapMeter
    };

/** A line in two-dimensional space. */
class TLine
    {
    public:
    TPoint iStart;
    TPoint iEnd;
    };

/** Types used by TOutlinePoint. */
enum class TPointType
    {
    /** A point on the curve. */
    OnCurve,
    /** A control point for a quadratic (conic) Bezier spline curve. */
    Quadratic,
    /** A control point for a cubic Bezier spline curve. */
    Cubic
    };

/** A point on a path. Paths are made from straight line segments and Bezier splines. */
class TOutlinePoint: public TPoint
    {
    public:
    /** Create an on-curve outline point with the coordinates (0,0). */
    constexpr TOutlinePoint() noexcept { }
    /** Create an on-curve outline point from a TPoint. */
    constexpr TOutlinePoint(const TPoint& aPoint) noexcept : TPoint(aPoint) { }
    /** Create an on-curve outline point from a TPoint and a type. */
    constexpr TOutlinePoint(const TPoint& aPoint,TPointType aType) noexcept : TPoint(aPoint), iType(aType) { }
    /** Create an outline point, specifying coordinates and type. */
    constexpr TOutlinePoint(int32_t aX,int32_t aY,TPointType aType) noexcept : TPoint(aX,aY), iType(aType) { }
    /** The equality operator. */
    constexpr bool operator==(const TOutlinePoint& aPoint) const noexcept { return iX == aPoint.iX && iY == aPoint.iY && iType == aPoint.iType; }
    /** The inequality operator. */
    constexpr bool operator!=(const TOutlinePoint& aPoint) const noexcept { return !(*this == aPoint); }

    /** The type of an outline point: on-curve, or a quadratic or cubic control point. */
    TPointType iType { TPointType::OnCurve };
    };

/**
A rectangle in two-dimensional space, aligned with the coordinate system and defined by its
top-left and bottom-right corners.
*/
class TRect
    {
    public:
    /** Create an empty rectangle with both corners at the point (0,0). */
    TRect() { }
    /** Create a rectangle with the specified edges. */
    TRect(int32_t aLeft,int32_t aTop,int32_t aRight,int32_t aBottom) :
        iTopLeft(aLeft,aTop),
        iBottomRight(aRight,aBottom)
        {
        }
    explicit TRect(const TRectFP& aRectFP) noexcept;
    /** The equality operator. */
    bool operator==(const TRect& aRect) const { return iTopLeft == aRect.iTopLeft && iBottomRight == aRect.iBottomRight; }
    /** The inequality operator. */
    bool operator!=(const TRect& aRect) const { return !(*this == aRect); }
    /** Return the left edge. */
    int32_t Left() const { return iTopLeft.iX; }
    /** Return the top edge. */
    int32_t Top() const { return iTopLeft.iY; }
    /** Return the right edge. */
    int32_t Right() const { return iBottomRight.iX; }
    /** Return the bottom edge. */
    int32_t Bottom() const { return iBottomRight.iY; }
    /**
    Return true if the rectangle is empty, defined as having a width or height
    less than or equal to zero.
    */
    bool IsEmpty() const { return Left() >= Right() || Top() >= Bottom(); }
    /**
    Return true if the rectangle is maximal, defined as having left and top of INT32_MIN
    and bottom and right of INT32_MAX.
    */
    bool IsMaximal() const { return iTopLeft.iX == INT32_MIN && iTopLeft.iY == INT32_MIN && iBottomRight.iX == INT32_MAX && iBottomRight.iY == INT32_MAX; }
    /** Return the width. */
    int32_t Width() const { return Right() - Left(); }
    /** Return the height. */
    int32_t Height() const { return Bottom() - Top(); }
    /** Return the top right corner. */
    TPoint TopRight() const { return TPoint(iBottomRight.iX,iTopLeft.iY); }
    /** Return the bottom left corner. */
    TPoint BottomLeft() const { return TPoint(iTopLeft.iX,iBottomRight.iY); }
    /** Return the center. */
    TPoint Center() const { return TPoint((iTopLeft.iX + iBottomRight.iX) / 2,(iTopLeft.iY + iBottomRight.iY) / 2); }
    /**
    Return true if the rectangle contains the point, where containment is defined
    using half-open intervals: the rectangle includes points on its top and
    left edges but not its right and bottom edges.
    */
    bool Contains(const TPoint& aPoint) const
        { return aPoint.iX >= iTopLeft.iX && aPoint.iY >= iTopLeft.iY &&
                 aPoint.iX < iBottomRight.iX && aPoint.iY < iBottomRight.iY; }
    /** Return true if the rectangle contains another rectangle. */
    bool Contains(const TRect& aRect) const
        {
        return iTopLeft.iX <= aRect.iTopLeft.iX &&
               iTopLeft.iY <= aRect.iTopLeft.iY &&
               iBottomRight.iX >= aRect.iBottomRight.iX &&
               iBottomRight.iY >= aRect.iBottomRight.iY;
        }
    bool IsOnEdge(const TPoint& aPoint) const noexcept;
    bool Intersects(const TRect& aRect) const noexcept;
    bool Intersects(const TPoint& aStart,const TPoint& aEnd,
                    TPoint* aIntersectionStart = nullptr,TPoint* aIntersectionEnd = nullptr) const noexcept;
    void Intersection(const TRect& aRect) noexcept;
    void Combine(const TRect& aRect) noexcept;
    void Combine(const TPoint& aPoint) noexcept;

    /** Constants for bits combined to make a return value for Region. */
    static constexpr unsigned KLeftRegion = 1;
    static constexpr unsigned KRightRegion = 1;
    static constexpr unsigned KTopRegion = 1;
    static constexpr unsigned KBottomRegion = 1;
    static constexpr unsigned KTopLeftRegion = KTopRegion | KLeftRegion;
    static constexpr unsigned KTopRightRegion = KTopRegion | KRightRegion;
    static constexpr unsigned KBottomLeftRegion = KBottomRegion | KLeftRegion;
    static constexpr unsigned KBottomRightRegion = KBottomRegion | KRightRegion;

    /**
    Return the region of a point relative to a rectangle.
    A rectangle defines nine possible regions. Region 0 is inside the rectangle,
    and is only returned if the point is inside and doesn't touch the boundary,
    which is useful when clipping paths to a rectangle.
    */
    int32_t Region(const TPoint& aPoint) const
        {
        int32_t region;
        if (aPoint.iX <= iTopLeft.iX)
            region = KLeftRegion;
        else if (aPoint.iX >= iBottomRight.iX)
            region = KRightRegion;
        else
            region = 0;
        if (aPoint.iY <= iTopLeft.iY)
            region |= KTopRegion;
        else if (aPoint.iY >= iBottomRight.iY)
            region |= KBottomRegion;
        return region;
        }


    /** The top left corner. */
    TPoint iTopLeft;
    /** The bottom right corner. */
    TPoint iBottomRight;
    };

/**
A floating-point rectangle in two-dimensional space, aligned with the coordinate system and defined by its
top-left (minimum coords) and bottom-right (maximum coords) corners.

The terms top-left and bottom-right are misleading but are retained for compatibility with TRect and with
existing CartoType practice.
*/
class TRectFP
    {
    public:
    /** Create an empty rectangle with both corners at the point (0,0). */
    TRectFP() { }
    /** Create a rectangle with the specified edges. */
    TRectFP(double aMinX,double aMinY,double aMaxX,double aMaxY):
        iTopLeft(aMinX,aMinY),
        iBottomRight(aMaxX,aMaxY)
        {
        }
    /** Create a rectangle from an integer rectangle. */
    TRectFP(const TRect& aRect):
        iTopLeft(aRect.iTopLeft),
        iBottomRight(aRect.iBottomRight)
        {
        }
    /** The equality operator. */
    bool operator==(const TRectFP& aRect) const { return iTopLeft == aRect.iTopLeft && iBottomRight == aRect.iBottomRight; }
    /** The inequality operator. */
    bool operator!=(const TRectFP& aRect) const { return !(*this == aRect); }
    /** The less-than operator. */
    bool operator<(const TRectFP& aRect) const { return iTopLeft < aRect.iTopLeft || (iTopLeft == aRect.iTopLeft && iBottomRight < aRect.iBottomRight); }
    /** Return the left edge. */
    double Left() const noexcept { return iTopLeft.iX; }
    /** Return the top edge. */
    double Top() const noexcept { return iTopLeft.iY; }
    /** Return the right edge. */
    double Right() const noexcept { return iBottomRight.iX; }
    /** Return the bottom edge. */
    double Bottom() const noexcept { return iBottomRight.iY; }
    /**
    Return true if the rectangle is empty, defined as having a width or height
    less than or equal to zero.
    */
    bool IsEmpty() const noexcept { return Left() >= Right() || Top() >= Bottom(); }
    /** Return the width. */
    double Width() const noexcept { return Right() - Left(); }
    /** Return the height. */
    double Height() const noexcept { return Bottom() - Top(); }
    /** Return the top right corner. */
    TPointFP TopRight() const noexcept { return TPointFP(iBottomRight.iX,iTopLeft.iY); }
    /** Return the bottom left corner. */
    TPointFP BottomLeft() const noexcept { return TPointFP(iTopLeft.iX,iBottomRight.iY); }
    /** Return the center. */
    TPointFP Center() const noexcept { return TPointFP((iTopLeft.iX + iBottomRight.iX) / 2.0,(iTopLeft.iY + iBottomRight.iY) / 2.0); }
    /**
    Return true if the rectangle contains the point, where containment is defined
    using half-open intervals: the rectangle includes points on its top and
    left edges but not its right and bottom edges.
    */
    bool Contains(const TPointFP& aPoint) const noexcept
        { return aPoint.iX >= iTopLeft.iX && aPoint.iY >= iTopLeft.iY &&
                 aPoint.iX < iBottomRight.iX && aPoint.iY < iBottomRight.iY; }
    /** Return true if the rectangle contains another rectangle. */
    bool Contains(const TRectFP& aRect) const noexcept
        {
        return iTopLeft.iX <= aRect.iTopLeft.iX &&
               iTopLeft.iY <= aRect.iTopLeft.iY &&
               iBottomRight.iX >= aRect.iBottomRight.iX &&
               iBottomRight.iY >= aRect.iBottomRight.iY;
        }
    /** Update a rectangle such that it contains the specified point. */
    void Combine(const TPointFP& aPoint)
        {
        if (aPoint.iX < iTopLeft.iX)
            iTopLeft.iX = aPoint.iX;
        if (aPoint.iY < iTopLeft.iY)
            iTopLeft.iY = aPoint.iY;
        if (aPoint.iX > iBottomRight.iX)
            iBottomRight.iX = aPoint.iX;
        if (aPoint.iY > iBottomRight.iY)
            iBottomRight.iY = aPoint.iY;
        }
    /** Set a rectangle to the smallest new rectangle that contains itself and aRect. */
    void Combine(const TRectFP& aRect)
        {
        if (!aRect.IsEmpty())
            {
            if (IsEmpty())
                *this = aRect;
            else
                {
                if (aRect.iTopLeft.iX < iTopLeft.iX)
                    iTopLeft.iX = aRect.iTopLeft.iX;
                if (aRect.iTopLeft.iY < iTopLeft.iY)
                    iTopLeft.iY = aRect.iTopLeft.iY;
                if (aRect.iBottomRight.iX > iBottomRight.iX)
                    iBottomRight.iX = aRect.iBottomRight.iX;
                if (aRect.iBottomRight.iY > iBottomRight.iY)
                    iBottomRight.iY = aRect.iBottomRight.iY;
                }
            }
        }
    /** Set a rectangle to its intersection with aRect. */
    void Intersection(const TRectFP& aRect)
        {
        if (iTopLeft.iX < aRect.iTopLeft.iX)
            iTopLeft.iX = aRect.iTopLeft.iX;
        if (iBottomRight.iX > aRect.iBottomRight.iX)
            iBottomRight.iX = aRect.iBottomRight.iX;
        if (iTopLeft.iY < aRect.iTopLeft.iY)
            iTopLeft.iY = aRect.iTopLeft.iY;
        if (iBottomRight.iY > aRect.iBottomRight.iY)
            iBottomRight.iY = aRect.iBottomRight.iY;
        }
    /**
    Return true if the rectangle and aRect have an intersection.
    If both rectangles are non-empty, return true only
    if the intersection is non-empy.
    */
    bool Intersects(const TRectFP& aRect) const noexcept
        {
        return iTopLeft.iX < aRect.iBottomRight.iX &&
               iBottomRight.iX > aRect.iTopLeft.iX &&
               iTopLeft.iY < aRect.iBottomRight.iY &&
               iBottomRight.iY > aRect.iTopLeft.iY;
        }
    bool Intersects(const TPointFP& aStart,const TPointFP& aEnd,
                    TPointFP* aIntersectionStart = nullptr,TPointFP* aIntersectionEnd = nullptr) const noexcept;
    TRect Rounded() const noexcept;

    /** The top left corner. */
    TPointFP iTopLeft;
    /** The bottom right corner. */
    TPointFP iBottomRight;
    };

/**
A templated interface class defining dictionaries in
which a key type is used to access a value.
*/
template <class Key,class Value> class MDictionary
    {
    public:
    /**
    Although pointers to this class are not owned,
    a virtual destructor is provided for safety and
    to avoid compiler warnings.
    */
    virtual ~MDictionary() { }

    /**
    Return true if aKey exists, false if not.
    If aKey exists in the dictionary, set aValue to
    the value associated with aKey.
    */
    virtual bool Find(const Key& aKey,Value& aValue) const = 0;
    };

/**
A templated interface class defining dictionaries in
which a key is used to access a pointer to a value.
*/
template <class Key,class Value> class MPointerDictionary
    {
    public:
    /**
    Although pointers to this class are not owned,
    a virtual destructor is provided for safety and
    to avoid compiler warnings.
    */
    virtual ~MPointerDictionary() { }

    /**
    Return true if aKey exists, false if not.
    If aKey exists in the dictionary, set aValue to point to
    the value associated with aKey.
    */
    virtual bool Find(const Key& aKey,const Value*& aValue) const = 0;
    };

/** File types of interest to CartoType. */
enum class TFileType
    {
    /** PNG (Portable Network Graphics) image files. */
    PNG,
    /** JPEG (Joint Photographic Expert Group) image files. */
    JPEG,
    /** TIFF (Tagged Image File Format) image files. */
    TIFF,
    /** CTM1 (CartoType Map Data Type 1) map data files. */
    CTM1,
    /** CTMS (CartoType Map Data, Serialized) map data files. */
    CTMS,
    /** KML (Keyhole Markup Language) map data files. */
    KML,
    /** CTSQL (CartoType SQLite format) map data files. */
    CTSQL,
    /** CTROUTE (CartoType XML route) files. */
    CTROUTE,
    /** GPX (GPS Exchange) files. */
    GPX,

    /** Unknown or unspecified file type. */
    None = -1
    };

/** A file location: used for returning the location of an XML or other parsing error. */
class TFileLocation
    {
    public:
    size_t iColumnNumber { 0 };
    size_t iLineNumber { 0 };
    size_t iByteIndex { 0 };
    };

/** Types of writable maps. */
enum class TWritableMapType
    {
    /** A writable map stored in memory. */
    Memory,

    /** A map stored as an SQLite database. */
    SQLite,

    /** Not a writable map type but the number of different writable map types. */
    Count,

    /** A value to use where a null type is needed. */
    None = -1
    };

/** An enumerated type for map objects. */
enum class TMapObjectType
    {
    /** A point in two-dimensional space. */
    Point,

    /** A line in two-dimensional space, consisting of one or more open contours. */
    Line,

    /** A polygon in two-dimensional space, consisting of one or more closed contours. */
    Polygon,

    /** Information arranged as an array of values, such as an image or height data. */
    Array,

    /** The total number of map object types. This is not an actual type. */
    Count,

    /** A value to use where a null type is needed. */
    None = -1
    };

/** Values used in the user-data value of polygon objects in the vector tile drawing system. */
enum class TPolygonClipType
    {
    /** This polygon can be drawn normally, */
    Standard,
    /** This polygon has been clipped and only the interior should be drawn, not the border. */
    Fill,
    /** This is a polyline representing the clipped border of a polygon. */
    Border
    };

/** Positions for notices like the legend or scale bar. Use TExtendedNoticePosition for greater control. */
enum class TNoticePosition
    {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    Top,
    Right,
    Left,
    Bottom,
    Center
    };

/**
Positions for notices like the legend or scale bar.

Notices are positioned by moving them inwards from the designated position
by the specified insets. For central positions (e.g., the X inset in Top,
or both insets in Center) X insets move the position right and Y insets move it down.
Units may be 'cm', 'mm', 'in', 'pt' (points: 1/72in), or 'pc' (picas: 1/6in);
anything else, including null, means pixels.

The default inset is 3mm, or zero for central positions.
*/
class TExtendedNoticePosition
    {
    public:
    TExtendedNoticePosition(TNoticePosition aBasePosition);
    TExtendedNoticePosition(TNoticePosition aBasePosition,double aXInset,const char* aXInsetUnit,double aYInset,const char* aYInsetUnit);
    TNoticePosition BasePosition() const { return m_base_position; }
    double XInset() const { return m_x_inset; }
    const char* XInsetUnit() const { return m_x_inset_unit.data(); }
    double YInset() const { return m_y_inset; }
    const char* YInsetUnit() const { return m_y_inset_unit.data(); }

    private:
    TNoticePosition m_base_position = TNoticePosition::TopLeft;
    double m_x_inset = 0;
    std::array<char,3> m_x_inset_unit = { };
    double m_y_inset = 0;
    std::array<char,3> m_y_inset_unit = { };
    };

/** A holder for arbitrary data. */
class MUserData
    {
    public:
    virtual ~MUserData() { }
    };

/**
The maximum length for a map label in characters.
The large size allows the use of complex multi-line labels.
There has to be a fixed maximum length because of the fixed size
of the glyph cache (see CEngine::EGlyphLogEntries).
*/
constexpr int32_t KMaxLabelLength = 1024;

/**
A constant to convert radians to metres for the Mercator and other projections.
It is the semi-major axis (equatorial radius) used by the WGS 84 datum (see http://en.wikipedia.org/wiki/WGS84).
*/
constexpr int32_t KEquatorialRadiusInMetres = 6378137;

/** The flattening constant used by the WGS84 reference ellipsoid. */
constexpr double KWGS84Flattening = 1.0 / 298.257223563;

constexpr double KRadiansToMetres = KEquatorialRadiusInMetres;
constexpr double KPiDouble = 3.14159265358979;
constexpr float KPiFloat = 3.141592654f;
constexpr double KRadiansToDegreesDouble = 180.0 / KPiDouble;
constexpr double KDegreesToRadiansDouble = KPiDouble / 180.0;

/**
Finds the great-circle distance in metres, assuming a spherical earth, between two lat-long points in degrees.
@see GreatCircleDistanceInMetersUsingEllipsoid, which is a little slower but uses the WGS84 ellipsoid.
*/
inline double GreatCircleDistanceInMeters(double aLong1,double aLat1,double aLong2,double aLat2)
    {
    aLong1 *= KDegreesToRadiansDouble;
    aLat1 *= KDegreesToRadiansDouble;
    aLong2 *= KDegreesToRadiansDouble;
    aLat2 *= KDegreesToRadiansDouble;
    double cos_angle = std::sin(aLat1) * std::sin(aLat2) + std::cos(aLat1) * std::cos(aLat2) * std::cos(aLong2 - aLong1);

    /*
    Inaccurate trig functions can cause cos_angle to be a tiny amount greater than 1 if the two positions are very close.
    That in turn causes acos to gives a domain error and return the special floating point value -1.#IND000000000000,
    meaning 'indefinite', which becomes INT32_MIN when converted to an integer. Observed on 64-bit Windows. See case 1496.
    We fix the problem by means of the following check.
    */
    if (cos_angle >= 1)
        return 0;

    double angle = std::acos(cos_angle);
    return angle * KEquatorialRadiusInMetres;
    }

template<class T> static inline void Reverse(T* aStart,size_t aLength)
    {
    if (aLength > 1)
        {
        T* end = aStart + aLength - 1;
        while (aStart < end)
            {
            T temp = *aStart;
            *aStart = *end;
            *end = temp;
            aStart++;
            end--;
            }
        }
    }

double Length(const TCoordSet& aCoordSet) noexcept;
double SphericalPolygonArea(const TCoordSet& aCoordSet) noexcept;
double SphericalPolygonArea(std::function<const TPointFP*()> aNextPoint) noexcept;
double AzimuthInDegrees(double aLong1,double aLat1,double aLong2,double aLat2) noexcept;
TPointFP PointAtAzimuth(const TPointFP& aPoint,double aDir,double aDistanceInMetres) noexcept;
double GreatCircleDistanceInMetersUsingEllipsoid(double aLong1,double aLat1,double aLong2,double aLat2) noexcept;

/** The standard number of levels of the text index to load into RAM when loading a CTM1 file. */
constexpr int32_t KDefaultTextIndexLevels = 1;

class CMapDataBase;
/** A type for an array of map databases: used internally. */
using CMapDataBaseArray = std::vector<std::unique_ptr<CMapDataBase>>;
/** A type for an array of non-owning pointers to map databases: used internally. */
using CMapDataBaseSet = std::vector<CMapDataBase*>;

inline uint8_t ReadBigEndian(const uint8_t* aP)
    {
    return *aP;
    }
inline void WriteBigEndian(uint8_t* aP,uint8_t aValue)
    {
    *aP = aValue;
    }
inline int16_t ReadBigEndian(const int16_t* aP)
    {
    return (int16_t)( (((const uint8_t*)aP)[0] << 8) | ((const uint8_t*)aP)[1] );
    }
inline void WriteBigEndian(int16_t* aP,int16_t aValue)
    {
    ((uint8_t*)aP)[0] = (uint8_t)(aValue >> 8);
    ((uint8_t*)aP)[1] = (uint8_t)aValue;
    }

/**
Use bilinear interpolation to get the value at (aX,aY) from a rectangular table of data.
Each data item has aChannels channels, and each channel takes up aWidth * sizeof(DataType) bytes.
The UnknownValue is ignored when interpolating.
*/
template<class DataType,int32_t UnknownValue = INT32_MIN> double InterpolatedValue(const DataType* aDataStart,int32_t aWidth,int32_t aHeight,int32_t aStride,int32_t aChannels,double aX,double aY,int aChannel)
    {
    double x_fraction = 1 - (aX - std::floor(aX));
    if ((int)aX == aWidth - 1)
        x_fraction = 1;
    double y_fraction = 1 - (aY - std::floor(aY));
    if ((int)aY == aHeight - 1)
        y_fraction = 1;
    int index = (int)aY * aWidth * aChannels + (int)aX * aChannels + aChannel;
    double top_value = ReadBigEndian(aDataStart + index);
    if (x_fraction < 1)
        {
        double top_right_value = ReadBigEndian(aDataStart + index + aChannels);
        if (top_right_value != UnknownValue)
            top_value = top_value * x_fraction + top_right_value * (1.0 - x_fraction);
        }
    double value = top_value;
    if (y_fraction < 1)
        {
        index += aStride * aChannels;
        double bottom_value = ReadBigEndian(aDataStart + index);
        if (x_fraction < 1)
            {
            double bottom_right_value = ReadBigEndian(aDataStart + index + aChannels);
            if (bottom_right_value != UnknownValue)
                bottom_value = bottom_value * x_fraction + bottom_right_value * (1.0 - x_fraction);
            }
        if (bottom_value != UnknownValue)
            value = top_value * y_fraction + bottom_value * (1.0 - y_fraction);
        }
    return value;
    }

class TPointInPolygonHelper
    {
    public:
    TPointInPolygonHelper(const TPointFP& aTestPoint,const TPointFP& aFirstVertex):
        m_test_point(aTestPoint),
        m_prev_point(aFirstVertex)
        {
        }
    bool Inside() const { return m_inside; }

    // See 'Horman and Agathos: The Point in Polygon Problem for Arbitrary Polygons' for an explanation of the algorithm.
    void operator()(const TPointFP& aPoint)
        {
        if ((aPoint.iY < m_test_point.iY) != (m_prev_point.iY < m_test_point.iY)) // if crossing
            {
            if (m_prev_point.iX >= m_test_point.iX)
                {
                if (aPoint.iX > m_test_point.iX || RightCrossing(aPoint))
                    m_inside = !m_inside;
                }
            else if (aPoint.iX > m_test_point.iX && RightCrossing(aPoint))
                m_inside = !m_inside;
            }

        m_prev_point = aPoint;
        }

    private:
    double Det(const TPointFP& aPoint) const { return (m_prev_point.iX - m_test_point.iX) * (aPoint.iY - m_test_point.iY) - (aPoint.iX - m_test_point.iX) * (m_prev_point.iY - m_test_point.iY); }
    bool RightCrossing(const TPointFP& aPoint) { return (Det(aPoint) > 0) == (aPoint.iY > m_prev_point.iY); }

    const TPointFP m_test_point;
    TPointFP m_prev_point;
    bool m_inside = false;
    };

} // namespace CartoType

#ifdef _WIN32_WCE
#ifdef __cplusplus
extern "C" {
#endif
char* getenv(const char* name);
void abort(void);
#ifdef __cplusplus
}
#endif
#endif

#endif
