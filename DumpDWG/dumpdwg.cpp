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
// DUMPDWG.CPP
//
// DESCRIPTION:
//
// This console application prints the values of entity data
// members.  The 's' version is linked with acisstub.dll.
// It uses the dumpcore.cpp module, like many of these samples,
// for walking through the DWG file. 

#include "afxwin.h"
#include "shlwapi.h"
#include "urlmon.h"
#include "Wininet.h"
#include "adesk.h"
#include "dbsymtb.h"
#include "dbents.h"
#include "dbelipse.h"
#include "dbspline.h"
#include "dblead.h"
#include "dbray.h"
#include "dbxline.h"
#include "dbmline.h"
#include "dbbody.h"
#include "dbregion.h"
#include "dbsol3d.h"
#include "acgi.h"
#include "myacgi.h"
#include "acestext.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"
#include "dbmstyle.h"
#include "rxregsvc.h"

#include "dbapserv.h"
#include "dir.h"
#include "acdbxref.h"
#include "dbacis.h"

#define PI ((double)3.14159265358979323846)
#define PIOVER180 ((double)PI/180)

extern "C" Acad::ErrorStatus acdbGetDimBlock(AcDbDimension *pDim, 
    AcDbObjectId& id);

void usage();
void readDrawing(const TCHAR *pFileName);
void dumpDatabase(AcDbDatabase *pDatabase);
void saveDwg(AcDbDatabase *pDb);
void saveAsR2000Dwg(AcDbDatabase *pDb);

void entInfo(AcDbEntity *pEnt, int size);
void dumpEntInfo();
void dumpDwgHeaderVars(AcDbDatabase *pDatabase);
void deleteEntMap();

int Verbose = 0;
int Copy = 0;
int Header = 0;
int Tables = 0;
int doExtents = 0;
int showErased = 0;
int resolveXrefs = 0;
bool bCorruptDrawing = false;

static const TCHAR acIsmObjDbxFile[] = _T("acIsmObj23.dbx");

class DumpDwgHostApp : public AcDbHostApplicationServices
{
public:
    ~DumpDwgHostApp();
    Acad::ErrorStatus findFile(TCHAR* pcFullPathOut, int nBufferLength,
                         const TCHAR* pcFilename, AcDbDatabase* pDb = NULL,
                         AcDbHostApplicationServices::FindFileHint = kDefault);
    // These two functions return the full path to the root folder where roamable/local 
    // customizable files were installed. Note that the user may have reconfigured 
    // the location of some the customizable files using the Options Dialog 
    // therefore these functions should not be used to locate customizable files. 
    // To locate customizable files either use the findFile function or the 
    // appropriate system variable for the given file type. 
    //
    Acad::ErrorStatus getRoamableRootFolder(const TCHAR*& folder);
    Acad::ErrorStatus getLocalRootFolder(const TCHAR*& folder);

    //URL related services
    Adesk::Boolean isURL(const TCHAR* pszURL) const override;
    bool isRemoteFile(const TCHAR* pszLocalFile, TCHAR* pszURL, size_t nUrlSize) const override;
    Acad::ErrorStatus  getRemoteFile(const TCHAR* pszURL, TCHAR* pszLocalFile,
                                     size_t nFilePath, bool bIgnoreCache) const override;

    // make sure you implement getAlternateFontName. In case your findFile implementation
    // fails to find a font you should return a font name here that is guaranteed to exist.
    virtual const TCHAR * getAlternateFontName() const override
    {
        return _T("txt.shx"); //findFile will be called again with this name
    }

    bool notifyCorruptDrawingFoundOnOpen(AcDbObjectId id, Acad::ErrorStatus es);
#ifdef REALDWGMSI //for AutoCAD Realdwg test install only
    const ACHAR * getMachineRegistryProductRootKey ();
#endif

private:
    mutable CMapStringToString m_localToUrl;
};

#ifdef REALDWGMSI //for AutoCAD Realdwg test install only
CString registryRoot = _T(/*MSG0*/"Software\\ADNCAD\\R23.0"); 
const ACHAR * DumpDwgHostApp::getMachineRegistryProductRootKey ()
{
    return registryRoot;
}
#endif

