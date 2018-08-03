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


#include "adesk.h"
#include "acgi.h"
#include "myacgi.h"
#include "stdio.h"


void _print(TCHAR *format, ...);

#ifdef WORLD_DRAW


AcDbDatabase *    
myContext::database() const
{
    return m_pDb;
}

bool myContext::isBoundaryClipping() const
{
    _print(_T("\t\tmyContext::isBoundaryClipping()\n"));
    return false;
}


// If this returns TRUE then we are regening for extents information.
// Either draw yourself normally or pass one or more WCS points to the
// candidate point function, whichever is quicker for you
//
Adesk::Boolean      
myContext::isRegenForExtents() const
{
    return Adesk::kFalse;
}

void                
myContext::candidateWCSPoint(const AcGePoint3d & ptWCSCand)
{
    _print(_T("\t\tmyContext::candidateWCSPoint\n"));
}


Adesk::Boolean      
myContext::isPsOut() const
{
    return Adesk::kFalse;
}


Adesk::Boolean      
myContext::isPlotGeneration() const
{
    return Adesk::kFalse;
}


// Model to World transform manipulations, these **MUST** be match paired
//
void 
myContext::pushModelTransform(const AcGeVector3d & vNormal)
{
    _print(_T("\t\tmyContext::pushModelTransform(vector)\n"));
}

// Uses arbitrary Axis algorythm
void 
myContext::pushModelTransform(const AcGeMatrix3d & xMat)
{
    _print(_T("\t\tmyContext::pushModelTransform(matrix)\n"));
}


void 
myContext::popModelTransform()
{
    _print(_T("\t\tmyContext::popModelTransform()\n"));
}


const AcGeMatrix3d & 
myContext::modelTransform() const
{
    static AcGeMatrix3d s_xIdent;

    return s_xIdent;
}


const AcGeMatrix3d & 
myContext::inverseModelTransform() const
{
    static AcGeMatrix3d s_xIdent;

    return s_xIdent;
}


// These calls must be paired, it sets up the byblock information and
// transformation of the given block reference
//
void 
myContext::pushBlockReference(AcDbBlockReference * pBlockReference)
{
    _print(_T("\t\tmyContext::pushBlockReference()\n"));
}


void 
myContext::popBlockReference()
{
    _print(_T("\t\tmyContext::popBlockReference()\n"));
}


AcDbBlockReference * 
myContext::blockReference() const
{
    return NULL;
}



#endif //WORLD_DRAW


void myWorldGeometry::getModelToWorldTransform(AcGeMatrix3d &pMatrix) const
{
    _print(_T("\t\tmyWorldGeometry::modelToWorldTransform\n"));
}

void myWorldGeometry::getWorldToModelTransform(AcGeMatrix3d &pMatrix) const
{
    _print(_T("\t\tmyWorldGeometry::worldToModelTransform\n"));
}

void myWorldGeometry::setExtents(AcGePoint3d *pNewExtents) const
{
    _print(_T("\t\tmyWorldGeometry::setExtents\n"));
}

Adesk::Boolean myWorldGeometry::circle(const AcGePoint3d& center,
                const double radius,
                const AcGeVector3d& normal
                                ) const
{
    _print(_T("\t\tmyWorldGeometry::circle\n"));
    return 0;
}
                                
Adesk::Boolean myWorldGeometry::circle(const AcGePoint3d& pt1,
                const AcGePoint3d& pt2,
                const AcGePoint3d& pt3
                                ) const
{
    _print(_T("\t\tmyWorldGeometry::circle\n"));
    return 0;
}
                                
Adesk::Boolean myWorldGeometry::circularArc(const AcGePoint3d& center,
                const double radius,
                const AcGeVector3d& normal,
                const AcGeVector3d& startVector,
                const double sweepAngle,
                const AcGiArcType arcType // = kAcGiArcSimple
                                ) const
{
    _print(_T("\t\tmyWorldGeometry::circularArc\n"));
    return 0;
}
                                
