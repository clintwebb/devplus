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

#include <DpPipeCommand.h>


//-----------------------------------------------------------------------------
// CJW: Constructor.  
DpPipeCommand::DpPipeCommand()
{
	Lock();
	_bStop = false;
	Unlock();
}


//-----------------------------------------------------------------------------
// CJW: Deconstructor.  
DpPipeCommand::~DpPipeCommand()
{
	Lock();
	ASSERT(_bStop == false);
	_bStop = true;
	Unlock();
	
	// **** How do we signal to the thread that it needs to break out of the block waiting for the file?
	WaitForThread();
}


//-----------------------------------------------------------------------------
// CJW: We have virtualised this function in case we need to add any 
// 		functionality to it.  Originally it was planned that the pipe path 
// 		would be specified and then Start() issued.  But instead, we will just 
// 		use a single Pipe() function that passes in the pathname and that 
// 		starts the thread....
void DpPipeCommand::Start(void)
{
	DpThreadObject::Start();
}


//-----------------------------------------------------------------------------
// CJW: This is the thread.  Basically it attempts to open the piped file. 
// 		When it opens, it will read all teh data from it, and then close it.
void DpPipeCommand::OnThreadRun(void)
{
	Lock();
	while(_bStop == false) {
		Unlock();
		
		// attempt to open the named-pipe file.
		// read in all the data for the name-pipe file.
		// close the file-handle.		
		
		Lock();
	}
	Unlock();
}