DumpDwgHostApp::~DumpDwgHostApp()
{
    CString local,url;
    for (POSITION pos = m_localToUrl.GetStartPosition();pos!=NULL;)
    {
        m_localToUrl.GetNextAssoc(pos,local,url);
        DeleteUrlCacheEntry(url);
    }
}
// Return the Install directory for customizable files
Acad::ErrorStatus 
DumpDwgHostApp::getRoamableRootFolder(const TCHAR*& folder)
{
    Acad::ErrorStatus ret = Acad::eOk;
    static TCHAR buf[MAX_PATH] = _T("\0"); //MDI SAFE
    if (buf[0]==0)
        if (GetModuleFileName(NULL, buf, MAX_PATH) != 0)
            ret = Acad::eRegistryAccessError;
    folder = buf;
    return ret;
}

// Return the Install directory for customizable files
Acad::ErrorStatus 
DumpDwgHostApp::getLocalRootFolder(const TCHAR*& folder)
{
    Acad::ErrorStatus ret = Acad::eOk;
    static TCHAR buf[MAX_PATH] = _T("\0"); //MDI SAFE
    if (buf[0]==0)
        if (GetModuleFileName(NULL, buf, MAX_PATH) != 0)
            ret = Acad::eRegistryAccessError;
    folder = buf;
    return ret;
}

Acad::ErrorStatus 
DumpDwgHostApp::findFile(TCHAR* pcFullPathOut, int nBufferLength,
    const TCHAR* pcFilename, AcDbDatabase* pDb, 
    AcDbHostApplicationServices::FindFileHint hint)
{
    TCHAR pExtension[5];
    switch (hint)
    {
        case kCompiledShapeFile:
            _tcscpy(pExtension, _T(".shx"));
            break;
        case kTrueTypeFontFile:
            _tcscpy(pExtension, _T(".ttf"));
            break;
        case kPatternFile:
            _tcscpy(pExtension, _T(".pat"));
            break;
        case kARXApplication:
            _tcscpy(pExtension, _T(".dbx"));
            break;
        case kFontMapFile:
            _tcscpy(pExtension, _T(".fmp"));
            break;
        case kXRefDrawing:
            _tcscpy(pExtension, _T(".dwg"));
            break;
        case kFontFile:                // Fall through.  These could have
        case kEmbeddedImageFile:       // various extensions
        default:
            pExtension[0] = _T('\0');
            break;
    }
    TCHAR* filePart;
    DWORD result;
    result = SearchPath(NULL, pcFilename, pExtension, nBufferLength, 
                        pcFullPathOut, &filePart);
    if (result && result < (DWORD)nBufferLength)
        return Acad::eOk;
    else
        return Acad::eFileNotFound;
}
Adesk::Boolean 
DumpDwgHostApp::isURL(const TCHAR* pszURL) const
{
    return !!PathIsURL(pszURL);
}

bool
DumpDwgHostApp::isRemoteFile(const TCHAR* pszLocalFile, TCHAR* pszURL, size_t nUrlSize) const
{
    if (pszURL == nullptr || nUrlSize < 1)
        return false;
    pszURL[0] = L'\0';  // assume error
    CString value;
    if (m_localToUrl.Lookup(pszLocalFile,value))
    {
        // may throw exception if string doesn't fit in dest buffer
        _tcscpy_s(pszURL, nUrlSize, value);
        return TRUE;
    }
    return FALSE;
}

Acad::ErrorStatus  
DumpDwgHostApp::getRemoteFile(const TCHAR* pszURL, TCHAR* pszLocalFile, size_t nFilePath,
                              bool bIgnoreCache) const
{
    if (pszLocalFile == nullptr || nFilePath < 1)
        return Acad::eInvalidInput;
    pszLocalFile[0] = L'\0';    // assume error
    DWORD err = ERROR_FILE_NOT_FOUND;
    if (!bIgnoreCache)
    {
        DWORD size = 0;
        if (GetUrlCacheEntryInfo(pszURL,NULL,&size))
            return Acad::eInetFileGenericError; //this shouldn't succeed
        err = GetLastError();
        if (err == ERROR_INSUFFICIENT_BUFFER)
        {
            INTERNET_CACHE_ENTRY_INFO* pCacheEntry = (INTERNET_CACHE_ENTRY_INFO*)malloc(size);
            if (GetUrlCacheEntryInfo(pszURL,pCacheEntry,&size))
            {
                // may throw exception if string doesn't fit in dest buffer
                _tcscpy_s(pszLocalFile, nFilePath, pCacheEntry->lpszLocalFileName);
                m_localToUrl.SetAt(pszLocalFile,pszURL);
                free(pCacheEntry);
                return Acad::eInetOk;
            }
            err = GetLastError();
        }
    }
    if (err == ERROR_FILE_NOT_FOUND)
    {
        if (SUCCEEDED(URLDownloadToCacheFile(nullptr, pszURL, pszLocalFile,
                                             static_cast<DWORD>(nFilePath), 0, nullptr)))
        {
            m_localToUrl.SetAt(pszLocalFile,pszURL);
            return Acad::eInetOk;
        }
    }
    return Acad::eInetFileGenericError;
}

