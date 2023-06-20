#version 430

#define FILTER_BUFF1_IMAGE_UNIT    0
#define FILTER_BUFF2_IMAGE_UNIT    1
#define FILTER_BUFF3_IMAGE_UNIT    2
#define LUM_AVG_IMAGE_UNIT         3

#define LUM_HISTOGRAM_BUFFER_INDEX 3

#define GROUP_SIZE 256
#define THREADS_X  256
#define THREADS_Y  1

// Uniforms:
uniform vec4 u_params;
#define MIN_LOG_LUM   u_params.x
#define LOG_LUM_RANGE u_params.y
#define TIME_COEFF    u_params.z
#define NUM_PIXELS    u_params.w

layout(binding=LUM_AVG_IMAGE_UNIT,           r16f) uniform          image2D img_target;
layout(binding=LUM_HISTOGRAM_BUFFER_INDEX, std430)         readonly buffer  buff_histogram
{
  uint histogram[];
};

shared uint histogramShared[GROUP_SIZE];

layout (local_size_x = THREADS_X, local_size_y = THREADS_Y) in;

void main()
{
  // Get the count from the histogram buffer.
  uint countForThisBin = histogram[gl_LocalInvocationIndex];
  histogramShared[gl_LocalInvocationIndex] = countForThisBin * gl_LocalInvocationIndex;

  barrier();

  // This loop will perform a weighted count of the luminance range.
  //UNROLL - if only!
  for (uint cutoff = (GROUP_SIZE >> 1); cutoff > 0; cutoff >>= 1)
  {
    if (uint(gl_LocalInvocationIndex) < cutoff)
    {
      histogramShared[gl_LocalInvocationIndex] += histogramShared[gl_LocalInvocationIndex + cutoff];
    }

    barrier();
  }

  // We only need to calculate this once, so only a single thread is needed.
  if (gl_LocalInvocationIndex == 0)
  {
    // Here we take our weighted sum and divide it by the number of pixels
    // that had luminance greater than zero (since the index == 0, we can
    // use countForThisBin to find the number of black pixels).
    float weightedLogAverage = (histogramShared[0] / max(NUM_PIXELS - float(countForThisBin), 1.0)) - 1.0;

    // Map from our histogram space to actual luminance.
    float weightedAvgLum = exp2(((weightedLogAverage / 254.0) * LOG_LUM_RANGE) + MIN_LOG_LUM);

    // The new stored value will be interpolated using the last frames value
    // to prevent sudden shifts in the exposure.
    float lumLastFrame = imageLoad(img_target, ivec2(0, 0)).x;
    float adaptedLum   = lumLastFrame + (TIME_COEFF * (weightedAvgLum - lumLastFrame));

    //adaptedLum = count0;
    //adaptedLum = histogramShared[gl_LocalInvocationIndex];
    //adaptedLum = weightedAvgLum;
    //adaptedLum = lumLastFrame;
    //adaptedLum = 10.1;

    imageStore(img_target, ivec2(0, 0), vec4(adaptedLum, 0.0, 0.0, 0.0));
  }
}
