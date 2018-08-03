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
// Name:        DIR.CC
//
// Remarks:     None
//
//////////////////////////////////////////////////////////////////////////////


#include "windows.h"
#include <direct.h>
#include <malloc.h>
#include <stdio.h>
#include <io.h>
#include "dir.h"
#include "tchar.h"

/* build_dir
 * builds a list of files in the dta buffer format. this is used by
 * all of the built-in file-oriented functions for wildcard operations.
 * the directory list must be freed by whoever calls build_dir after
 * they are done with the directory list.
 */


static int sort = BY_NAME;
#define    SEARCHALL    _T(/*MSG0*/"*.*")

#define findfirst(a,b,c) _tfindfirst(a,c)
#define findnext(a,b) _tfindnext(a,b)
#define findclose(a) _findclose(a)

#ifdef _DEBUG
//#define new DEBUG_NEW
#endif

DirEnum::DirEnum()
{
    pParasite = 0;
    recurse = FALSE;
    keepGoing = 0;
    didOne = FALSE;
}

int DirEnum::enumPath(const TCHAR *FilePath)
{
TCHAR path_spec[256];
TCHAR root_path[256];
TCHAR file_spec[256];
TCHAR *ptr;
OFSTRUCT of;

    pParasite->fileStart();
    keepGoing = TRUE;
    char szTemp[MAX_PATH] = "";
#ifndef UNICODE
    strcpy(szTemp,FilePath);
#else
    WideCharToMultiByte(CP_ACP, 0, FilePath, -1, szTemp, MAX_PATH, NULL, NULL);
#endif
    if (OpenFile (szTemp, &of, OF_PARSE) >= 0)
    {
        TCHAR szTemp[MAX_PATH] = _T("");
#ifndef UNICODE
        strcpy(szTemp,of.szPathName);
#else       
        MultiByteToWideChar(CP_ACP, 0, of.szPathName, -1, szTemp, MAX_PATH);
#endif      
        _tcscpy (path_spec, szTemp);
    }
    else
    {
        return 0;
    }

    _tcscpy (root_path, path_spec);

    ptr = root_path;
    while (*ptr) ptr++;
    while (ptr >= root_path && *ptr != _T('\\') && *ptr != _T(':')) ptr--;
    ptr++;
    _tcscpy (file_spec, ptr);
    *ptr = 0;
    if (root_path[0] == 0)
    {
        _tcscpy (root_path, _T(/*MSG0*/".\\"));
    }
    do_dir_2 (path_spec, _countof(path_spec), root_path, file_spec);
    pParasite->fileEnd();
    return didOne;
}


void DirEnum::do_dir_2 (TCHAR *dir_path, size_t nDirPath,TCHAR *root_path, TCHAR *file_spec)
{
PDIR pDirHead;
PDIR pFileHead;
PDIR pDir;
TCHAR *ptr;
size_t nPtr = 0;
TCHAR temp_root_path[256];

    if (pFileHead = build_dir(dir_path, ALL_FILES))
    {
        pDir = pFileHead;
        pParasite->fileDir(root_path);
        while (pDir != NULL && keepGoing)
        {
            didOne = TRUE;
            keepGoing = pParasite->fileCb(root_path, pDir);
            pDir = pDir->next_dir;
        }
        free_dir (pFileHead);
    }
    if (recurse && keepGoing)
    {
        _tcscpy_s(dir_path, nDirPath, root_path);
        _tcscat_s(dir_path, nDirPath, SEARCHALL);
        pDirHead = build_dir(dir_path, _A_SUBDIR);
        pDir = pDirHead;
        while (pDir != NULL && keepGoing)
        {
            if (pDir->attrib & _A_SUBDIR)
            {
                _tcscpy_s(dir_path, nDirPath, root_path);
                ptr = dir_path;
                nPtr = nDirPath;
                while (*ptr)
                {
                    ptr++;
                    nPtr --;
                }
                while (ptr > dir_path && *ptr != _T('\\') && *ptr != _T(':')) 
                {
                    ptr--;
                    nPtr++;
                }
                ptr++;
                nPtr--;
                _tcscpy_s (ptr, nPtr, pDir->name);
                while (*ptr)
                {
                    ptr++;
                    nPtr--;
                }
                _tcscpy_s(ptr, nPtr, file_spec);
                if ((_tcscmp (pDir->name, _T(/*MSG0*/".\\")) != 0) &&
                    (_tcscmp (pDir->name, _T(/*MSG0*/"..\\")) != 0))
                {
                    _tcscpy (temp_root_path, root_path);
                    ptr = temp_root_path;
                    nPtr = _countof(temp_root_path);
                    while (*ptr) ptr++;
                    _tcscpy_s(ptr, nPtr, pDir->name);
                    do_dir_2(dir_path, nDirPath, temp_root_path, file_spec);
                }
            }
            pDir = (DIR *)pDir->next_dir;
        }
        free_dir (pDirHead);
    }
}