bool DumpDwgHostApp::notifyCorruptDrawingFoundOnOpen(AcDbObjectId id, Acad::ErrorStatus es)
{
    _tprintf(_T("Corrupt drawing detected!\n\n"));
    bCorruptDrawing = true;

    // Ignore the error for now and don't shut down.  You may choose to
    // continue using this drawing or not after readDwgFile returns (the
    // return code can still be eOk)
    return true;
}

class myDirParasite : public DirParasite
{
public:
    myDirParasite(DirEnum &de);
    virtual int fileCb(const TCHAR *path, DIR *pDir);
private:
};

DumpDwgHostApp gDumpDwgHostApp;

int _tmain(int argc, TCHAR *argv[])
{
    acdbSetHostApplicationServices(&gDumpDwgHostApp);
    acdbValidateSetup(AcLocale(L"en", L"us"));

    int i;

    // Check command line options.
    for (i = 1; i < argc; i++)
    {
        TCHAR *p;
        if (argv[i][0] == _T('-') || argv[i][0] == _T('/'))
        {
            p = argv[i] + 1;

            if ( _tcsicmp(p, _T("v")) == 0 )
                Verbose = 1;
            else if ( _tcsicmp(p, _T("vv")) == 0 )
                Verbose = 2;
            else if ( _tcsicmp(p, _T("h")) == 0 )
                Header = 1;
            else if ( _tcsicmp(p, _T("c")) == 0 )
                Copy = 1;
            else if ( _tcsicmp(p, _T("cr2000")) == 0 )
                Copy = 4; // Save as R2000 dwg file.
            else if ( _tcsicmp(p, _T("t")) == 0 )
                Tables = 1;
            else if ( _tcsicmp(p, _T("x")) == 0 )
                doExtents = 1;
            else if ( _tcsicmp(p, _T("e")) == 0 )
                showErased = 1;
            else if ( _tcsicmp(p, _T("r")) == 0 )
                resolveXrefs = 1;
            else if ( _tcsicmp(p, _T("?")) == 0 )
                usage();
            else
                usage();
            // The first character of the options will be set to NULL.
            argv[i][0] = 0;
        }
    }

    // Load the ISM dll
    acrxLoadModule(acIsmObjDbxFile, 0);

    int atLeastOneArg = FALSE;
    for (i = 1; i < argc; i++)
    {
        // Check other command line arguments.
        if (argv[i][0])
        {
            if (acdbHostApplicationServices()->isURL(argv[i]))
            {
                TCHAR szLocalFile[MAX_PATH];
                Acad::ErrorStatus es = acdbHostApplicationServices()->getRemoteFile(argv[i],
                                                        szLocalFile);
                if ((Acad::eInetOk == es || Acad::eInetInCache == es))
                    readDrawing(szLocalFile);
                else
                    _tprintf(_T("Not Found or Cannot be Downloaded: %ls\n"), argv[i]);
            }
            else
            {
                DirEnum de;
                myDirParasite dp(de);
                if (!de.enumPath(argv[i]))
                    _tprintf(_T("Not Found: %ls\n"), argv[i]);
            }
            atLeastOneArg = TRUE;
        }
    }
    if (!atLeastOneArg)
        usage();

    if (acdbIsModelerStarted())
        acdbModelerEnd(); 
    // Unload the ISM dll
    acrxUnloadModule(acIsmObjDbxFile);

    acdbCleanUp();
    return 0;
}

myDirParasite::myDirParasite(DirEnum &de)
    : DirParasite(de)
{
}