Adesk::Boolean myWorldGeometry::circularArc(const AcGePoint3d& start,
                const AcGePoint3d& point,
                const AcGePoint3d& end,
                const AcGiArcType arcType // = kAcGiArcSimple
                                ) const
{
    _print(_T("\t\tmyWorldGeometry::circularArc\n"));
    return 0;
}
                                
Adesk::Boolean myWorldGeometry::polyline(const Adesk::UInt32 nbPoints,
                const AcGePoint3d* pVertexList,
                const AcGeVector3d* pNormal, // = NULL
                Adesk::LongPtr lBaseSubEntMarker
                                ) const
{
    _print(_T("\t\tmyWorldGeometry::polyline %d\n"), nbPoints);
    Adesk::UInt32 i;
    for (i = 0; i < nbPoints; i++)
    {
        _print(_T("\t\t\tAcGePoint3d %g,%g,%g\n"), 
            pVertexList[i].x,
            pVertexList[i].y,
            pVertexList[i].z);
    }
    if (pNormal)
    {
        _print(_T("\t\t\tAcGeVector3d %g,%g,%g\n"), 
            pNormal->x,
            pNormal->y,
            pNormal->z);
    }
    _print(_T("\t\tAdesk::Int32 %d\n"), lBaseSubEntMarker);
    return 0;
}

Adesk::Boolean 
myWorldGeometry::polyline  (const AcGiPolyline& polylineObj) const
{
    _print(_T("\t\tmyWorldGeometry::polyline2\n"));
    polyline (polylineObj.points(), 
              polylineObj.vertexList(), 
              polylineObj.normal(), 
              polylineObj.baseSubEntMarker());
    return Adesk::kFalse;
}

Adesk::Boolean 
myWorldGeometry::polyPolyline (Adesk::UInt32 nbPolylines,
                               const AcGiPolyline * pPolylines) const
{
    _print(_T("\t\tmyWorldGeometry::polylines\n"));
    for (Adesk::UInt32 i = 0; i < nbPolylines; ++i)
        polyline (pPolylines[i]);
    return Adesk::kFalse;
}
                                
Adesk::Boolean myWorldGeometry::polygon(const Adesk::UInt32 nbPoints,
                const AcGePoint3d* pVertexList
                                ) const
{
    _print(_T("\t\tmyWorldGeometry::polygon %d\n"), nbPoints);
    Adesk::UInt32 i;
    for (i = 0; i < nbPoints; i++)
    {
        _print(_T("\t\t\tAcGePoint3d %g,%g,%g\n"), 
            pVertexList[i].x,
            pVertexList[i].y,
            pVertexList[i].z);
    }
    return 0;
}
Adesk::Boolean 
myWorldGeometry::polyPolygon(const Adesk::UInt32 numPolygonIndices,
                             const Adesk::UInt32* numPolygonPositions,
                             const AcGePoint3d* polygonPositions,
                             const Adesk::UInt32* numPolygonPoints,
                             const AcGePoint3d* polygonPoints,
                             const AcCmEntityColor* outlineColors,
                             const AcGiLineType* outlineTypes,
                             const AcCmEntityColor* fillColors,
                             const AcCmTransparency* fillOpacities
                             ) const
{
    _print(_T("\t\tmyWorldGeometry::polyPolygon\n"));
    return Adesk::kFalse;
}
                                
Adesk::Boolean myWorldGeometry::mesh(const Adesk::UInt32 rows,
                const Adesk::UInt32 columns,
                const AcGePoint3d* pVertexList,
                const AcGiEdgeData* pEdgeData,
                const AcGiFaceData* pFaceData,
                const AcGiVertexData* pVertexData, // = NULL
                const bool bAutoGenerateNormals    // = true
                ) const
{
    _print(_T("\t\tmyWorldGeometry::mesh\n"));
    return 0;
}
                                
