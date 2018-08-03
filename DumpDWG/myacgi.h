//
//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2018 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
//////////////////////////////////////////////////////////////////////////////
//
// MYACGI.H
#include "tchar.h"

class myWorldGeometry : public AcGiWorldGeometry
{
public:
    virtual void getModelToWorldTransform(AcGeMatrix3d &pMatrix) const;
    virtual void getWorldToModelTransform(AcGeMatrix3d &pMatrix) const;

    /*NEW*/virtual Adesk::Boolean pushModelTransform(const AcGeVector3d & vNormal);
    /*NEW*/virtual Adesk::Boolean pushModelTransform(const AcGeMatrix3d & xMat);
    /*NEW*/virtual Adesk::Boolean popModelTransform();

    virtual void setExtents(AcGePoint3d *pNewExtents) const;
    virtual Adesk::Boolean  circle(const AcGePoint3d& center,
                const double radius,
                const AcGeVector3d& normal
                                ) const;
    virtual Adesk::Boolean  circle(const AcGePoint3d& pt1,
                const AcGePoint3d& pt2,
                const AcGePoint3d& pt3
                                ) const;
    virtual Adesk::Boolean  circularArc(const AcGePoint3d& center,
                const double radius,
                const AcGeVector3d& normal,
                const AcGeVector3d& startVector,
                const double sweepAngle,
                const AcGiArcType arcType = kAcGiArcSimple
                                ) const;
    virtual Adesk::Boolean  circularArc(const AcGePoint3d& start,
                const AcGePoint3d& point,
                const AcGePoint3d& end,
                const AcGiArcType arcType = kAcGiArcSimple
                                ) const;
    virtual Adesk::Boolean  polyline(const Adesk::UInt32 nbPoints,
                                     const AcGePoint3d* pVertexList,
                                     const AcGeVector3d* pNormal = NULL,
                                     Adesk::LongPtr lBaseSubEntMarker = -1
                                    ) const;
    virtual Adesk::Boolean  polyline(const AcGiPolyline  & polyline
                                    ) const;
    virtual Adesk::Boolean  polyPolyline(Adesk::UInt32 nbPolylines,
                                         const AcGiPolyline * pPolylines
                                        ) const;
    virtual Adesk::Boolean  polygon(const Adesk::UInt32 nbPoints,
                const AcGePoint3d* pVertexList
                                ) const;
    virtual Adesk::Boolean  polyPolygon(const Adesk::UInt32 numPolygonIndices,
                                const Adesk::UInt32* numPolygonPositions,
                                const AcGePoint3d* polygonPositions,
                                const Adesk::UInt32* numPolygonPoints,
                                const AcGePoint3d* polygonPoints,
                                const AcCmEntityColor* outlineColors = NULL,
                                const AcGiLineType* outlineTypes = NULL,
                                const AcCmEntityColor* fillColors = NULL,
                                const AcCmTransparency* fillOpacities = NULL) const;
    virtual Adesk::Boolean  mesh(const Adesk::UInt32 rows,
                const Adesk::UInt32 columns,
                const AcGePoint3d* pVertexList,
                const AcGiEdgeData* pEdgeData,
                const AcGiFaceData* pFaceData,
                const AcGiVertexData* pVertexData = NULL,
                const bool bAutoGenerateNormals = true
                ) const;
    virtual Adesk::Boolean  shell(const Adesk::UInt32 nbVertex,
                const AcGePoint3d* pVertexList,
                const Adesk::UInt32 faceListSize,
                const Adesk::Int32* pFaceList,
                const AcGiEdgeData* pEdgeData,
                const AcGiFaceData* pFaceData,
                const AcGiVertexData* pVertexData = NULL,
                const struct resbuf* pResBuf = NULL,
                const bool bAutoGenerateNormals = true
                ) const;
    virtual Adesk::Boolean  text(const AcGePoint3d& position,
                const AcGeVector3d& normal,
                const AcGeVector3d& direction,
                const double height,
                const double width,
                const double oblique,
                const TCHAR* pMsg
                                ) const;
    virtual Adesk::Boolean  text(const AcGePoint3d& position,
                const AcGeVector3d& normal,
                const AcGeVector3d& direction,
                const TCHAR* pMsg,
                const Adesk::Int32 length,
                const Adesk::Boolean raw,
                const AcGiTextStyle &pTextStyle
                                ) const;
    virtual Adesk::Boolean  xline(const AcGePoint3d& one_xline_point,
                                const AcGePoint3d& a_different_xline_point
                                ) const;
    virtual Adesk::Boolean  ray(const AcGePoint3d& rays_starting_point,
                                const AcGePoint3d& a_different_ray_point
                                ) const;

    virtual Adesk::Boolean  pline(const AcDbPolyline& lwBuf,
                                        Adesk::UInt32 fromIndex,
                                        Adesk::UInt32 numSegs
                                  ) const;
    /*NEW*/virtual Adesk::Boolean  draw(AcGiDrawable*) const;
    /*NEW*/virtual Adesk::Boolean  pushClipBoundary(AcGiClipBoundary *pBoundary);
    /*NEW*/virtual void popClipBoundary();

    virtual AcGeMatrix3d pushPositionTransform (AcGiPositionTransformBehavior behavior, const AcGePoint3d& offset);

