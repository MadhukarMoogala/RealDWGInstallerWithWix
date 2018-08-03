#ifndef STDARX_HH
#define STDARX_HH 1
#ifndef __AFX_H__		// MFC modules won't like this
//
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
// The minimum stdafx.h needed to get MFC's mapping dictionaries over to DOS
// and SUNOS.
// Get rid of this file when MFC is eventually ported to these environments
// 06-10-94, 10:46, Fri, Thuan-Tit Ewe


#ifdef _DEBUG
#undef _DEBUG
#endif


// We defined 5 objects which are redifined to not conflict to MFC.

#define CObject		profile_CObject
#define CMapPtrToWord	profile_CMapPtrToWord
#define CMapWordToPtr	profile_CMapWordToPtr
#define CMapPtrToPtr	profile_CMapPtrToPtr
#define CPlex		profile_CPlex
#define CMapStringToOb	profile_CMapStringToOb
#define afxEmptyString	profile_afxEmptyString


// The hashtablesize has been bumped up to increase performance.

#define RX_HASHTABLESIZE	1009

#define AfxIsValidAddress(x, y)		(1)

class CObject {
  };

#define IMPLEMENT_DYNAMIC(x,y)
#define IMPLEMENT_SERIAL(x, y, z)
#define DECLARE_DYNAMIC(x)
#define DECLARE_SERIAL(x)

#define RXPOSITION	void *	

#define BEFORE_START_RXPOSITION		((void *)-1)


#define _AFXCOLL_INLINE inline

#ifndef FAR
#define FAR
#endif

// Inclusion of standard headers
#include <memory.h>



// Temporary defines !!!!
// ASSERT warning, to nothing for now
// #define ASSERT(x)
#define ASSERT_VALID(x)

#define DEBUG_NEW	new



// Class definitions below

class CMapPtrToWord : public CObject
{

	DECLARE_DYNAMIC(CMapPtrToWord)
public:
	// Association
	struct CAssoc
	{
		CAssoc* pNext;
		UINT_PTR nHashValue;  // needed for efficient iteration
		void* key;
		WORD value;
	};
public:

// Construction
	CMapPtrToWord(int nBlockSize=10);

// Attributes
	// number of elements
	int GetCount() const;
	BOOL IsEmpty() const;

	// Lookup
	BOOL Lookup(void* key, WORD& rValue) const;

// Operations
	// Lookup and add if not there
	WORD& operator[](void* key);

	// add a new (key, value) pair
	void SetAt(void* key, WORD newValue);

	// removing existing (key, ?) pair
	BOOL RemoveKey(void* key);
	void RemoveAll();

	// iterating all (key, value) pairs
	RXPOSITION GetStartPosition() const;
	void GetNextAssoc(RXPOSITION& rNextPosition, void*& rKey, WORD& rValue) const;

	// advanced features for derived classes
	UINT_PTR GetHashTableSize() const;
	void InitHashTable(UINT_PTR hashSize);

// Overridables: special non-virtual (see map implementation for details)
	// Routine used to user-provided hash keys
	UINT_PTR HashKey(void* key) const;

// Implementation
protected:
	CAssoc** m_pHashTable;
	UINT_PTR m_nHashTableSize;
	int m_nCount;
	CAssoc* m_pFreeList;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CAssoc* NewAssoc();
	void FreeAssoc(CAssoc*);
	CAssoc* GetAssocAt(void*, UINT_PTR&) const;

public:
	~CMapPtrToWord();
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
};
////////////////////////////////////////////////////////////////////////////

_AFXCOLL_INLINE int CMapPtrToWord::GetCount() const
	{ return m_nCount; }
_AFXCOLL_INLINE BOOL CMapPtrToWord::IsEmpty() const
	{ return m_nCount == 0; }
_AFXCOLL_INLINE void CMapPtrToWord::SetAt(void* key, WORD newValue)
	{ (*this)[key] = newValue; }
_AFXCOLL_INLINE RXPOSITION CMapPtrToWord::GetStartPosition() const
	{ return (m_nCount == 0) ? NULL : BEFORE_START_RXPOSITION; }
_AFXCOLL_INLINE UINT_PTR CMapPtrToWord::GetHashTableSize() const
	{ return m_nHashTableSize; }



/////////////////////////////////////////////////////////////////////////////

class CMapWordToPtr : public CObject
{

