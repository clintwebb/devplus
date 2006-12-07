//------------------------------------------------------------------------------
//  CJDJ Creations
//  DevPlus C++ Library.
//
/***************************************************************************
 *   Copyright (C) 2003-2005 by Clinton Webb,,,                            *
 *   devplus@cjdj.org                                                      *
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
      **    http://cjdj.org/products/devplus/                       **
      **                                                            **
      ****************************************************************
*/ 
 
/*
    Description:
        DevPlus is a bunch of classes that maintain a sensible interface as 
        closely as possible throughout the various classes and functions.  It 
        is designed to be a powerful substitute or enhancement to the various 
        incompatible methods within MFC and other libraries.  

        DevPlus is intended to be compiled into the application, rather than 
        linked in.  This has some added advantages.  Of course, if you wanted 
        you could create a library out of it and link it in however you want.

        DevPlus is provided as Source Code, but that does not mean that it is 
        without limitations.  DevPlus can only be used in accordance with the 
        licence you choose.
        
    Versions
        See the ChangeLog file for a description of all versions and changes.

        
  ------------------------------------------------------------------------------
*/

#ifndef __DEVPLUS_H
#define __DEVPLUS_H


//------------------------------------------------------------------------------
// Provide our assertion mapping function.  This would also depend eventually on 
// what compiler we are using to compile with.  VC++ uses a graphical assertion, 
// where DigitalMars provides an application stop assertion.
#ifndef ASSERT 
	#include <assert.h>
	#define ASSERT(x) assert(x);
#endif






//------------------------------------------------------------------------------
// CJW: Global defines go in here that affect the over-all compilation of all 
// 		DevPlus components.


#define DP_MAX_PACKET_SIZE 4096
#define DP_MAX_HOST_LEN 255


#define DP_SERVER_ACCEPT_CYCLES	10



#endif  // __DEVPLUS_H
