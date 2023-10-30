uniform float u_gamma;

#define NO_TONE_MAP 1
#define EXPOSURE_TONE_MAP 2
#define REINHARD_TONE_MAP 3
#define REINHARD2_TONE_MAP 4
#define LOTTES_TONE_MAP 5
#define UCHIMURA_TONE_MAP 6
#define ACES_TONE_MAP 7
#define FILMIC_TONE_MAP 8
#define UNCHARTED2_TONE_MAP 9
#define UNREAL_TONE_MAP 10

//#define toneMapToUse NO_TONE_MAP
//#define toneMapToUse EXPOSURE_TONE_MAP
//#define toneMapToUse REINHARD_TONE_MAP
//#define toneMapToUse REINHARD2_TONE_MAP
#define toneMapToUse LOTTES_TONE_MAP
//#define toneMapToUse UCHIMURA_TONE_MAP
//#define toneMapToUse ACES_TONE_MAP
//#define toneMapToUse FILMIC_TONE_MAP
//#define toneMapToUse UNCHARTED2_TONE_MAP
//#define toneMapToUse UNREAL_TONE_MAP

// Exposure Tone Map
vec3 ExposureToneMap(vec3 x, float exposure)
{
  return vec3(1.0) - exp(-exposure * x);
}

// Reinhard Tone Map
vec3 Reinhard(vec3 x)
{
  return x / (1.0 + x);
}

vec3 Reinhard2(vec3 x)
{
  const float L_white = 4.0;

  return (x * (1.0 + x / (L_white * L_white))) / (1.0 + x);
}

// Lottes 2016, "Advanced Techniques and Optimization of HDR Color Pipelines"
vec3 Lottes(vec3 x)
{
  const vec3 a      = vec3(1.6);
  const vec3 d      = vec3(0.977);
  const vec3 hdrMax = vec3(8.0);
  const vec3 midIn  = vec3(0.18);
  const vec3 midOut = vec3(0.267);

  const vec3 b = (-pow(midIn, a) + pow(hdrMax, a) * midOut) /
                 ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);
  const vec3 c =
      (pow(hdrMax, a * d) * pow(midIn, a) - pow(hdrMax, a) * pow(midIn, a * d) * midOut) /
      ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);

  return pow(x, a) / (pow(x, a * d) * b + c);
}

// Uchimura 2017, "HDR theory and practice"
// Math: https://www.desmos.com/calculator/gslcdxvipg
// Source: https://www.slideshare.net/nikuque/hdr-theory-and-practicce-jp
vec3 Uchimura(vec3 x, float P, float a, float m, float l, float c, float b)
{
  float l0 = ((P - m) * l) / a;
  float L0 = m - m / a;
  float L1 = m + (1.0 - m) / a;
  float S0 = m + l0;
  float S1 = m + a * l0;
  float C2 = (a * P) / (P - S1);
  float CP = -C2 / P;

  vec3 w0 = vec3(1.0 - smoothstep(0.0, m, x));
  vec3 w2 = vec3(step(m + l0, x));
  vec3 w1 = vec3(1.0 - w0 - w2);

  vec3 T = vec3(m * pow(x / m, vec3(c)) + b);
  vec3 S = vec3(P - (P - S1) * exp(CP * (x - S0)));
  vec3 L = vec3(m + a * (x - m));

  return T * w0 + L * w1 + S * w2;
}

vec3 Uchimura(vec3 x)
{
  const float P = 1.0; // max display brightness
  const float a = 1.0; // contrast
  const float m = 0.22; // linear section start
  const float l = 0.4; // linear section length
  const float c = 1.03; // black
  const float b = 0.0; // pedestal

  return Uchimura(x, P, a, m, l, c, b);
}

// Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
vec3 Aces(vec3 x)
{
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;
  return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

// Filmic Tonemapping Operators http://filmicworlds.com/blog/filmic-tonemapping-operators/
vec3 Filmic(vec3 x)
{
  vec3 X      = max(vec3(0.0), x - 0.004);
  vec3 result = (X * (6.2 * X + 0.5)) / (X * (6.2 * X + 1.7) + 0.06);
  return pow(result, vec3(2.2));
}

// Uncharted Tone Map
vec3 Uncharted2Tonemap(vec3 x)
{
  float A = 0.15;
  float B = 0.50;
  float C = 0.10;
  float D = 0.20;
  float E = 0.02;
  float F = 0.30;
  float W = 11.2;
  return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 Uncharted2(vec3 color)
{
  const float W      = 11.2;
  float exposureBias = 2.0;
  vec3 curr          = Uncharted2Tonemap(exposureBias * color);
  vec3 whiteScale    = 1.0 / Uncharted2Tonemap(vec3(W));
  return curr * whiteScale;
}

// Unreal 3, Documentation: "Color Grading"
// Adapted to be close to Tonemap_ACES, with similar range
// Gamma 2.2 correction is baked in, don't use with sRGB conversion!
vec3 Unreal(vec3 x)
{
  return x / (x + 0.155) * 1.019;
}

vec3 ToGamma(vec3 color)
{
  return pow(color, vec3(1.0 / u_gamma));
}

vec3 GetToneMappedColor(vec3 color)
{
#if (toneMapToUse == NO_TONE_MAP)
  {
    const float exposureBias = 1.0;
    return ToGamma(exposureBias * color);
  }
#elif (toneMapToUse == EXPOSURE_TONE_MAP)
  {
    const float exposureBias = 2.5;
    return ToGamma(ExposureToneMap(color, exposureBias));
  }
#elif (toneMapToUse == REINHARD_TONE_MAP)
  {
    const float exposureBias = 2.0;
    return ToGamma(Reinhard(exposureBias * color));
  }
#elif (toneMapToUse == REINHARD2_TONE_MAP)
  {
    const float exposureBias = 4.0;
    return ToGamma(Reinhard2(exposureBias * color));
  }
#elif (toneMapToUse == LOTTES_TONE_MAP)
  {
    const float exposureBias = 2.0;
    return ToGamma(Lottes(exposureBias * color));
  }
#elif (toneMapToUse == UCHIMURA_TONE_MAP)
  {
    const float exposureBias = 2.0;
    return ToGamma(Uchimura(exposureBias * color));
  }
#elif (toneMapToUse == ACES_TONE_MAP)
  {
    const float exposureBias = 1.5;
    return ToGamma(Aces(exposureBias * color));
  }
#elif (toneMapToUse == FILMIC_TONE_MAP)
  {
    const float exposureBias = 5.0;
    return Filmic(exposureBias * color);
  }
#elif (toneMapToUse == UNCHARTED2_TONE_MAP)
  {
    const float exposureBias = 5.0;
    return ToGamma(Uncharted2(exposureBias * color));
  }
#elif (toneMapToUse == UNREAL_TONE_MAP)
  {
    const float exposureBias = 1.0;
    return Unreal(exposureBias * color);
  }
#endif
}
