/*
CARTOTYPE_TRANSFORM.H
Copyright (C) 2004-2018 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_TRANSFORM_H__
#define CARTOTYPE_TRANSFORM_H__

#include <cartotype_base.h>
#include <cartotype_arithmetic.h>
#include <cartotype_stream.h>
#include <array>

namespace CartoType
{

/** Transform type flag bits; used to reduce calculation. */
namespace TTransformType
    {
    /** The transform has no effect. */
    constexpr uint32_t Identity = 0;
    /** The transform reflects around the line (y = 0). */
    constexpr uint32_t YReflect = 1;
    /** The transform does translation. */
    constexpr uint32_t Translate = 2;
    /** The transform has an effect, and does something other than reflection and translation. */ 
    constexpr uint32_t General = 4;
    };

/**
A two-dimensional affine transform.
The parameter names A, B, C, D, Tx, Ty are those used in the
PostScript Language Reference Manual.
*/
class TTransform
    {
    public:
    /** Create an identity transform. */
    TTransform(): iA(65536,TFixed::TRaw::Value), iD(65536,TFixed::TRaw::Value), iType(TTransformType::Identity) { }
    /** Return true if the transform is identity (has no effect). */
    bool IsIdentity() const { return iType == TTransformType::Identity; }
    /** Set the transform to identity. */
    void SetIdentity() 
        {
        if (iType != TTransformType::Identity)
            {
            iA.SetRawValue(65536);
            iB.SetRawValue(0);
            iC.SetRawValue(0);
            iD.SetRawValue(65536);
            iTx.SetRawValue(0);
            iTy.SetRawValue(0);
            iType = TTransformType::Identity;
            }
        }
    /** Return the A (x scale) parameter. */
    TFixed A() const { return iA; }
    /** Return the B (y shear) parameter. */
    TFixed B() const { return iB; }
    /** Return the C (x shear) parameter. */
    TFixed C() const { return iC; }
    /** Return the D (y scale) parameter. */
    TFixed D() const { return iD; }
    /** Return the TX (x translation) parameter. */
    TFixed Tx() const { return iTx; }
    /** Return the TY (y translation) parameter. */
    TFixed Ty() const { return iTy; }
    TTransform(TFixed aA,TFixed aB,TFixed aC,TFixed aD,TFixed aTx,TFixed aTy);
    bool operator==(const TTransform& aTransform) const;
    /** The inequality operator. */
    bool operator!=(const TTransform& aTransform) const { return !(*this == aTransform); }
    void Concat(const TTransform& aTransform);
    void Prefix(const TTransform& aTransform);
    void Transform(TPoint& aPoint) const;
    void TransformPointIn64ths(TPoint& aPoint) const;
    void Transform(TFixed& aX,TFixed& aY) const;
    /** Transform a TPointFixed object in place. */
    void Transform(TPointFixed& aPoint) const { Transform(aPoint.iX,aPoint.iY); }
    void ExtractScale(TFixed& aXScale,TFixed& aYScale);
    void Rotate(TFixed aX,TFixed aY);
    void Scale(TFixed aXScale,TFixed aYScale);
    void Translate(TFixed aXOffset,TFixed aYOffset);
    TResult Invert();

    uint32_t Type() const { return iType; }

    private:
    TFixed iA, iB, iC, iD, iTx, iTy;    // parameters
    uint32_t iType;                     // non-zero if the transform is not identity
    };

/** The components of a transform. */
class TTransformComponents
    {
    public:
    TPointFP iTranslation;
    TPointFP iCenterOfRotation;
    double iRotation = 0;
    TPointFP iScale = { 1, 1 };
    TPointFP iShear;
    };

