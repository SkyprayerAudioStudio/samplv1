// samplv1widget_status.h
//
/****************************************************************************
   Copyright (C) 2012, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#ifndef __samplv1widget_status_h
#define __samplv1widget_status_h

#include <QStatusBar>

// Forward declarations.
class QLabel;


//-------------------------------------------------------------------------
// samplv1widget_status - Custom status-bar widget.

class samplv1widget_status : public QStatusBar
{
	Q_OBJECT

public:

	// Constructor.
	samplv1widget_status(QWidget *pParent = 0);
};


#endif  // __samplv1widget_status_h

// end of samplv1widget_status.h
