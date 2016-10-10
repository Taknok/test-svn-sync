/****************************************************************************

    CRectangle Class
	Copyright (C) 2008 Andre Deperrois adeperrois@xflr5.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*****************************************************************************/


#ifndef CRECTANGLE_H
#define CRECTANGLE_H




#include "CVector.h"

class CRectangle
{
public:
	CRectangle(void)
	{
			left   = 0;
			right  = 0;
			top    = 0;
			bottom = 0;
	};

	CRectangle(double const &l, double const &t, double const&r, double const&b)
	{
			left   = l;
			right  = r;
			top    = t;
			bottom = b;
	};

	CRectangle(CVector const &TopLeft, CVector const &BottomRight)
	{
			left   = TopLeft.x;
			right  = BottomRight.x;
			top    = TopLeft.y;
			bottom = BottomRight.y;
	};

	void copyRect(CRectangle *pRect)
	{
		left   = pRect->left;
		right  = pRect->right;
		top    = pRect->top;
		bottom = pRect->bottom;
	};


	CRectangle(CRectangle const &Rect)
	{
		left   = Rect.left;
		right  = Rect.right;
		top    = Rect.top;
		bottom = Rect.bottom;
	}
//	~CRectangle(void);


	bool isRectEmpty()
	{
		if(bottom==top && right==left) return true;
		else                           return false;
	};
	bool ptInRect(CVector const &pt)
	{
		if(left<pt.x && pt.x<right && bottom<pt.y && pt.y<top ) return true;
		return false;
	};
	double width(){return (right-left);};
	double height(){return(top-bottom);};
	void setRectEmpty(){left = right = top = bottom = 0;};
	void deflateRect(double const &x, double const&y)
	{
		//DeflateRect adds units to the left and top and subtracts units from the right and bottom
		left   +=x;
		right  -=x;
		top    +=y;
		bottom -=y;
	};

	void deflateRect(double const &l, double const &t, double const&r, double const&b)
	{
		//DeflateRect adds units to the left and top and subtracts units from the right and bottom
		left   +=l;
		right  -=r;
		top    +=t;
		bottom -=b;
	};

	void inflateRect(double const &x, double const&y)
	{
		//InflateRect subtracts units from the left and top and adds units to the right and bottom
		left   -=x;
		right  +=x;
		top    -=y;
		bottom +=y;
	};
	void inflateRect(double const &l, double const &t, double const&r, double const&b)
	{
		//InflateRect subtracts units from the left and top and adds units to the right and bottom
		left   -=l;
		right  +=r;
		top    -=t;
		bottom +=b;
	};
	void setRect(double const &l, double const &t, double const&r, double const&b)
	{
		left   =l;
		right  =r;
		top    =t;
		bottom =b;
	};
	void setRect(CRectangle const &Rect)
	{
		left   = Rect.left;
		right  = Rect.right;
		top    = Rect.top;
		bottom = Rect.bottom;
	};
	void normalizeRect()
	{
		double tmp;
		if (left > right)
		{
			tmp = left;
			left = right;
			right = tmp;
		}
		if(bottom>top)
		{
			tmp = bottom;
			bottom = top;
			top = tmp;
		}
	};

	double left;
	double top;
	double right;
	double bottom;
};
#endif
