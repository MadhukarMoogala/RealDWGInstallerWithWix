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


// DUMPCORE.CPP

#include "windows.h"
#include "adesk.h"
#include "dbsymtb.h"
#include "dbents.h"
#include "dbelipse.h"
#include "dbspline.h"
#include "dbhatch.h"
#include "dblead.h"
#include "dbray.h"
#include "dbxline.h"
#include "dbmline.h"
#include "dbbody.h"
#include "dbimage.h"
#include "imgdef.h"
#include "imgent.h"
#include "dbregion.h"
#include "dbsol3d.h"
#include "dbole.h"
#include "dbfcf.h"
#include "dbproxy.h"
#include "acgi.h"
#include "myacgi.h"
#include "acestext.h"
#include "maps.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"

#include "dbapserv.h"

#include "dbmstyle.h"

#include "gelnsg3d.h"
#include "tchar.h"

#define PI ((double)3.14159265358979323846)
#define PIOVER180 ((double)PI/180)

void _print(TCHAR *format, ...);

void dumpDatabase(AcDbDatabase *pDatabase);

void entInfo(AcDbEntity *pEnt, int size);
void dumpEntInfo();
void dumpDwgHeaderVars(AcDbDatabase *pDatabase);

void dumpBlockTable(AcDbBlockTable *pBlockTable);
void dumpLayerTable(AcDbLayerTable *pBlockTable);
void dumpLinetypeTable(AcDbLinetypeTable *pLinetypeTable);
void dumpTextStyleTable(AcDbTextStyleTable *pTextStyleTable);
void dumpViewTable(AcDbViewTable *pViewTable);
void dumpUCSTable(AcDbUCSTable *pUCSTable);
void dumpViewportTable(AcDbViewportTable *pViewportTable);
void dumpRegAppTable(AcDbRegAppTable *pRegAppTable);
void dumpDimStyleTable(AcDbDimStyleTable *pDimStyleTable);
void dumpDictionary(const TCHAR *pName, AcDbDictionary *pDictionary);
void dumpEntity(AcDbEntity *pEnt);

void dumpAcDbEntity(AcDbEntity *pEnt);

void dumpAcDb3dSolid(AcDbEntity *pEnt);
void dumpAcDbBlockBegin(AcDbEntity *pEnt);
void dumpAcDbBlockEnd(AcDbEntity *pEnt);
void dumpAcDbBlockReference(AcDbEntity *pEnt);
void    dumpAcDbMInsertBlock(AcDbEntity *pEnt);
void dumpAcDbBody(AcDbEntity *pEnt);
void dumpAcDbCurve(AcDbEntity *pEnt);
void    dumpAcDb2dPolyline(AcDbEntity *pEnt);
void    dumpAcDb3dPolyline(AcDbEntity *pEnt);
void    dumpAcDbArc(AcDbEntity *pEnt);
void    dumpAcDbCircle(AcDbEntity *pEnt);
void    dumpAcDbEllipse(AcDbEntity *pEnt);
void    dumpAcDbLeader(AcDbEntity *pEnt);
void    dumpAcDbLine(AcDbEntity *pEnt);
void    dumpAcDbPolyline(AcDbEntity *pEnt);
void    dumpAcDbRay(AcDbEntity *pEnt);
void    dumpAcDbSpline(AcDbEntity *pEnt);
void    dumpAcDbXline(AcDbEntity *pEnt);
void dumpAcDbDimension(AcDbEntity *pEnt);
void    dumpAcDb2LineAngularDimension(AcDbEntity *pEnt);
void    dumpAcDb3PointAngularDimension(AcDbEntity *pEnt);
void    dumpAcDbAlignedDimension(AcDbEntity *pEnt);
void    dumpAcDbDiametricDimension(AcDbEntity *pEnt);
void    dumpAcDbOrdinateDimension(AcDbEntity *pEnt);
void    dumpAcDbRadialDimension(AcDbEntity *pEnt);
void    dumpAcDbRotatedDimension(AcDbEntity *pEnt);
void dumpAcDbFace(AcDbEntity *pEnt);
void dumpAcDbFcf(AcDbEntity *pEnt);
void dumpAcDbFrame(AcDbEntity *pEnt);
void    dumpAcDbOleFrame(AcDbEntity *pEnt);
void       dumpAcDbOle2Frame(AcDbEntity *pEnt);
void dumpAcDbHatch(AcDbEntity *pEnt);
void dumpAcDbRasterImage(AcDbEntity *pEnt);
void dumpAcDbMline(AcDbEntity *pEnt);
void dumpAcDbMText(AcDbEntity *pEnt);
void dumpAcDbPoint(AcDbEntity *pEnt);
void dumpAcDbPolyFaceMesh(AcDbEntity *pEnt);
void dumpAcDbPolygonMesh(AcDbEntity *pEnt);
void dumpAcDbProxyEntity(AcDbEntity *pEnt);
void dumpAcDbRegion(AcDbEntity *pEnt);
void dumpAcDbSequenceEnd(AcDbEntity *pEnt);
void dumpAcDbShape(AcDbEntity *pEnt);
void dumpAcDbSolid(AcDbEntity *pEnt);
void dumpAcDbText(AcDbEntity *pEnt);
void    dumpAcDbAttribute(AcDbEntity *pEnt);
void    dumpAcDbAttributeDefinition(AcDbEntity *pEnt);
void dumpAcDbTrace(AcDbEntity *pEnt);
void dumpAcDbVertex(AcDbEntity *pEnt);
void    dumpAcDb2dVertex(AcDbEntity *pEnt);
void    dumpAcDb3dPolylineVertex(AcDbEntity *pEnt);
void    dumpAcDbFaceRecord(AcDbEntity *pEnt);
void    dumpAcDbPolyFaceMeshVertex(AcDbEntity *pEnt);
void    dumpAcDbPolygonMeshVertex(AcDbEntity *pEnt);
void dumpAcDbViewport(AcDbEntity *pEnt);

extern int Verbose;
extern int Header;
extern int Tables;
extern int doExtents;

myWorldDraw *pAcgiThing = 0;

#ifdef ACAP
extern "C" void __stdcall StartCAP();
extern "C" void __stdcall StopCAP();
extern "C" int __stdcall DumpCAP(int (*showProgress) (const TCHAR *msg));
#endif

AcGeMatrix3d mat;

void dumpDatabase(AcDbDatabase *pDatabase)
{
    if (!pAcgiThing)
        pAcgiThing = new myWorldDraw;

    // Set the context database to pDatabase
    ((myContext*)(pAcgiThing->context()))->setDatabase(pDatabase);

    AcGePoint3d origin = pDatabase->ucsorg();
    AcGeVector3d xAxis = pDatabase->ucsxdir();
    AcGeVector3d yAxis = pDatabase->ucsydir();
    AcGeVector3d zAxis = xAxis.crossProduct(yAxis);
    _print(_T("    UCS\n"));
    _print(_T("        origin %g %g %g\n"),
        origin.x, origin.y, origin.z);
    _print(_T("        xAxis %g %g %g\n"),
        xAxis.x, xAxis.y, xAxis.z);
    _print(_T("        yAxis %g %g %g\n"),
        yAxis.x, yAxis.y, yAxis.z);
    _print(_T("        zAxis %g %g %g\n"),
        zAxis.x, zAxis.y, zAxis.z);

    mat.setCoordSystem(origin, xAxis, yAxis, zAxis);

    if (Header)
        dumpDwgHeaderVars(pDatabase);

/*  // expanded macro looks like this:
    AcDbBlockTable* pBlockTable;
    if (pDatabase->blockTable(pBlockTable, AcDb::kForRead) == Acad::eOk)
    {
        dumpBlockTable(pBlockTable);
        pBlockTable->close();
    }
*/

#define DUMPTABLE(name) {                                               \
    AcDb##name##Table *p;                                               \
    if (pDatabase->get##name##Table(p, AcDb::kForRead) == Acad::eOk) {  \
        dump##name##Table(p);                                           \
        p->close();                                                     \
    } }

    DUMPTABLE(Block)
    DUMPTABLE(Layer)
    DUMPTABLE(Linetype)
    DUMPTABLE(TextStyle)
    DUMPTABLE(View)
    DUMPTABLE(UCS)
    DUMPTABLE(Viewport)
    DUMPTABLE(RegApp)
    DUMPTABLE(DimStyle)

#undef DUMPTABLE

    AcDbDictionary* pDict = 0;
    if (pDatabase->getGroupDictionary(pDict, AcDb::kForRead)
        == Acad::eOk)
    {
        dumpDictionary(_T("GroupDictionary"), pDict);
        pDict->close();
    }
    pDict = 0;
    if (pDatabase->getMLStyleDictionary(pDict, AcDb::kForRead)
        == Acad::eOk)
    {
        dumpDictionary(_T("LStyleDictionary"), pDict);
        pDict->close();
    }

    pDict = 0;
    if (pDatabase->getNamedObjectsDictionary(pDict, AcDb::kForRead)
        == Acad::eOk)
    {
        dumpDictionary(_T("NamedObjectsDictionary"), pDict);
        pDict->close();
    }
    delete pAcgiThing;
    pAcgiThing = nullptr;
}

void dumpBlockTable(AcDbBlockTable *pBlockTable)
{
    AcDbBlockTableIterator *pIter;
    AcDbBlockTableRecord *pRecord;
    pBlockTable->newIterator(pIter);

    _print(_T("    Blocks:\n"));
    while (!pIter->done())
    {
        if (pIter->getRecord(pRecord, AcDb::kForRead) == Acad::eOk)
        {
            TCHAR *pName;
            if (pRecord->getName(pName) == Acad::eOk)
            {
                int isExtern = pRecord->isFromExternalReference();
                TCHAR *pFileName = nullptr;
                pRecord->pathName(pFileName);

                _print(_T("        %ls %ls %ls%ls\n"), 
                    pName, isExtern ? _T("xref") : _T(""), 
                    pFileName ? _T("File: ") : _T(""),
                    pFileName ? pFileName : _T(""));
                acdbFree(pName);
                if (pFileName)
                    acdbFree(pFileName);
            }
            AcDbBlockTableRecordIterator *pRecordIter;
            if (Verbose && pRecord->newIterator(pRecordIter) == Acad::eOk)
            {
                AcDbEntity *pEnt;
                while (!pRecordIter->done())
                {
                    if (pRecordIter->getEntity(pEnt, AcDb::kForRead) 
                        == Acad::eOk)
                    {
                        dumpEntity(pEnt);
                        pEnt->close();
                    }
                    pRecordIter->step();
                }                            
                delete pRecordIter;
            }
            pRecord->close();
        }
        pIter->step();
    }
    delete pIter;
}

void dumpLayerTable(AcDbLayerTable *pLayerTable)
{
    AcDbLayerTableIterator *pIter;
    AcDbLayerTableRecord *pRecord;
    pLayerTable->newIterator(pIter);

    _print(_T("    Layers:\n"));
    while (!pIter->done())
    {
        if (pIter->getRecord(pRecord, AcDb::kForRead) == Acad::eOk)
        {
            TCHAR *pName;
            if (pRecord->getName(pName) == Acad::eOk)
            {
                TCHAR ltName[128] = _T("????????");
                AcDbObjectId linetypeid = pRecord->linetypeObjectId();
                AcDbLinetypeTableRecord *pLinetypeRecord;
            
                Acad::ErrorStatus es = acdbOpenObject(pLinetypeRecord, 
                    linetypeid, AcDb::kForRead);
                if (pLinetypeRecord)
                {
                    TCHAR *pLinetypeName = 0;
                    if (pLinetypeRecord->getName(pLinetypeName) == Acad::eOk)
                    {
                        _tcscpy(ltName, pLinetypeName);
                        acdbFree(pLinetypeName);
                    }
                    pLinetypeRecord->close();
                }
                else
                {
                    _print(_T("acdbopenObject: %ls\n"), acadErrorStatusText(es));

                    AcDbObject* pObject;
                    AcDbObjectId linetypeid = pRecord->linetypeObjectId();
                    Acad::ErrorStatus es = acdbOpenAcDbObject(pObject, 
                        linetypeid, AcDb::kForRead);
                    if (es == Acad::eOk)
                    {
                        const TCHAR *p = pObject->isA()->name();
                        _print(_T("it's really a %ls try a cast <%Ix>\n"), p,
                            AcDbLinetypeTableRecord::cast(pObject));

                        pObject->close();
                    }
                }
                _print(_T("        %ls color %d lt %ls\n"), 
                    pName, pRecord->color().colorIndex(), ltName);
                acdbFree(pName);
            }
            pRecord->close();
        }
        pIter->step();
    }
    delete pIter;
}

void dumpLinetypeTable(AcDbLinetypeTable *pLinetypeTable)
{
    AcDbLinetypeTableIterator *pIter;
    AcDbLinetypeTableRecord *pRecord;
    pLinetypeTable->newIterator(pIter);

    _print(_T("    Linetypes:\n"));
    while (!pIter->done())
    {
        if (pIter->getRecord(pRecord, AcDb::kForRead) == Acad::eOk)
        {
            TCHAR *pName;
            if (pRecord->getName(pName) == Acad::eOk)
            {
                _print(_T("        %ls\n"), pName);
                acdbFree(pName);
            }
            pRecord->close();
        }
        pIter->step();
    }
    delete pIter;
}

void dumpTextStyleTable(AcDbTextStyleTable *pTextStyleTable)
{
    AcDbTextStyleTableIterator *pIter;
    AcDbTextStyleTableRecord *pRecord;
    pTextStyleTable->newIterator(pIter);

    _print(_T("    TextStyles:\n"));
    while (!pIter->done())
    {
        if (pIter->getRecord(pRecord, AcDb::kForRead) == Acad::eOk)
        {
            TCHAR *pName;
            if (pRecord->getName(pName) == Acad::eOk)
            {
                _print(_T("        %ls\n"), pName);
                acdbFree(pName);

                if (Tables)
                {
                    _print(_T("\t\tAdesk::Boolean isShapeFile() %x\n"), 
                        pRecord->isShapeFile());
                    _print(_T("\t\tAdesk::Boolean isVertical() %x\n"), 
                        pRecord->isVertical());
                    _print(_T("\t\tdouble textSize() %g\n"), 
                        pRecord->textSize());
                    _print(_T("\t\tdouble xScale() %g\n"), 
                        pRecord->xScale());
                    _print(_T("\t\tdouble obliquingAngle() %g\n"), 
                        pRecord->obliquingAngle());
                    _print(_T("\t\tAdesk::UInt8 flagBits() %x\n"), 
                        pRecord->flagBits());
                    _print(_T("\t\tdouble priorSize() %g\n"), 
                        pRecord->priorSize());
                    TCHAR str[] = _T("<>");
                    TCHAR *pStr = str;
                    pRecord->fileName(pStr);
                    _print(_T("\t\tAcad::ErrorStatus fileName(char*&) %ls\n"), 
                        pStr);
                    pRecord->bigFontFileName(pStr);
                    _print(_T("\t\tAcad::ErrorStatus bigFontFileName(char*&) %ls\n"), 
                        pStr);
                }
            }
            pRecord->close();
        }
        pIter->step();
    }
    delete pIter;
}

void dumpViewTable(AcDbViewTable *pViewTable)
{
    AcDbViewTableIterator *pIter;
    AcDbViewTableRecord *pRecord;
    pViewTable->newIterator(pIter);

    _print(_T("    Views:\n"));
    while (!pIter->done())
    {
        if (pIter->getRecord(pRecord, AcDb::kForRead) == Acad::eOk)
        {
            TCHAR *pName;
            if (pRecord->getName(pName) == Acad::eOk)
            {
                _print(_T("        %ls\n"), pName);

                _print(_T("\t\tisPaperspaceView %d\n"),
                    pRecord->isPaperspaceView());
                if (Tables)
                {
                    _print(_T("\t\tcenterPoint %g,%g\n"),
                        pRecord->centerPoint().x,
                        pRecord->centerPoint().y);

                    _print(_T("\t\theight() %g\n"),
                        pRecord->height());

                    _print(_T("\t\twidth() %g\n"),
                        pRecord->width());

                    _print(_T("\t\ttarget() %g,%g,%g\n"),
                        pRecord->target().x,
                        pRecord->target().y,
                        pRecord->target().z);

                    _print(_T("\t\tviewDirection() %g,%g,%g\n"),
                        pRecord->viewDirection().x,
                        pRecord->viewDirection().y,
                        pRecord->viewDirection().z);

                    _print(_T("\t\tviewTwist() %g\n"),
                        pRecord->viewTwist());

                    _print(_T("\t\tlensLength() %g\n"),
                        pRecord->lensLength());

                    _print(_T("\t\tfrontClipDistance() %g\n"),
                        pRecord->frontClipDistance());

                    _print(_T("\t\tbackClipDistance() %g\n"),
                        pRecord->backClipDistance());

                    _print(_T("\t\tperspectiveEnabled() %d\n"),
                        pRecord->perspectiveEnabled());

                    _print(_T("\t\tfrontClipEnabled() %d\n"),
                        pRecord->frontClipEnabled());

                    _print(_T("\t\tbackClipEnabled() %d\n"),
                        pRecord->backClipEnabled());

                    _print(_T("\t\tfrontClipAtEye() %d\n"),
                        pRecord->frontClipAtEye());

                }
                acdbFree(pName);
            }
            pRecord->close();
        }
        pIter->step();
    }
    delete pIter;
}

