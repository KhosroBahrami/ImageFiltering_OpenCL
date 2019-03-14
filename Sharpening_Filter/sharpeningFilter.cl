
__constant sampler_t image_sampler = CLK_NORMALIZED_COORDS_FALSE
     | CLK_ADDRESS_CLAMP_TO_EDGE| CLK_FILTER_NEAREST;


__kernel void sharpeningFilter(__read_only image2d_t iimage,
     __write_only image2d_t oimage, __constant float *filter, int windowSize)
{

     unsigned int x = get_global_id(0);
     unsigned int y = get_global_id(1);
     int halfWindow = windowSize/2;
     float4 pixelValue;
     float4 computedFilter = 0.0f;
     int i , j, ifilter, jfilter;
     for(i=-halfWindow, ifilter=0; i<=halfWindow; i++, ifilter++)
     {
         for(j=-halfWindow, jfilter=0; j<=halfWindow; j++, jfilter++)
         { 
             pixelValue = read_imagef(iimage, image_sampler, (int2)(x+i, y+j));
             computedFilter += filter[ifilter*windowSize+jfilter]*pixelValue;
         }
     }
     write_imagef(oimage, (int2)(x, y), computedFilter);
}