int myDirParasite::fileCb(const TCHAR *path, DIR *pDir)
{
    // Get the drawing name with path.
    if (!(pDir->attrib & _A_SUBDIR))
    {
        TCHAR str[_MAX_PATH];
        _tcscpy(str, path);
        _tcscat(str, pDir->name);
        readDrawing(str);
    }
    return TRUE;
}

void usage()
{
    _tprintf(_T("Usage: dumpdwg <-chtver> file.dwg\n"));
    _tprintf(_T("            default     print symbol tables\n"));
    _tprintf(_T("            -h          print header vars\n"));
    _tprintf(_T("            -v          verbose: print entities\n"));
    _tprintf(_T("            -vv         really verbose: print entity members\n"));
    _tprintf(_T("            -c          copy\n"));
    _tprintf(_T("            -cr2000     save as a R2000 dwg file\n"));
    _tprintf(_T("            -t          dump tables (viewport, dim, etc)\n"));
    _tprintf(_T("            -e          dump erased entities\n"));
    _tprintf(_T("            -r          resolve xrefs\n"));
    _tprintf(_T("            -?          this message\n"));
    exit(0);
}

extern "C" void checkFileHandles();

void readDrawing(const TCHAR *pFileName)
{
    AcDbDatabase *pDb = new AcDbDatabase(Adesk::kFalse);
    if (pDb == NULL)
        return;

    _tprintf(_T("%ls\n"), pFileName);

    TCHAR drive[_MAX_DRIVE] = _T("");
    TCHAR dir[_MAX_DIR] = _T("");
    TCHAR fname[_MAX_FNAME] = _T("");
    TCHAR ext[_MAX_EXT] = _T("");

    _tsplitpath_s(pFileName, drive, dir, fname, ext);

    if (_tcsicmp(ext, _T(".dwg")) && _tcscmp(ext, _T(".dxf")))
    {
        _tprintf(_T("Can't open %ls\n"), pFileName);
        _tprintf(_T("File must have .dwg or .dxf extension\n"));
        delete pDb;
        return;
    }

    // Read the drawing  or dxf file.
    if ((_tcsicmp(ext, _T(".dwg")) ? pDb->dxfIn(pFileName) 
                            : pDb->readDwgFile(pFileName)) == Acad::eOk
        /*&& !bCorruptDrawing*/)
    {
        assert(Copy != 2);   // no more saveasr13 support
        assert(Copy != 3);   // no more saveasr14 support
        acdbHostApplicationServices()->setWorkingDatabase(pDb);
        if (resolveXrefs)
            acdbResolveCurrentXRefs(pDb);
        dumpDatabase(pDb);
        if (Copy == 1)
            saveDwg(pDb);
        else if (Copy == 4)
        {
            pDb->closeInput(true);
            saveAsR2000Dwg(pDb);
        }
        dumpEntInfo();
    }
    else
        _tprintf(_T("Can't open %ls\n"), pFileName);

    if (resolveXrefs)
        acdbReleaseHostDwg(pDb);
    else
        delete pDb;

    deleteEntMap();
    _tprintf(_T("\n"));
}

void saveDwg(AcDbDatabase* pDb)
{
    static int nSaveFile = 0;

    TCHAR str[_MAX_PATH];
    _stprintf(str, _T("save%d.dwg"), nSaveFile++);
    _tremove(str);
    _tprintf(_T("    copy to %ls\n"), str);
    pDb->setSaveproxygraphics(AcDb::kNeverSave);
    // Save the file.
    if (pDb->saveAs(str) == Acad::eOk)
        _tprintf(_T("    wrote %ls\n"), str);
    else
        _tprintf(_T("    Could not write %ls\n"), str);
}


void saveAsR2000Dwg(AcDbDatabase* pDb)
{
    static int nSaveFile = 0;

    TCHAR str[_MAX_PATH];
    _stprintf(str, _T("r2000Save%d.dwg"), nSaveFile++);
    _tremove(str);
    _tprintf(_T("    copy to %ls\n"), str);
    pDb->setSaveproxygraphics(AcDb::kNeverSave);
    // Save the file.
    if (acdbSaveAs2000(pDb, str) == Acad::eOk)
        _tprintf(_T("    wrote %ls\n"), str);
    else
        _tprintf(_T("    Could not write %ls\n"), str);
}


void _print(TCHAR *format, ...)
{
    va_list marker;
    va_start(marker, format);
    _vtprintf(format, marker);
    va_end(marker);
}
