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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <DpIniFile.h>


//------------------------------------------------------------------------------
// CJW: Constructor.  The Ini file object will be used to open an ini file and get the values out of it.  All values in the file will be contained in a group.
DpIniFile::DpIniFile()
{
    _nGroups = 0;
    _pGroupList = NULL;
    _nCurrentGroup = 0;
    
}

//------------------------------------------------------------------------------
// CJW: Deconstructor.  Clear out any resources that we have been allocated.
DpIniFile::~DpIniFile()
{
    while(_nGroups > 0) {
        _nGroups--;
        ASSERT(_pGroupList != NULL);
        delete _pGroupList[_nGroups];
        _pGroupList[_nGroups] = NULL;
    }
    
    if (_pGroupList != NULL) {
        free(_pGroupList);
        _pGroupList = NULL;
    }
}


//------------------------------------------------------------------------------
// CJW: Load the file into an array.  We want to save the name of the file, and
//      we will read in all the stuff, ignoring comments, setting groups, etc.
bool DpIniFile::Load(char *path)
{
    bool ok = false;
    FILE *fp;
    char *buffer = NULL, ch;
    int length = 0;
    int len;
    int count;
    char szGroup[256], szName[256], szValue[5000];
    
    enum {
        unknown,
        newline,
        comment,
        group,
        name,
        value,
        eol,
        fail
    } state = unknown;
    
    ASSERT(path != NULL);
    fp = fopen(path, "r");
    if (fp != NULL) {
    
        // first load the entire contents of the file into a buffer.
        buffer = (char *) malloc(1024);
        len = fread(buffer, 1, 1024, fp);
        while(len > 0) {
            length += len;
            buffer = (char *) realloc(buffer, length+1024);
            len = fread(&buffer[length], 1, 1024, fp);
        }
        fclose(fp);
    
        // then go thru the buffer looking for groups.
        state = newline;
        len = 0;
        szGroup[0] = '\0';
        for(count=0; count<length; count++) {
            ch = buffer[count];
    
            if (ch != '\r') {
    
                switch(state) {
                    case newline:
                        if (ch == '[') {
                            state = group;
                        }
                        else if (ch == '#') {
                            state = comment;
                        }
                        else if (ch != ' ' && ch != '\n') {
                            state = name;
                            szName[0] = ch;
                            szName[1] = '\0';
                            len = 1;
                        }
                        break;
    
                    case group:
                        if (ch == ']') {
                            // when a group is found, we need to then create a group object
                            _pGroupList = (DpIniFileGroup **) realloc(_pGroupList, sizeof(DpIniFileGroup *) * (_nGroups + 1));
                            _pGroupList[_nGroups] = new DpIniFileGroup;
                            _pGroupList[_nGroups]->SetGroup(szGroup);
                            _nGroups++;
                            state = comment;
    
                            szGroup[0] = '\0';
                            len = 0;
                        }
                        else if (ch == '#' || ch == '\n') {
                            state = fail;
                            count = length;
                        }
                        else {
                            ASSERT(len < 256);
                            szGroup[len+1] = 0;
                            szGroup[len] = ch;
                            len++;
                        }
    
                        break;
    
                    case name:
                        if (ch == '=') {
                            state = value;
                            szValue[0] = '\0';
                            len = 0;
                        }
                        else if (ch == '#' || ch == '\n') {
                            state = fail;
                            count = length;
                        }
                        else {
                            if (_nGroups <= 0) {
                                state = fail;
                                count = length;
                            }
                            else {
                                ASSERT(len < 256);
                                szName[len+1] = 0;
                                szName[len] = ch;
                                len++;
                            }
                        }
                        break;
    
                    case value:
                        if (ch == '\n' || ch == '#') {
                            if (ch == '#') {
                                state = comment;
                            }
                            else {
                                state = newline;
                            }
    
                            ASSERT(_nGroups > 0);
                            _pGroupList[_nGroups-1]->Add(szName, szValue);
    
                            szGroup[0] = '\0';
                            len = 0;
                        }
                        else {
                            ASSERT(len < 5000);
                            szValue[len+1] = 0;
                            szValue[len] = ch;
                            len++;
                        }
                        break;

                    case comment:
                        if (ch == '\n') {
                            state = newline;
                        }
                        break;
    
                    default:
                        break;
                }
            }
        }
    
        if (state != fail) {
            ok = true;
        }
    
        if (buffer != NULL) {
            free(buffer);
        }
    }
    
    return(ok);
}