DIR *DirEnum::build_dir(TCHAR *dir_path, int attrib)
{
DIR *dir_head;
DIR *dir_tail;
intptr_t code;
#ifndef UNICODE
struct _finddata_t File_T;
#else
struct _wfinddata_t File_T;
#endif

    if ((dir_head = (DIR *)new char [sizeof(DIR)]) == NULL)
    {
        return FALSE;
    }
    dir_head->prev_dir = NULL;
    dir_head->next_dir = NULL;

    if ((code = findfirst (dir_path, attrib, &File_T)) > 0)
    {
        dir_head->size = File_T.size;
        dir_head->time = File_T.time_write;
        dir_head->attrib = File_T.attrib;
        _tcscpy (dir_head->name, File_T.name);
        if (File_T.attrib & _A_SUBDIR)
            _tcscat(dir_head->name, _T(/*MSG0*/"\\"));

        if ((dir_tail = (DIR *)new char[ sizeof (DIR)]) == NULL)
        {
            delete dir_head;
            return FALSE;
        }
        dir_head->next_dir = dir_tail;
        dir_tail->next_dir = NULL;
        dir_tail->prev_dir = dir_head;
        while (!findnext (code, &File_T))
        {
            dir_tail->size = File_T.size;
            dir_tail->time = File_T.time_write;
            dir_tail->attrib = File_T.attrib;
            _tcscpy (dir_tail->name, File_T.name);
            if (File_T.attrib & _A_SUBDIR)
                _tcscat (dir_tail->name, _T(/*MSG0*/"\\"));

            if ((dir_tail->next_dir = (DIR *)new char[sizeof (DIR)]) == NULL)
            {
                free_dir (dir_head);
                return FALSE;
            }
            ((DIR *)dir_tail->next_dir)->prev_dir = dir_tail;
            dir_tail = (DIR *)dir_tail->next_dir;
            dir_tail->next_dir = NULL;
        }
        ((DIR *)dir_tail->prev_dir)->next_dir = NULL;
        delete dir_tail;
        findclose(code);
    }
    else
    {
        free_dir (dir_head);
        return FALSE;
    }

    dir_head = dir_sort (dir_head);
    return dir_head;
}

/* free_dir
 * frees a directory list.
 */

void DirEnum::free_dir (DIR *dir_head)
{
DIR *temp;

    while (dir_head != NULL)
    {
        temp = (DIR *)dir_head->next_dir;
        delete dir_head;
        dir_head = temp;
    }
}

/* dir_sort
 * alphabetizes a directory list.
 */

DIR *DirEnum::dir_sort (DIR *dir_head)
{
PDIR pDir;
PDIR dir_a;
PDIR dir_b;
PDIR dir_c;
PDIR dir_d;
char not_sorted = TRUE;
int test = 0;
char found_file;

    while (not_sorted)
    {
        pDir = dir_head;
        not_sorted = FALSE;
        found_file = FALSE;
        while (pDir->next_dir != NULL)
        {
            switch (sort)
            {
            case BY_NAME:
                test = _tcscmp (pDir->name, pDir->next_dir->name);
                break;
            case BY_SIZE:
                if (pDir->size != pDir->next_dir->size)
                    test = pDir->size > pDir->next_dir->size;
                else
                    test = _tcscmp (pDir->name, pDir->next_dir->name);
                break;
            case BY_DATE:
                test = pDir->time > pDir->next_dir->time;
                break;
            }
            if (test > 0)
            {
                dir_a = (DIR *)pDir->prev_dir;
                dir_b = pDir;
                dir_c = (DIR *)pDir->next_dir;
                dir_d = (DIR *)dir_c->next_dir;
                if (dir_b == dir_head)
                    dir_head = dir_c;
                pDir = dir_c;
                dir_c->next_dir = dir_b;
                dir_c->prev_dir = dir_a;
                dir_b->next_dir = dir_d;
                dir_b->prev_dir = dir_c;
                if (dir_a != NULL)
                    dir_a->next_dir = dir_c;
                if (dir_d != NULL)
                    dir_d->prev_dir = dir_b;
                not_sorted = TRUE;
            }
            pDir = (DIR *)pDir->next_dir;
        }
    }
    return dir_head;
}

void DirEnum::set_sort(int type)
{
    sort = type;
}

void DirEnum::setParasite(DirParasite *pparasite)
{
    pParasite = pparasite;
}

DirParasite::DirParasite(DirEnum &de)
{
    de.setParasite(this);
}

DirParasite::~DirParasite()
{
}

int DirParasite::fileCb(const TCHAR *path, DIR *pDir)
{
    return TRUE;
}

int DirParasite::fileStart()
{
    return TRUE;
}

int DirParasite::fileEnd()
{
    return TRUE;
}

int DirParasite::fileDir(const TCHAR *path)
{
    return TRUE;
}