void dumpUCSTable(AcDbUCSTable *pUCSTable)
{
    AcDbUCSTableIterator *pIter;
    AcDbUCSTableRecord *pRecord;
    pUCSTable->newIterator(pIter);

    _print(_T("    UCSTables:\n"));
    while (!pIter->done())
    {
        if (pIter->getRecord(pRecord, AcDb::kForRead) == Acad::eOk)
        {
            TCHAR *pName;
            if (pRecord->getName(pName) == Acad::eOk)
            {
                _print(_T("        %ls\n"), pName);
                acdbFree(pName);
            }
            pRecord->close();
        }
        pIter->step();
    }
    delete pIter;
}

void dumpViewportTable(AcDbViewportTable *pViewportTable)
{
    AcDbViewportTableIterator *pIter;
    AcDbViewportTableRecord *pRecord;
    pViewportTable->newIterator(pIter);

    _print(_T("    ViewportTables:\n"));
    while (!pIter->done())
    {
        if (pIter->getRecord(pRecord, AcDb::kForRead) == Acad::eOk)
        {
            TCHAR *pName;
            if (pRecord->getName(pName) == Acad::eOk)
            {
                _print(_T("        %ls\n"), pName);
                if (Tables)
                {
                    _print(_T("\t\tlowerLeftCorner %g,%g\n"),
                        pRecord->lowerLeftCorner().x,
                        pRecord->lowerLeftCorner().y);
                    _print(_T("\t\tupperRightCorner %g,%g\n"),
                        pRecord->upperRightCorner().x,
                        pRecord->upperRightCorner().y);
                    _print(_T("\t\tucsFollowMode %d\n"),
                        pRecord->ucsFollowMode());
                    _print(_T("\t\tcircleSides %d\n"),
                        pRecord->circleSides());
                    _print(_T("\t\tfastZoomsEnabled %d\n"),
                        pRecord->fastZoomsEnabled());
                    _print(_T("\t\ticonEnabled %d\n"),
                        pRecord->iconEnabled());
                    _print(_T("\t\ticonAtOrigin %d\n"),
                        pRecord->iconAtOrigin());
                    _print(_T("\t\tgridEnabled %d\n"),
                        pRecord->gridEnabled());
                    _print(_T("\t\tgridIncrements %d\n"),
                        pRecord->gridIncrements());
                    _print(_T("\t\tsnapEnabled %d\n"),
                        pRecord->snapEnabled());
                    _print(_T("\t\tisometricSnapEnabled %d\n"),
                        pRecord->isometricSnapEnabled());
                    _print(_T("\t\tsnapPair %d\n"),
                        pRecord->snapPair());
                    _print(_T("\t\tsnapAngle %g\n"),
                        pRecord->snapAngle());
                    _print(_T("\t\tsnapBase %g,%g\n"),
                        pRecord->snapBase().x,
                        pRecord->snapBase().y);
                    _print(_T("\t\tsnapIncrements %g,%g\n"),
                        pRecord->snapIncrements().x,
                        pRecord->snapIncrements().y);

                    _print(_T("\t\tcenterPoint %g,%g\n"),
                        pRecord->centerPoint().x,
                        pRecord->centerPoint().y);

                    _print(_T("\t\theight() %g\n"),
                        pRecord->height());

                    _print(_T("\t\twidth() %g\n"),
                        pRecord->width());

                    _print(_T("\t\ttarget() %g,%g,%g\n"),
                        pRecord->target().x,
                        pRecord->target().y,
                        pRecord->target().z);

                    _print(_T("\t\tviewDirection() %g,%g,%g\n"),
                        pRecord->viewDirection().x,
                        pRecord->viewDirection().y,
                        pRecord->viewDirection().z);

                    _print(_T("\t\tviewTwist() %g\n"),
                        pRecord->viewTwist());

                    _print(_T("\t\tlensLength() %g\n"),
                        pRecord->lensLength());

                    _print(_T("\t\tfrontClipDistance() %g\n"),
                        pRecord->frontClipDistance());

                    _print(_T("\t\tbackClipDistance() %g\n"),
                        pRecord->backClipDistance());

                    _print(_T("\t\tperspectiveEnabled() %d\n"),
                        pRecord->perspectiveEnabled());

                    _print(_T("\t\tfrontClipEnabled() %d\n"),
                        pRecord->frontClipEnabled());

                    _print(_T("\t\tbackClipEnabled() %d\n"),
                        pRecord->backClipEnabled());

                    _print(_T("\t\tfrontClipAtEye() %d\n"),
                        pRecord->frontClipAtEye());

                }
                acdbFree(pName);
            }
            pRecord->close();
        }
        pIter->step();
    }
    delete pIter;
}

void dumpRegAppTable(AcDbRegAppTable *pRegAppTable)
{
    AcDbRegAppTableIterator *pIter;
    AcDbRegAppTableRecord *pRecord;
    pRegAppTable->newIterator(pIter);

    _print(_T("    RegApps:\n"));
    while (!pIter->done())
    {
        if (pIter->getRecord(pRecord, AcDb::kForRead) == Acad::eOk)
        {
            TCHAR *pName;
            if (pRecord->getName(pName) == Acad::eOk)
            {
                _print(_T("        %ls\n"), pName);
                acdbFree(pName);
            }
            pRecord->close();
        }
        pIter->step();
    }
    delete pIter;
}

void dumpDimStyleTable(AcDbDimStyleTable *pDimStyleTable)
{
    AcDbDimStyleTableIterator *pIter;
    AcDbDimStyleTableRecord *pRecord;
    pDimStyleTable->newIterator(pIter);

    _print(_T("    DimStyles:\n"));
    while (!pIter->done())
    {
        if (pIter->getRecord(pRecord, AcDb::kForRead) == Acad::eOk)
        {
            TCHAR *pName;
            if (pRecord->getName(pName) == Acad::eOk)
            {
                _print(_T("        %ls\n"), pName);
                acdbFree(pName);

                if (Tables)
                {
                    _print(_T("\tAdesk::Boolean dimtol() %x\n"), pRecord->dimtol());
                    _print(_T("\tAdesk::Boolean dimlim() %x\n"), pRecord->dimlim());
                    _print(_T("\tAdesk::Boolean dimtih() %x\n"), pRecord->dimtih());
                    _print(_T("\tAdesk::Boolean dimtoh() %x\n"), pRecord->dimtoh());
                    _print(_T("\tAdesk::Boolean dimse1() %x\n"), pRecord->dimse1());
                    _print(_T("\tAdesk::Boolean dimse2() %x\n"), pRecord->dimse2());
                    _print(_T("\tAdesk::Boolean dimalt() %x\n"), pRecord->dimalt());
                    _print(_T("\tAdesk::Boolean dimtofl() %x\n"), pRecord->dimtofl());
                    _print(_T("\tAdesk::Boolean dimsah() %x\n"), pRecord->dimsah());
                    _print(_T("\tAdesk::Boolean dimtix() %x\n"), pRecord->dimtix());
                    _print(_T("\tAdesk::Boolean dimsoxd() %x\n"), pRecord->dimsoxd());
                    _print(_T("\tAdesk::Boolean dimsd1() %x\n"), pRecord->dimsd1());
                    _print(_T("\tAdesk::Boolean dimsd2() %x\n"), pRecord->dimsd2());
                    _print(_T("\tAdesk::Int8  dimtolj() %x\n"), pRecord->dimtolj());
                    _print(_T("\tAdesk::Boolean dimupt() %x\n"), pRecord->dimupt());
                _print(_T("\tint dimaltd() %x\n"), pRecord->dimaltd());
                _print(_T("\tint dimzin() %x\n"), pRecord->dimzin());
                _print(_T("\tint dimjust() %x\n"), pRecord->dimjust());
                _print(_T("\tint dimfit() %x\n"), pRecord->dimfit());
                _print(_T("\tint dimtzin() %x\n"), pRecord->dimtzin());
                _print(_T("\tint dimaltz() %x\n"), pRecord->dimaltz());
                _print(_T("\tint dimalttz() %x\n"), pRecord->dimalttz());
                _print(_T("\tint dimtad() %x\n"), pRecord->dimtad());
                _print(_T("\tint dimunit() %x\n"), pRecord->dimunit());
                _print(_T("\tint dimaunit() %x\n"), pRecord->dimaunit());
                _print(_T("\tint dimdec() %x\n"), pRecord->dimdec());
                _print(_T("\tint dimtdec() %x\n"), pRecord->dimtdec());
                _print(_T("\tint dimaltu() %x\n"), pRecord->dimaltu());
                _print(_T("\tint dimalttd() %x\n"), pRecord->dimalttd());
                _print(_T("\tAcDbObjectId dimtxsty() %Ix\n"), pRecord->dimtxsty());
                _print(_T("\tdouble dimscale() %g\n"), pRecord->dimscale());
                _print(_T("\tdouble dimasz() %g\n"), pRecord->dimasz());
                _print(_T("\tdouble dimexo() %g\n"), pRecord->dimexo());
                _print(_T("\tdouble dimdli() %g\n"), pRecord->dimdli());
                _print(_T("\tdouble dimexe() %g\n"), pRecord->dimexe());
                _print(_T("\tdouble dimrnd() %g\n"), pRecord->dimrnd());
                _print(_T("\tdouble dimdle() %g\n"), pRecord->dimdle());
                _print(_T("\tdouble dimtp() %g\n"), pRecord->dimtp());
                _print(_T("\tdouble dimtm() %g\n"), pRecord->dimtm());
                _print(_T("\tdouble dimtxt() %g\n"), pRecord->dimtxt());
                _print(_T("\tdouble dimcen() %g\n"), pRecord->dimcen());
                _print(_T("\tdouble dimtsz() %g\n"), pRecord->dimtsz());
                _print(_T("\tdouble dimaltf() %g\n"), pRecord->dimaltf());
                _print(_T("\tdouble dimlfac() %g\n"), pRecord->dimlfac());
                _print(_T("\tdouble dimtvp() %g\n"), pRecord->dimtvp());
                _print(_T("\tdouble dimtfac() %g\n"), pRecord->dimtfac());
                _print(_T("\tdouble dimgap() %g\n"), pRecord->dimgap());

                static TCHAR str[128] = _T("<empty>");
                TCHAR *pStr;
                pStr = str;
                pRecord->getDimpost(pStr);
                _print(_T("\tAcad::ErrorStatus getDimpost(char*&) %ls\n"), pStr);
                pStr = str;
                pRecord->getDimapost(pStr);
                _print(_T("\tAcad::ErrorStatus getDimapost(char*&) %ls\n"), pStr);
                pStr = str;
                pRecord->getDimblk(pStr);
                _print(_T("\tAcad::ErrorStatus getDimblk(char*&) %ls\n"), pStr);
                pStr = str;
                pRecord->getDimblk1(pStr);
                _print(_T("\tAcad::ErrorStatus getDimblk1(char*&) %ls\n"), pStr);
                pStr = str;
                pRecord->getDimblk2(pStr);
                _print(_T("\tAcad::ErrorStatus getDimblk2(char*&) %ls\n"), pStr);

                _print(_T("\tAcCmColor dimclrd() %d\n"), pRecord->dimclrd().colorIndex());
                _print(_T("\tAcCmColor dimclre() %d\n"), pRecord->dimclre().colorIndex());
                _print(_T("\tAcCmColor dimclrt() %d\n"), pRecord->dimclrt().colorIndex());
                }
            }
            pRecord->close();
        }
        pIter->step();
    }
    delete pIter;
}

void dumpDictionary(const TCHAR *pName, AcDbDictionary *pDictionary)
{
    AcDbDictionaryIterator *pIter;
    AcDbObject *pObj;
    pIter = pDictionary->newIterator();

    _print(_T("    Dictionary: %ls\n"), pName);
    while (!pIter->done())
    {
        _print(_T("        %ls\n"), pIter->name() ? pIter->name() : _T("???") );
        if (pIter->getObject(pObj, AcDb::kForRead) == Acad::eOk)
        {
            const TCHAR *p = pObj->isA()->name();
            _print(_T("        %ls\n"), p);
            pObj->close();
        }
        pIter->next();
    }
    delete pIter;
}

struct ENTNAMES
{
    const TCHAR *pName;
    void (* func)(AcDbEntity *);
};

ENTNAMES EntityList[] =
{
    _T("AcDb3dSolid"),                      dumpAcDb3dSolid,
    _T("AcDbBlockBegin"),                   dumpAcDbBlockBegin,
    _T("AcDbBlockEnd"),                     dumpAcDbBlockEnd,
    _T("AcDbBlockReference"),               dumpAcDbBlockReference,
        _T("AcDbMInsertBlock"),             dumpAcDbMInsertBlock,
    _T("AcDbBody"),                         dumpAcDbBody,
    _T("AcDbCurve"),                        dumpAcDbCurve,
        _T("AcDb2dPolyline"),               dumpAcDb2dPolyline,
        _T("AcDb3dPolyline"),               dumpAcDb3dPolyline,
        _T("AcDbArc"),                      dumpAcDbArc,
        _T("AcDbCircle"),                   dumpAcDbCircle,
        _T("AcDbEllipse"),                  dumpAcDbEllipse,
        _T("AcDbLeader"),                   dumpAcDbLeader,
        _T("AcDbLine"),                     dumpAcDbLine,
        _T("AcDbPolyline"),                 dumpAcDbPolyline,
        _T("AcDbRay"),                      dumpAcDbRay,
        _T("AcDbSpline"),                   dumpAcDbSpline,
        _T("AcDbXline"),                    dumpAcDbXline,
    _T("AcDbDimension"),                    dumpAcDbDimension,
        _T("AcDb2LineAngularDimension"),    dumpAcDb2LineAngularDimension,
        _T("AcDb3PointAngularDimension"),   dumpAcDb3PointAngularDimension,
        _T("AcDbAlignedDimension"),         dumpAcDbAlignedDimension,
        _T("AcDbDiametricDimension"),       dumpAcDbDiametricDimension,
        _T("AcDbOrdinateDimension"),        dumpAcDbOrdinateDimension,
        _T("AcDbRadialDimension"),          dumpAcDbRadialDimension,
        _T("AcDbRotatedDimension"),         dumpAcDbRotatedDimension,
    _T("AcDbFace"),                         dumpAcDbFace,
    _T("AcDbFcf"),                          dumpAcDbFcf,
    _T("AcDbFrame"),                        dumpAcDbFrame,
        _T("AcDbOleFrame"),                 dumpAcDbOleFrame,
            _T("AcDbOle2Frame"),            dumpAcDbOle2Frame,
    _T("AcDbHatch"),                        dumpAcDbHatch,
    _T("AcDbRasterImage"),                  dumpAcDbRasterImage,
    _T("AcDbMline"),                        dumpAcDbMline,
    _T("AcDbMText"),                        dumpAcDbMText,
    _T("AcDbPoint"),                        dumpAcDbPoint,
    _T("AcDbPolyFaceMesh"),                 dumpAcDbPolyFaceMesh,
    _T("AcDbPolygonMesh"),                  dumpAcDbPolygonMesh,
    _T("AcDbZombieEntity"),                 dumpAcDbProxyEntity,
    _T("AcDbRegion"),                       dumpAcDbRegion,
    _T("AcDbSequenceEnd"),                  dumpAcDbSequenceEnd,
    _T("AcDbShape"),                        dumpAcDbShape,
    _T("AcDbSolid"),                        dumpAcDbSolid,
    _T("AcDbText"),                         dumpAcDbText,
        _T("AcDbAttribute"),                dumpAcDbAttribute,
        _T("AcDbAttributeDefinition"),      dumpAcDbAttributeDefinition,
    _T("AcDbTrace"),                        dumpAcDbTrace,
    _T("AcDbVertex"),                       dumpAcDbVertex,
        _T("AcDb2dVertex"),                 dumpAcDb2dVertex,
        _T("AcDb3dPolylineVertex"),         dumpAcDb3dPolylineVertex,
        _T("AcDbFaceRecord"),               dumpAcDbFaceRecord,
        _T("AcDbPolyFaceMeshVertex"),       dumpAcDbPolyFaceMeshVertex,
        _T("AcDbPolygonMeshVertex"),        dumpAcDbPolygonMeshVertex,
    _T("AcDbViewport"),                     dumpAcDbViewport,
};

#define ELEMENTS(array) (sizeof(array)/sizeof((array)[0]))

#define nEntNames ELEMENTS(EntityList)

static const TCHAR *keyName;

