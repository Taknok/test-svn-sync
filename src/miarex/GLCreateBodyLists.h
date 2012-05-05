/****************************************************************************

	GLCreateLists 

	Copyright (C) 2012 Andre Deperrois XFLR5@yahoo.com
		 All rights reserved

*****************************************************************************/


#ifndef GLCREATEBODYLISTS_H
#define GLCREATEBODYLISTS_H

#include <QtOpenGL>
#include "../objects/Body.h"
#include "../params.h"

void GLCreateBody3DFlatPanels(void *pParent, int iList, CBody *pBody);
void GLCreateBody3DSplines(void *pParent, int iList, CBody *pBody);
void GLCreateBodyMesh(void *pParent, int iList, CBody *pBody);

#endif
