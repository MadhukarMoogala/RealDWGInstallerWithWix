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
// Name:        DIR.H
//
// Remarks:     None
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _DIR_H
#define _DIR_H

#include "io.h"

typedef struct _DIR DIR, *PDIR;

typedef struct _DIR {
    TCHAR name[_MAX_PATH];
    time_t time;
    unsigned int attrib;
    long size;
    PDIR next_dir;
    PDIR prev_dir;
} DIR, *PDIR;

enum { BY_NAME, BY_DATE, BY_SIZE };

#define _A_VOLID    0x08

#define ALL_FILES (_A_SYSTEM|_A_HIDDEN|_A_SUBDIR|_A_RDONLY|_A_NORMAL|_A_VOLID)

class DirEnum;

class DirParasite
{
public:
    DirParasite(DirEnum &de);
    virtual ~DirParasite();
    virtual int fileCb(const TCHAR *path, DIR *pDir);
    virtual int fileDir(const TCHAR *path);
    virtual int fileStart();
    virtual int fileEnd();
private:
};

class DirEnum
{
public:
    DirEnum();
    void setParasite(DirParasite *psarasite);
    int enumPath(const TCHAR *path);
private:
    void set_sort(int type);
    DIR *dir_sort(DIR *dir_head);
    void free_dir (DIR *dir_head);
    DIR *build_dir(TCHAR *dir_path, int attrib);
    void do_dir_2(TCHAR *dir_path, size_t nDirPath, TCHAR *root_path, TCHAR *file_spec);

    DirParasite *pParasite;
    int recurse;
    int keepGoing;
    int didOne;
};

#endif // _DIR_H