void dumpEntity(AcDbEntity *pEnt)
{
    const TCHAR *p = pEnt->isA()->name();
    _print(_T("\t  %ls\n"), p);
    int i;

    if (Verbose > 1)
    {
        dumpAcDbEntity(pEnt);

        for (i = 0; i < nEntNames; i++)
        {
            if (_tcscmp(p, EntityList[i].pName) == 0)
            {
                keyName = EntityList[i].pName;
                (*EntityList[i].func)(pEnt);
                break;
            }
        }
        if (i == nEntNames)     // not found
        {
            int i = 0;
        }
        // test to exclude block reference and dimension objects.
        if ( (i != 3) && ((i < 18) || (i>25)) )
            pEnt->worldDraw(pAcgiThing);
    }
}

void dumpAcDbEntity(AcDbEntity *pEnt)
{
    TCHAR *pLayer = pEnt->layer();
    TCHAR *pLinetype = pEnt->linetype();
    if (pEnt->colorIndex() == 0 || pEnt->colorIndex() == 256)
    {
        _print(_T("\t\tcolor %ls layer %ls linetype %ls linetypeScale %g\n"),
            pEnt->colorIndex() == 0 ? _T("BYBLOCK") : _T("BYLAYER"), 
            pLayer, pLinetype, pEnt->linetypeScale());
    }
    else
    {
        _print(_T("\t\tcolor %d layer %ls linetype %ls linetypeScale %g\n"),
            pEnt->colorIndex(), pLayer, pLinetype,
            pEnt->linetypeScale());
    }
    acdbFree(pLayer);
    acdbFree(pLinetype);

#if TEST_ACDBOPENACDBENTITY
    AcDbObjectId id = pEnt->objectId();
    AcDbEntity *pEntity = 0;
    Acad::ErrorStatus es = acdbOpenAcDbEntity(pEntity, id, AcDb::kForRead);
    if (pEntity)
    {
        pEntity->close();
    }
    else
        _print(_T("acdbopenacdbentity: %ls\n"), acadErrorStatusText(es));
#endif

    //AcGeMatrix3d mat;
    //pEnt->ecs(mat);

    if (doExtents)
    {
        AcDbExtents ext;
        if (pEnt->getGeomExtents(ext) == Acad::eOk)
        {
            _print(_T("\t\textents %g,%g,%g %g,%g,%g\n"),
                ext.minPoint().x, ext.minPoint().y, ext.minPoint().z,
                ext.maxPoint().x, ext.maxPoint().y, ext.maxPoint().z);
        }
    }

    AcGePoint3dArray gripPoints;
    AcDbIntArray osnapModes;
    AcDbIntArray geomIds;
    int i;
    pEnt->getGripPoints(gripPoints, osnapModes, geomIds);
    for (i = 0; i < gripPoints.length(); i++)
    {
        _print(_T("\t\tgripPoints[%d] %g,%g,%g\n"), i,
            gripPoints[i].x, gripPoints[i].y, gripPoints[i].z);
    }

    resbuf *pBuf;
    pBuf = pEnt->xData(nullptr);
    if (pBuf)
    {
        resbuf *p = pBuf;
        _print(_T("\t\txData:\n"));
        while (p)
        {
            switch (p->restype)
            {
            case AcDb::kDxfXdAsciiString:
                _print(_T("\t\t\tAsciiString: %ls\n"), p->resval.rstring);
                break;
            case AcDb::kDxfRegAppName:
                _print(_T("\t\t\tRegAppName: %ls\n"), p->resval.rstring);
                break;
            case AcDb::kDxfXdControlString:
                _print(_T("\t\t\tControlString: %ls\n"), p->resval.rstring);
                break;
            case AcDb::kDxfXdLayerName:
                _print(_T("\t\t\tLayerName: %ls\n"), p->resval.rstring);
                break;
            case AcDb::kDxfXdBinaryChunk:
                _print(_T("\t\t\tBinaryChunk:"));
                {
                    int i;
                    for (i = 0; i < p->resval.rbinary.clen; i++)
                    {
                        if (i % 16 == 0)
                            _print(_T("\n\t\t\t    "));
                        _print(_T("%02x "), (BYTE)p->resval.rbinary.buf[i]);
                    }
                    _print(_T("\n"));
                }
                break;
            case AcDb::kDxfXdHandle:
                _print(_T("\t\t\tHandle: %x %x %ls\n"), 
                    p->resval.rlname[0],
                    p->resval.rlname[1],
                    p->resval.rstring);
                break;
//            case AcDb::kDxfXdSoftRefHandle:
//            case AcDb::kDxfXdHardRefHandle:
//            case AcDb::kDxfXdSoftOwnHandle:
//            case AcDb::kDxfXdHardOwnHandle:
//                break;
            case AcDb::kDxfXdXCoord:
                _print(_T("\t\t\tCoord: %g,%g,%g\n"), 
                    p->resval.rpoint[0],
                    p->resval.rpoint[1],
                    p->resval.rpoint[2]);
                break;
            case AcDb::kDxfXdWorldXCoord:
                _print(_T("\t\t\tWorldCoord: %g,%g,%g\n"), 
                    p->resval.rpoint[0],
                    p->resval.rpoint[1],
                    p->resval.rpoint[2]);
                break;
            case AcDb::kDxfXdWorldXDisp:
                _print(_T("\t\t\tWorldDisplacement: %g,%g,%g\n"), 
                    p->resval.rpoint[0],
                    p->resval.rpoint[1],
                    p->resval.rpoint[2]);
                break;
            case AcDb::kDxfXdWorldXDir:
                _print(_T("\t\t\tWorldDirection: %g,%g,%g\n"), 
                    p->resval.rpoint[0],
                    p->resval.rpoint[1],
                    p->resval.rpoint[2]);
                break;
            case AcDb::kDxfXdReal:
                _print(_T("\t\t\tReal: %g\n"), p->resval.rreal);
                break;
            case AcDb::kDxfXdDist:
                _print(_T("\t\t\tDistance: %g\n"), p->resval.rreal);
                break;
            case AcDb::kDxfXdScale:
                _print(_T("\t\t\tScaleFactor: %g\n"), p->resval.rreal);
                break;
            case AcDb::kDxfXdInteger16:
                _print(_T("\t\t\tInteger16: %d\n"), p->resval.rint);
                break;
            case AcDb::kDxfXdInteger32:
                _print(_T("\t\t\tInteger32: 0x%Ix\n"), p->resval.rlong);
                break;
            default:
                _print(_T("\t\t\t??? type: %d\n"), p->restype);
                break;
            }
            p = p->rbnext;
        }
        acutRelRb(pBuf);
    }
}

void dumpAcDb3dSolid(AcDbEntity *pEnt)
{
    entInfo(pEnt, sizeof(AcDb3dSolid));
    AcDb3dSolid *pAcDb3dSolid = (AcDb3dSolid *)pEnt;
}

void dumpAcDbBlockBegin(AcDbEntity *pEnt)
{
    AcDbBlockBegin *pAcDbBlockBegin = (AcDbBlockBegin *)pEnt;
    entInfo(pEnt, sizeof(AcDbBlockBegin));
}

void dumpAcDbBlockEnd(AcDbEntity *pEnt)
{
    AcDbBlockEnd *pAcDbBlockEnd = (AcDbBlockEnd *)pEnt;
    entInfo(pEnt, sizeof(AcDbBlockEnd));
}

void dumpAcDbBlockReference(AcDbEntity *pEnt)
{
    AcDbBlockReference *pAcDbBlockReference = (AcDbBlockReference *)pEnt;
    entInfo(pEnt, sizeof(AcDbBlockReference));
    
    AcDbObjectId blockId = pAcDbBlockReference->blockTableRecord();
    AcGePoint3d position = pAcDbBlockReference->position();
    AcGeScale3d scale = pAcDbBlockReference->scaleFactors();
    double rotation = pAcDbBlockReference->rotation();
    AcGeVector3d normal = pAcDbBlockReference->normal();
    AcGeMatrix3d trans = pAcDbBlockReference->blockTransform();

    AcDbBlockTableRecord *pRecord = 0;
    acdbOpenObject(pRecord, blockId, AcDb::kForRead);
    if (pRecord)
    {
        TCHAR *pName;
        if (pRecord->getName(pName) == Acad::eOk)
        {
            _print(_T("\t\tBlock: %ls\n"), pName);
            acdbFree(pName);
        }
        pRecord->close();
    }
    _print(_T("\t\tposition %g,%g,%g\n"),
        position.x, position.y, position.z);
    _print(_T("\t\tscale %g,%g,%g\n"),
        scale.sx, scale.sy, scale.sz); 
    _print(_T("\t\trotation %g\n"),
        rotation);
    _print(_T("\t\tnormal %g,%g,%g\n"),
        normal.x, normal.y, normal.z);
    _print(_T("\t\ttransform\t%g %g %g %g\n"),
        trans.entry[0][0], trans.entry[0][1], 
        trans.entry[0][2], trans.entry[0][3]);
    _print(_T("\t\t\t\t%g %g %g %g\n"),
        trans.entry[1][0], trans.entry[1][1], 
        trans.entry[1][2], trans.entry[1][3]);
    _print(_T("\t\t\t\t%g %g %g %g\n"),
        trans.entry[2][0], trans.entry[2][1], 
        trans.entry[2][2], trans.entry[2][3]);
    _print(_T("\t\t\t\t%g %g %g %g\n"),
        trans.entry[3][0], trans.entry[3][1], 
        trans.entry[3][2], trans.entry[3][3]);

    AcDbObjectIterator* pIter = pAcDbBlockReference->attributeIterator();
    while (!pIter->done())
    {
        AcDbObjectId objId = pIter->objectId();
        AcDbAttribute *pAtt;
        if (pAcDbBlockReference->openAttribute(pAtt, objId, AcDb::kForRead)
            == Acad::eOk)
        {
            dumpEntity(pAtt);
            pAtt->close();
        }
        pIter->step();
    }
    delete pIter;

}

void dumpAcDbMInsertBlock(AcDbEntity *pEnt)
{
    AcDbMInsertBlock *pAcDbMInsertBlock = (AcDbMInsertBlock *)pEnt;
//    entInfo(pEnt, sizeof(AcDbMInsertBlock));
    dumpAcDbBlockReference(pEnt);
    
    Adesk::UInt16 columns = pAcDbMInsertBlock->columns();
    Adesk::UInt16 rows = pAcDbMInsertBlock->rows();
    double columnSpacing = pAcDbMInsertBlock->columnSpacing();
    double rowSpacing = pAcDbMInsertBlock->rowSpacing();

    _print(_T("\t\tcolumns\t%d\trows\t%d\n"), columns, rows);
    _print(_T("\t\tcolumnSpacing\t%d\trowSpacing\t%d\n"), 
        columnSpacing, rowSpacing);
}

void dumpAcDbBody(AcDbEntity *pEnt)
{
    entInfo(pEnt, sizeof(AcDbBody));
    AcDbBody *pAcDbBody = (AcDbBody *)pEnt;
}

void dumpAcDbCurve(AcDbEntity *pEnt)
{
    AcDbCurve *pAcDbCurve = (AcDbCurve *)pEnt;
//    entInfo(pEnt, sizeof(AcDbCurve));
    _print(_T("\t\tclosed %d periodic %d planar %d\n"),
        pAcDbCurve->isClosed(),
        pAcDbCurve->isPeriodic(),
        pAcDbCurve->isPlanar());
}

void dumpAcDb2dPolyline(AcDbEntity *pEnt)
{
    dumpAcDbCurve(pEnt);
    entInfo(pEnt, sizeof(AcDb2dPolyline));
    AcDb2dPolyline *pAcDb2dPolyline = (AcDb2dPolyline *)pEnt;

    AcDb::Poly2dType type = pAcDb2dPolyline->polyType();
    double defaultStartWidth = pAcDb2dPolyline->defaultStartWidth();
    double defaultEndWidth = pAcDb2dPolyline->defaultEndWidth();
    double thickness = pAcDb2dPolyline->thickness();
    double elev = pAcDb2dPolyline->elevation();
    AcGeVector3d normal = pAcDb2dPolyline->normal();
    AcGePoint3d vertexPosition;
    pAcDb2dPolyline->vertexPosition(vertexPosition);

    static TCHAR *szPolyType[] = { _T("k2dSimplePoly"),
                                  _T("k2dFitCurvePoly"),
                                  _T("k2dQuadSplinePoly"),
                                  _T("k2dCubicSplinePoly") };

    _print(_T("\t\tPoly2dType: %ls\n"), szPolyType[type]);

    _print(_T("\t\tdefStartWidth %g\tdefEndWidth %g\n"), 
        defaultStartWidth, defaultEndWidth);
    _print(_T("\t\tthickness %g\n"), thickness);
    _print(_T("\t\televation %g\n"), elev);
    _print(_T("\t\tnormal %g,%g,%g\n"),
        normal.x, normal.y, normal.z);
    _print(_T("\t\tWCS vertexPosition %g,%g,%g\n"),
        vertexPosition.x, vertexPosition.y, vertexPosition.z);

    AcDbObjectIterator* pIter = pAcDb2dPolyline->vertexIterator();
    while (!pIter->done())
    {
        AcDbObjectId objId = pIter->objectId();
        AcDb2dVertex *pVert;
        if (pAcDb2dPolyline->openVertex(pVert, objId, AcDb::kForRead)
            == Acad::eOk)
        {
            dumpEntity(pVert);
            pVert->close();
        }
        pIter->step();
    }
    delete pIter;
}

void dumpAcDb3dPolyline(AcDbEntity *pEnt)
{
    dumpAcDbCurve(pEnt);
    entInfo(pEnt, sizeof(AcDb3dPolyline));
    AcDb3dPolyline *pAcDb3dPolyline = (AcDb3dPolyline *)pEnt;

    AcDb::Poly3dType type = pAcDb3dPolyline->polyType();

    static TCHAR *szPolyType[] = { _T("k3dSimplePoly"),
                                  _T("k3dQuadSplinePoly"),
                                  _T("k3dCubicSplinePoly") };

    _print(_T("\t\tPoly3dType: %ls\n"), szPolyType[type]);

    AcDbObjectIterator* pIter = pAcDb3dPolyline->vertexIterator();
    while (!pIter->done())
    {
        AcDbObjectId objId = pIter->objectId();
        AcDb3dPolylineVertex *pVert;
        if (pAcDb3dPolyline->openVertex(pVert, objId, AcDb::kForRead)
            == Acad::eOk)
        {
            dumpEntity(pVert);
            pVert->close();
        }
        pIter->step();
    }
    delete pIter;
}

void dumpAcDbArc(AcDbEntity *pEnt)
{
    dumpAcDbCurve(pEnt);
    entInfo(pEnt, sizeof(AcDbArc));
    AcDbArc *pAcDbArc = (AcDbArc *)pEnt;

    double radius = pAcDbArc->radius();
    AcGePoint3d center = pAcDbArc->center();
    double startAngle = pAcDbArc->startAngle();
    double endAngle = pAcDbArc->endAngle();
    double thickness = pAcDbArc->thickness();
    AcGeVector3d normal = pAcDbArc->normal();

    _print(_T("\t\tradius %g\n"), radius);
    _print(_T("\t\tcenter %g,%g,%g\n"),
        center.x, center.y, center.z);
    _print(_T("\t\tstartAngle %g\n"), startAngle);
    _print(_T("\t\tendAngle %g\n"), endAngle);
    _print(_T("\t\tthickness %g\n"), thickness);
    _print(_T("\t\tnormal %g,%g,%g\n"),
        normal.x, normal.y, normal.z);
}

void dumpAcDbCircle(AcDbEntity *pEnt)
{
    dumpAcDbCurve(pEnt);
    entInfo(pEnt, sizeof(AcDbCircle));
    AcDbCircle *pAcDbCircle = (AcDbCircle *)pEnt;

    double radius = pAcDbCircle->radius();
    AcGePoint3d center = pAcDbCircle->center();
    double thickness = pAcDbCircle->thickness();
    AcGeVector3d normal = pAcDbCircle->normal();

    _print(_T("\t\tradius %g\n"), radius);
    _print(_T("\t\tcenter %g,%g,%g\n"),
        center.x, center.y, center.z);
    _print(_T("\t\tthickness %g\n"), thickness);
    _print(_T("\t\tnormal %g,%g,%g\n"),
        normal.x, normal.y, normal.z);
}

void dumpAcDbEllipse(AcDbEntity *pEnt)
{
    dumpAcDbCurve(pEnt);
    entInfo(pEnt, sizeof(AcDbEllipse));
    AcDbEllipse *pAcDbEllipse = (AcDbEllipse *)pEnt;

    AcGeVector3d normal = pAcDbEllipse->normal();
    AcGeVector3d majorAxis = pAcDbEllipse->majorAxis();
    AcGeVector3d minorAxis = pAcDbEllipse->minorAxis();
    AcGePoint3d center = pAcDbEllipse->center();
    double startAngle = pAcDbEllipse->startAngle();
    double endAngle = pAcDbEllipse->endAngle();
    double radiusRatio = pAcDbEllipse->radiusRatio();

    _print(_T("\t\tnormal %g,%g,%g\n"),
        normal.x, normal.y, normal.z);
    _print(_T("\t\tmajorAxis %g,%g,%g\n"),
        majorAxis.x, majorAxis.y, majorAxis.z);
    _print(_T("\t\tminorAxis %g,%g,%g\n"),
        minorAxis.x, minorAxis.y, minorAxis.z);
    _print(_T("\t\tcenter %g,%g,%g\n"),
        center.x, center.y, center.z);

    if (!pAcDbEllipse->isClosed())
    {
        _print(_T("\t\tstartAngle %g\n"), startAngle / PIOVER180);
        _print(_T("\t\tendAngle %g\n"), endAngle / PIOVER180);
    }
    _print(_T("\t\tradiusRatio %g\n"), radiusRatio);
}