Adesk::Boolean myWorldGeometry::shell(const Adesk::UInt32 nbVertex,
                const AcGePoint3d* pVertexList,
                const Adesk::UInt32 faceListSize,
                const Adesk::Int32* pFaceList,
                const AcGiEdgeData* pEdgeData,     // = NULL
                const AcGiFaceData* pFaceData,     // = NULL
                const AcGiVertexData* pVertexData, // = NULL
                const struct resbuf* pResBuf,      // = NULL
                const bool bAutoGenerateNormals    // = true
                ) const
{
    _print(_T("\t\tmyWorldGeometry::shell\n"));
    return 0;
}
                                
Adesk::Boolean myWorldGeometry::text(const AcGePoint3d& position,
                const AcGeVector3d& normal,
                const AcGeVector3d& direction,
                const double height,
                const double width,
                const double oblique,
                const TCHAR* pMsg
                                ) const
{
    _print(_T("\t\tmyWorldGeometry::text\n"));
    return 0;
}
                                
Adesk::Boolean myWorldGeometry::text(const AcGePoint3d& position,
                const AcGeVector3d& normal,
                const AcGeVector3d& direction,
                const TCHAR* pMsg,
                const Adesk::Int32 length,
                const Adesk::Boolean raw,
                const AcGiTextStyle &pTextStyle
                                ) const
{
    _print(_T("\t\tmyWorldGeometry::text\n"));
    return 0;
}
                                
Adesk::Boolean myWorldGeometry::xline(const AcGePoint3d& one_xline_point,
                                const AcGePoint3d& a_different_xline_point
                                ) const
{
    _print(_T("\t\tmyWorldGeometry::xline\n"));
    return 0;
}
                                
Adesk::Boolean myWorldGeometry::ray(const AcGePoint3d& rays_starting_point,
                                const AcGePoint3d& a_different_ray_point
                                ) const
{
    _print(_T("\t\tmyWorldGeometry::ray\n"));
    return 0;
}
                                
Adesk::Boolean myWorldGeometry::pline(const AcDbPolyline& lwBuf,
                                        Adesk::UInt32 fromIndex,  // = 0
                                        Adesk::UInt32 numSegs     // = 0
                                        ) const
{
    _print(_T("\t\tmyWorldGeometry::pline\n"));
    return Adesk::kTrue;
}

Adesk::Boolean myWorldGeometry::draw(AcGiDrawable* draw) const
{
    _print(_T("\t\tmyWorldGeometry::draw()\n"));
    return Adesk::kTrue;
}

Adesk::Boolean myWorldGeometry::pushModelTransform(const AcGeVector3d& vNormal)
{
    _print(_T("\t\tmyWorldGeometry::pushModelTransform\n"));
    return Adesk::kTrue;
}

Adesk::Boolean myWorldGeometry::pushModelTransform(const AcGeMatrix3d &xMat)
{
    _print(_T("\t\tmyWorldGeometry::pushModelTransform\n"));
    return Adesk::kTrue;
}

Adesk::Boolean myWorldGeometry::popModelTransform()
{
    _print(_T("\t\tmyWorldGeometry::popModelTransform\n"));
    return Adesk::kTrue;
}

Adesk::Boolean myWorldGeometry::pushClipBoundary(AcGiClipBoundary* pBoundary)
{
    _print(_T("\t\tmyWorldGeometry::pushClipBoundary()\n"));
    return Adesk::kTrue;
}

void myWorldGeometry::popClipBoundary()
{
    _print(_T("\t\tmyWorldGeometry::popClipBoundary\n"));
}

Adesk::Boolean
myWorldGeometry::image   (
                const AcGiImageBGRA32& imageSource,
                const AcGePoint3d& position,
                const AcGeVector3d& u,
                const AcGeVector3d& v,
                AcGiGeometry::TransparencyMode transparencyMode
                ) const
{
    _print(_T("\t\tmyWorldGeometry::image\n"));
    return Adesk::kFalse;
}

Adesk::Boolean 
myWorldGeometry::rowOfDots   (
                int count,
                const AcGePoint3d&     start,
                const AcGeVector3d&     step
                ) const
{
    _print(_T("\t\tmyWorldGeometry::rowOfDots\n"));
    return Adesk::kFalse;
}

