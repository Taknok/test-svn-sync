/****************************************************************************

    gl3dWingView Class
    Copyright (C) 2016 Andre Deperrois

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

#pragma once

#include <xfl3d/gl3dview.h>

class Wing;
class GL3dWingDlg;

class gl3dWingView : public gl3dView
{
    public:
        gl3dWingView(QWidget *pParent = nullptr);
        void setWing(Wing const*pWing);
        void glMakeWingSectionHighlight(Wing const *pWing, int iSectionHighLight, bool bRightSide);

        void resetglWing() {m_bResetglWing=true;}
        void resetglHighlight() {m_bResetglSectionHighlight=true;}

    private:
        void glRenderView() override;

        void set3DRotationCenter(QPoint point) override;
        void glMake3dObjects() override;

    public slots:
        void on3DReset() override;

    private:
        Wing const*m_pWing;
        GL3dWingDlg *m_pGL3dWingDlg;

        bool m_bResetglWing;
        bool m_bResetglSectionHighlight;

};

