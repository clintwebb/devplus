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
 

#ifndef __DP_HTTPCLIENT_H
#define __DP_HTTPCLIENT_H

#include <DevPlus.h>
#include <DpSocket.h>
#include <DpDataQueue.h>

//------------------------------------------------------------------------------
// CJW: This class provides and easy to use and intuitive HTTP interface client 
//      object.
//------------------------------------------------------------------------------

class DpHttpClient
{
    // functions, and methods.
    public:
        DpHttpClient();
        virtual ~DpHttpClient();

		void SetURL(char *url);
// 		void SetMethod(char *method);
		void AddParam(char *name, char *value);
		void AddParam(char *name, int value);
		char * GetURL(void);
				
        int Get(char *url=NULL);
        int GetLength(void) { return(_nLength); }
        int GetCode(void)   { return(_nCode);   }
		DpDataQueue * GetBody();
		
		char * Encode(char *str);

    protected:

    private:
        bool ParseUrl(void);
        void BuildHeader(void);
        bool SendHeader(void);
        bool ReceiveHeader(void); 
        void Sleep(void);

    // data and objects.
    public:

    protected:

    private:
        DpSocket _xSocket;
        DpDataQueue _xQueue;
        char *_szUrl;
        char *_szHeaders;
		char *_pBody;
        char  _szHost[DP_MAX_HOST_LEN+1];
		int   _nPort;
		int   _nCode, _nLength;
};



#endif
