#version 330

uniform int maxiters;
uniform float maxlength;

in vec2 pos;

layout(location = 0) out vec4 FragmentColor;


void main(void)
{
//    The Mandelbrot set is the set of complex numbers c for which the function f_c(z) = z²+c
//    does not diverge when iterated from z=0
    float ptx, pty, ptx2, pty2;
    float tmpx;
    int iter=0;
    do
    {
        ptx2 = ptx*ptx;
        pty2 = pty*pty;
        tmpx = ptx;
        ptx = ptx2 - pty2  + float(pos.x);
        pty = 2.0*tmpx*pty + float(pos.y);
        iter++;
    }
    while(ptx2 + pty2<maxlength*maxlength && iter<maxiters);

    if(iter == maxiters) FragmentColor = vec4(0,0,0,1);
    else                 FragmentColor = vec4(float(iter)/float(maxiters), 0, 0, 1);
}
