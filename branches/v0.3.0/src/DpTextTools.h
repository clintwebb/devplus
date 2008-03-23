//------------------------------------------------------------------------------
//  CJDJ Creations
//  DevPlus C++ Library.
//  
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
 
 
/*
      ****************************************************************
      **                                                            **
      **  NOTE                                                      **
      **  ----                                                      **
      **                                                            **
      **  A commercial licence of this library is available if the  **
      **  GPL licence is not suitable for your task.                **
      **                                                            **
      **  This means that if you are developing a project and       **
      **  intend to release it under the GPL or equivilent , you do **
      **  NOT need to purchase a commercial licence.                **
      **                                                            **
      **  But if you do not intend to distribute your product under **
      **  the GPL, you are required to purchase a commercial        **
      **  licence.  This licence allows you to use this DevPlus     **
      **  library without providing the source.                     **
      **                                                            **
      **  You can purchase a commercial licence at:                 **
      **    http://hyper-active.com.au/products/devplus/            **
      **                                                            **
      ****************************************************************
*/ 
 

#ifndef __DP_TEXTTOOLS_H
#define __DP_TEXTTOOLS_H

#include <DevPlus.h>

#define TEXT_MAX_LINE_LENGTH    4096

struct DpTextLine_t {
    char *data;
    int length;
};


class DpTextTools 
{
    // Methods, functions.
    public:
        DpTextTools();
        ~DpTextTools();

        bool Load(char *buffer);
        bool LoadFromFile(char *file);

        int FindLine(char *txt);
        int MoveNextLine(void);
        int TrimBefore(void);
        int DeleteToEnd(void);
        int MoveFirstLine(void);
        int InsertLineOnStr(char *str);
        int InsertNewLine(void);
        void RemoveHtmlOnLine(bool spaces);
        int DeleteFromLine(char *str);

        char **GetWordArray(void);

    protected:

    private:
        void ClearWordArray(void);


    // Data, internal objects.
    public:
    protected:
    private:
        DpTextLine_t *_pData;
        int           _nItems;
        char         *_pWordArray;
        int           _nCurrentLine;
        int           _nCurrentChar;
};





#endif