Adesk::Boolean 
myWorldGeometry::ellipticalArc   (
                 const AcGePoint3d&         center,
                 const AcGeVector3d&        normal,
                 double                     majorAxisLength,
                 double                     minorAxisLength,
                 double                     startDegreeInRads,
                 double                     endDegreeInRads,
                 double                     tiltDegreeInRads,
                 AcGiArcType                arcType) const
{
    _print(_T("\t\tmyWorldGeometry::ellipticalArc\n"));
    return Adesk::kFalse;
}

AcGeMatrix3d 
myWorldGeometry::pushPositionTransform (AcGiPositionTransformBehavior behavior, const AcGePoint3d& offset)
{
    _print(_T("\t\tmyWorldGeometry::pushPositionTransform\n"));
    return AcGeMatrix3d::kIdentity;
}

AcGeMatrix3d 
myWorldGeometry::pushPositionTransform (AcGiPositionTransformBehavior behavior, const AcGePoint2d& offset)
{
    _print(_T("\t\tmyWorldGeometry::pushPositionTransform\n"));
    return AcGeMatrix3d::kIdentity;
}

AcGeMatrix3d 
myWorldGeometry::pushScaleTransform (AcGiScaleTransformBehavior behavior, const AcGePoint3d& extents)
{
    _print(_T("\t\tmyWorldGeometry::pushScaleTransform\n"));
    return AcGeMatrix3d::kIdentity;
}

AcGeMatrix3d 
myWorldGeometry::pushScaleTransform (AcGiScaleTransformBehavior behavior, const AcGePoint2d& extents)
{
    _print(_T("\t\tmyWorldGeometry::pushScaleTransform\n"));
    return AcGeMatrix3d::kIdentity;
}


AcGeMatrix3d 
myWorldGeometry::pushOrientationTransform (AcGiOrientationTransformBehavior behavior)
{
    _print(_T("\t\tmyWorldGeometry::pushOrientationTransform\n"));
    return AcGeMatrix3d::kIdentity;
}

Adesk::Boolean myWorldGeometry::edge (const AcArray<AcGeCurve2d*>& edge) const
{
    _print(_T("\t\tmyWorldGeometry::edge\n"));
    return Adesk::kFalse;
}

mySubEntityTraits::mySubEntityTraits()
: m_dThickness(0.)
{
}

mySubEntityTraits::~mySubEntityTraits()
{
}

void mySubEntityTraits::setColor(const Adesk::UInt16 color)
{
    _print(_T("\t\tmySubEntityTraits::setColor %d\n"), color);
}

void mySubEntityTraits::setTrueColor(const AcCmEntityColor& color)
{
    _print(_T("\t\tmySubEntityTraits::setTrueColor %lx\n"), color.color());
}

void mySubEntityTraits::setTransparency( const AcCmTransparency &transparency )
{
    _print(_T("\t\tmySubEntityTraits::setTransparency %lx\n"), transparency.serializeOut());
}

void mySubEntityTraits::setLayer(const AcDbObjectId layerId)
{
    _print(_T("\t\tmySubEntityTraits::setLayer layerId %Ix\n"), &layerId);
}

void mySubEntityTraits::setLineType(const AcDbObjectId linetypeId)
{
    _print(_T("\t\tmySubEntityTraits::setLineType id %Ix\n"), &linetypeId);
}

void mySubEntityTraits::setSelectionMarker(const Adesk::LongPtr markerId)
{
    _print(_T("\t\tmySubEntityTraits::setSelectionMarker %Ix\n"), markerId);
}

void mySubEntityTraits::setFillType(const AcGiFillType ft)
{
    _print(_T("\t\tmySubEntityTraits::setFillType %d\n"), static_cast<int>(ft));
}

void mySubEntityTraits::setLineWeight(const AcDb::LineWeight lw)
{
    _print(_T("\t\tmySubEntityTraits::setLineWeight() %d\n"), static_cast<int>(lw));
}