void dumpAcDbLeader(AcDbEntity *pEnt)
{
    dumpAcDbCurve(pEnt);
    entInfo(pEnt, sizeof(AcDbLeader));
    AcDbLeader *pAcDbLeader = (AcDbLeader *)pEnt;

    AcGeVector3d normal = pAcDbLeader->normal();
    int numVertices = pAcDbLeader->numVertices();
    Adesk::Boolean hasArrowHead = pAcDbLeader->hasArrowHead();
    Adesk::Boolean hasHookLine = pAcDbLeader->hasHookLine();
    Adesk::Boolean isSplined = pAcDbLeader->isSplined();
    _print(_T("\t\tAnnoType %d\n"), pAcDbLeader->annoType());
    _print(_T("\t\tAnnoHeight %g\n"), pAcDbLeader->annoHeight());
    _print(_T("\t\tAnnoWidth %g\n"), pAcDbLeader->annoWidth());

    _print(_T("\t\tnormal %g,%g,%g\n"),
        normal.x, normal.y, normal.z);
    int i;
    for (i = 0; i < numVertices; i++)
    {
        AcGePoint3d pt;
        pt = pAcDbLeader->vertexAt(i);
        _print(_T("\t\t\tvertex[%d] %g,%g,%g\n"), i,
            pt.x, pt.y, pt.z);
    }
}

void dumpAcDbLine(AcDbEntity *pEnt)
{
    dumpAcDbCurve(pEnt);
    entInfo(pEnt, sizeof(AcDbLine));
    AcDbLine *pAcDbLine = (AcDbLine *)pEnt;

    double thickness = pAcDbLine->thickness();
    AcGeVector3d normal = pAcDbLine->normal();
    AcGePoint3d start = pAcDbLine->startPoint();
    AcGePoint3d end = pAcDbLine->endPoint();

    _print(_T("\t\tstartPoint %g,%g,%g\n"),
        start.x, start.y, start.z);
    _print(_T("\t\tendPoint %g,%g,%g\n"),
        end.x, end.y, end.z);
    _print(_T("\t\tthickness %g\n"), thickness);
    _print(_T("\t\tnormal %g,%g,%g\n"),
        normal.x, normal.y, normal.z);
}

void dumpAcDbPolyline(AcDbEntity *pEnt)
{
    dumpAcDbCurve(pEnt);
    entInfo(pEnt, sizeof(AcDbPolyline));
    AcDbPolyline *pAcDbPolyline = (AcDbPolyline *)pEnt;

    _print(_T("\t\tisOnlyLines %d hasPlinegen %d\n"),
        pAcDbPolyline->isOnlyLines(),
        pAcDbPolyline->hasPlinegen());

    double  Elevation;
    double  Thickness;
    double  ConstantWidth;
    AcGeVector3d    Normal;

    Elevation = pAcDbPolyline->elevation();
    _print(_T("\t\televation %g\n"), Elevation);
    Thickness = pAcDbPolyline->thickness();
    _print(_T("\t\tthickness %g\n"), Thickness);
    if ( pAcDbPolyline->getConstantWidth(ConstantWidth) == Acad::eOk ) {
        _print(_T("\t\tconstant width %g\n"), ConstantWidth);
    }
    Normal = pAcDbPolyline->normal();
    _print(_T("\t\tnormal %g,%g,%g\n"), Normal.x, Normal.y, Normal.z);
    
    int NumVerts = pAcDbPolyline->numVerts();
    if (NumVerts < 2) {
        return;
    }

    int i;
    int SegmentType;
    double  Bulge;
    double  StartWidth, EndWidth;
    AcGePoint3d position;
    static TCHAR *szSegType[] = { _T("kLine"),
                                 _T("kArc"),
                                 _T("kCoincident"),
                                 _T("kPoint"),
                                 _T("kEmpty") };

    for ( i = 0; i < NumVerts; i++) {
        _print(_T("\t\tvertex %d:\n"), i);
        if ( pAcDbPolyline->getPointAt(i, position) == Acad::eOk ) {
            _print(_T("\t\t\tposition %g %g %g\n"),
                position.x, position.y, position.z );
        }
        if ( pAcDbPolyline->getBulgeAt(i, Bulge) == Acad::eOk ) {
            _print(_T("\t\t\tbulge %g\n"), Bulge);
        }
        if ( pAcDbPolyline->getWidthsAt(i, StartWidth, EndWidth) == Acad::eOk ) {
            _print(_T("\t\t\tstart width %g\n"), StartWidth);
            _print(_T("\t\t\tend width %g\n"), EndWidth);
        }
        SegmentType = pAcDbPolyline->segType(i);
        _print(_T("\t\t\tsegment type: %ls\n"), szSegType[SegmentType]);
    }
}

void dumpAcDbRay(AcDbEntity *pEnt)
{
    dumpAcDbCurve(pEnt);
    entInfo(pEnt, sizeof(AcDbRay));
    AcDbRay *pAcDbRay = (AcDbRay *)pEnt;

    AcGePoint3d basePoint = pAcDbRay->basePoint();
    AcGeVector3d unitDir = pAcDbRay->unitDir();

    _print(_T("\t\tbasePoint %g,%g,%g\n"),
        basePoint.x, basePoint.y, basePoint.z);
    _print(_T("\t\tunitDir %g,%g,%g\n"),
        unitDir.x, unitDir.y, unitDir.z);
}

void dumpAcDbSpline(AcDbEntity *pEnt)
{
    dumpAcDbCurve(pEnt);
    entInfo(pEnt, sizeof(AcDbSpline));
    AcDbSpline *pAcDbSpline = (AcDbSpline *)pEnt;

    Adesk::Boolean isNull = pAcDbSpline->isNull();
    Adesk::Boolean isRational = pAcDbSpline->isRational();
    Adesk::Boolean isPeriodic = pAcDbSpline->isPeriodic();
    Adesk::Boolean isClosed = pAcDbSpline->isClosed();
    int degree = pAcDbSpline->degree();
    int nControlPoints = pAcDbSpline->numControlPoints();
    Adesk::Boolean hasFitData = pAcDbSpline->hasFitData();

    double fitTol = 0.0;
    AcGeVector3d startTangent, endTangent;
    int nFitPoints = 0;
    if (hasFitData)
    {
        fitTol = pAcDbSpline->fitTolerance();
        pAcDbSpline->getFitTangents(startTangent, endTangent);
        nFitPoints = pAcDbSpline->numFitPoints();
    }

    _print(_T("\t\tcontrolPoints %d\n"), nControlPoints);
    _print(_T("\t\tfitPoints %d\n"), nFitPoints);
    _print(_T("\t\tisNull %d isRational %d degree %d\n"), 
        isNull, isRational, degree);

    _print(_T("\t\thasFitData %d\n"), hasFitData);
    if (hasFitData)
    {
        _print(_T("\t\t\tfitTolerance %g\n"), fitTol);
        _print(_T("\t\t\ttangents start %g,%g,%g end %g,%g,%g\n"),
            startTangent.x, startTangent.y, startTangent.z,
            endTangent.x, endTangent.y, endTangent.z);
    }
    int i;
    for (i = 0; i < nControlPoints; i++)
    {
        AcGePoint3d pt;
        pAcDbSpline->getControlPointAt(i, pt);
        double weight = pAcDbSpline->weightAt(i);
        _print(_T("\t\t\tControlPoint[%d] %g,%g,%g weight %g\n"), i,
            pt.x, pt.y, pt.z,
            weight);
    }
    for (i = 0; i < nFitPoints; i++)
    {
        AcGePoint3d pt;
        pAcDbSpline->getFitPointAt(i, pt);
        _print(_T("\t\t\tFitPoint[%d] %g,%g,%g\n"), i,
            pt.x, pt.y, pt.z);
    }

    _print(_T("\t\tNow...from getNurbsData\n"));
    AcGePoint3dArray controlPoints;
    AcGeDoubleArray knots;
    AcGeDoubleArray weights;
    double controlPtTol;
    double knotTol;
    pAcDbSpline->getNurbsData(degree, isRational, isClosed, isPeriodic,
        controlPoints, knots, weights, controlPtTol, knotTol);

    _print(_T("\t\tisNull %d isRational %d\n"), isNull, isRational);
    _print(_T("\t\tisClosed %d isPeriodic %d\n"), isClosed, isPeriodic);
    _print(_T("\t\tdegree %d controlPtTol %g knotTol %g\n"), 
        degree, controlPtTol, knotTol);

    for (i = 0; i < controlPoints.length(); i++)
    {
        _print(_T("\t\t\tControlPoint[%d] %g,%g,%g\n"), i,
            controlPoints[i].x, 
            controlPoints[i].y, 
            controlPoints[i].z);
    }
    for (i = 0; i < knots.length(); i++)
    {
        _print(_T("\t\t\tknots[%d] %g\n"), i, knots[i]);
    }
    for (i = 0; i < weights.length(); i++)
    {
        _print(_T("\t\t\tweights[%d] %g\n"), i, weights[i]);
    }
    

}

void dumpAcDbXline(AcDbEntity *pEnt)
{
    dumpAcDbCurve(pEnt);
    entInfo(pEnt, sizeof(AcDbXline));
    AcDbXline *pAcDbXline = (AcDbXline *)pEnt;

    AcGePoint3d basePoint = pAcDbXline->basePoint();
    AcGeVector3d unitDir = pAcDbXline->unitDir();

    _print(_T("\t\tbasePoint %g,%g,%g\n"),
        basePoint.x, basePoint.y, basePoint.z);
    _print(_T("\t\tunitDir %g,%g,%g\n"),
        unitDir.x, unitDir.y, unitDir.z);
}

void dumpAcDbDimension(AcDbEntity *pEnt)
{
    AcDbDimension *pAcDbDimension = (AcDbDimension *)pEnt;
    entInfo(pEnt, sizeof(AcDbDimension));

    AcGePoint3d      textPosition = pAcDbDimension->textPosition();
    Adesk::Boolean   isUsingDefaultTextPosition = 
        pAcDbDimension->isUsingDefaultTextPosition();
    AcDbObjectId     blockId = pAcDbDimension->dimBlockId();
    AcGePoint3d      dimBlockPosition = pAcDbDimension->dimBlockPosition();

    AcGeVector3d     normal = pAcDbDimension->normal();
    TCHAR*            dimensionText= pAcDbDimension->dimensionText(); 
    double           textRotation = pAcDbDimension->textRotation();
    AcDbObjectId     dimensionStyle = pAcDbDimension->dimensionStyle();
    double           horizontalRotation = pAcDbDimension->horizontalRotation();

    AcDbBlockTableRecord *pRecord;
    if (acdbOpenObject(pRecord, blockId, AcDb::kForRead) == Acad::eOk)
    {
        TCHAR *pName;
        if (pRecord->getName(pName) == Acad::eOk)
        {
            _print(_T("\t\tBlock: %ls\n"), pName);
            acdbFree(pName);
        }
        pRecord->close();
    }

    _print(_T("\t\ttextPosition %g,%g,%g\n"),
        textPosition.x, textPosition.y, textPosition.z);
    _print(_T("\t\tdimBlockPosition %g,%g,%g\n"),
        dimBlockPosition.x, dimBlockPosition.y, dimBlockPosition.z);
    _print(_T("\t\tnormal %g,%g,%g\n"),
        normal.x, normal.y, normal.z);
    _print(_T("\t\tText: <%ls>\n"), dimensionText);
    _print(_T("\t\tdimStyle %Ix\n"), dimensionStyle);
    _print(_T("\t\thorzRotation %g\n"), horizontalRotation);
    _print(_T("\t\ttextRotation %g\n"), textRotation);

    acdbFree(dimensionText);
}

void dumpAcDb2LineAngularDimension(AcDbEntity *pEnt)
{
    AcDb2LineAngularDimension *pAcDbDimension = (AcDb2LineAngularDimension *)pEnt;
    entInfo(pEnt, sizeof(AcDb2LineAngularDimension));
    dumpAcDbDimension(pEnt);

    AcGePoint3d arcPoint = pAcDbDimension->arcPoint(); 
    AcGePoint3d xLine1Start = pAcDbDimension->xLine1Start(); 
    AcGePoint3d xLine1End = pAcDbDimension->xLine1End(); 
    AcGePoint3d xLine2Start = pAcDbDimension->xLine2Start(); 
    AcGePoint3d xLine2End = pAcDbDimension->xLine2End(); 

    _print(_T("\t\tarcPoint %g,%g,%g\n"),
        arcPoint.x, arcPoint.y, arcPoint.z);
    _print(_T("\t\txLine1Start %g,%g,%g\n"),
        xLine1Start.x, xLine1Start.y, xLine1Start.z);
    _print(_T("\t\txLine1End %g,%g,%g\n"),
        xLine1End.x, xLine1End.y, xLine1End.z);
    _print(_T("\t\txLine2Start %g,%g,%g\n"),
        xLine2Start.x, xLine2Start.y, xLine2Start.z);
    _print(_T("\t\txLine2End %g,%g,%g\n"),
        xLine2End.x, xLine2End.y, xLine2End.z);
}

void dumpAcDb3PointAngularDimension(AcDbEntity *pEnt)
{
    AcDb3PointAngularDimension *pAcDbDimension = (AcDb3PointAngularDimension *)pEnt;
    entInfo(pEnt, sizeof(AcDb3PointAngularDimension));
    dumpAcDbDimension(pEnt);

    AcGePoint3d arcPoint = pAcDbDimension->arcPoint(); 
    AcGePoint3d xLine1Point = pAcDbDimension->xLine1Point(); 
    AcGePoint3d xLine2Point = pAcDbDimension->xLine2Point(); 
    AcGePoint3d centerPoint = pAcDbDimension->centerPoint(); 

    _print(_T("\t\tarcPoint %g,%g,%g\n"),
        arcPoint.x, arcPoint.y, arcPoint.z);
    _print(_T("\t\txLine1Point %g,%g,%g\n"),
        xLine1Point.x, xLine1Point.y, xLine1Point.z);
    _print(_T("\t\txLine2Point %g,%g,%g\n"),
        xLine2Point.x, xLine2Point.y, xLine2Point.z);
    _print(_T("\t\tcenterPoint %g,%g,%g\n"),
        centerPoint.x, centerPoint.y, centerPoint.z);
}

void dumpAcDbAlignedDimension(AcDbEntity *pEnt)
{
    AcDbAlignedDimension *pAcDbDimension = (AcDbAlignedDimension *)pEnt;
    entInfo(pEnt, sizeof(AcDbAlignedDimension));
    dumpAcDbDimension(pEnt);

    AcGePoint3d xLine1Point = pAcDbDimension->xLine1Point(); 
    AcGePoint3d xLine2Point = pAcDbDimension->xLine2Point(); 
    AcGePoint3d dimLinePoint = pAcDbDimension->dimLinePoint(); 
    double oblique = pAcDbDimension->oblique();

    _print(_T("\t\txLine1Point %g,%g,%g\n"),
        xLine1Point.x, xLine1Point.y, xLine1Point.z);
    _print(_T("\t\txLine2Point %g,%g,%g\n"),
        xLine2Point.x, xLine2Point.y, xLine2Point.z);
    _print(_T("\t\tdimLinePoint %g,%g,%g\n"),
        dimLinePoint.x, dimLinePoint.y, dimLinePoint.z);
    _print(_T("\t\toblique %g\n"), oblique);
}

void dumpAcDbDiametricDimension(AcDbEntity *pEnt)
{
    AcDbDiametricDimension *pAcDbDimension = (AcDbDiametricDimension *)pEnt;
    entInfo(pEnt, sizeof(AcDbDiametricDimension));
    dumpAcDbDimension(pEnt);

    double leaderLength = pAcDbDimension->leaderLength();
    AcGePoint3d chordPoint = pAcDbDimension->chordPoint();
    AcGePoint3d farChordPoint = pAcDbDimension->farChordPoint();

    _print(_T("\t\tleaderLength %g\n"), leaderLength);
    _print(_T("\t\tchordPoint %g,%g,%g\n"),
        chordPoint.x, chordPoint.y, chordPoint.z);
    _print(_T("\t\tfarChordPoint %g,%g,%g\n"),
        farChordPoint.x, farChordPoint.y, farChordPoint.z);
}

