#version 330

uniform int julia;
uniform float paramx; // the real part of the Julia set parameter
uniform float paramy; // the imag part of the Julia set parameter

uniform int maxiters;
uniform float maxlength;

in vec2 pos;

layout(location = 0) out vec4 FragmentColor;


void main(void)
{
    float ptx=0.0;
    float pty=0.0;
    float ptx2=0.0;
    float pty2=0.0;
    float tmpx=0.0f;

    if(julia==1)
    {
        ptx=pos.x;
        pty=pos.y;
        ptx2=pos.x*pos.x;
        pty2=pos.y*pos.y;
    }

    int iter=0;
    do
    {
        ptx2 = ptx*ptx;
        pty2 = pty*pty;
        tmpx = ptx;
        if(julia==1)
        {
            ptx = ptx2 - pty2  + paramx;
            pty = 2.0*tmpx*pty + paramy;
        }
        else
        {
            ptx = ptx2 - pty2  + pos.x;
            pty = 2.0*tmpx*pty + pos.y;
        }
        iter++;
    }
    while(ptx2 + pty2<maxlength*maxlength && iter<maxiters);

    if(iter == maxiters) FragmentColor = vec4(0,0,0,1);
//    else                 FragmentColor = vec4(float(iter)/float(maxiters), 0, 0, 1);
    else                 FragmentColor = vec4(min(32.0, float(iter))/32.0, 0, 0, 1);
}


