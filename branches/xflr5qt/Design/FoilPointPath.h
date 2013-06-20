/***************************************************************************
 *   Copyright (C) 2007 by Andrés Chavarría Krauser  *
 *   el_andrecillo@users.sourceforge.net   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef FOILPOINTPATH_H
#define FOILPOINTPATH_H

#include "FoilPath.h"


class FoilPointPath : public FoilPath
{
public:
		FoilPointPath(){;}
    FoilPointPath(Foil &foil,double alpha, double scalex, double scaley,
 					 QPoint Offset, QRect DrawRect): FoilPath(foil,alpha,scalex,scaley,Offset,DrawRect){;};
	virtual ~FoilPointPath(){;};

	virtual void CreatePath(Foil &foil,double alpha, double scalex, double scaley,QPoint Offset, QRectF DrawRect){
		if(foil.Empty())return;
		drawRect=DrawRect;
		offset=Offset;
		Scalex=scalex;
		Scaley=scaley;
		Alpha=alpha;


		int width=2;
		double xa,ya;
		const double cosa = cos(ToRad(alpha));
		const double sina = sin(ToRad(alpha));

		std::vector<double>::const_iterator iterX=foil.X().begin();
		std::vector<double>::const_iterator iterY=foil.Y().begin();

		while(iterX!=foil.X().end()&&iterY!=foil.Y().end()){
			xa = (((*iterX)-0.5)*cosa - (*iterY)*sina + 0.5)*scalex+Offset.x();
			ya = -(((*iterX)-0.5)*sina + (*iterY)*cosa)*scaley+Offset.y();
			QRectF pointRect(xa-width, ya-width, 2*width, 2*width);

			/*if(DrawRect.contains(pointRect))*/addRect(pointRect);
			++iterX; ++iterY;
		}
	};
};



#endif