void dumpAcDbOrdinateDimension(AcDbEntity *pEnt)
{
    AcDbOrdinateDimension *pAcDbDimension = (AcDbOrdinateDimension *)pEnt;
    entInfo(pEnt, sizeof(AcDbOrdinateDimension));
    dumpAcDbDimension(pEnt);

    Adesk::Boolean isUsingXAxis = pAcDbDimension->isUsingXAxis();
    Adesk::Boolean isUsingYAxis = pAcDbDimension->isUsingYAxis();
    AcGePoint3d origin = pAcDbDimension->origin();
    AcGePoint3d definingPoint = pAcDbDimension->definingPoint(); 
    AcGePoint3d leaderEndPoint = pAcDbDimension->leaderEndPoint(); 

    _print(_T("\t\t\tisUsingXAxis %d\t isUsingYAxis %d\n"),
        isUsingXAxis, isUsingYAxis);
    _print(_T("\t\torigin %g,%g,%g\n"),
        origin.x, origin.y, origin.z);
    _print(_T("\t\tdefiningPoint %g,%g,%g\n"),
        definingPoint.x, definingPoint.y, definingPoint.z);
    _print(_T("\t\tleaderEndPoint %g,%g,%g\n"),
        leaderEndPoint.x, leaderEndPoint.y, leaderEndPoint.z);
}

void dumpAcDbRadialDimension(AcDbEntity *pEnt)
{
    AcDbRadialDimension *pAcDbDimension = (AcDbRadialDimension *)pEnt;
    entInfo(pEnt, sizeof(AcDbRadialDimension));
    dumpAcDbDimension(pEnt);

    double leaderLength = pAcDbDimension->leaderLength();
    AcGePoint3d center = pAcDbDimension->center();
    AcGePoint3d chordPoint = pAcDbDimension->chordPoint();

    _print(_T("\t\tleaderLength %g\n"), leaderLength);
    _print(_T("\t\tcenter %g,%g,%g\n"),
        center.x, center.y, center.z);
    _print(_T("\t\tchordPoint %g,%g,%g\n"),
        chordPoint.x, chordPoint.y, chordPoint.z);
}

void dumpAcDbRotatedDimension(AcDbEntity *pEnt)
{
    AcDbRotatedDimension *pAcDbDimension = (AcDbRotatedDimension *)pEnt;
    entInfo(pEnt, sizeof(AcDbRotatedDimension));
    dumpAcDbDimension(pEnt);

    AcGePoint3d xLine1Point = pAcDbDimension->xLine1Point(); 
    AcGePoint3d xLine2Point = pAcDbDimension->xLine2Point(); 
    AcGePoint3d dimLinePoint = pAcDbDimension->dimLinePoint(); 
    double oblique = pAcDbDimension->oblique();
    double rotation = pAcDbDimension->rotation();

    _print(_T("\t\txLine1Point %g,%g,%g\n"),
        xLine1Point.x, xLine1Point.y, xLine1Point.z);
    _print(_T("\t\txLine2Point %g,%g,%g\n"),
        xLine2Point.x, xLine2Point.y, xLine2Point.z);
    _print(_T("\t\tdimLinePoint %g,%g,%g\n"),
        dimLinePoint.x, dimLinePoint.y, dimLinePoint.z);
    _print(_T("\t\toblique %g\n"), oblique);
    _print(_T("\t\trotation %g\n"), rotation);
}

void dumpAcDbFace(AcDbEntity *pEnt)
{
    AcDbFace *pAcDbFace = (AcDbFace *)pEnt;
    entInfo(pEnt, sizeof(AcDbFace));
    int i;
    for (i = 0; i < 4; i++)
    {
        AcGePoint3d pt;
        Adesk::Boolean visible;
        pAcDbFace->getVertexAt(i, pt);
        pAcDbFace->isEdgeVisibleAt(i, visible);
        _print(_T("\t\t%d pt %g,%g,%g     visible %d\n"), 
            i, pt.x, pt.y, pt.z, visible);
    }
}

void dumpAcDbFcf(AcDbEntity *pEnt)
{
    entInfo(pEnt, sizeof(AcDbFcf));
    AcDbFcf *pAcDbFcf = (AcDbFcf *)pEnt;
    int line = 0;

    while (1)
    {
        const TCHAR *pText = pAcDbFcf->text(line);
        if (!pText)
            break;
        _print(_T("\t\ttext[%d] %ls\n"), line++, pText);
        acdbFree((void *)pText);
    }

    AcGePoint3d location = pAcDbFcf->location();
    AcGeVector3d normal = pAcDbFcf->normal();
    AcGeVector3d direction = pAcDbFcf->direction();

    AcDbHardPointerId dimensionStyle = pAcDbFcf->dimensionStyle();

    AcDbDimStyleTableRecord *pRecord;
    if (acdbOpenObject(pRecord, dimensionStyle, AcDb::kForRead) == Acad::eOk)
    {
        TCHAR *pName;
        if (pRecord->getName(pName) == Acad::eOk)
        {
            _print(_T("\t\tDimension Style: %ls\n"), pName);
            acdbFree(pName);
        }
        pRecord->close();
    }

    _print(_T("\t\tlocation %g,%g,%g\n"),
        location.x, location.y, location.z);
    _print(_T("\t\tnormal %g,%g,%g\n"),
        normal.x, normal.y, normal.z);
    _print(_T("\t\tdirection %g,%g,%g\n"),
        direction.x, direction.y, direction.z);
}

void dumpAcDbFrame(AcDbEntity *pEnt)
{
    entInfo(pEnt, sizeof(AcDbFrame));
    AcDbFrame *pAcDbFrame = (AcDbFrame *)pEnt;
}

void dumpAcDbOleFrame(AcDbEntity *pEnt)
{
    entInfo(pEnt, sizeof(AcDbOleFrame));
    AcDbOleFrame *pAcDbOleFrame = (AcDbOleFrame *)pEnt;
}

void dumpAcDbOle2Frame(AcDbEntity *pEnt)
{
    entInfo(pEnt, sizeof(AcDbOle2Frame));
    AcDbOle2Frame *pAcDbOle2Frame = (AcDbOle2Frame *)pEnt;
}

void dumpAcDbHatch(AcDbEntity *pEnt)
{
    entInfo(pEnt, sizeof(AcDbHatch));
    AcDbHatch *pAcDbHatch = (AcDbHatch *)pEnt;

    _print(_T("\t\tassociative %d\n"), pAcDbHatch->associative());
    _print(_T("\t\tstyle %d\n"), pAcDbHatch->hatchStyle());

    _print(_T("\t\televation %g\n"), pAcDbHatch->elevation());
    AcGeVector3d    Normal(pAcDbHatch->normal());
    _print(_T("\t\tnormal %g,%g,%g\n"), Normal.x, Normal.y, Normal.z);
    
    _print(_T("\t\tpattern type %d\n"), pAcDbHatch->patternType());
    _print(_T("\t\tpattern name %ls\n"), pAcDbHatch->patternName());
    _print(_T("\t\tpattern angle %g\n"), pAcDbHatch->patternAngle());
    _print(_T("\t\tpattern space %g\n"), pAcDbHatch->patternSpace());
    _print(_T("\t\tpattern scale %g\n"), pAcDbHatch->patternScale());
    _print(_T("\t\tpattern double %d\n"), pAcDbHatch->patternDouble());

    int i;
    int NumLoops = pAcDbHatch->numLoops();
    for ( i = 0; i < NumLoops; i++ ) {
        _print(_T("\t\tloop %d:\n"), i);
        _print(_T("\t\t\tloop type %d\n"), pAcDbHatch->loopTypeAt(i));
    }

    int NumPatDefs = pAcDbHatch->numPatternDefinitions();
    double  angle;
    double  baseX;
    double  baseY;
    double  offsetX;
    double  offsetY;
    AcGeDoubleArray dashes;
    for ( i = 0; i < NumPatDefs; i++ ) {
        _print(_T("\t\tpattern definition %d:\n"), i);
        if ( pAcDbHatch->getPatternDefinitionAt(i, angle,
            baseX, baseY, offsetX, offsetY, dashes) == Acad::eOk ) {
            _print(_T("\t\t\tpattern angle %g\n"), angle);
            _print(_T("\t\t\tpattern baseX %g\n"), baseX);
            _print(_T("\t\t\tpattern baseY %g\n"), baseY);
            _print(_T("\t\t\tpattern offsetX %g\n"), offsetX);
            _print(_T("\t\t\tpattern offsetY %g\n"), offsetY);
        }
    }
}

void dumpAcDbRasterImage(AcDbEntity *pEnt)
{
    entInfo(pEnt, sizeof(AcDbRasterImage));
    AcDbRasterImage *pAcDbRasterImage = (AcDbRasterImage *)pEnt;

    // Image name.
    TCHAR*               pImageName = 0;
    AcDbObjectId dictId = AcDbRasterImageDef::imageDictionary(acdbHostApplicationServices()->workingDatabase());
    if (dictId.asOldId() != 0) {
        AcDbDictionary* pRasDict = 0;
        if (acdbOpenObject(pRasDict, dictId, AcDb::kForRead) == Acad::eOk) {
            pRasDict->nameAt(pAcDbRasterImage->imageDefId(), pImageName);
            pRasDict->close();
        }
    }
    _print(_T("\t\tImage:\t\t%ls\n"), pImageName ? pImageName : _T(""));

    // Insertion point.
    AcGePoint3d     insertion;
    AcGeVector3d    u, v;
    pAcDbRasterImage->getOrientation(insertion, u, v);
    _print(_T("\t\tInsertion:\tX=%9.5g,  Y=%9.5g,  Z=%9.5g\n"),
        insertion.x, insertion.y, insertion.z);

    // Image scale.
    AcGeVector2d    scale;
    scale = pAcDbRasterImage->scale();
    _print(_T("\t\tScale:\t\tX=%9.5g,  Y=%9.5g\n"), scale.x, scale.y);

    // Brightness.
    _print(_T("\t\tBrightness:\t%d\n"), pAcDbRasterImage->brightness());

    // Contrast.
    _print(_T("\t\tContrast:\t%d\n"), pAcDbRasterImage->contrast());
    
    // Fade.
    _print(_T("\t\tFade:\t\t%d\n"), pAcDbRasterImage->fade());

    // Show image.
    Adesk::Boolean  flag;
    flag = pAcDbRasterImage->isSetDisplayOpt(AcDbRasterImage::kShow);
    _print(_T("\t\tShow image:\t%ls\n"), flag ? _T("Yes") : _T("No"));

    // Show non-ortho.
    flag = pAcDbRasterImage->isSetDisplayOpt(AcDbRasterImage::kShowUnAligned);
    _print(_T("\t\tShow non-ortho:\t%ls\n"), flag ? _T("Yes") : _T("No"));

    // Transparency.
    flag = pAcDbRasterImage->isSetDisplayOpt(AcDbRasterImage::kTransparent);
    _print(_T("\t\tTransparency:\t%ls\n"), flag ? _T("On") : _T("Off"));

    // Clipping.
    flag = pAcDbRasterImage->isSetDisplayOpt(AcDbRasterImage::kClip);
    _print(_T("\t\tClipping:\t%ls\n"), flag ? _T("On") : _T("Off"));

    // Clip boundary type.
    switch ( pAcDbRasterImage->clipBoundaryType() )
    {
    case AcDbRasterImage::kPoly :
        _print(_T("\t\tClip boundary type: %ls\n"), _T("Polygonal"));
        break;
    case AcDbRasterImage::kRect :
        _print(_T("\t\tClip boundary type: %ls\n"), _T("Rectangular"));
        break;
    default :
        _print(_T("\t\tClip boundary type: %ls\n"), _T("Invalid"));
        break;
    }

    // Boundary points.
    int length;
    AcGePoint3dArray vertices;
    pAcDbRasterImage->getVertices(vertices);
    length = vertices.length() - 1;
    _print(_T("\t\tNumber of boundary points: %d\n"), length);
    for ( int i=0; i<length; i++ )
    {
        _print(_T("\t\t\tat point, X=%9.5g,  Y=%9.5g,  Z=%9.5g\n"),
            vertices.at(i).x, vertices.at(i).y, vertices.at(i).z);
    }
}

void dumpAcDbMline(AcDbEntity *pEnt)
{
    int i;
    entInfo(pEnt, sizeof(AcDbMline));
    AcDbMline *pAcDbMline = (AcDbMline *)pEnt;

    int numVertices = pAcDbMline->numVertices();
    Adesk::Boolean closedMline = pAcDbMline->closedMline();
    Adesk::Boolean supressStartCaps = pAcDbMline->supressStartCaps();
    Adesk::Boolean supressEndCaps = pAcDbMline->supressEndCaps();
    AcGePoint3d pt;
    pAcDbMline->element(pt);

    _print(_T("\t\tvertexCount %d\n"), numVertices);
    _print(_T("\t\tcloseMline %d\n"), closedMline);
    _print(_T("\t\tsupressStartCaps %d\n"), supressStartCaps);
    _print(_T("\t\tsupressEndCaps %d\n"), supressEndCaps);
    _print(_T("\t\telement %g,%g,%g\n"), pt.x, pt.y, pt.z);

    Mline::MlineJustification just = pAcDbMline->justification();
    switch (just)
    {
    case Mline::kTop:
        _print(_T("\t\tjustification kTop\n"));
        break;
    case Mline::kZero:
        _print(_T("\t\tjustification kZero\n"));
        break;
    case Mline::kBottom:
        _print(_T("\t\tjustification kBottom\n"));
        break;
    }
    _print(_T("\t\tscale %g\n"), pAcDbMline->scale());

    AcCmColor   fillColor;
    AcDbObjectId id = pAcDbMline->style();
    AcDbMlineStyle *pObj = 0;
    Acad::ErrorStatus es = acdbOpenObject(pObj, id, AcDb::kForRead);
    if (pObj)
    {
        _print(_T("\t\t%ls\n"), pObj->isA()->name());

        _print(_T("\t\t\tdescription()\t%ls\n"), pObj->description());
        _print(_T("\t\t\tname()\t%ls\n"), pObj->name());
        _print(_T("\t\t\tAdesk::Boolean showMiters()\t%d\n"), pObj->showMiters());
        _print(_T("\t\t\tAdesk::Boolean startSquareCap()\t%d\n"), pObj->startSquareCap());
        _print(_T("\t\t\tAdesk::Boolean startRoundCap()\t%d\n"), pObj->startRoundCap());
        _print(_T("\t\t\tAdesk::Boolean startInnerArcs()\t%d\n"), pObj->startInnerArcs());
        _print(_T("\t\t\tAdesk::Boolean endSquareCap()\t%d\n"), pObj->endSquareCap());
        _print(_T("\t\t\tAdesk::Boolean endRoundCap()\t%d\n"), pObj->endRoundCap());
        _print(_T("\t\t\tAdesk::Boolean endInnerArcs()\t%d\n"), pObj->endInnerArcs());

        fillColor = pObj->fillColor();
        if( fillColor.isByACI() )
        {
            _print(_T("\t\t\tint  fillColor()\t%lx (%lx)\n"), fillColor.color(),
            fillColor.entityColor().trueColor());
        }
        else
            _print(_T("\t\t\tint  fillColor()\t%lx\n"), fillColor.color());

        _print(_T("\t\t\tAdesk::Boolean  filled()\t%d\n"), pObj->filled());
        _print(_T("\t\t\tdouble  startAngle()\t%g\n"), pObj->startAngle());
        _print(_T("\t\t\tdouble  endAngle()\t%g\n"), pObj->endAngle());
        _print(_T("\t\t\tint numElements()\t%d\n"), pObj->numElements());

        for (i = 0; i < pObj->numElements(); i++)
        {
            double offset;
            AcCmColor colorIndex;
            AcDbObjectId lineTypeIndex;
            pObj->getElementAt(i, offset, colorIndex,
                lineTypeIndex);
            _print(_T("\t\t\telem[%d] offset %g colorIndex %d lineTypeIndex %Ix\n"),
                i, offset, colorIndex.colorIndex(), lineTypeIndex);
        }
        pObj->close();
    }
    else
        _print(_T("acdbopenacdbentity: %ls\n"), acadErrorStatusText(es));

    _print(_T("\t\tmline vertices:\n"));
    for (i = 0; i < numVertices; i++)
    {
        AcGePoint3d pt;
        AcGeVector3d axis, miter;
        pt = pAcDbMline->vertexAt(i);
        axis = pAcDbMline->axisAt(i);
        miter = pAcDbMline->miterAt(i);
        _print(_T("\t\tpt %g,%g,%g\n"), pt.x, pt.y, pt.z);
        _print(_T("\t\taxis %g,%g,%g\n"), axis.x, axis.y, axis.z);
        _print(_T("\t\tmiter %g,%g,%g\n"), miter.x, miter.y, miter.z);

        AcGeVoidPointerArray params;
        pAcDbMline->getParametersAt(i, params);
        for (void *pVoid : params)
        {
            AcGeDoubleArray* pElementParameters = (AcGeDoubleArray*)pVoid;
            int pcnt = pElementParameters->length();
            _print(_T("\t\tNumber of params for this element is %d\n"), pcnt);
            for (int k = 0; k < pcnt; k++)
            {
               // Notice:  Dereference pElementParameters before taking index
                _print(_T("\t\tElement Parameter %d is %f\n"), k, 
                       (*pElementParameters)[k]);
            }
        }
    }
}

