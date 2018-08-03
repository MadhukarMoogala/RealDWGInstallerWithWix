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
// MAPS.CPP
//
// DESCRIPTION:
//
// Port of MFC maps.
//

/////////////////////////////////////////////////////////////////////////////
//
// Implementation of parmeterized Map
//
/////////////////////////////////////////////////////////////////////////////
#define DEBUG_NEW new

#include "assert.h"
#ifdef _DEBUG
#define ASSERT(a) assert(a)
#else
#define ASSERT(a)
#endif

#ifdef AFX_COLL2_SEG
#pragma code_seg(AFX_COLL2_SEG)
#endif

#include "wtypes.h"
#include "maps.h"

IMPLEMENT_DYNAMIC(CMapWordToPtr, CObject)

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif



#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////

CMapWordToPtr::CMapWordToPtr(int nBlockSize)
{
    ASSERT(nBlockSize > 0);

    m_pHashTable = NULL;
    m_nHashTableSize = RX_HASHTABLESIZE;  // default size
    m_nCount = 0;
    m_pFreeList = NULL;
    m_pBlocks = NULL;
    m_nBlockSize = nBlockSize;
}

inline UINT_PTR CMapWordToPtr::HashKey(WORD key) const
{
    // Default identity hash, works for most primitive values.
    return ((DWORD_PTR)key) >> 4;
}


void CMapWordToPtr::InitHashTable(UINT_PTR nHashSize)
//
// Used to force allocation of a hash table or to override the default
//   hash table size of (which is fairly small).
{
    ASSERT_VALID(this);
    ASSERT(m_nCount == 0);
    ASSERT(nHashSize > 0);

    // If had a hash table, get rid of it.
    if (m_pHashTable != NULL)
        delete [] m_pHashTable;
    m_pHashTable = NULL;

    m_pHashTable = new CAssoc* [nHashSize];
    memset(m_pHashTable, 0, sizeof(CAssoc*) * nHashSize);
    m_nHashTableSize = nHashSize;
}

void CMapWordToPtr::RemoveAll()
{
    ASSERT_VALID(this);

    if (m_pHashTable != NULL)
    {
        // Free hash table.
        delete [] m_pHashTable;
        m_pHashTable = NULL;
    }

    m_nCount = 0;
    m_pFreeList = NULL;
    m_pBlocks->FreeDataChain();
    m_pBlocks = NULL;
}

CMapWordToPtr::~CMapWordToPtr()
{
    RemoveAll();
    ASSERT(m_nCount == 0);
}

/////////////////////////////////////////////////////////////////////////////
// Assoc helpers
// same as CList implementation except we store CAssoc's not CNode's
//    and CAssoc's are singly linked all the time

CMapWordToPtr::CAssoc* CMapWordToPtr::NewAssoc()
{
    if (m_pFreeList == NULL)
    {
        // Add another block.
        CPlex* newBlock = CPlex::Create(m_pBlocks, m_nBlockSize, sizeof(CMapWordToPtr::CAssoc));
        // Chain them into free list.
        CMapWordToPtr::CAssoc* pAssoc = (CMapWordToPtr::CAssoc*) newBlock->data();
        // Free in reverse order to make it easier to debug.
        pAssoc += m_nBlockSize - 1;
        for (int i = m_nBlockSize-1; i >= 0; i--, pAssoc--)
        {
            pAssoc->pNext = m_pFreeList;
            m_pFreeList = pAssoc;
        }
    }
    ASSERT(m_pFreeList != NULL);  // we must have something

    CMapWordToPtr::CAssoc* pAssoc = m_pFreeList;
    m_pFreeList = m_pFreeList->pNext;
    m_nCount++;
    ASSERT(m_nCount > 0);  // Make sure we don't overflow.
    memset(&pAssoc->key, 0, sizeof(WORD));

    memset(&pAssoc->value, 0, sizeof(void*));

    return pAssoc;
}

void CMapWordToPtr::FreeAssoc(CMapWordToPtr::CAssoc* pAssoc)
{

    pAssoc->pNext = m_pFreeList;
    m_pFreeList = pAssoc;
    m_nCount--;
    ASSERT(m_nCount >= 0);  // Make sure we don't underflow.
}