//------------------------------------------------------------------------------
// CJW: Go thru the list of groups and find the one that this matches and
//      record the index.  If it is found, then return true, otherwise return
//      false.  if grp is NULL, then we will set the current group to the first
//      one in the file.
bool DpIniFile::SetGroup(char *grp)
{
    bool ok = false;
    int nCount;
    char *tmp;
    
    ASSERT(_nGroups > 0);
    ASSERT(_pGroupList != NULL);
    
    if (grp == NULL) {
        _nCurrentGroup = 0;
        ok = true;
    }
    else {
        for(nCount = 0; nCount < _nGroups; nCount++) {
            ASSERT(_pGroupList[nCount] != NULL);
            tmp = _pGroupList[nCount]->GroupName();
            ASSERT(tmp != NULL);
            if (strcmp(grp, tmp) == 0) {
                _nCurrentGroup = nCount;
                ok = true;
                nCount = _nGroups;
            }
        }
    }
    
    return(ok);
}


//------------------------------------------------------------------------------
bool DpIniFile::GetValue(char *name, int *value)
{
    bool ok = false;
    char *buffer;
    
    ASSERT(name != NULL);
    ASSERT(value != NULL);
    
    ok = GetValue(name, &buffer);
    if (ok == true) {
        if (buffer == NULL) {
            *value = 0;
        }
        else {
            *value = atoi(buffer);
            free(buffer);
        }
    }
    
    return(ok);
}

//------------------------------------------------------------------------------
bool DpIniFile::GetValue(char *name, char **value)
{
    bool ok = false;
    char *tmp;
    
    ASSERT(name != NULL);
    ASSERT(value != NULL);
    
    ASSERT(_nGroups > 0);
    ASSERT(_pGroupList != NULL);
    
    ASSERT(_nCurrentGroup < _nGroups);
    ASSERT(_nCurrentGroup >= 0);
    
    tmp = _pGroupList[_nCurrentGroup]->GetValue(name);
    if (tmp != NULL) {
        *value = (char *) malloc(strlen(tmp)+1);
        strcpy(*value, tmp);
        ok = true;
    }
    
    return(ok);
}



DpIniFileGroup::DpIniFileGroup()
{
    _szGroup = NULL;
    _nItems = 0;
    _pItemList = NULL;
}

DpIniFileGroup::~DpIniFileGroup()
{
    if (_szGroup != NULL) {
        free(_szGroup);
    }
    
    while(_nItems > 0) {
        ASSERT(_pItemList != NULL);
        _nItems--;
        if (_pItemList[_nItems].szName != NULL) {
            free(_pItemList[_nItems].szName);
        }
        if (_pItemList[_nItems].szValue != NULL) {
            free(_pItemList[_nItems].szValue);
        }
    }
    
    if (_pItemList != NULL) {
        free(_pItemList);
    }
}

void DpIniFileGroup::SetGroup(char *grp)
{
    ASSERT(grp != NULL);
    ASSERT(_szGroup == NULL);
    
    _szGroup = (char *) malloc(strlen(grp)+1);
    strcpy(_szGroup, grp);
}

void DpIniFileGroup::Add(char *name, char *value)
{
    ASSERT(name != NULL);
    ASSERT(value != NULL);
    
    if (_nItems == 0) {
        _pItemList = (struct DpIniFileItemData *) malloc(sizeof(struct DpIniFileItemData));
    }
    else {
        _pItemList = (struct DpIniFileItemData *) realloc(_pItemList, sizeof(struct DpIniFileItemData) * (_nItems+1));
    }
    
    if (_pItemList == NULL) {
        _nItems = 0;
    }
    else {
        _pItemList[_nItems].szName = (char *) malloc(strlen(name) + 1);
        strcpy(_pItemList[_nItems].szName, name);
    
        _pItemList[_nItems].szValue = (char *) malloc(strlen(value) + 1);
        strcpy(_pItemList[_nItems].szValue, value);
    
        _nItems ++;
    }
}

char *DpIniFileGroup::GroupName(void)
{
    ASSERT(_szGroup != NULL);
    return(_szGroup);
}

char *DpIniFileGroup::GetValue(char *name)
{
    char *value=NULL;
    char *tmp;
    int nCount;
    
    ASSERT(name != NULL);
    
    ASSERT(_nItems > 0);
    ASSERT(_pItemList != NULL);
    
    for(nCount=0; nCount<_nItems; nCount++) {
        tmp = _pItemList[nCount].szName;
        ASSERT(tmp != NULL);
        if (strcmp(name, tmp) == 0) {
            value = _pItemList[nCount].szValue;
            nCount = _nItems;
            ASSERT(value != NULL);
        }
    }
    
    return(value);
}