void dumpAcDbMText(AcDbEntity *pEnt)
{
    entInfo(pEnt, sizeof(AcDbMText));
    AcDbMText *pAcDbMText = (AcDbMText *)pEnt;

    AcGePoint3d location = pAcDbMText->location();
    AcGeVector3d normal = pAcDbMText->normal();
    AcGeVector3d direction = pAcDbMText->direction();
    double rotation = pAcDbMText->rotation();
    double width = pAcDbMText->width();
    AcDbObjectId textStyle = pAcDbMText->textStyle();
    double textHeight = pAcDbMText->textHeight();
    AcDbMText::AttachmentPoint attachment = pAcDbMText->attachment();
    AcDbMText::FlowDirection flowDirection = pAcDbMText->flowDirection();
    TCHAR *contents = pAcDbMText->contents();
    double actualHeight = pAcDbMText->actualHeight();
    double actualWidth = pAcDbMText->actualWidth();
    AcGePoint3dArray boundingPoints;
    pAcDbMText->getBoundingPoints(boundingPoints);

    _print(_T("\t\tlocation %g,%g,%g\n"),
        location.x, location.y, location.z);
    _print(_T("\t\tnormal %g,%g,%g\n"),
        normal.x, normal.y, normal.z);
    _print(_T("\t\tdirection %g,%g,%g\n"),
        direction.x, direction.y, direction.z);
    _print(_T("\t\trotation %g\n"), rotation);
    _print(_T("\t\twidth %g\n"), width);
    _print(_T("\t\ttextHeight %g\n"), textHeight);
    _print(_T("\t\tactualWidth %g\n"), actualWidth);
    _print(_T("\t\tactualHeight %g\n"), actualHeight);
    _print(_T("\t\tboundingPoints\n"));
    for (const AcGePoint3d & pt : boundingPoints)
    {
        _print(_T("\t\t\t%g,%g,%g\n"),
            pt.x, pt.y, pt.z);
    }

    _print(_T("\t\tattachmentPoint\n"));
    switch (attachment)
    {
    case AcDbMText::kTopLeft:
        _print(_T("\t\t\tkTopLeft\n"));
        break;
    case AcDbMText::kTopCenter:
        _print(_T("\t\t\tkTopCenter\n"));
        break;
    case AcDbMText::kTopRight:
        _print(_T("\t\t\tkTopRight\n"));
        break;
    case AcDbMText::kMiddleLeft:
        _print(_T("\t\t\tkMiddleLeft\n"));
        break;
    case AcDbMText::kMiddleCenter:
        _print(_T("\t\t\tkMiddleCenter\n"));
        break;
    case AcDbMText::kMiddleRight:
        _print(_T("\t\t\tkMiddleRight\n"));
        break;
    case AcDbMText::kBottomLeft:
        _print(_T("\t\t\tkBottomLeft\n"));
        break;
    case AcDbMText::kBottomCenter:
        _print(_T("\t\t\tkBottomCenter\n"));
        break;
    case AcDbMText::kBottomRight:
        _print(_T("\t\t\tkBottomRight\n"));
        break;
    }

    _print(_T("\t\tflowDirection\n"));
    switch (flowDirection)
    {
    case AcDbMText::kLtoR:
        _print(_T("\t\t\tkLtoR\n"));
        break;
    case AcDbMText::kRtoL:
        _print(_T("\t\t\tkRtoL\n"));
        break;
    case AcDbMText::kTtoB:
        _print(_T("\t\t\tkTtoB\n"));
        break;
    case AcDbMText::kBtoT:
        _print(_T("\t\t\tkBtoT\n"));
        break;
    case AcDbMText::kByStyle:
        _print(_T("\t\t\tkByStyle\n"));
        break;
    };
    _print(_T("\t\tcontents \"%ls\"\n"), contents);

    acdbFree(contents);
}

void dumpAcDbPoint(AcDbEntity *pEnt)
{
    AcDbPoint *pAcDbPoint = (AcDbPoint *)pEnt;
    entInfo(pEnt, sizeof(AcDbPoint));

    AcGePoint3d position = pAcDbPoint->position();
    double thickness = pAcDbPoint->thickness();
    AcGeVector3d normal = pAcDbPoint->normal();
    double ecsRotation = pAcDbPoint->ecsRotation();

    _print(_T("\t\tposition %g,%g,%g\n"),
        position.x, position.y, position.z);
    _print(_T("\t\tthickness %g\n"), thickness);
    _print(_T("\t\tnormal %g,%g,%g\n"),
        normal.x, normal.y, normal.z);
    _print(_T("\t\tecsRotation %g\n"), ecsRotation);
}

void dumpAcDbPolyFaceMesh(AcDbEntity *pEnt)
{
    AcDbPolyFaceMesh *pAcDbPolyFaceMesh = (AcDbPolyFaceMesh *)pEnt;
    entInfo(pEnt, sizeof(AcDbPolyFaceMesh));
    _print(_T("\t\tvertexCount %d faceCount %d\n"), 
        pAcDbPolyFaceMesh->numVertices(),
        pAcDbPolyFaceMesh->numFaces());

    int iVerts = 0;
    AcDbObjectIterator* pIter = pAcDbPolyFaceMesh->vertexIterator();
    while (!pIter->done())
    {
        AcDbObjectId objId = pIter->objectId();
        AcDbVertex *pVert;
        if (pAcDbPolyFaceMesh->openVertex(pVert, objId, AcDb::kForRead)
            == Acad::eOk)
        {
//            if (pEnt->isKindOf(AcDbPolyFaceMeshVertex::desc()))
            {
                iVerts++;
                dumpEntity(pVert);
            }
            pVert->close();
        }
        pIter->step();
    }
    delete pIter;
    _print(_T("\t\t\t%d vertices\n"), iVerts);

}

void dumpAcDbPolygonMesh(AcDbEntity *pEnt)
{
    AcDbPolygonMesh *pAcDbPolygonMesh = (AcDbPolygonMesh *)pEnt;
    entInfo(pEnt, sizeof(AcDbPolygonMesh));

    AcDb::PolyMeshType polyMeshType = pAcDbPolygonMesh->polyMeshType();
    Adesk::Int16 mSize = pAcDbPolygonMesh->mSize();
    Adesk::Int16 nSize = pAcDbPolygonMesh->nSize();
    Adesk::Boolean isMClosed = pAcDbPolygonMesh->isMClosed();
    Adesk::Boolean isNClosed = pAcDbPolygonMesh->isNClosed();
    Adesk::Int16 mSurfaceDensity = pAcDbPolygonMesh->mSurfaceDensity();
    Adesk::Int16 nSurfaceDensity = pAcDbPolygonMesh->nSurfaceDensity();

    _print(_T("\t\tsize\tm %d n %d\n"), mSize, nSize);
    _print(_T("\t\tclose\tm %d n %d\n"), isMClosed, isNClosed);
    _print(_T("\t\tsurfaceDesity\tm %d n %d\n"), mSurfaceDensity, nSurfaceDensity);

    AcDbObjectIterator* pIter = pAcDbPolygonMesh->vertexIterator();
    while (!pIter->done())
    {
        AcDbObjectId objId = pIter->objectId();
        AcDbPolygonMeshVertex *pVert;
        if (pAcDbPolygonMesh->openVertex(pVert, objId, AcDb::kForRead)
            == Acad::eOk)
        {
            dumpEntity(pVert);
            pVert->close();
        }
        pIter->step();
    }
    delete pIter;

}

void dumpAcDbProxyEntity(AcDbEntity *pEnt)
{
    AcDbProxyEntity *pAcDbProxyEntity = (AcDbProxyEntity *)pEnt;
    entInfo(pEnt, sizeof(AcDbProxyEntity));

    const TCHAR* outstr;
    outstr = pAcDbProxyEntity->originalDxfName();
    _print(_T("\t\tDXF name:\t %ls\n"), outstr);
    outstr = pAcDbProxyEntity->originalClassName();
    _print(_T("\t\tClass name:\t %ls\n"), outstr);
}

void dumpAcDbRegion(AcDbEntity *pEnt)
{
    entInfo(pEnt, sizeof(AcDbRegion));
    AcDbRegion *pAcDbRegion = (AcDbRegion *)pEnt;

    double perimeter, regionArea;
    AcGePlane regionPlane;
    AcGeVector3d normal;
    pAcDbRegion->getPerimeter (perimeter);
    pAcDbRegion->getArea(regionArea);
    pAcDbRegion->getPlane(regionPlane);
    pAcDbRegion->getNormal(normal);

    AcGePoint3d planeOrigin;
    AcGeVector3d uAxis;
    AcGeVector3d vAxis;
    regionPlane.get(planeOrigin, uAxis, vAxis);

    _print(_T("\t\tperimeter %g\n"), perimeter);
    _print(_T("\t\tregionString %g\n"), regionArea);
    _print(_T("\t\tregionPlane origin %g,%g,%g\n"),
        planeOrigin.x, planeOrigin.y, planeOrigin.z);
    _print(_T("\t\t\tuAxis %g,%g,%g\n"),
        uAxis.x, uAxis.y, uAxis.z);
    _print(_T("\t\t\tvAxis %g,%g,%g\n"),
        vAxis.x, vAxis.y, vAxis.z);
    _print(_T("\t\tnormal %g,%g,%g\n"),
        normal.x, normal.y, normal.z);
}

void dumpAcDbSequenceEnd(AcDbEntity *pEnt)
{
    AcDbSequenceEnd *pAcDbSequenceEnd = (AcDbSequenceEnd *)pEnt;
    entInfo(pEnt, sizeof(AcDbSequenceEnd));
}

void dumpAcDbShape(AcDbEntity *pEnt)
{
    AcDbShape *pAcDbShape = (AcDbShape *)pEnt;
    entInfo(pEnt, sizeof(AcDbShape));

    AcGePoint3d      position = pAcDbShape->position();
    double           size = pAcDbShape->size();
    TCHAR*            name = pAcDbShape->name();
    double           rotation = pAcDbShape->rotation();
    double           widthFactor = pAcDbShape->widthFactor();
    double           oblique = pAcDbShape->oblique();
    double           thickness = pAcDbShape->thickness();
    AcGeVector3d     normal = pAcDbShape->normal();

    _print(_T("\t\tname <%ls>\n"), name);
    _print(_T("\t\tposition %g,%g,%g\n"),
        position.x, position.y, position.z);
    _print(_T("\t\tnormal %g,%g,%g\n"),
        normal.x, normal.y, normal.z);
    _print(_T("\t\tsize %g\n"), size);
    _print(_T("\t\trotation %g\n"), rotation);
    _print(_T("\t\twidthFactor %g\n"), widthFactor);
    _print(_T("\t\toblique %g\n"), oblique);
    _print(_T("\t\tthickness %g\n"), thickness);

    if (name)
        acdbFree(name);
}

void dumpAcDbSolid(AcDbEntity *pEnt)
{
    AcDbSolid *pAcDbSolid = (AcDbSolid *)pEnt;
    entInfo(pEnt, sizeof(AcDbSolid));

    double thickness = pAcDbSolid->thickness();
    AcGeVector3d normal = pAcDbSolid->normal();

    _print(_T("\t\tthickness %g\n"), thickness);
    _print(_T("\t\tnormal %g,%g,%g\n"),
        normal.x, normal.y, normal.z);

    int i;
    for (i = 0; i < 4; i++)
    {
        AcGePoint3d pt;
        pAcDbSolid->getPointAt(i, pt);
        _print(_T("\t\tpt %g,%g,%g\n"), pt.x, pt.y, pt.z);
    }
}

void dumpAcDbText(AcDbEntity *pEnt)
{
    AcDbText *pAcDbText = (AcDbText *)pEnt;
    entInfo(pEnt, sizeof(AcDbText));

    AcDbObjectId blockId = pAcDbText->textStyle();
    AcGePoint3d position = pAcDbText->position();
    AcGePoint3d align = pAcDbText->alignmentPoint();
    Adesk::Boolean isDefAlign = pAcDbText->isDefaultAlignment();
    AcGeVector3d normal = pAcDbText->normal();
    double thickness = pAcDbText->thickness();
    double oblique = pAcDbText->oblique();
    double rotation = pAcDbText->rotation();
    double height = pAcDbText->height();
    double widthFactor = pAcDbText->widthFactor();
    const TCHAR *pText = pAcDbText->textStringConst();
    Adesk::Boolean isMirrorX = pAcDbText->isMirroredInX();
    Adesk::Boolean isMirrorY = pAcDbText->isMirroredInY();
    AcDb::TextHorzMode horzMode = pAcDbText->horizontalMode();
    AcDb::TextVertMode vertMode = pAcDbText->verticalMode();

    AcDbTextStyleTableRecord *pRecord = 0;
    acdbOpenObject(pRecord, blockId, AcDb::kForRead);
    if (pRecord)
    {
        TCHAR *pName;
        if (pRecord->getName(pName) == Acad::eOk)
        {
            _print(_T("\t\tText Style: %ls\n"), pName);
            acdbFree(pName);
        }
        pRecord->close();
    }
    _print(_T("\t\tText: %ls\n"), pText);
    _print(_T("\t\tposition %g,%g,%g\n"),
        position.x, position.y, position.z);
    _print(_T("\t\talignmentPoint %g,%g,%g\n"),
        align.x, align.y, align.z);
    _print(_T("\t\tnormal %g,%g,%g\n"),
        normal.x, normal.y, normal.z);
    _print(_T("\t\tthickness %g\n"),
        thickness);
    _print(_T("\t\toblique %g\n"),
        oblique);
    _print(_T("\t\trotation %g\n"),
        rotation);
    _print(_T("\t\theight %g\n"),
        height);
    _print(_T("\t\twidthFactor %g\n"),
        widthFactor);
    _print(_T("\t\tmirror X %d Y %d\n"),
        isMirrorX, isMirrorY);
    _print(_T("\t\tmode horz %d vert %d\n"),
        horzMode, vertMode);
}

void dumpAcDbAttribute(AcDbEntity *pEnt)
{
    AcDbAttribute *pAcDbAttribute = (AcDbAttribute *)pEnt;
//    entInfo(pEnt, sizeof(AcDbAttribute));
    dumpAcDbText(pAcDbAttribute);

    const TCHAR* tag = pAcDbAttribute->tagConst();
    Adesk::Boolean isInvisible = pAcDbAttribute->isInvisible();
    Adesk::Boolean isConstant = pAcDbAttribute->isConstant();
    Adesk::Boolean isVerifiable = pAcDbAttribute->isVerifiable();
    Adesk::Boolean isPreset = pAcDbAttribute->isPreset();
    Adesk::UInt16 fieldLength = pAcDbAttribute->fieldLength();

    _print(_T("\t\ttag %ls\n"), tag);
    _print(_T("\t\tisInvisible %d isConstant %d isVerifiable %d isPreset %d\n"), 
        isInvisible, isConstant, isVerifiable, isPreset);
    _print(_T("\t\tfieldLength %u\n"), fieldLength);

}

void dumpAcDbAttributeDefinition(AcDbEntity *pEnt)
{
    AcDbAttributeDefinition *pAcDbAttributeDefinition = (AcDbAttributeDefinition *)pEnt;
//    entInfo(pEnt, sizeof(AcDbAttributeDefinition));
    dumpAcDbText(pAcDbAttributeDefinition);

    const TCHAR* prompt = pAcDbAttributeDefinition->promptConst();
    const TCHAR* tag = pAcDbAttributeDefinition->tagConst();
    Adesk::Boolean isInvisible = pAcDbAttributeDefinition->isInvisible();
    Adesk::Boolean isConstant = pAcDbAttributeDefinition->isConstant();
    Adesk::Boolean isVerifiable = pAcDbAttributeDefinition->isVerifiable();
    Adesk::Boolean isPreset = pAcDbAttributeDefinition->isPreset();
    Adesk::UInt16 fieldLength = pAcDbAttributeDefinition->fieldLength();

    _print(_T("\t\tprompt %ls\n"), prompt);
    _print(_T("\t\ttag %ls\n"), tag);
    _print(_T("\t\tisInvisible %d isConstant %d isVerifiable %d isPreset %d\n"), 
        isInvisible, isConstant, isVerifiable, isPreset);
    _print(_T("\t\tfieldLength %d\n"), fieldLength);
}

void dumpAcDbTrace(AcDbEntity *pEnt)
{
    AcDbTrace *pAcDbTrace = (AcDbTrace *)pEnt;
    entInfo(pEnt, sizeof(AcDbTrace));

    double thickness = pAcDbTrace->thickness();
    AcGeVector3d normal = pAcDbTrace->normal();

    _print(_T("\t\tthickness %g\n"), thickness);
    _print(_T("\t\tnormal %g,%g,%g\n"),
        normal.x, normal.y, normal.z);

    int i;
    for (i = 0; i < 4; i++)
    {
        AcGePoint3d pt;
        pAcDbTrace->getPointAt(i, pt);
        _print(_T("\t\tpt %g,%g,%g\n"), pt.x, pt.y, pt.z);
    }
}

void dumpAcDbVertex(AcDbEntity *pEnt)
{
    AcDbVertex *pAcDbVertex = (AcDbVertex *)pEnt;
    entInfo(pEnt, sizeof(AcDbVertex));
}

