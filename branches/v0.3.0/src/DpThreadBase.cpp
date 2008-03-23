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

#include <DpThreadBase.h>


#ifdef __GNUC__
    #include <time.h>
#endif


//------------------------------------------------------------------------------
// CJW: Constructor for DpThreadbase... nothing so far.
DpThreadBase::DpThreadBase()
{
}


//------------------------------------------------------------------------------
DpThreadBase::~DpThreadBase()
{
}


//------------------------------------------------------------------------------
// CJW: If we are using GNUC then we need to provide our own sleep function
//      that uses miliseconds.
#ifdef __GNUC__
void DpThreadBase::Sleep(DWORD dTime)
{
    struct timespec strTime;
    
	if (dTime < 1000) 	{ strTime.tv_sec = 0; }
	else 				{ strTime.tv_sec = dTime / 1000; }
	
	strTime.tv_nsec = (dTime % 1000) * 1000000;
    
    nanosleep(&strTime, NULL);
}
#endif