	DECLARE_DYNAMIC(CMapWordToPtr)
public:
	// Association
	struct CAssoc
	{
		CAssoc* pNext;
		UINT_PTR nHashValue;  // needed for efficient iteration
		WORD key;
		void* value;
	};
public:

// Construction
	CMapWordToPtr(int nBlockSize=10);

// Attributes
	// number of elements
	int GetCount() const;
	BOOL IsEmpty() const;

	// Lookup
	BOOL Lookup(WORD key, void*& rValue) const;

// Operations
	// Lookup and add if not there
	void*& operator[](WORD key);

	// add a new (key, value) pair
	void SetAt(WORD key, void* newValue);

	// removing existing (key, ?) pair
	BOOL RemoveKey(WORD key);
	void RemoveAll();

	// iterating all (key, value) pairs
	RXPOSITION GetStartPosition() const;
	void GetNextAssoc(RXPOSITION& rNextPosition, WORD& rKey, void*& rValue) const;

	// advanced features for derived classes
	UINT_PTR GetHashTableSize() const;
	void InitHashTable(UINT_PTR hashSize);

// Overridables: special non-virtual (see map implementation for details)
	// Routine used to user-provided hash keys
	UINT_PTR HashKey(WORD key) const;

// Implementation
protected:
	CAssoc** m_pHashTable;
	UINT_PTR m_nHashTableSize;
	int m_nCount;
	CAssoc* m_pFreeList;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CAssoc* NewAssoc();
	void FreeAssoc(CAssoc*);
	CAssoc* GetAssocAt(WORD, UINT_PTR&) const;

public:
	~CMapWordToPtr();
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
};
////////////////////////////////////////////////////////////////////////////

_AFXCOLL_INLINE int CMapWordToPtr::GetCount() const
	{ return m_nCount; }
_AFXCOLL_INLINE BOOL CMapWordToPtr::IsEmpty() const
	{ return m_nCount == 0; }
_AFXCOLL_INLINE void CMapWordToPtr::SetAt(WORD key, void* newValue)
	{ (*this)[key] = newValue; }
_AFXCOLL_INLINE RXPOSITION CMapWordToPtr::GetStartPosition() const
	{ return (m_nCount == 0) ? NULL : BEFORE_START_RXPOSITION; }
_AFXCOLL_INLINE UINT_PTR CMapWordToPtr::GetHashTableSize() const
	{ return m_nHashTableSize; }


/////////////////////////////////////////////////////////////////////////////

class CMapPtrToPtr : public CObject
{

	DECLARE_DYNAMIC(CMapPtrToPtr)
public:
	// Association
	struct CAssoc
	{
		CAssoc* pNext;
		UINT_PTR nHashValue;  // needed for efficient iteration
		void* key;
		void* value;
	};
public:

// Construction
	CMapPtrToPtr(int nBlockSize=10);

// Attributes
	// number of elements
	int GetCount() const;
	BOOL IsEmpty() const;

	// Lookup
	BOOL Lookup(void* key, void*& rValue) const;

// Operations
	// Lookup and add if not there
	void*& operator[](void* key);

	// add a new (key, value) pair
	void SetAt(void* key, void* newValue);

	// removing existing (key, ?) pair
	BOOL RemoveKey(void* key);
	void RemoveAll();

	// iterating all (key, value) pairs
	RXPOSITION GetStartPosition() const;
	void GetNextAssoc(RXPOSITION& rNextPosition, void*& rKey, void*& rValue) const;

	// advanced features for derived classes
	UINT_PTR GetHashTableSize() const;
	void InitHashTable(UINT_PTR hashSize);

// Overridables: special non-virtual (see map implementation for details)
	// Routine used to user-provided hash keys
	UINT_PTR HashKey(void* key) const;

// Implementation
protected:
	CAssoc** m_pHashTable;
	UINT_PTR m_nHashTableSize;
	int m_nCount;
	CAssoc* m_pFreeList;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CAssoc* NewAssoc();
	void FreeAssoc(CAssoc*);
	CAssoc* GetAssocAt(void*, UINT_PTR&) const;

public:
	~CMapPtrToPtr();
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
};
////////////////////////////////////////////////////////////////////////////

_AFXCOLL_INLINE int CMapPtrToPtr::GetCount() const
	{ return m_nCount; }