void dumpAcDb2dVertex(AcDbEntity *pEnt)
{
    dumpAcDbVertex(pEnt);
    entInfo(pEnt, sizeof(AcDb2dVertex));
    AcDb2dVertex *pAcDb2dVertex = (AcDb2dVertex *)pEnt;

    AcDb::Vertex2dType vertexType = pAcDb2dVertex->vertexType();
    AcGePoint3d position = pAcDb2dVertex->position();
    double startWidth = pAcDb2dVertex->startWidth();
    double endWidth = pAcDb2dVertex->endWidth();
    double bulge = pAcDb2dVertex->bulge();
    Adesk::Boolean isTangentUsed = pAcDb2dVertex->isTangentUsed();
    double tangent = pAcDb2dVertex->tangent();

    _print(_T("\t\t\tvertexType %d\n"), vertexType);
    _print(_T("\t\t\tposition %g,%g,%g\n"), position.x, position.y, position.z);
    if (startWidth != 0.0 || endWidth != 0.0)
        _print(_T("\t\t\twidth start %g end %g\n"), startWidth, endWidth);
    if (bulge)
        _print(_T("\t\t\tbulge %g\n"), bulge);
    if (isTangentUsed)
        _print(_T("\t\t\ttangent %g\n"), tangent);
}

void dumpAcDb3dPolylineVertex(AcDbEntity *pEnt)
{
    dumpAcDbVertex(pEnt);
    entInfo(pEnt, sizeof(AcDb3dPolylineVertex));
    AcDb3dPolylineVertex *pAcDb3dPolylineVertex = (AcDb3dPolylineVertex *)pEnt;

    AcDb::Vertex3dType vertexType = pAcDb3dPolylineVertex->vertexType();
    AcGePoint3d position = pAcDb3dPolylineVertex->position();

    _print(_T("\t\t\tvertexType %d\n"), vertexType);
    _print(_T("\t\t\tposition %g,%g,%g\n"), position.x, position.y, position.z);
}

void dumpAcDbFaceRecord(AcDbEntity *pEnt)
{
    AcDbFaceRecord *pAcDbFaceRecord = (AcDbFaceRecord *)pEnt;
    entInfo(pEnt, sizeof(AcDbFaceRecord));

    for (int i = 0; i < 4; i++)
    {
        Adesk::Int16 vtxIndex;
        Adesk::Boolean visible;
        pAcDbFaceRecord->getVertexAt(i, vtxIndex);
        pAcDbFaceRecord->isEdgeVisibleAt(i, visible);
        _print(_T("\t\t\t\tvtxIndex %d\tvisible %d\n"), vtxIndex, visible);
    }
}

void dumpAcDbPolyFaceMeshVertex(AcDbEntity *pEnt)
{
    dumpAcDbVertex(pEnt);
    entInfo(pEnt, sizeof(AcDbPolyFaceMeshVertex));
    AcDbPolyFaceMeshVertex *pAcDbPolyFaceMeshVertex = (AcDbPolyFaceMeshVertex *)pEnt;

    AcGePoint3d position = pAcDbPolyFaceMeshVertex->position();

    _print(_T("\t\t\tposition %g,%g,%g\n"), position.x, position.y, position.z);
}

void dumpAcDbPolygonMeshVertex(AcDbEntity *pEnt)
{
    dumpAcDbVertex(pEnt);
    entInfo(pEnt, sizeof(AcDbPolygonMeshVertex));
    AcDbPolygonMeshVertex *pAcDbPolygonMeshVertex = (AcDbPolygonMeshVertex *)pEnt;

    AcDb::Vertex3dType vertexType = pAcDbPolygonMeshVertex->vertexType();
    AcGePoint3d position = pAcDbPolygonMeshVertex->position();

    _print(_T("\t\t\tvertexType %d\n"), vertexType);
    _print(_T("\t\t\tposition %g,%g,%g\n"), position.x, position.y, position.z);
}

void dumpAcDbViewport(AcDbEntity *pEnt)
{
    AcDbViewport *pAcDbViewport = (AcDbViewport *)pEnt;
    entInfo(pEnt, sizeof(AcDbViewport));

    double height = pAcDbViewport->height();
    double width = pAcDbViewport->width();
    AcGePoint3d centerPoint = pAcDbViewport->centerPoint();
    Adesk::Int16 number = pAcDbViewport->number();
    AcGePoint3d viewTarget = pAcDbViewport->viewTarget();
    AcGeVector3d viewDirection = pAcDbViewport->viewDirection();
    double viewHeight = pAcDbViewport->viewHeight();
    AcGePoint2d viewCenter = pAcDbViewport->viewCenter();

    double twistAngle = pAcDbViewport->twistAngle();
    double lensLength = pAcDbViewport->lensLength();
    Adesk::Boolean isFrontClipOn = pAcDbViewport->isFrontClipOn();
    Adesk::Boolean isBackClipOn = pAcDbViewport->isBackClipOn();
    Adesk::Boolean isFrontClipAtEyeOn = pAcDbViewport->isFrontClipAtEyeOn();
    double frontClipDistance = pAcDbViewport->frontClipDistance();
    double backClipDistance = pAcDbViewport->backClipDistance();
    Adesk::Boolean isPerspectiveOn = pAcDbViewport->isPerspectiveOn();
    Adesk::Boolean isUcsFollowModeOn = pAcDbViewport->isUcsFollowModeOn();
    Adesk::Boolean isUcsIconVisible = pAcDbViewport->isUcsIconVisible();
    Adesk::Boolean isUcsIconAtOrigin = pAcDbViewport->isUcsIconAtOrigin();
    Adesk::Boolean isFastZoomOn = pAcDbViewport->isFastZoomOn();
    Adesk::UInt16 circleSides = pAcDbViewport->circleSides();
    Adesk::Boolean isSnapOn = pAcDbViewport->isSnapOn();
    Adesk::Boolean isSnapIsometric = pAcDbViewport->isSnapIsometric();
    double snapAngle = pAcDbViewport->snapAngle();

    AcGePoint2d snapBasePoint = pAcDbViewport->snapBasePoint();
    AcGeVector2d snapIncrement = pAcDbViewport->snapIncrement();
    Adesk::UInt16 snapIsoPair = pAcDbViewport->snapIsoPair();
    Adesk::Boolean isGridOn = pAcDbViewport->isGridOn();
    AcGeVector2d gridIncrement = pAcDbViewport->gridIncrement();
    Adesk::Boolean hiddenLinesRemoved = pAcDbViewport->hiddenLinesRemoved();

    _print(_T("\t\theight %g width %g\n"), height, width);
    _print(_T("\t\tcenterPoint() %g %g %g\n"),
        centerPoint.x, centerPoint.y, centerPoint.z);
    _print(_T("\t\tnumber %d\n"), number);
    _print(_T("\t\tviewTarget %g %g %g\n"),
        viewTarget.x, viewTarget.y, viewTarget.z);
    _print(_T("\t\tviewDirection %g %g %g\n"),
        viewDirection.x, viewDirection.y, viewDirection.z);
    _print(_T("\t\tviewHeight %g\n"), viewHeight);
    _print(_T("\t\tviewCenter %g %g\n"),
        viewCenter.x, viewCenter.y);

    _print(_T("\t\ttwistAngle\t%g\n"), twistAngle);
    _print(_T("\t\tlensLength\t%g\n"), lensLength);
    _print(_T("\t\tisFrontClipOn\t%d\n"), isFrontClipOn);
    _print(_T("\t\tisBackClipOn\t%d\n"), isBackClipOn);
    _print(_T("\t\tisFrontClipAtEyeOn\t%d\n"), isFrontClipAtEyeOn);
    _print(_T("\t\tfrontClipDistance\t%g\n"), frontClipDistance);
    _print(_T("\t\tbackClipDistance\t%g\n"), backClipDistance);
    _print(_T("\t\tisPerspectiveOn\t%d\n"), isPerspectiveOn);
    _print(_T("\t\tisUcsFollowModeOn\t%d\n"), isUcsFollowModeOn);
    _print(_T("\t\tisUcsIconVisible\t%d\n"), isUcsIconVisible);
    _print(_T("\t\tisUcsIconAtOrigin\t%d\n"), isUcsIconAtOrigin);
    _print(_T("\t\tisFastZoomOn\t%d\n"), isFastZoomOn);
    _print(_T("\t\tcircleSides\t%d\n"), circleSides);
    _print(_T("\t\tisSnapOn\t%d\n"), isSnapOn);
    _print(_T("\t\tisSnapIsometric\t%d\n"), isSnapIsometric);
    _print(_T("\t\tsnapAngle\t%d\n"), snapAngle);
    _print(_T("\t\tsnapBasePoint %g %g\n"),
        snapBasePoint.x, snapBasePoint.y);
    _print(_T("\t\tsnapIncrement %g %g\n"),
        snapIncrement.x, snapIncrement.y);
    _print(_T("\t\tsnapIsopair\t%d\n"), snapIsoPair);
    _print(_T("\t\tisGridOn\t%d\n"), isGridOn);
    _print(_T("\t\tgridIncrement %g %g\n"),
        gridIncrement.x, gridIncrement.y);
    _print(_T("\t\thiddenLinesRemoved\t%d\n"), hiddenLinesRemoved);
}

CMapPtrToPtr *entMap = 0;

class EntInfo
{
public:
    EntInfo()
    {
        mpName = 0;
        mSize = 0;
        mCount = 0;
    }
    ~EntInfo()
    {
    }
    const TCHAR *mpName;
    int mSize;
    int mCount;
};
void deleteEntMap()
{
    if (entMap)
    {
        EntInfo *pInfo = 0;
        for (void* pos = entMap->GetStartPosition();pos != nullptr;)
        {
            entMap->GetNextAssoc(pos,(void*&)keyName,(void*&)pInfo);
            delete pInfo;
        }
        delete entMap;
        entMap = nullptr;
    }
}
void entInfo(AcDbEntity *pEnt, int size)
{
    if (!entMap)
    {
        entMap = new CMapPtrToPtr;
    }
    EntInfo *pInfo = 0;
    if (!entMap->Lookup((void *)keyName, (void *&)pInfo))
    {
        pInfo = new EntInfo;
        pInfo->mpName = keyName;
        pInfo->mSize = size;
        entMap->SetAt((void *)keyName, (void *&)pInfo);
    }
    pInfo->mCount++;
}

extern "C" void printObjectPagerStatistics();

void dumpEntInfo()
{
    EntInfo *pInfo = 0;

    _print(_T("\nEntity information:\n"));
    _print(_T("%-30ls %15ls %15ls %15ls\n"), _T("Entity Type"), _T("Size w/o Imp"), _T("Size with Imp"),
        _T("# of Entities"));

    if (entMap)
    {
        for ( int i = 0; i < nEntNames; i++ ) {
            if (entMap->Lookup((void *)EntityList[i].pName, (void *&)pInfo)) {
                _print(_T("%-30ls %15d %15d %15d\n"),
                    pInfo->mpName, pInfo->mSize, 
                    /*acdbGetObjectSize(pInfo->mpName)*/0,
                    pInfo->mCount);
            }
        }
    }
}

void printDate(const TCHAR *pStr, AcDbDate td)
{
}