    virtual AcGeMatrix3d pushPositionTransform (AcGiPositionTransformBehavior behavior, const AcGePoint2d& offset); 

    virtual AcGeMatrix3d pushScaleTransform(AcGiScaleTransformBehavior behavior, const AcGePoint3d& extents); 

    virtual AcGeMatrix3d pushScaleTransform(AcGiScaleTransformBehavior behavior, const AcGePoint2d& extents); 

    virtual AcGeMatrix3d pushOrientationTransform(AcGiOrientationTransformBehavior behavior);

    virtual Adesk::Boolean image   (
                const AcGiImageBGRA32& imageSource,
                const AcGePoint3d& position,
                const AcGeVector3d& u,
                const AcGeVector3d& v,
                TransparencyMode transparencyMode = kTransparency8Bit
                ) const;

    virtual Adesk::Boolean rowOfDots   (
                int count,
                const AcGePoint3d&     start,
                const AcGeVector3d&     step
                ) const;

    virtual Adesk::Boolean ellipticalArc  (
                 const AcGePoint3d&         center,
                 const AcGeVector3d&        normal,
                 double                     majorAxisLength,
                 double                     minorAxisLength,
                 double                     startDegreeInRads,
                 double                     endDegreeInRads,
                 double                     tiltDegreeInRads,
                 AcGiArcType                arcType
                 ) const;

    virtual Adesk::Boolean edge (const AcArray<AcGeCurve2d*>& edge) const;
};

#ifdef WORLD_DRAW

class myContext : public AcGiContext
{
public:
    myContext() : m_pDb(NULL) {};

    // If this returns TRUE then we are regening for extents information.
    // Either draw yourself normally or pass one or more WCS points to the
    // candidate point function, whichever is quicker for you
    //
    virtual Adesk::Boolean      isRegenForExtents() const;
    virtual void                candidateWCSPoint(const AcGePoint3d & ptWCSCand);
    virtual Adesk::Boolean      isPsOut() const;
    virtual Adesk::Boolean      isPlotGeneration() const;
    /*NEW*/virtual bool isBoundaryClipping() const;
    // Model to World transform manipulations, these **MUST** be match paired
    //
    virtual void pushModelTransform(const AcGeVector3d & vNormal);// Uses arbitrary Axis algorythm
    virtual void pushModelTransform(const AcGeMatrix3d & xMat);
    virtual void popModelTransform();
    virtual const AcGeMatrix3d & modelTransform() const;
    virtual const AcGeMatrix3d & inverseModelTransform() const;
    // These calls must be paired, it sets up the byblock information and
    // transformation of the given block reference
    //
    virtual void pushBlockReference(AcDbBlockReference * pBlockReference);
    virtual void popBlockReference();
    virtual AcDbBlockReference * blockReference() const;
    virtual AcDbDatabase * database() const;

    void setDatabase(AcDbDatabase *  pDb)
    {
        m_pDb = pDb;
    }
    AcDbDatabase * m_pDb;
};

#endif //WORLD_DRAW


class mySubEntityTraits : public AcGiSubEntityTraits
{
public:
             mySubEntityTraits();
    virtual ~mySubEntityTraits();

    virtual void setColor(const Adesk::UInt16 color);
    /*NEW*/ virtual void setTrueColor(const AcCmEntityColor& color);
    virtual void setLayer(const AcDbObjectId layerId);
    virtual void setLineType(const AcDbObjectId linetypeId);
    virtual void setSelectionMarker(const Adesk::LongPtr markerId);
    virtual void setFillType(const AcGiFillType);
    /*NEW*/ virtual void setLineWeight(const AcDb::LineWeight lw);
    virtual Adesk::UInt16      color() const;
    /*NEW*/ virtual AcCmEntityColor trueColor() const;
    virtual AcDbObjectId       layerId() const;
    virtual AcDbObjectId       lineTypeId() const;
    virtual AcGiFillType       fillType() const;
    /*NEW*/ AcDb::LineWeight   lineWeight() const;
    virtual void setTransparency( const AcCmTransparency &transparency );


    void setLineTypeScale(double);
    double lineTypeScale() const;
    double m_dLTScale;

    virtual void setThickness(double dThickness);
    virtual double thickness() const;
    double m_dThickness;
};

class myWorldDraw : public AcGiWorldDraw
{
public:
    myWorldDraw();
    ~myWorldDraw();
    virtual AcGiRegenType           regenType() const;
    virtual Adesk::Boolean          regenAbort() const;
    virtual AcGiSubEntityTraits&    subEntityTraits() const;
    virtual AcGiWorldGeometry&      geometry() const;
    virtual AcGiGeometry*           rawGeometry() const;
    virtual Adesk::Boolean      isDragging() const;
    virtual double          deviation(const AcGiDeviationType, 
                          const AcGePoint3d&) const;
    virtual Adesk::UInt32       numberOfIsolines() const;

#ifdef WORLD_DRAW
    virtual AcGiContext  *          context();
    virtual void                    regenerate(AcDbEntity * pEntity);
    virtual void                    regenerate(AcDbBlockTableRecord * pBTR);
#endif // WORLD_DRAW

    mySubEntityTraits *mpTraits;
    myWorldGeometry *mpGeom;
#ifdef WORLD_DRAW
    myContext       *mpContext;
#endif // WORLD_DRAW
};