/** A floating point affine 2D transformation. */
class TTransformFP
    {
    public:
    TTransformFP();
    TTransformFP(double aA,double aB,double aC,double aD,double aTx,double aTy);
    TTransformFP(const TTransform& aTransform);
    void Transform(double& aX,double& aY) const
        {
        double new_x(iA * aX + iC * aY + iTx);
        double new_y(iB * aX + iD * aY + iTy);
        aX = new_x;
        aY = new_y;
        }
    void Transform(TPoint& aPoint) const;
    void Transform(TPointFP& aPoint) const;
    void Transform(TRect& aRect) const;
    void Transform(TRectFP& aRect) const;
    void Concat(const TTransformFP& aTransform);
    void Scale(double aXScale,double aYScale);
    TResult Invert();
    void Translate(double aX,double aY) { iTx += aX; iTy += aY; }
    void Rotate(double aAngle);
    void Rotate(double aX,double aY) { Rotate(Atan2(aY,aX)); }
    void ReflectY(double aY)
        {
        iTy -= aY;
        Scale(1,-1);
        iTy += aY;
        }
    double A() const { return iA; }
    double B() const { return iB; }
    double C() const { return iC; }
    double D() const { return iD; }
    double Tx() const { return iTx; }
    double Ty() const { return iTy; }
    TPointFP CenterOfRotation() const;
    TTransformComponents Components() const;
    TTransformFP Interpolate(const TTransformFP& aOther,double aTime);
    bool operator==(const TTransformFP& aTransform) const;
    bool operator!=(const TTransformFP& aTransform) const { return !(*this == aTransform); }
    uint32_t Type() const;
    TResult Write(TDataOutputStream& aOutput) const;
    static TTransformFP Read(TResult& aError,TDataInputStream& aInput);

    private:
    double iA, iB, iC, iD, iTx, iTy;
    };

/** A 3D transformation. */
class TTransform3FP
    {
    public:
    TTransform3FP();
    TTransform3FP(const TTransformFP& aTransform);
    bool operator==(const TTransform3FP& aOther) const { return iM == aOther.iM; }
    void Transform(TPoint3FP& aPoint) const;
    void Transform(double& aX,double& aY,double& aZ,double& aW) const;
    void Concat(const TTransform3FP& aTransform);
    void Translate(double aX,double aY,double aZ);
    void Scale(double aXScale,double aYScale,double aZScale);
    void RotateX(double aAngle);
    void RotateY(double aAngle);
    void RotateZ(double aAngle);
    void ReflectY(double aY);
    TResult Invert();
    double Determinant() const;
    TResult Perspective(double aFieldOfViewYDegrees,double aAspect,double aNear,double aFar);
    TResult Frustum(double aLeft,double aRight,double aBottom,double aTop,double aNear,double aFar);
    TTransformFP Affine2DTransform() const;

    const double* Data() const { return iM.data(); }
   
    private:
    std::array<double,16> iM; // the transform matrix
    };

/** 
Parameters defining a camera position relative to a flat plane
representing the earth's surface projected on to a map.
*/
class TCameraParam
    {
    public:
    /** The position of the camera. The z coordinate represents height above the surface. */
    TPoint3FP iPosition;
    /** The azimuth of the camera in degrees going clockwise, where 0 is N, 90 is E, etc. */
    double iAzimuthDegrees = 0;
    /** The declination of the camera downward from the horizontal plane. */
    double iDeclinationDegrees = 30;
    /** The amount by which the camera is rotated about its axis, after applying the declination, in degrees going clockwise. */
    double iRotationDegrees = 0;
    /** The camera's field of view in degrees. */
    double iFieldOfViewDegrees = 22.5;
    /**
    The display rectangle on to which the camera view is projected.
    The view width is scaled to that of the display, and the aspect
    ratio of the camera view is preserved.
    */
    TRectFP iDisplay = { 0,0,1,1 };
    /** True if y values increase upwards in the display coordinate system. */
    bool iYAxisUp = false;
    };

class TPerspectiveTransformFP
    {
    public:
    TPerspectiveTransformFP() { }
    explicit TPerspectiveTransformFP(const TCameraParam& aCameraParam);
    void Transform(TPoint3FP& aPoint) const;
    void Transform(TPointFP& aPoint) const;
    void TransformToCamera(TPoint3FP& aPoint) const;
    void TransformFromCamera(TPoint3FP& aPoint) const;
    void InverseTransform(TPointFP& aPoint) const;
    static void InverseTransform(const TTransform3FP& aInverseTransform,TPointFP& aPoint);
    const TTransform3FP& Transform() const { return iTransform; }
    
    private:
    TTransform3FP iTransform;
    TTransform3FP iInverseTransform;
    TTransform3FP iTransformToCamera;
    TTransform3FP iInverseTransformToCamera;
    };

} // namespace CartoType

#endif