void dumpDwgHeaderVars(AcDbDatabase *pDatabase)
{
    static TCHAR str[128] = _T("<empty>");
    TCHAR *pStr;

    // Header variable access

    // Adesk::Boolean dimaso()
    _print(_T("\tAdesk::Boolean dimaso() %x\n"), pDatabase->dimaso());

    // Adesk::Boolean dimsho()
    _print(_T("\tAdesk::Boolean dimsho() %x\n"), pDatabase->dimsho());

    // Adesk::Boolean plinegen()
    _print(_T("\tAdesk::Boolean plinegen() %x\n"), pDatabase->plinegen());

    // Adesk::Boolean orthomode()
    _print(_T("\tAdesk::Boolean orthomode() %x\n"), pDatabase->orthomode());

    // Adesk::Boolean regenmode()
    _print(_T("\tAdesk::Boolean regenmode() %x\n"), pDatabase->regenmode());

    // Adesk::Boolean fillmode()
    _print(_T("\tAdesk::Boolean fillmode() %x\n"), pDatabase->fillmode());

    // Adesk::Boolean qtextmode()
    _print(_T("\tAdesk::Boolean qtextmode() %x\n"), pDatabase->qtextmode());

    // Adesk::Boolean psltscale()
    _print(_T("\tAdesk::Boolean psltscale() %x\n"), pDatabase->psltscale());

    // Adesk::Boolean limcheck()
    _print(_T("\tAdesk::Boolean limcheck() %x\n"), pDatabase->limcheck());

    // Adesk::Boolean blipmode()
    _print(_T("\tAdesk::Boolean blipmode() %x\n"), pDatabase->blipmode());

    // Adesk::Int16 saveproxygraphics()
    _print(_T("\tAdesk::Int16 saveproxygraphics() %x\n"), 
        pDatabase->saveproxygraphics());

    // Adesk::Boolean delUsedObj()
    _print(_T("\tAdesk::Int16 delUsedObj() %x\n"), pDatabase->delUsedObj());

    // Adesk::Boolean dispSilh()
    _print(_T("\tAdesk::Boolean dispSilh() %x\n"), pDatabase->dispSilh());

    // Adesk::Boolean plineEllipse()
    _print(_T("\tAdesk::Boolean plineEllipse() %x\n"), pDatabase->plineEllipse());

    // Adesk::Boolean usrtimer()
    _print(_T("\tAdesk::Boolean usrtimer() %x\n"), pDatabase->usrtimer());

    // Adesk::Boolean skpoly()
    _print(_T("\tAdesk::Boolean skpoly() %x\n"), pDatabase->skpoly());

    // Adesk::Boolean angdir()
    _print(_T("\tAdesk::Boolean angdir() %x\n"), pDatabase->angdir());

    // Adesk::Boolean splframe()
    _print(_T("\tAdesk::Boolean splframe() %x\n"), pDatabase->splframe());

    // Adesk::Boolean attreq()
    _print(_T("\tAdesk::Boolean attreq() %x\n"), pDatabase->attreq());

    // Adesk::Boolean attdia()
    _print(_T("\tAdesk::Boolean attdia() %x\n"), pDatabase->attdia());

    // Adesk::Boolean mirrtext()
    _print(_T("\tAdesk::Boolean mirrtext() %x\n"), pDatabase->mirrtext());

    // Adesk::Boolean worldview()
    _print(_T("\tAdesk::Boolean worldview() %x\n"), pDatabase->worldview());

    // Adesk::Boolean tilemode()
    _print(_T("\tAdesk::Boolean tilemode() %x\n"), pDatabase->tilemode());

    // Adesk::Boolean plimcheck()
    _print(_T("\tAdesk::Boolean plimcheck() %x\n"), pDatabase->plimcheck());

    // Adesk::Boolean visretain()
    _print(_T("\tAdesk::Boolean visretain() %x\n"), pDatabase->visretain());

    // Adesk::Int16 dragmode()
    _print(_T("\tAdesk::Int16 dragmode() %x\n"), pDatabase->dragmode());

    // Adesk::Int16 treedepth()
    _print(_T("\tAdesk::Int16 treedepth() %x\n"), pDatabase->treedepth());

    // Adesk::Int16 lunits()
    _print(_T("\tAdesk::Int16 lunits() %x\n"), pDatabase->lunits());

    // Adesk::Int16 luprec()
    _print(_T("\tAdesk::Int16 luprec() %x\n"), pDatabase->luprec());

    // Adesk::Int16 aunits()
    _print(_T("\tAdesk::Int16 aunits() %x\n"), pDatabase->aunits());

    // Adesk::Int16 auprec()
    _print(_T("\tAdesk::Int16 auprec() %x\n"), pDatabase->auprec());

    // Adesk::Int16 attmode()
    _print(_T("\tAdesk::Int16 attmode() %x\n"), pDatabase->attmode());

    // Adesk::Int16 coords()
    _print(_T("\tAdesk::Int16 coords() %x\n"), pDatabase->coords());

    // Adesk::Int16 pdmode()
    _print(_T("\tAdesk::Int16 pdmode() %x\n"), pDatabase->pdmode());

    // Adesk::Int16 pickstyle()
    _print(_T("\tAdesk::Int16 pickstyle() %x\n"), pDatabase->pickstyle());

    // Adesk::Int16 useri1()
    _print(_T("\tAdesk::Int16 useri1() %x\n"), pDatabase->useri1());

    // Adesk::Int16 useri2()
    _print(_T("\tAdesk::Int16 useri2() %x\n"), pDatabase->useri2());

    // Adesk::Int16 useri3()
    _print(_T("\tAdesk::Int16 useri3() %x\n"), pDatabase->useri3());

    // Adesk::Int16 useri4()
    _print(_T("\tAdesk::Int16 useri4() %x\n"), pDatabase->useri4());

    // Adesk::Int16 useri5()
    _print(_T("\tAdesk::Int16 useri5() %x\n"), pDatabase->useri5());

    // Adesk::Int16 splinesegs()
    _print(_T("\tAdesk::Int16 splinesegs() %x\n"), pDatabase->splinesegs());

    // Adesk::Int16 surfu()
    _print(_T("\tAdesk::Int16 surfu() %x\n"), pDatabase->surfu());

    // Adesk::Int16 surfv()
    _print(_T("\tAdesk::Int16 surfv() %x\n"), pDatabase->surfv());

    // Adesk::Int16 surftype()
    _print(_T("\tAdesk::Int16 surftype() %x\n"), pDatabase->surftype());

    // Adesk::Int16 surftab1()
    _print(_T("\tAdesk::Int16 surftab1() %x\n"), pDatabase->surftab1());

    // Adesk::Int16 surftab2()
    _print(_T("\tAdesk::Int16 surftab2() %x\n"), pDatabase->surftab2());

    // Adesk::Int16 splinetype()
    _print(_T("\tAdesk::Int16 splinetype() %x\n"), pDatabase->splinetype());

    // Adesk::Int16 shadedge()
    _print(_T("\tAdesk::Int16 shadedge() %x\n"), pDatabase->shadedge());

    // Adesk::Int16 shadedif()
    _print(_T("\tAdesk::Int16 shadedif() %x\n"), pDatabase->shadedif());

    // Adesk::Int16 unitmode()
    _print(_T("\tAdesk::Int16 unitmode() %x\n"), pDatabase->unitmode());

    // Adesk::Int16 maxactvp()
    _print(_T("\tAdesk::Int16 maxactvp() %x\n"), pDatabase->maxactvp());

    // Adesk::Int16 isolines()
    _print(_T("\tAdesk::Int16 isolines() %x\n"), pDatabase->isolines());

    // double ltscale()
    _print(_T("\tdouble ltscale() %g\n"), pDatabase->ltscale());

    // double textsize()
    _print(_T("\tdouble textsize() %g\n"), pDatabase->textsize());

    // double tracewid()
    _print(_T("\tdouble tracewid() %g\n"), pDatabase->tracewid());

    // double sketchinc()
    _print(_T("\tdouble sketchinc() %g\n"), pDatabase->sketchinc());

    // double filletrad()
    _print(_T("\tdouble filletrad() %g\n"), pDatabase->filletrad());

    // double thickness()
    _print(_T("\tdouble thickness() %g\n"), pDatabase->thickness());

    // double angbase()
    _print(_T("\tdouble angbase() %g\n"), pDatabase->angbase());

    // double pdsize()
    _print(_T("\tdouble pdsize() %g\n"), pDatabase->pdsize());

    // double plinewid()
    _print(_T("\tdouble plinewid() %g\n"), pDatabase->plinewid());

    // double userr1()
    _print(_T("\tdouble userr1() %g\n"), pDatabase->userr1());

    // double userr2()
    _print(_T("\tdouble userr2() %g\n"), pDatabase->userr2());

    // double userr3()
    _print(_T("\tdouble userr3() %g\n"), pDatabase->userr3());

    // double userr4()
    _print(_T("\tdouble userr4() %g\n"), pDatabase->userr4());

    // double userr5()
    _print(_T("\tdouble userr5() %g\n"), pDatabase->userr5());

    // double chamfera()
    _print(_T("\tdouble chamfera() %g\n"), pDatabase->chamfera());

    // double chamferb()
    _print(_T("\tdouble chamferb() %g\n"), pDatabase->chamferb());

    // double chamferc()
    _print(_T("\tdouble chamferc() %g\n"), pDatabase->chamferc());

    // double chamferd()
    _print(_T("\tdouble chamferd() %g\n"), pDatabase->chamferd());

    // double facetres()
    _print(_T("\tdouble facetres() %g\n"), pDatabase->facetres());

    // Acad::ErrorStatus menu(char*&)
    pDatabase->getMenu(pStr);
    _print(_T("\tAcad::ErrorStatus getMenu(char*&) %ls\n"), pStr);

    // AcDbDate tdcreate()
    printDate(_T("\tAcDbDate tdcreat()"), pDatabase->tdcreate());

    // AcDbDate tdupdate()
    printDate(_T("\tAcDbDate tdupdate()"), pDatabase->tdupdate());

    // AcDbDate tdindwg()
    printDate(_T("\tAcDbDate tdindwg()"), pDatabase->tdindwg());

    // AcDbDate tdusrtimer()
    printDate(_T("\tAcDbDate tdusrtimer()"), pDatabase->tdusrtimer());

    // AcCmColor cecolor()
    _print(_T("\tAcCmColor cecolor() %x\n"), pDatabase->cecolor().colorIndex());

    // AcDbHandle handseed()
    _print(_T("\tAcDbHandle handseed() %Ix\n"), pDatabase->handseed());

    // AcDbObjectId clayer()
    _print(_T("\tAcDbObjectId clayer() %Ix\n"), pDatabase->clayer());

    // AcDbObjectId textstyle()
    _print(_T("\tAcDbObjectId textstyle() %Ix\n"), pDatabase->textstyle());

    // AcDbObjectId celtype()
    _print(_T("\tAcDbObjectId celtype() %Ix\n"), pDatabase->celtype());

    // AcDbObjectId celtype()
    _print(_T("\tAcDbObjectId cmaterial() %Ix\n"), pDatabase->cmaterial());

    // AcDbObjectId dimstyle()
    _print(_T("\tAcDbObjectId dimstyle() %Ix\n"), pDatabase->dimstyle());

    // AcDbObjectId cmlstyleID()
    _print(_T("\tAcDbObjectId cmlstyleID() %Ix\n"), pDatabase->cmlstyleID());

    // Adesk::Int16 cmljust()
    _print(_T("\tAdesk::Int16 cmljust() %x\n"), pDatabase->cmljust());

    // double cmlscale()
    _print(_T("\tdouble cmlscale() %g\n"), pDatabase->cmlscale());

    // double celtscale()
    _print(_T("\tdouble celtscale() %g\n"), pDatabase->celtscale());

    // Paper space variable access

    // AcGePoint3d pinsbase()
    _print(_T("\tAcGePoint3d pinsbase() %g,%g,%g\n"), 
        pDatabase->pinsbase().x,
        pDatabase->pinsbase().y,
        pDatabase->pinsbase().z);

    // AcGePoint3d pextmin()
    _print(_T("\tAcGePoint3d pextmin() %g,%g,%g\n"), 
        pDatabase->pextmin().x,
        pDatabase->pextmin().y,
        pDatabase->pextmin().z);

    // AcGePoint3d pextmax()
    _print(_T("\tAcGePoint3d pextmax() %g,%g,%g\n"), 
        pDatabase->pextmax().x,
        pDatabase->pextmax().y,
        pDatabase->pextmax().z);

    // AcGePoint2d plimmin()
    _print(_T("\tAcGePoint2d plimmin() %g,%g\n"), 
        pDatabase->plimmin().x,
        pDatabase->plimmin().y);

    // AcGePoint2d plimmax()
    _print(_T("\tAcGePoint2d plimmax() %g,%g\n"), 
        pDatabase->plimmax().x,
        pDatabase->plimmax().y);

    // double pelevation()
    _print(_T("\tdouble pelevation() %g\n"), pDatabase->pelevation());

    // AcGePoint3d pucsorg()
    _print(_T("\tAcGePoint3d pucsorg() %g,%g,%g\n"), 
        pDatabase->pucsorg().x,
        pDatabase->pucsorg().y,
        pDatabase->pucsorg().z);

    // AcGeVector3d pucsxdir()
    _print(_T("\tAcGeVector3d pucsxdir() %g,%g,%g\n"), 
        pDatabase->pucsxdir().x,
        pDatabase->pucsxdir().y,
        pDatabase->pucsxdir().z);

    // AcGeVector3d pucsydir()
    _print(_T("\tAcGeVector3d pucsydir() %g,%g,%g\n"), 
        pDatabase->pucsydir().x,
        pDatabase->pucsydir().y,
        pDatabase->pucsydir().z);

    // AcDbObjectId pucsname()
    _print(_T("\tAcDbObjectId pucsname() %Ix\n"), pDatabase->pucsname());


    // Model space variable access

    // AcGePoint3d insbase()
    _print(_T("\tAcGePoint3d insbase() %g,%g,%g\n"), 
        pDatabase->insbase().x,
        pDatabase->insbase().y,
        pDatabase->insbase().z);

    // AcGePoint3d extmin()
    _print(_T("\tAcGePoint3d extmin() %g,%g,%g\n"), 
        pDatabase->extmin().x,
        pDatabase->extmin().y,
        pDatabase->extmin().z);

    // AcGePoint3d extmax()
    _print(_T("\tAcGePoint3d extmax() %g,%g,%g\n"), 
        pDatabase->extmax().x,
        pDatabase->extmax().y,
        pDatabase->extmax().z);

    // AcGePoint2d limmin()
    _print(_T("\tAcGePoint2d limmin() %g,%g\n"), 
        pDatabase->limmin().x,
        pDatabase->limmin().y);

    // AcGePoint2d limmax()
    _print(_T("\tAcGePoint2d limmax() %g,%g\n"), 
        pDatabase->limmax().x,
        pDatabase->limmax().y);

    // double elevation()
    _print(_T("\tdouble elevation() %g\n"), pDatabase->elevation());

    // AcGePoint3d ucsorg()
    _print(_T("\tAcGePoint3d ucsorg() %g,%g,%g\n"), 
        pDatabase->ucsorg().x,
        pDatabase->ucsorg().y,
        pDatabase->ucsorg().z);

    // AcGeVector3d ucsxdir()
    _print(_T("\tAcGeVector3d ucsxdir() %g,%g,%g\n"), 
        pDatabase->ucsxdir().x,
        pDatabase->ucsxdir().y,
        pDatabase->ucsxdir().z);

    // AcGeVector3d ucsydir()
    _print(_T("\tAcGeVector3d ucsydir() %g,%g,%g\n"), 
        pDatabase->ucsydir().x,
        pDatabase->ucsydir().y,
        pDatabase->ucsydir().z);

    // AcDbObjectId ucsname()
    _print(_T("\tAcDbObjectId ucsname() %Ix\n"), pDatabase->ucsname());


    // Dimvar access

    // Adesk::Boolean dimtol()
    _print(_T("\tAdesk::Boolean dimtol() %x\n"), pDatabase->dimtol());

    // Adesk::Boolean dimlim()
    _print(_T("\tAdesk::Boolean dimlim() %x\n"), pDatabase->dimlim());

    // Adesk::Boolean dimtih()
    _print(_T("\tAdesk::Boolean dimtih() %x\n"), pDatabase->dimtih());

    // Adesk::Boolean dimtoh()
    _print(_T("\tAdesk::Boolean dimtoh() %x\n"), pDatabase->dimtoh());

    // Adesk::Boolean dimse1()
    _print(_T("\tAdesk::Boolean dimse1() %x\n"), pDatabase->dimse1());

    // Adesk::Boolean dimse2()
    _print(_T("\tAdesk::Boolean dimse2() %x\n"), pDatabase->dimse2());

    // Adesk::Boolean dimtad()
    _print(_T("\tAdesk::Boolean dimtad() %x\n"), pDatabase->dimtad());

    // Adesk::Boolean dimalt()
    _print(_T("\tAdesk::Boolean dimalt() %x\n"), pDatabase->dimalt());

    // Adesk::Boolean dimtofl()
    _print(_T("\tAdesk::Boolean dimtofl() %x\n"), pDatabase->dimtofl());

    // Adesk::Boolean dimsah()
    _print(_T("\tAdesk::Boolean dimsah() %x\n"), pDatabase->dimsah());

    // Adesk::Boolean dimtix()
    _print(_T("\tAdesk::Boolean dimtix() %x\n"), pDatabase->dimtix());

    // Adesk::Boolean dimsoxd()
    _print(_T("\tAdesk::Boolean dimsoxd() %x\n"), pDatabase->dimsoxd());

    // Adesk::Boolean dimsd1()
    _print(_T("\tAdesk::Boolean dimsd1() %x\n"), pDatabase->dimsd1());

    // Adesk::Boolean dimsd2()
    _print(_T("\tAdesk::Boolean dimsd2() %x\n"), pDatabase->dimsd2());

    // Adesk::Int8 dimtolj()
    _print(_T("\tAdesk::Int8 dimtolj() %x\n"), pDatabase->dimtolj());

    // int dimaltd()
    _print(_T("\tint dimaltd() %x\n"), pDatabase->dimaltd());

    // int dimzin()
    _print(_T("\tint dimzin() %x\n"), pDatabase->dimzin());

    // int dimjust()
    _print(_T("\tint dimjust() %x\n"), pDatabase->dimjust());

    // int dimtzin()
    _print(_T("\tint dimtzin() %x\n"), pDatabase->dimtzin());

    // int dimaltz()
    _print(_T("\tint dimaltz() %x\n"), pDatabase->dimaltz());

    // int dimalttz()
    _print(_T("\tint dimalttz() %x\n"), pDatabase->dimalttz());

    // int dimunit()
    _print(_T("\tint dimunit() %x\n"), pDatabase->dimunit());

    // int dimdec()
    _print(_T("\tint dimdec() %x\n"), pDatabase->dimdec());

    // int dimtdec()
    _print(_T("\tint dimtdec() %x\n"), pDatabase->dimtdec());

    // int dimaltu()
    _print(_T("\tint dimaltu() %x\n"), pDatabase->dimaltu());

    // int dimalttd()
    _print(_T("\tint dimalttd() %x\n"), pDatabase->dimalttd());

    // AcDbObjectId dimtxsty()
    _print(_T("\tAcDbObjectId dimtxsty() %Ix\n"), pDatabase->dimtxsty());

    // double dimscale()
    _print(_T("\tdouble dimscale() %g\n"), pDatabase->dimscale());

    // double dimasz()
    _print(_T("\tdouble dimasz() %g\n"), pDatabase->dimasz());

    // double dimexo()
    _print(_T("\tdouble dimexo() %g\n"), pDatabase->dimexo());

    // double dimdli()
    _print(_T("\tdouble dimdli() %g\n"), pDatabase->dimdli());

    // double dimexe()
    _print(_T("\tdouble dimexe() %g\n"), pDatabase->dimexe());

    // double dimrnd()
    _print(_T("\tdouble dimrnd() %g\n"), pDatabase->dimrnd());

    // double dimdle()
    _print(_T("\tdouble dimdle() %g\n"), pDatabase->dimdle());

    // double dimtp()
    _print(_T("\tdouble dimtp() %g\n"), pDatabase->dimtp());

    // double dimtm()
    _print(_T("\tdouble dimtm() %g\n"), pDatabase->dimtm());

    // double dimtxt()
    _print(_T("\tdouble dimtxt() %g\n"), pDatabase->dimtxt());

    // double dimcen()
    _print(_T("\tdouble dimcen() %g\n"), pDatabase->dimcen());

    // double dimtsz()
    _print(_T("\tdouble dimtsz() %g\n"), pDatabase->dimtsz());

    // double dimaltf()
    _print(_T("\tdouble dimaltf() %g\n"), pDatabase->dimaltf());

    // double dimlfac()
    _print(_T("\tdouble dimlfac() %g\n"), pDatabase->dimlfac());

    // double dimtvp()
    _print(_T("\tdouble dimtvp() %g\n"), pDatabase->dimtvp());

    // double dimtfac()
    _print(_T("\tdouble dimtfac() %g\n"), pDatabase->dimtfac());

    // double dimgap()
    _print(_T("\tdouble dimgap() %g\n"), pDatabase->dimgap());

    // Acad::ErrorStatus getDimpost(char*& pOutput)
    pDatabase->getDimpost(pStr);
    _print(_T("\tAcad::ErrorStatus getDimpost(char*& pOutput) %ls\n"), pStr);

    // Acad::ErrorStatus getDimapost(char*& pOutput)
    pDatabase->getDimapost(pStr);
    _print(_T("\tAcad::ErrorStatus getDimapost(char*& pOutput) %ls\n"), pStr);

    // Acad::ErrorStatus getDimblk(char*& pOutput)
    pDatabase->getDimblk(pStr);
    _print(_T("\tAcad::ErrorStatus getDimblk(char*& pOutput) %ls\n"), pStr);

    // Acad::ErrorStatus getDimblk1(char*& pOutput)
    pDatabase->getDimblk1(pStr);
    _print(_T("\tAcad::ErrorStatus getDimblk1(char*& pOutput) %ls\n"), pStr);

    // Acad::ErrorStatus getDimblk2(char*& pOutput)
    pDatabase->getDimblk2(pStr);
    _print(_T("\tAcad::ErrorStatus getDimblk2(char*& pOutput) %ls\n"), pStr);

    // AcCmColor dimclrd()
    _print(_T("\tAcCmColor dimclrd() %d\n"), pDatabase->dimclrd().colorIndex());

    // AcCmColor dimclre()
    _print(_T("\tAcCmColor dimclre() %d\n"), pDatabase->dimclre().colorIndex());

    // AcCmColor dimclrt()
    _print(_T("\tAcCmColor dimclrt() %d\n"), pDatabase->dimclrt().colorIndex());
}