Adesk::UInt16 mySubEntityTraits::color() const
{
    _print(_T("\t\tmySubEntityTraits::color()\n"));
    return 0;
}

AcCmEntityColor mySubEntityTraits::trueColor() const
{
    static AcCmEntityColor tColor;
    _print(_T("\t\tmySubEntityTraits::trueColor()\n"));
    return tColor;
}

AcDbObjectId mySubEntityTraits::layerId() const
{
    _print(_T("\t\tmySubEntityTraits::layer\n"));
    return 0;
}

AcDbObjectId mySubEntityTraits::lineTypeId() const
{
    _print(_T("\t\tmySubEntityTraits::lineTypeIndex\n"));
    return 0;
}

AcGiFillType mySubEntityTraits::fillType() const
{
    _print(_T("\t\tmySubEntityTraits::fillType\n"));
    return kAcGiFillNever;
}

AcDb::LineWeight mySubEntityTraits::lineWeight() const
{
    _print(_T("\t\tmySubEntityTraits::lineWeight()\n"));
    return AcDb::kLnWt000;
}

void 
mySubEntityTraits::setThickness(double dThickness)
{
    m_dThickness = dThickness;
}

double 
mySubEntityTraits::thickness() const
{
    return m_dThickness;
}


void 
mySubEntityTraits::setLineTypeScale(double dLTScale)
{
    m_dLTScale = dLTScale;
}

double 
mySubEntityTraits::lineTypeScale() const
{
    return m_dLTScale;
}


#ifdef WORLD_DRAW
AcGiContext  *          
myWorldDraw::context()
{
    return mpContext;
}

void                    
myWorldDraw::regenerate(AcDbEntity * pEntity)
{
    _print(_T("\t\tmyWorldDraw::regenerate(Entity)\n"));
}

void                    
myWorldDraw::regenerate(AcDbBlockTableRecord * pBTR)
{
    _print(_T("\t\tmyWorldDraw::regenerate(BTR)\n"));
}

#endif // WORLD_DRAW



myWorldDraw::myWorldDraw()
    : AcGiWorldDraw()
{
    mpTraits = new mySubEntityTraits;
    mpGeom = new myWorldGeometry;
#ifdef WORLD_DRAW
    mpContext = new myContext;
#endif // WORLD_DRAW
}

myWorldDraw::~myWorldDraw()
{
#ifdef WORLD_DRAW
    delete mpContext;
#endif // WORLD_DRAW

    delete mpTraits;
    delete mpGeom;
}

AcGiRegenType myWorldDraw::regenType() const
{
    _print(_T("\t\tmyWorldDraw::regenType\n"));
    return kAcGiStandardDisplay;
}

Adesk::Boolean myWorldDraw::regenAbort() const
{
    //printf("\t\tmyWorldDraw::regenAbort\n");
    return 0;
}

AcGiSubEntityTraits& myWorldDraw::subEntityTraits() const
{
    //printf("\t\tmyWorldDraw::subEntityTraits\n");
    return *mpTraits;
}

AcGiGeometry* myWorldDraw::rawGeometry() const
{
    //printf("\t\tmyWorldDraw::rawGeometry\n");
    return mpGeom;
}

AcGiWorldGeometry& myWorldDraw::geometry() const
{
    //printf("\t\tmyWorldDraw::geometry\n");
    return *mpGeom;
}

Adesk::Boolean    myWorldDraw::isDragging() const
{
    _print(_T("\t\tmyWorldDraw::isDragging\n"));
    return 0;
}

double myWorldDraw::deviation(const AcGiDeviationType, 
    const AcGePoint3d&) const
{
    _print(_T("\t\tmyWorldDraw::deviation\n"));
    return .1;
}
                                              
Adesk::UInt32 myWorldDraw::numberOfIsolines() const
{
    _print(_T("\t\tmyWorldDraw::numberOfIsolines\n"));
    return 10;
}
