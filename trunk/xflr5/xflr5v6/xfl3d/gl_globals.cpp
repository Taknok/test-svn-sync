/****************************************************************************

    GL_Globals
    Copyright (C) André Deperrois

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

#include <QVector>

#include "gl_globals.h"


#include <xflgeom/geom3d/vector3d.h>

void glMakeCube(Vector3d const &pt, double dx, double dy, double dz,
                QOpenGLBuffer &vboFaces, QOpenGLBuffer &vboEdges)
{
    // 12 triangles
    // 3 vertices/triangle
    // (3 position + 3 normal) components/vertex
    int buffersize = 12 *3 * 6;
    QVector<GLfloat> CubeVertexArray(buffersize, 0);

    // 8 vertices
    Vector3d T000 = {pt.x-dx/2, pt.y-dy/2, pt.z-dz/2};
    Vector3d T001 = {pt.x-dx/2, pt.y-dy/2, pt.z+dz/2};
    Vector3d T010 = {pt.x-dx/2, pt.y+dy/2, pt.z-dz/2};
    Vector3d T011 = {pt.x-dx/2, pt.y+dy/2, pt.z+dz/2};
    Vector3d T100 = {pt.x+dx/2, pt.y-dy/2, pt.z-dz/2};
    Vector3d T101 = {pt.x+dx/2, pt.y-dy/2, pt.z+dz/2};
    Vector3d T110 = {pt.x+dx/2, pt.y+dy/2, pt.z-dz/2};
    Vector3d T111 = {pt.x+dx/2, pt.y+dy/2, pt.z+dz/2};

    Vector3d N;

    int iv = 0;
    // X- face
    N.set(-1,0,0);
    //   first triangle
    CubeVertexArray[iv++] = T000.xf();
    CubeVertexArray[iv++] = T000.yf();
    CubeVertexArray[iv++] = T000.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T001.xf();
    CubeVertexArray[iv++] = T001.yf();
    CubeVertexArray[iv++] = T001.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T010.xf();
    CubeVertexArray[iv++] = T010.yf();
    CubeVertexArray[iv++] = T010.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();

    //   second triangle
    CubeVertexArray[iv++] = T010.xf();
    CubeVertexArray[iv++] = T010.yf();
    CubeVertexArray[iv++] = T010.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T001.xf();
    CubeVertexArray[iv++] = T001.yf();
    CubeVertexArray[iv++] = T001.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T011.xf();
    CubeVertexArray[iv++] = T011.yf();
    CubeVertexArray[iv++] = T011.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();


    // X+ face
    N.set(1,0,0);
    //   first triangle
    CubeVertexArray[iv++] = T110.xf();
    CubeVertexArray[iv++] = T110.yf();
    CubeVertexArray[iv++] = T110.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T101.xf();
    CubeVertexArray[iv++] = T101.yf();
    CubeVertexArray[iv++] = T101.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T100.xf();
    CubeVertexArray[iv++] = T100.yf();
    CubeVertexArray[iv++] = T100.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();

    //   second triangle
    CubeVertexArray[iv++] = T110.xf();
    CubeVertexArray[iv++] = T110.yf();
    CubeVertexArray[iv++] = T110.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T111.xf();
    CubeVertexArray[iv++] = T111.yf();
    CubeVertexArray[iv++] = T111.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T101.xf();
    CubeVertexArray[iv++] = T101.yf();
    CubeVertexArray[iv++] = T101.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();

    // Y- face
    N.set(0,-1,0);
    //   first triangle
    CubeVertexArray[iv++] = T000.xf();
    CubeVertexArray[iv++] = T000.yf();
    CubeVertexArray[iv++] = T000.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T100.xf();
    CubeVertexArray[iv++] = T100.yf();
    CubeVertexArray[iv++] = T100.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T001.xf();
    CubeVertexArray[iv++] = T001.yf();
    CubeVertexArray[iv++] = T001.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    //   second triangle
    CubeVertexArray[iv++] = T100.xf();
    CubeVertexArray[iv++] = T100.yf();
    CubeVertexArray[iv++] = T100.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T101.xf();
    CubeVertexArray[iv++] = T101.yf();
    CubeVertexArray[iv++] = T101.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T001.xf();
    CubeVertexArray[iv++] = T001.yf();
    CubeVertexArray[iv++] = T001.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();

    // Y+ face
    N.set(0,1,0);
    //   first triangle
    CubeVertexArray[iv++] = T010.xf();
    CubeVertexArray[iv++] = T010.yf();
    CubeVertexArray[iv++] = T010.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T011.xf();
    CubeVertexArray[iv++] = T011.yf();
    CubeVertexArray[iv++] = T011.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T110.xf();
    CubeVertexArray[iv++] = T110.yf();
    CubeVertexArray[iv++] = T110.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    //   second triangle
    CubeVertexArray[iv++] = T110.xf();
    CubeVertexArray[iv++] = T110.yf();
    CubeVertexArray[iv++] = T110.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T011.xf();
    CubeVertexArray[iv++] = T011.yf();
    CubeVertexArray[iv++] = T011.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T111.xf();
    CubeVertexArray[iv++] = T111.yf();
    CubeVertexArray[iv++] = T111.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();


    // Z- face
    N.set(0,0,-1);
    //   first triangle
    CubeVertexArray[iv++] = T000.xf();
    CubeVertexArray[iv++] = T000.yf();
    CubeVertexArray[iv++] = T000.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T010.xf();
    CubeVertexArray[iv++] = T010.yf();
    CubeVertexArray[iv++] = T010.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T100.xf();
    CubeVertexArray[iv++] = T100.yf();
    CubeVertexArray[iv++] = T100.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    //   second triangle
    CubeVertexArray[iv++] = T010.xf();
    CubeVertexArray[iv++] = T010.yf();
    CubeVertexArray[iv++] = T010.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T110.xf();
    CubeVertexArray[iv++] = T110.yf();
    CubeVertexArray[iv++] = T110.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T100.xf();
    CubeVertexArray[iv++] = T100.yf();
    CubeVertexArray[iv++] = T100.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();

    // Z+ face
    N.set(0,0,1);
    //   first triangle
    CubeVertexArray[iv++] = T001.xf();
    CubeVertexArray[iv++] = T001.yf();
    CubeVertexArray[iv++] = T001.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T101.xf();
    CubeVertexArray[iv++] = T101.yf();
    CubeVertexArray[iv++] = T101.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T011.xf();
    CubeVertexArray[iv++] = T011.yf();
    CubeVertexArray[iv++] = T011.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    //   second triangle
    CubeVertexArray[iv++] = T101.xf();
    CubeVertexArray[iv++] = T101.yf();
    CubeVertexArray[iv++] = T101.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T111.xf();
    CubeVertexArray[iv++] = T111.yf();
    CubeVertexArray[iv++] = T111.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();
    CubeVertexArray[iv++] = T011.xf();
    CubeVertexArray[iv++] = T011.yf();
    CubeVertexArray[iv++] = T011.zf();
    CubeVertexArray[iv++] = N.xf();
    CubeVertexArray[iv++] = N.yf();
    CubeVertexArray[iv++] = N.zf();

    Q_ASSERT(iv==buffersize);

    vboFaces.destroy();
    vboFaces.create();
    vboFaces.bind();
    vboFaces.allocate(CubeVertexArray.data(), buffersize* int(sizeof(GLfloat)));
    vboFaces.release();

    buffersize = 12 * 2 *3; //12 edges x2 vertices x3 components
    QVector<float> EdgeVertexArray(buffersize);
    iv=0;

    //bottom face
    {
        EdgeVertexArray[iv++] = T000.xf();
        EdgeVertexArray[iv++] = T000.yf();
        EdgeVertexArray[iv++] = T000.zf();
        EdgeVertexArray[iv++] = T100.xf();
        EdgeVertexArray[iv++] = T100.yf();
        EdgeVertexArray[iv++] = T100.zf();

        EdgeVertexArray[iv++] = T100.xf();
        EdgeVertexArray[iv++] = T100.yf();
        EdgeVertexArray[iv++] = T100.zf();
        EdgeVertexArray[iv++] = T110.xf();
        EdgeVertexArray[iv++] = T110.yf();
        EdgeVertexArray[iv++] = T110.zf();

        EdgeVertexArray[iv++] = T110.xf();
        EdgeVertexArray[iv++] = T110.yf();
        EdgeVertexArray[iv++] = T110.zf();
        EdgeVertexArray[iv++] = T010.xf();
        EdgeVertexArray[iv++] = T010.yf();
        EdgeVertexArray[iv++] = T010.zf();

        EdgeVertexArray[iv++] = T010.xf();
        EdgeVertexArray[iv++] = T010.yf();
        EdgeVertexArray[iv++] = T010.zf();
        EdgeVertexArray[iv++] = T000.xf();
        EdgeVertexArray[iv++] = T000.yf();
        EdgeVertexArray[iv++] = T000.zf();
    }

    //top face
    {
        EdgeVertexArray[iv++] = T001.xf();
        EdgeVertexArray[iv++] = T001.yf();
        EdgeVertexArray[iv++] = T001.zf();
        EdgeVertexArray[iv++] = T101.xf();
        EdgeVertexArray[iv++] = T101.yf();
        EdgeVertexArray[iv++] = T101.zf();

        EdgeVertexArray[iv++] = T101.xf();
        EdgeVertexArray[iv++] = T101.yf();
        EdgeVertexArray[iv++] = T101.zf();
        EdgeVertexArray[iv++] = T111.xf();
        EdgeVertexArray[iv++] = T111.yf();
        EdgeVertexArray[iv++] = T111.zf();

        EdgeVertexArray[iv++] = T111.xf();
        EdgeVertexArray[iv++] = T111.yf();
        EdgeVertexArray[iv++] = T111.zf();
        EdgeVertexArray[iv++] = T011.xf();
        EdgeVertexArray[iv++] = T011.yf();
        EdgeVertexArray[iv++] = T011.zf();

        EdgeVertexArray[iv++] = T011.xf();
        EdgeVertexArray[iv++] = T011.yf();
        EdgeVertexArray[iv++] = T011.zf();
        EdgeVertexArray[iv++] = T001.xf();
        EdgeVertexArray[iv++] = T001.yf();
        EdgeVertexArray[iv++] = T001.zf();
    }

    //lateral edges
    {
        EdgeVertexArray[iv++] = T000.xf();
        EdgeVertexArray[iv++] = T000.yf();
        EdgeVertexArray[iv++] = T000.zf();
        EdgeVertexArray[iv++] = T001.xf();
        EdgeVertexArray[iv++] = T001.yf();
        EdgeVertexArray[iv++] = T001.zf();

        EdgeVertexArray[iv++] = T100.xf();
        EdgeVertexArray[iv++] = T100.yf();
        EdgeVertexArray[iv++] = T100.zf();
        EdgeVertexArray[iv++] = T101.xf();
        EdgeVertexArray[iv++] = T101.yf();
        EdgeVertexArray[iv++] = T101.zf();

        EdgeVertexArray[iv++] = T110.xf();
        EdgeVertexArray[iv++] = T110.yf();
        EdgeVertexArray[iv++] = T110.zf();
        EdgeVertexArray[iv++] = T111.xf();
        EdgeVertexArray[iv++] = T111.yf();
        EdgeVertexArray[iv++] = T111.zf();

        EdgeVertexArray[iv++] = T010.xf();
        EdgeVertexArray[iv++] = T010.yf();
        EdgeVertexArray[iv++] = T010.zf();
        EdgeVertexArray[iv++] = T011.xf();
        EdgeVertexArray[iv++] = T011.yf();
        EdgeVertexArray[iv++] = T011.zf();
    }

    Q_ASSERT(iv==buffersize);

    vboEdges.destroy();
    vboEdges.create();
    vboEdges.bind();
    vboEdges.allocate(EdgeVertexArray.data(), buffersize* int(sizeof(GLfloat)));
    vboEdges.release();
}

