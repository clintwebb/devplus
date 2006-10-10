//-----------------------------------------------------------------------------
// DevPlus.   
//-----------------------------------------------------------------------------


/***************************************************************************
 *   Copyright (C) 2006-2007 by Hyper-Active Systems,,,                    *
 *   Copyright (C) 2003-2005 by Clinton Webb,,,                            *
 *   devplus@hyper-active.com.au                                           *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/




//-----------------------------------------------------------------------------
// CJW: If we are compiling under visual studio, then we need to include this
//      file or we will have linker problems regarding some already defined
//      functions.  This is rather annoying and really quite stupid, but that
//      is what you get I suppose.
#ifdef _MSC_VER
    #include "stdafx.h"
#endif
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>

#include <DpSettings.h>



int DpSettings::_nInstances = 0;
int DpSettings::_nItems = 0;
SettingsData *DpSettings::_pDataList = NULL;


//------------------------------------------------------------------------------
// CJW: Constructor.
DpSettings::DpSettings()
{
    Lock();
    _nInstances ++;
    Unlock();
}

//------------------------------------------------------------------------------
// CJW: Deconstructor.
DpSettings::~DpSettings()
{
    int nCount, i;
    
    Lock();
    
    ASSERT(_nInstances > 0);
    _nInstances --;
    if (_nInstances == 0) {
    
        // ** clear out all the settings.
        for(nCount = 0; nCount < _nItems; nCount++) {
            if (_pDataList[nCount].szName != NULL) {
                free(_pDataList[nCount].szName);
                _pDataList[nCount].szName = NULL;
            }
    
            if (_pDataList[nCount].eType == TYPE_STRING) {
                i = _pDataList[nCount].nItems;
                while(i > 0) {
                    i--;
    
                    if (_pDataList[nCount].pData[i].string != NULL) {
                        free(_pDataList[nCount].pData[i].string);
                        _pDataList[nCount].pData[i].string = NULL;
                    }
                }
            }
    
            if (_pDataList[nCount].pData != NULL) {
                free(_pDataList[nCount].pData);
            }
        }
    
        if (_pDataList != NULL) {
            free(_pDataList);
            _nItems = 0;
            _pDataList = NULL;
        }
    
    }
    Unlock();
}

//------------------------------------------------------------------------------
// CJW: Set a string value for the name.
bool DpSettings::Set(char *name, char *value, int index)
{
    bool bResult = false;
    int entry;
    
    ASSERT(name != NULL);
    ASSERT(index >= 0);
    
    Lock();
    
    // first we need to find the named setting in our list.  If it is not there,
    // an empty entry will be created for it.
    entry = FindName(name);
    
    if (entry < 0) {
        entry = CreateName(name);
    }
    
    if (entry >= 0) {
        // make sure there are enough entries in our list for this object.
        if (VerifyIndex(entry, index) == false) {
            CreateIndex(entry, index);
        }
    
        _pDataList[entry].eType = TYPE_STRING;
        _pDataList[entry].pData[index].string = (char *) malloc(strlen(value)+1);
        strcpy(_pDataList[entry].pData[index].string, value);
    
        bResult = true;
    }
    
    Unlock();
    
    return(bResult);
}


//------------------------------------------------------------------------------
// CJW: Set a string value for the name.
bool DpSettings::Set(char *name, int value, int index)
{
    bool bResult = false;
    int entry;
    
    ASSERT(name != NULL);
    ASSERT(index >= 0);
    
    Lock();
    
    // first we need to find the named setting in our list.  If it is not there,
    // an empty entry will be created for it.
    entry = FindName(name);
    
    if (entry < 0) {
        entry = CreateName(name);
    }
    
    if (entry >= 0) {
        // make sure there are enough entries in our list for this object.
        if (VerifyIndex(entry, index) == false) {
            CreateIndex(entry, index);
        }
    
        _pDataList[entry].eType = TYPE_INTEGER;
        _pDataList[entry].pData[index].integer = value;
    
        bResult = true;
    }
    
    Unlock();
    
    return(bResult);
}


bool DpSettings::Get(char *name, char *value,  int index)
{
    bool bResult = false;
    int entry;
    
    ASSERT(name != NULL);
    ASSERT(index >= 0);
    ASSERT(value != NULL);
    
    Lock();
    
    // first we need to find the named setting in our list.
    entry = FindName(name);
    if (entry >= 0) {
        // make sure there are enough entries in our list for this object.
        if (VerifyIndex(entry, index) != false) {
            if (_pDataList[entry].eType == TYPE_STRING) {
                strcpy(value, _pDataList[entry].pData[index].string);
                bResult = true;
            }
        }
    }
    
    Unlock();
    
    return(bResult);
}

bool DpSettings::Get(char *name, int *value,  int index)
{
    bool bResult = false;
    int entry;
    
    ASSERT(name != NULL);
    ASSERT(index >= 0);
    ASSERT(value != NULL);
    
    Lock();
    
    // first we need to find the named setting in our list.
    entry = FindName(name);
    if (entry >= 0) {
        // make sure there are enough entries in our list for this object.
        if (VerifyIndex(entry, index) != false) {
            if (_pDataList[entry].eType == TYPE_INTEGER) {
                *value = _pDataList[entry].pData[index].integer;
                bResult = true;
            }
            else if (_pDataList[entry].eType == TYPE_STRING) {
                *value = atoi(_pDataList[entry].pData[index].string);
                bResult = true;
            }
        }
    }
    
    Unlock();
    
    return(bResult);
}

//------------------------------------------------------------------------------
// CJW: We have a named setting, and we want to find the entry number for it so
//      that we can find it in our array.  Basically we will go thru each
//      element and check the name of each one.
int DpSettings::FindName(char *name)
{
    int nEntry=-1;
    int nCount = 0;
    
    while(nCount < _nItems) {
        if(strcmp(name, _pDataList[nCount].szName) == 0) {
            nEntry = nCount;
            nCount = _nItems;
        }
        nCount++;
    }
    
    return(nEntry);
}


//------------------------------------------------------------------------------
// CJW: The name evidently is not in our list yet, so we are going to create it.
int DpSettings::CreateName(char *name)
{
    int nEntry;
    
    nEntry = _nItems;
    _nItems++;
    _pDataList = (SettingsData *) realloc(_pDataList, sizeof(SettingsData) * _nItems);
    _pDataList[nEntry].szName = (char *) malloc(strlen(name) + 1);
    strcpy(_pDataList[nEntry].szName, name);
    _pDataList[nEntry].nItems = 0;
    
    return(nEntry);
}


//------------------------------------------------------------------------------
// CJW: We need to verify that there are enough indexes for this entry.  If
//      there are, then return true, otherwise return false.
bool DpSettings::VerifyIndex(int entry, int index)
{
    bool bValid = false;
    
    ASSERT(entry >= 0);
    ASSERT(index >= 0);
    
    ASSERT(_pDataList != NULL);
    ASSERT(entry < _nItems);
    
    if (_pDataList[entry].nItems > index) {
        bValid = true;
    }
    
    return(bValid);
}


//------------------------------------------------------------------------------
// CJW: Create the right number of blank index entries if we dont already have
//      enough.
void DpSettings::CreateIndex(int entry, int index)
{
    ASSERT(_pDataList != NULL);
    ASSERT(entry < _nItems);

    if (index >= _pDataList[entry].nItems) {
        _pDataList[entry].pData = (union SettingsUnion *) realloc(_pDataList[entry].pData, sizeof(union SettingsUnion) * (index + 1));
    
        while(_pDataList[entry].nItems <= index) {
            _pDataList[entry].pData[_pDataList[entry].nItems].pointer = NULL;
            _pDataList[entry].nItems++;
        }
    }
}