CMapWordToPtr::CAssoc*
CMapWordToPtr::GetAssocAt(WORD key, UINT_PTR& nHash) const
// Find association (or return NULL).
{
    nHash = HashKey(key) % m_nHashTableSize;

    if (m_pHashTable == NULL)
        return NULL;

    // See if it exists.
    CAssoc* pAssoc;
    for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL; pAssoc = pAssoc->pNext)
    {
        if (pAssoc->key == key)
            return pAssoc;
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CMapWordToPtr::Lookup(WORD key, void*& rValue) const
{
    ASSERT_VALID(this);

    UINT_PTR nHash;
    CAssoc* pAssoc = GetAssocAt(key, nHash);
    if (pAssoc == NULL)
        return FALSE;  // Not in map.

    rValue = pAssoc->value;
    return TRUE;
}

void*& CMapWordToPtr::operator[](WORD key)
{
    ASSERT_VALID(this);

    UINT_PTR nHash;
    CAssoc* pAssoc;
    if ((pAssoc = GetAssocAt(key, nHash)) == NULL)
    {
        if (m_pHashTable == NULL)
            InitHashTable(m_nHashTableSize);

        // It doesn't exist, add a new Association.
        pAssoc = NewAssoc();
        pAssoc->nHashValue = nHash;
        pAssoc->key = key;
        // 'pAssoc->value' is a constructed object, nothing more

        // Put into hash table.
        pAssoc->pNext = m_pHashTable[nHash];
        m_pHashTable[nHash] = pAssoc;
    }
    return pAssoc->value;  // return new reference
}


BOOL CMapWordToPtr::RemoveKey(WORD key)
// Remove key - return TRUE if removed.
{
    ASSERT_VALID(this);

    if (m_pHashTable == NULL)
        return FALSE;  // nothing in the table

    CAssoc** ppAssocPrev;
    ppAssocPrev = &m_pHashTable[HashKey(key) % m_nHashTableSize];

    CAssoc* pAssoc;
    for (pAssoc = *ppAssocPrev; pAssoc != NULL; pAssoc = pAssoc->pNext)
    {
        if (pAssoc->key == key)
        {
            // Remove it.
            *ppAssocPrev = pAssoc->pNext;  // Remove from list.
            FreeAssoc(pAssoc);
            return TRUE;
        }
        ppAssocPrev = &pAssoc->pNext;
    }
    return FALSE;  // Not found.
}


/////////////////////////////////////////////////////////////////////////////
// Iterating

void CMapWordToPtr::GetNextAssoc(RXPOSITION& rNextPosition,
    WORD& rKey, void*& rValue) const
{
    ASSERT_VALID(this);
    ASSERT(m_pHashTable != NULL);  // never call on empty map

    CAssoc* pAssocRet = (CAssoc*)rNextPosition;
    ASSERT(pAssocRet != NULL);

    if (pAssocRet == (CAssoc*) BEFORE_START_RXPOSITION)
    {
        // Find the first association.
        for (UINT nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
            if ((pAssocRet = m_pHashTable[nBucket]) != NULL)
                break;
        ASSERT(pAssocRet != NULL);  // Must find something.
    }

    // Find next association.
    ASSERT(AfxIsValidAddress(pAssocRet, sizeof(CAssoc)));
    CAssoc* pAssocNext;
    if ((pAssocNext = pAssocRet->pNext) == NULL)
    {
        // Go to next bucket.
        for (UINT_PTR nBucket = pAssocRet->nHashValue + 1;
          nBucket < m_nHashTableSize; nBucket++)
            if ((pAssocNext = m_pHashTable[nBucket]) != NULL)
                break;
    }

    rNextPosition = (RXPOSITION) pAssocNext;

    // Fill in return data.
    rKey = pAssocRet->key;
    rValue = pAssocRet->value;
}

/////////////////////////////////////////////////////////////////////////////
// Serialization


/////////////////////////////////////////////////////////////////////////////
// Diagnostics

#ifdef _DEBUG

void CMapWordToPtr::Dump(CDumpContext& dc) const
{
    ASSERT_VALID(this);

#define MAKESTRING(x) #x
    AFX_DUMP1(dc, "a " MAKESTRING(CMapWordToPtr) " with ", m_nCount);
    AFX_DUMP0(dc, " elements");
#undef MAKESTRING
    if (dc.GetDepth() > 0)
    {
        // Dump in format "[key] -> value".
        RXPOSITION pos = GetStartPosition();
        WORD key;
        void* val;

        AFX_DUMP0(dc, "\n");
        while (pos != NULL)
        {
            GetNextAssoc(pos, key, val);
            AFX_DUMP1(dc, "\n\t[", key);
            AFX_DUMP1(dc, "] = ", val);
        }
    }
}

void CMapWordToPtr::AssertValid() const
{
    CObject::AssertValid();

    ASSERT(m_nHashTableSize > 0);
    ASSERT(m_nCount == 0 || m_pHashTable != NULL);
        // Non-empty map should have hash table.
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////
//
// Implementation of parmeterized Map
//
/////////////////////////////////////////////////////////////////////////////


#ifdef AFX_COLL2_SEG
#pragma code_seg(AFX_COLL2_SEG)
#endif


IMPLEMENT_DYNAMIC(CMapPtrToWord, CObject)

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif



#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////

CMapPtrToWord::CMapPtrToWord(int nBlockSize)
{
    ASSERT(nBlockSize > 0);

    m_pHashTable = NULL;
    m_nHashTableSize = RX_HASHTABLESIZE;  // default size
    m_nCount = 0;
    m_pFreeList = NULL;
    m_pBlocks = NULL;
    m_nBlockSize = nBlockSize;
}

inline UINT_PTR CMapPtrToWord::HashKey(void* key) const
{
    // Default identity hash - works for most primitive values.
    return UINT_PTR(DWORD_PTR(key)>>4);
}


void CMapPtrToWord::InitHashTable(UINT_PTR nHashSize)
//
// Used to force allocation of a hash table or to override the default
//   hash table size of (which is fairly small).
{
    ASSERT_VALID(this);
    ASSERT(m_nCount == 0);
    ASSERT(nHashSize > 0);

    // if had a hash table - get rid of it
    if (m_pHashTable != NULL)
        delete [] m_pHashTable;
    m_pHashTable = NULL;

    m_pHashTable = new CAssoc* [nHashSize];
    memset(m_pHashTable, 0, sizeof(CAssoc*) * nHashSize);
    m_nHashTableSize = nHashSize;
}

void CMapPtrToWord::RemoveAll()
{
    ASSERT_VALID(this);

    if (m_pHashTable != NULL)
    {
        // free hash table
        delete [] m_pHashTable;
        m_pHashTable = NULL;
    }

    m_nCount = 0;
    m_pFreeList = NULL;
    m_pBlocks->FreeDataChain();
    m_pBlocks = NULL;
}

CMapPtrToWord::~CMapPtrToWord()
{
    RemoveAll();
    ASSERT(m_nCount == 0);
}

/////////////////////////////////////////////////////////////////////////////
// Assoc helpers
// same as CList implementation except we store CAssoc's not CNode's
//    and CAssoc's are singly linked all the time

CMapPtrToWord::CAssoc* CMapPtrToWord::NewAssoc()
{
    if (m_pFreeList == NULL)
    {
        // add another block
        CPlex* newBlock = CPlex::Create(m_pBlocks, m_nBlockSize, sizeof(CMapPtrToWord::CAssoc));
        // chain them into free list
        CMapPtrToWord::CAssoc* pAssoc = (CMapPtrToWord::CAssoc*) newBlock->data();
        // free in reverse order to make it easier to debug
        pAssoc += m_nBlockSize - 1;
        for (int i = m_nBlockSize-1; i >= 0; i--, pAssoc--)
        {
            pAssoc->pNext = m_pFreeList;
            m_pFreeList = pAssoc;
        }
    }
    ASSERT(m_pFreeList != NULL);  // we must have something

    CMapPtrToWord::CAssoc* pAssoc = m_pFreeList;
    m_pFreeList = m_pFreeList->pNext;
    m_nCount++;
    ASSERT(m_nCount > 0);  // make sure we don't overflow
    memset(&pAssoc->key, 0, sizeof(void*));

    memset(&pAssoc->value, 0, sizeof(WORD));

    return pAssoc;
}

void CMapPtrToWord::FreeAssoc(CMapPtrToWord::CAssoc* pAssoc)
{

    pAssoc->pNext = m_pFreeList;
    m_pFreeList = pAssoc;
    m_nCount--;
    ASSERT(m_nCount >= 0);  // make sure we don't underflow
}

CMapPtrToWord::CAssoc*
CMapPtrToWord::GetAssocAt(void* key, UINT_PTR& nHash) const
// find association (or return NULL)
{
    nHash = HashKey(key) % m_nHashTableSize;

    if (m_pHashTable == NULL)
        return NULL;

    // see if it exists
    CAssoc* pAssoc;
    for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL; pAssoc = pAssoc->pNext)
    {
        if (pAssoc->key == key)
            return pAssoc;
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CMapPtrToWord::Lookup(void* key, WORD& rValue) const
{
    ASSERT_VALID(this);

    UINT_PTR nHash;
    CAssoc* pAssoc = GetAssocAt(key, nHash);
    if (pAssoc == NULL)
        return FALSE;  // not in map

    rValue = pAssoc->value;
    return TRUE;
}

WORD& CMapPtrToWord::operator[](void* key)
{
    ASSERT_VALID(this);

    UINT_PTR nHash;
    CAssoc* pAssoc;
    if ((pAssoc = GetAssocAt(key, nHash)) == NULL)
    {
        if (m_pHashTable == NULL)
            InitHashTable(m_nHashTableSize);

        // it doesn't exist, add a new Association
        pAssoc = NewAssoc();
        pAssoc->nHashValue = nHash;
        pAssoc->key = key;
        // 'pAssoc->value' is a constructed object, nothing more

        // put into hash table
        pAssoc->pNext = m_pHashTable[nHash];
        m_pHashTable[nHash] = pAssoc;
    }
    return pAssoc->value;  // return new reference
}


BOOL CMapPtrToWord::RemoveKey(void* key)
// remove key - return TRUE if removed
{
    ASSERT_VALID(this);

    if (m_pHashTable == NULL)
        return FALSE;  // nothing in the table

    CAssoc** ppAssocPrev;
    ppAssocPrev = &m_pHashTable[HashKey(key) % m_nHashTableSize];

    CAssoc* pAssoc;
    for (pAssoc = *ppAssocPrev; pAssoc != NULL; pAssoc = pAssoc->pNext)
    {
        if (pAssoc->key == key)
        {
            // remove it
            *ppAssocPrev = pAssoc->pNext;  // remove from list
            FreeAssoc(pAssoc);
            return TRUE;
        }
        ppAssocPrev = &pAssoc->pNext;
    }
    return FALSE;  // not found
}


/////////////////////////////////////////////////////////////////////////////
// Iterating

void CMapPtrToWord::GetNextAssoc(RXPOSITION& rNextPosition,
    void*& rKey, WORD& rValue) const
{
    ASSERT_VALID(this);
    ASSERT(m_pHashTable != NULL);  // never call on empty map

    CAssoc* pAssocRet = (CAssoc*)rNextPosition;
    ASSERT(pAssocRet != NULL);

    if (pAssocRet == (CAssoc*) BEFORE_START_RXPOSITION)
    {
        // find the first association
        for (UINT nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
            if ((pAssocRet = m_pHashTable[nBucket]) != NULL)
                break;
        ASSERT(pAssocRet != NULL);  // must find something
    }

    // find next association
    ASSERT(AfxIsValidAddress(pAssocRet, sizeof(CAssoc)));
    CAssoc* pAssocNext;
    if ((pAssocNext = pAssocRet->pNext) == NULL)
    {
        // go to next bucket
        for (UINT_PTR nBucket = pAssocRet->nHashValue + 1;
          nBucket < m_nHashTableSize; nBucket++)
            if ((pAssocNext = m_pHashTable[nBucket]) != NULL)
                break;
    }

    rNextPosition = (RXPOSITION) pAssocNext;

    // fill in return data
    rKey = pAssocRet->key;
    rValue = pAssocRet->value;
}

/////////////////////////////////////////////////////////////////////////////
// Serialization


/////////////////////////////////////////////////////////////////////////////
// Diagnostics

#ifdef _DEBUG

void CMapPtrToWord::Dump(CDumpContext& dc) const
{
    ASSERT_VALID(this);

#define MAKESTRING(x) #x
    AFX_DUMP1(dc, "a " MAKESTRING(CMapPtrToWord) " with ", m_nCount);
    AFX_DUMP0(dc, " elements");
#undef MAKESTRING
    if (dc.GetDepth() > 0)
    {
        // Dump in format "[key] -> value"
        RXPOSITION pos = GetStartPosition();
        void* key;
        WORD val;

        AFX_DUMP0(dc, "\n");
        while (pos != NULL)
        {
            GetNextAssoc(pos, key, val);
            AFX_DUMP1(dc, "\n\t[", key);
            AFX_DUMP1(dc, "] = ", val);
        }
    }
}

void CMapPtrToWord::AssertValid() const
{
    CObject::AssertValid();

    ASSERT(m_nHashTableSize > 0);
    ASSERT(m_nCount == 0 || m_pHashTable != NULL);
        // non-empty map should have hash table
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////////
//
// Implementation of parmeterized Map
//
/////////////////////////////////////////////////////////////////////////////


#ifdef AFX_COLL2_SEG
#pragma code_seg(AFX_COLL2_SEG)
#endif


IMPLEMENT_DYNAMIC(CMapPtrToPtr, CObject)

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif



#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////

CMapPtrToPtr::CMapPtrToPtr(int nBlockSize)
{
    ASSERT(nBlockSize > 0);

    m_pHashTable = NULL;
    m_nHashTableSize = RX_HASHTABLESIZE;  // default size
    m_nCount = 0;
    m_pFreeList = NULL;
    m_pBlocks = NULL;
    m_nBlockSize = nBlockSize;
}

inline UINT_PTR CMapPtrToPtr::HashKey(void* key) const
{
    // default identity hash - works for most primitive values
    return UINT_PTR(DWORD_PTR(key)>>4);
}


void CMapPtrToPtr::InitHashTable(UINT_PTR nHashSize)
//
// Used to force allocation of a hash table or to override the default
//   hash table size of (which is fairly small)
{
    ASSERT_VALID(this);
    ASSERT(m_nCount == 0);
    ASSERT(nHashSize > 0);

    // if had a hash table - get rid of it
    if (m_pHashTable != NULL)
        delete [] m_pHashTable;
    m_pHashTable = NULL;

    m_pHashTable = new CAssoc* [nHashSize];
    memset(m_pHashTable, 0, sizeof(CAssoc*) * nHashSize);
    m_nHashTableSize = nHashSize;
}

void CMapPtrToPtr::RemoveAll()
{
    ASSERT_VALID(this);

    if (m_pHashTable != NULL)
    {
        // free hash table
        delete [] m_pHashTable;
        m_pHashTable = NULL;
    }

    m_nCount = 0;
    m_pFreeList = NULL;
    m_pBlocks->FreeDataChain();
    m_pBlocks = NULL;
}

CMapPtrToPtr::~CMapPtrToPtr()
{
    RemoveAll();
    ASSERT(m_nCount == 0);
}

/////////////////////////////////////////////////////////////////////////////
// Assoc helpers
// same as CList implementation except we store CAssoc's not CNode's
//    and CAssoc's are singly linked all the time

CMapPtrToPtr::CAssoc* CMapPtrToPtr::NewAssoc()
{
    if (m_pFreeList == NULL)
    {
        // add another block
        CPlex* newBlock = CPlex::Create(m_pBlocks, m_nBlockSize, sizeof(CMapPtrToPtr::CAssoc));
        // chain them into free list
        CMapPtrToPtr::CAssoc* pAssoc = (CMapPtrToPtr::CAssoc*) newBlock->data();
        // free in reverse order to make it easier to debug
        pAssoc += m_nBlockSize - 1;
        for (int i = m_nBlockSize-1; i >= 0; i--, pAssoc--)
        {
            pAssoc->pNext = m_pFreeList;
            m_pFreeList = pAssoc;
        }
    }
    ASSERT(m_pFreeList != NULL);  // we must have something

    CMapPtrToPtr::CAssoc* pAssoc = m_pFreeList;
    m_pFreeList = m_pFreeList->pNext;
    m_nCount++;
    ASSERT(m_nCount > 0);  // make sure we don't overflow
    memset(&pAssoc->key, 0, sizeof(void*));

    memset(&pAssoc->value, 0, sizeof(void*));

    return pAssoc;
}

void CMapPtrToPtr::FreeAssoc(CMapPtrToPtr::CAssoc* pAssoc)
{

    pAssoc->pNext = m_pFreeList;
    m_pFreeList = pAssoc;
    m_nCount--;
    ASSERT(m_nCount >= 0);  // make sure we don't underflow
}

CMapPtrToPtr::CAssoc*
CMapPtrToPtr::GetAssocAt(void* key, UINT_PTR& nHash) const
// find association (or return NULL)
{
    nHash = HashKey(key) % m_nHashTableSize;

    if (m_pHashTable == NULL)
        return NULL;

    // see if it exists
    CAssoc* pAssoc;
    for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL; pAssoc = pAssoc->pNext)
    {
        if (pAssoc->key == key)
            return pAssoc;
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CMapPtrToPtr::Lookup(void* key, void*& rValue) const
{
    ASSERT_VALID(this);

    UINT_PTR nHash;
    CAssoc* pAssoc = GetAssocAt(key, nHash);
    if (pAssoc == NULL)
        return FALSE;  // not in map

    rValue = pAssoc->value;
    return TRUE;
}

void*& CMapPtrToPtr::operator[](void* key)
{
    ASSERT_VALID(this);

    UINT_PTR nHash;
    CAssoc* pAssoc;
    if ((pAssoc = GetAssocAt(key, nHash)) == NULL)
    {
        if (m_pHashTable == NULL)
            InitHashTable(m_nHashTableSize);

        // it doesn't exist, add a new Association
        pAssoc = NewAssoc();
        pAssoc->nHashValue = nHash;
        pAssoc->key = key;
        // 'pAssoc->value' is a constructed object, nothing more

        // put into hash table
        pAssoc->pNext = m_pHashTable[nHash];
        m_pHashTable[nHash] = pAssoc;
    }
    return pAssoc->value;  // return new reference
}


BOOL CMapPtrToPtr::RemoveKey(void* key)
// remove key - return TRUE if removed
{
    ASSERT_VALID(this);

    if (m_pHashTable == NULL)
        return FALSE;  // nothing in the table

    CAssoc** ppAssocPrev;
    ppAssocPrev = &m_pHashTable[HashKey(key) % m_nHashTableSize];

    CAssoc* pAssoc;
    for (pAssoc = *ppAssocPrev; pAssoc != NULL; pAssoc = pAssoc->pNext)
    {
        if (pAssoc->key == key)
        {
            // remove it
            *ppAssocPrev = pAssoc->pNext;  // remove from list
            FreeAssoc(pAssoc);
            return TRUE;
        }
        ppAssocPrev = &pAssoc->pNext;
    }
    return FALSE;  // not found
}


/////////////////////////////////////////////////////////////////////////////
// Iterating

void CMapPtrToPtr::GetNextAssoc(RXPOSITION& rNextPosition,
    void*& rKey, void*& rValue) const
{
    ASSERT_VALID(this);
    ASSERT(m_pHashTable != NULL);  // never call on empty map

    CAssoc* pAssocRet = (CAssoc*)rNextPosition;
    ASSERT(pAssocRet != NULL);

    if (pAssocRet == (CAssoc*) BEFORE_START_RXPOSITION)
    {
        // find the first association
        for (UINT nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
            if ((pAssocRet = m_pHashTable[nBucket]) != NULL)
                break;
        ASSERT(pAssocRet != NULL);  // must find something
    }

    // find next association
    ASSERT(AfxIsValidAddress(pAssocRet, sizeof(CAssoc)));
    CAssoc* pAssocNext;
    if ((pAssocNext = pAssocRet->pNext) == NULL)
    {
        // go to next bucket
        for (UINT_PTR nBucket = pAssocRet->nHashValue + 1;
          nBucket < m_nHashTableSize; nBucket++)
            if ((pAssocNext = m_pHashTable[nBucket]) != NULL)
                break;
    }

    rNextPosition = (RXPOSITION) pAssocNext;

    // fill in return data
    rKey = pAssocRet->key;
    rValue = pAssocRet->value;
}

/////////////////////////////////////////////////////////////////////////////
// Serialization


/////////////////////////////////////////////////////////////////////////////
// Diagnostics

#ifdef _DEBUG

void CMapPtrToPtr::Dump(CDumpContext& dc) const
{
    ASSERT_VALID(this);

#define MAKESTRING(x) #x
    AFX_DUMP1(dc, "a " MAKESTRING(CMapPtrToPtr) " with ", m_nCount);
    AFX_DUMP0(dc, " elements");
#undef MAKESTRING
    if (dc.GetDepth() > 0)
    {
        // Dump in format "[key] -> value"
        RXPOSITION pos = GetStartPosition();
        void* key;
        void* val;

        AFX_DUMP0(dc, "\n");
        while (pos != NULL)
        {
            GetNextAssoc(pos, key, val);
            AFX_DUMP1(dc, "\n\t[", key);
            AFX_DUMP1(dc, "] = ", val);
        }
    }
}

void CMapPtrToPtr::AssertValid() const
{
    CObject::AssertValid();

    ASSERT(m_nHashTableSize > 0);
    ASSERT(m_nCount == 0 || m_pHashTable != NULL);
        // non-empty map should have hash table
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////////
//
// Implementation of parmeterized Map from CString to value
//
/////////////////////////////////////////////////////////////////////////////

#include <string.h>

#ifdef AFX_COLL2_SEG
#pragma code_seg(AFX_COLL2_SEG)
#endif


IMPLEMENT_SERIAL(CMapStringToOb, CObject, 0)

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif



#define new DEBUG_NEW


const char *afxEmptyString = "";
extern const char *afxEmptyString;  // for creating empty key strings

/////////////////////////////////////////////////////////////////////////////

CMapStringToOb::CMapStringToOb(int nBlockSize)
{
    ASSERT(nBlockSize > 0);

    m_pHashTable = NULL;
    m_nHashTableSize = RX_HASHTABLESIZE;  // default size
    m_nCount = 0;
    m_pFreeList = NULL;
    m_pBlocks = NULL;
    m_nBlockSize = nBlockSize;
}

inline UINT_PTR CMapStringToOb::HashKey(const char* key) const
{
    UINT_PTR nHash = 0;
    while (*key)
        nHash = (nHash<<5) + nHash + *key++;
    return nHash;
}

void CMapStringToOb::InitHashTable(UINT_PTR nHashSize)
//
// Used to force allocation of a hash table or to override the default
//   hash table size of (which is fairly small)
{
    ASSERT_VALID(this);
    ASSERT(m_nCount == 0);
    ASSERT(nHashSize > 0);

    // if had a hash table - get rid of it
    if (m_pHashTable != NULL)
        delete [] m_pHashTable;
    m_pHashTable = NULL;

    m_pHashTable = new CAssoc* [nHashSize];
    memset(m_pHashTable, 0, sizeof(CAssoc*) * nHashSize);
    m_nHashTableSize = nHashSize;
}

void CMapStringToOb::RemoveAll()
{
    ASSERT_VALID(this);

    if (m_pHashTable != NULL)
    {
        // destroy elements
        for (UINT nHash = 0; nHash < m_nHashTableSize; nHash++)
        {
            CAssoc* pAssoc;
            for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL;
              pAssoc = pAssoc->pNext)
            {
                delete pAssoc->key;  // free up string data

            }
        }

        // free hash table
        delete [] m_pHashTable;
        m_pHashTable = NULL;
    }

    m_nCount = 0;
    m_pFreeList = NULL;
    m_pBlocks->FreeDataChain();
    m_pBlocks = NULL;
}

CMapStringToOb::~CMapStringToOb()
{
    RemoveAll();
    ASSERT(m_nCount == 0);
}

/////////////////////////////////////////////////////////////////////////////
// Assoc helpers
// same as CList implementation except we store CAssoc's not CNode's
//    and CAssoc's are singly linked all the time

CMapStringToOb::CAssoc* CMapStringToOb::NewAssoc()
{
    if (m_pFreeList == NULL)
    {
        // add another block
        CPlex* newBlock = CPlex::Create(m_pBlocks, m_nBlockSize, 
                            sizeof(CMapStringToOb::CAssoc));
        // chain them into free list
        CMapStringToOb::CAssoc* pAssoc = 
                (CMapStringToOb::CAssoc*) newBlock->data();
        // free in reverse order to make it easier to debug
        pAssoc += m_nBlockSize - 1;
        for (int i = m_nBlockSize-1; i >= 0; i--, pAssoc--)
        {
            pAssoc->pNext = m_pFreeList;
            m_pFreeList = pAssoc;
        }
    }
    ASSERT(m_pFreeList != NULL);  // we must have something

    CMapStringToOb::CAssoc* pAssoc = m_pFreeList;
    m_pFreeList = m_pFreeList->pNext;
    m_nCount++;
    ASSERT(m_nCount > 0);  // make sure we don't overflow
    // memcpy(&pAssoc->key, &afxEmptyString, sizeof(CString));
        pAssoc->key = NULL;

    memset(&pAssoc->value, 0, sizeof(CObject*));

    return pAssoc;
}

void CMapStringToOb::FreeAssoc(CMapStringToOb::CAssoc* pAssoc)
{
    delete pAssoc->key;  // free up string data
    pAssoc->key = NULL;

    pAssoc->pNext = m_pFreeList;
    m_pFreeList = pAssoc;
    m_nCount--;
    ASSERT(m_nCount >= 0);  // make sure we don't underflow
}

CMapStringToOb::CAssoc*
CMapStringToOb::GetAssocAt(const char* key, UINT_PTR& nHash) const
// find association (or return NULL)
{
    nHash = HashKey(key) % m_nHashTableSize;

    if (m_pHashTable == NULL)
        return NULL;

    // see if it exists
    CAssoc* pAssoc;
    for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL; pAssoc = pAssoc->pNext)
    {
        if (!strcmp(pAssoc->key,key))
            return pAssoc;
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CMapStringToOb::Lookup(const char* key, CObject*& rValue) const
{
    ASSERT_VALID(this);

    UINT_PTR nHash;
    CAssoc* pAssoc = GetAssocAt(key, nHash);
    if (pAssoc == NULL)
        return FALSE;  // not in map

    rValue = pAssoc->value;
    return TRUE;
}

CObject*& CMapStringToOb::operator[](const char* key)
{
    ASSERT_VALID(this);

    UINT_PTR nHash;
    CAssoc* pAssoc;
    if ((pAssoc = GetAssocAt(key, nHash)) == NULL)
    {
        if (m_pHashTable == NULL)
            InitHashTable(m_nHashTableSize);

        // it doesn't exist, add a new Association
        pAssoc = NewAssoc();
        pAssoc->nHashValue = nHash;

        const size_t nKeySize = strlen(key)+1;
        pAssoc->key = new char [nKeySize];
        strcpy_s(pAssoc->key, nKeySize, key);

        // 'pAssoc->value' is a constructed object, nothing more

        // put into hash table
        pAssoc->pNext = m_pHashTable[nHash];
        m_pHashTable[nHash] = pAssoc;
    }
    return pAssoc->value;  // return new reference
}


BOOL CMapStringToOb::RemoveKey(const char* key)
// remove key - return TRUE if removed
{
    ASSERT_VALID(this);

    if (m_pHashTable == NULL)
        return FALSE;  // nothing in the table

    CAssoc** ppAssocPrev;
    ppAssocPrev = &m_pHashTable[HashKey(key) % m_nHashTableSize];

    CAssoc* pAssoc;
    for (pAssoc = *ppAssocPrev; pAssoc != NULL; pAssoc = pAssoc->pNext)
    {
        if (pAssoc->key == key)
        {
            // remove it
            *ppAssocPrev = pAssoc->pNext;  // remove from list
            FreeAssoc(pAssoc);
            return TRUE;
        }
        ppAssocPrev = &pAssoc->pNext;
    }
    return FALSE;  // not found
}


/////////////////////////////////////////////////////////////////////////////
// Iterating

void CMapStringToOb::GetNextAssoc(RXPOSITION& rNextPosition,
    char *& rKey, CObject*& rValue) const
{
    ASSERT_VALID(this);
    ASSERT(m_pHashTable != NULL);  // never call on empty map

    CAssoc* pAssocRet = (CAssoc*)rNextPosition;
    ASSERT(pAssocRet != NULL);

    if (pAssocRet == (CAssoc*) BEFORE_START_RXPOSITION)
    {
        // find the first association
        for (UINT nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
            if ((pAssocRet = m_pHashTable[nBucket]) != NULL)
                break;
        ASSERT(pAssocRet != NULL);  // must find something
    }

    // find next association
    ASSERT(AfxIsValidAddress(pAssocRet, sizeof(CAssoc)));
    CAssoc* pAssocNext;
    if ((pAssocNext = pAssocRet->pNext) == NULL)
    {
        // go to next bucket
        for (UINT_PTR nBucket = pAssocRet->nHashValue + 1;
          nBucket < m_nHashTableSize; nBucket++)
            if ((pAssocNext = m_pHashTable[nBucket]) != NULL)
                break;
    }

    rNextPosition = (RXPOSITION) pAssocNext;

    // fill in return data
    rKey = pAssocRet->key;
    rValue = pAssocRet->value;
}


/////////////////////////////////////////////////////////////////////////////
// Diagnostics

#ifdef _DEBUG

void CMapStringToOb::Dump(CDumpContext& dc) const
{
    ASSERT_VALID(this);

#define MAKESTRING(x) #x
    AFX_DUMP1(dc, "A " MAKESTRING(CMapStringToOb) " with ", m_nCount);
    AFX_DUMP0(dc, " elements\n");
#undef MAKESTRING
    if (dc.GetDepth() > 0)
    {
        // Dump in format "[key] -> value"
        AFX_DUMP0(dc, "\n");
        RXPOSITION pos = GetStartPosition();
        CString key;
        CObject* val;
        while (pos != NULL)
        {
            GetNextAssoc(pos, key, val);
            AFX_DUMP1(dc, "\n\t[", key);
            AFX_DUMP1(dc, "] = ", val);
        }
    }
}

void CMapStringToOb::AssertValid() const
{
    CObject::AssertValid();

    ASSERT(m_nHashTableSize > 0);
    ASSERT(m_nCount == 0 || m_pHashTable != NULL);
        // non-empty map should have hash table
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////



// Collection support
#ifdef AFX_COLL_SEG
#pragma code_seg(AFX_COLL_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

CPlex* CPlex::Create(CPlex*& pHead, UINT nMax, UINT cbElement)
{
    ASSERT(nMax > 0 && cbElement > 0);
    CPlex* p = (CPlex*) new BYTE[sizeof(CPlex) + nMax * cbElement];
            // may throw exception
    p->nMax = nMax;
    p->nCur = 0;
    p->pNext = pHead;
    pHead = p;  // change head (adds in reverse order for simplicity)
    return p;
}

void CPlex::FreeDataChain()     // free this one and links
{
    CPlex* p = this;
    while (p != NULL)
    {
        BYTE* bytes = (BYTE*) p;
        CPlex* pNextt = p->pNext;
        delete bytes;
        p = pNextt;
    }
}
