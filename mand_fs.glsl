#version 410

out vec4 colour;
uniform int width, height;
uniform double scale, x_shift, y_shift;
uniform int iterations;

void main(void) {
    double cx = 1.33333333333333333333333333 * (gl_FragCoord.x/width - 0.5) / scale + x_shift;

    double cy = (gl_FragCoord.y/height - 0.5) / scale + y_shift;

    double zx = cx;
    double zy = cy;

    int k;
    for(k = 0; k < iterations; k++) {

        double x = (zx * zx - zy * zy) + cx;
        double y = (zy * zx + zx * zy) + cy;

        if((x*x + y*y) > 4.0)
            break;

        zx = x;
        zy = y;
    }

    float i = float(k);
    float n = float(iterations);


    double pxc = k == iterations ? 0.0 : double(k)/(double(iterations)/2.56);

    colour = vec4(1.6*pxc, 1.1*pxc, 0 , 1.0);

    // you can customise you color like you like.
    /*

    if(k == iterations)
    {
        colour = vec4(0.0, 0.0, 0.0 , 1.0);
    }
    else if(k > (iterations/2) )
    {
        float d = i / (n);
        //colour = vec4(1,0,0,1);
        colour = vec4(d/2, d/1.3, d , 1.0);
    }
    else if(k > (iterations/5) )
    {
        float d = i / (n/2);
        //colour = vec4(0, 1,0,1);
        colour = vec4(d, 2.0*d, 2.0*d , 1.0);
    }
    else if(k > (iterations/25) )
    {
        float d = i / (n/2);
        //colour = vec4(0, 0,1,1);
        colour = vec4(d, 2.0*d, 2.0*d , 1.0);
    }
    else
    {
        colour = vec4(1.0,1.0,1.0,1.0);
        //colour = vec4(0, d, d , 1.0);
    }
    */
}