_AFXCOLL_INLINE BOOL CMapPtrToPtr::IsEmpty() const
	{ return m_nCount == 0; }
_AFXCOLL_INLINE void CMapPtrToPtr::SetAt(void* key, void* newValue)
	{ (*this)[key] = newValue; }
_AFXCOLL_INLINE RXPOSITION CMapPtrToPtr::GetStartPosition() const
	{ return (m_nCount == 0) ? NULL : BEFORE_START_RXPOSITION; }
_AFXCOLL_INLINE UINT_PTR CMapPtrToPtr::GetHashTableSize() const
	{ return m_nHashTableSize; }


/////////////////////////////////////////////////////////////////////////////

class CMapStringToOb : public CObject
{

	DECLARE_SERIAL(CMapStringToOb)
public:
	// Association
	struct CAssoc
	{
		CAssoc* pNext;
		UINT_PTR nHashValue;  // needed for efficient iteration
		char *key;
		CObject* value;
	};
public:

// Construction
	CMapStringToOb(int nBlockSize=10);

// Attributes
	// number of elements
	int GetCount() const;
	BOOL IsEmpty() const;

	// Lookup
	BOOL Lookup(const char* key, CObject*& rValue) const;

// Operations
	// Lookup and add if not there
	CObject*& operator[](const char* key);

	// add a new (key, value) pair
	void SetAt(const char* key, CObject* newValue);

	// removing existing (key, ?) pair
	BOOL RemoveKey(const char* key);
	void RemoveAll();

	// iterating all (key, value) pairs
	RXPOSITION GetStartPosition() const;
	void GetNextAssoc(RXPOSITION& rNextPosition, char *&rKey, CObject*& rValue) const;

	// advanced features for derived classes
	UINT_PTR GetHashTableSize() const;
	void InitHashTable(UINT_PTR hashSize);

// Overridables: special non-virtual (see map implementation for details)
	// Routine used to user-provided hash keys
	UINT_PTR HashKey(const char* key) const;

// Implementation
protected:
	CAssoc** m_pHashTable;
	UINT_PTR m_nHashTableSize;
	int m_nCount;
	CAssoc* m_pFreeList;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CAssoc* NewAssoc();
	void FreeAssoc(CAssoc*);
	CAssoc* GetAssocAt(const char*, UINT_PTR&) const;

public:
	~CMapStringToOb();

#if 0
	void Serialize(CArchive&);
#endif
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
};

////////////////////////////////////////////////////////////////////////////
_AFXCOLL_INLINE int CMapStringToOb::GetCount() const
	{ return m_nCount; }
_AFXCOLL_INLINE BOOL CMapStringToOb::IsEmpty() const
	{ return m_nCount == 0; }
_AFXCOLL_INLINE void CMapStringToOb::SetAt(const char* key, CObject* newValue)
	{ (*this)[key] = newValue; }
_AFXCOLL_INLINE RXPOSITION CMapStringToOb::GetStartPosition() const
	{ return (m_nCount == 0) ? NULL : BEFORE_START_RXPOSITION; }
_AFXCOLL_INLINE UINT_PTR CMapStringToOb::GetHashTableSize() const
	{ return m_nHashTableSize; }


#endif /* STDARX_HH */






// This is a part of the Microsoft Foundation Classes C++ library. 
// Copyright (C) 1992 Microsoft Corporation 
// All rights reserved. 
//  
// This source code is only intended as a supplement to the 
// Microsoft Foundation Classes Reference and Microsoft 
// QuickHelp and/or WinHelp documentation provided with the library. 
// See these sources for detailed information regarding the 
// Microsoft Foundation Classes product. 

#ifndef __PLEX_H__
#define __PLEX_H__

#ifdef AFX_COLL_SEG
#pragma code_seg(AFX_COLL_SEG)
#endif

struct CPlex    // warning variable length structure
{
	CPlex* pNext;
	UINT nMax;
	UINT nCur;
	/* BYTE data[maxNum*elementSize]; */

	void* data() { return this+1; }

	static CPlex* Create(CPlex*& head, UINT nMax, UINT cbElement);
			// like 'calloc' but no zero fill
			// may throw memory exceptions

	void FreeDataChain();       // free this one and links
};

#ifdef AFX_COLL_SEG
#pragma code_seg()
#endif

#endif // __AFX_H__
#endif //__PLEX_H__
