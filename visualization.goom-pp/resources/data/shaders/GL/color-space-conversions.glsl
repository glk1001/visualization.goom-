// ***********************
// Color space conversions

/**
// Converts a color from linear RGB to XYZ space
const mat3 RGB_2_XYZ = (mat3(
  0.4124564, 0.2126729, 0.0193339,
  0.3575761, 0.7151522, 0.1191920,
  0.1804375, 0.0721750, 0.9503041
));

// Used to convert from XYZ to linear RGB space
const mat3 XYZ_2_RGB = (mat3(
   3.2404542,-0.9692660, 0.0556434,
  -1.5371385, 1.8760108,-0.2040259,
  -0.4985314, 0.0415560, 1.0572252
));

vec3 rgb_to_xyz(vec3 rgb) {
  return RGB_2_XYZ * rgb;
}

vec3 xyz_to_rgb(vec3 xyz) {
  return XYZ_2_RGB * xyz;
}
**/

/***/
vec3 xyz_to_rgb(vec3 color)
{
  float var_X = color.r / 100.0; //X from 0.0 to  95.047 (Observer = 2.0 degrees, Illuminant = D65);
  float var_Y = color.g / 100.0; //Y from 0.0 to 100.000;
  float var_Z = color.b / 100.0; //Z from 0.0 to 108.883;

  float var_R = var_X * 3.2406 + var_Y * -1.5372 + var_Z * -0.4986;
  float var_G = var_X * -0.9689 + var_Y * 1.8758 + var_Z * 0.0415;
  float var_B = var_X * 0.0557 + var_Y * -0.2040 + var_Z * 1.0570;

  if (var_R > 0.0031308)
  {
    var_R = 1.055 * pow(var_R, (1.0 / 2.4)) - 0.055;
  }
  else
  {
    var_R = 12.92 * var_R;
  }
  if (var_G > 0.0031308)
  {
    var_G = 1.055 * pow(var_G, (1.0 / 2.4)) - 0.055;
  }
  else
  {
    var_G = 12.92 * var_G;
  }
  if (var_B > 0.0031308)
  {
    var_B = 1.055 * pow(var_B, (1.0 / 2.4)) - 0.055;
  } else
  {
    var_B = 12.92 * var_B;
  }

  float R = var_R;
  float G = var_G;
  float B = var_B;

  return vec3(R, G, B);
}

vec3 rgb_to_xyz(vec3 color)
{
  float var_R = (color.r); //R from 0.0 to 255.0
  float var_G = (color.g); //G from 0.0 to 255.0
  float var_B = (color.b); //B from 0.0 to 255.0

  if (var_R > 0.04045)
  {
    var_R = pow(((var_R + 0.055) / 1.055), 2.4);
  }
  else
  {
    var_R = var_R / 12.92;
  }
  if (var_G > 0.04045)
  {
    var_G = pow(((var_G + 0.055) / 1.055), 2.4);
  }
  else
  {
    var_G = var_G / 12.92;
  }

  if (var_B > 0.04045)
  {
    var_B = pow(((var_B + 0.055) / 1.055), 2.4);
  }
  else
  {
    var_B = var_B / 12.92;
  }

  var_R = var_R * 100.0;
  var_G = var_G * 100.0;
  var_B = var_B * 100.0;

  // Observer = 2.0°, Illuminant = D65
  float X = var_R * 0.4124 + var_G * 0.3576 + var_B * 0.1805;
  float Y = var_R * 0.2126 + var_G * 0.7152 + var_B * 0.0722;
  float Z = var_R * 0.0193 + var_G * 0.1192 + var_B * 0.9505;

  return vec3(X, Y, Z);
}

vec3 lab_to_lch(vec3 color)
{
  const float MPI = 3.14159265359;

  float var_H = atan(color.b, color.g); //in GLSL this is atan2

  if (var_H > 0.0)
  {
    var_H = (var_H / MPI) * 180.0;
  }
  else
  {
    var_H = 360.0 - (abs(var_H) / MPI) * 180.0;
  }

  float C = sqrt(pow(color.g, 2.0) + pow(color.b, 2.0));
  float H = var_H;

  return vec3(color.r, C, H);
}

vec3 lch_to_lab(vec3 color)
{
  float a = cos(radians(color.b)) * color.g;
  float b = sin(radians(color.b)) * color.g;

  return vec3(color.r, a, b);
}

vec3 xyz_to_lab(vec3 color)
{
  float ref_X = 95.047; //Observer= 2.0°, Illuminant= D65
  float ref_Y = 100.000;
  float ref_Z = 108.883;

  float var_X = color.r / ref_X;
  float var_Y = color.g / ref_Y;
  float var_Z = color.b / ref_Z;

  if (var_X > 0.008856)
  {
    var_X = pow(var_X, (1.0 / 3.0));
  }
  else
  {
    var_X = (7.787 * var_X) + (16.0 / 116.0);
  }
  if (var_Y > 0.008856)
  {
    var_Y = pow(var_Y, (1.0 / 3.0));
  }
  else
  {
    var_Y = (7.787 * var_Y) + (16.0 / 116.0);
  }
  if (var_Z > 0.008856)
  {
    var_Z = pow(var_Z, (1.0 / 3.0));
  }
  else
  {
    var_Z = (7.787 * var_Z) + (16.0 / 116.0);
  }

  float L = (116.0 * var_Y) - 16.0;
  float a = 500.0 * (var_X - var_Y);
  float b = 200.0 * (var_Y - var_Z);

  return vec3(L, a, b);
}

vec3 lab_to_xyz(vec3 color)
{
  float var_Y = (color.r + 16.0) / 116.0;
  float var_X = color.g / 500.0 + var_Y;
  float var_Z = var_Y - color.b / 200.0;

  if (pow(var_Y, 3.0) > 0.008856)
  {
    var_Y = pow(var_Y, 3.0);
  }
  else
  {
    var_Y = (var_Y - 16.0 / 116.0) / 7.787;
  }
  if (pow(var_X, 3.0) > 0.008856)
  {
    var_X = pow(var_X, 3.0);
  }
  else
  {
    var_X = (var_X - 16.0 / 116.0) / 7.787;
  }
  if (pow(var_Z, 3.0) > 0.008856)
  {
    var_Z = pow(var_Z, 3.0);
  }
  else
  {
    var_Z = (var_Z - 16.0 / 116.0) / 7.787;
  }

  float ref_X = 95.047; //Observer= 2.0 degrees, Illuminant= D65
  float ref_Y = 100.000;
  float ref_Z = 108.883;

  float X = ref_X * var_X;
  float Y = ref_Y * var_Y;
  float Z = ref_Z * var_Z;

  return vec3(X, Y, Z);
}

vec3 rgb_to_lab(vec3 color)
{
  vec3 xyz = rgb_to_xyz(color);
  vec3 lab = xyz_to_lab(xyz);
  return lab;
}

vec3 lab_to_rgb(vec3 color)
{
  vec3 xyz = lab_to_xyz(color);
  vec3 rgb = xyz_to_rgb(xyz);
  return rgb;
}

vec3 rgb_to_lch(vec3 color)
{
  vec3 xyz = rgb_to_xyz(color);
  vec3 lab = xyz_to_lab(xyz);
  vec3 lch = lab_to_lch(lab);
  return lch;
}

vec3 lch_to_rgb(vec3 color)
{
  vec3 lab = lch_to_lab(color);
  vec3 xyz = lab_to_xyz(lab);
  vec3 rgb = xyz_to_rgb(xyz);
  return rgb;
}
