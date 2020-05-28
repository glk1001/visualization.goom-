#ifndef MATHTOOLS_H
#define MATHTOOLS_H

#define _double2fixmagic (68719476736.0 * 1.5)
/* 2^36 * 1.5,  (52-_shiftamt=36) uses limited precision to floor */
#define _shiftamt 16
/* 16.16 fixed point representation */

#if BigEndian_
  #define iexp_ 0
  #define iman_ 1
#else
  #define iexp_ 1
  #define iman_ 0
#endif /* BigEndian_ */

#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif

extern const float sin256[256];
extern const float cos256[256];

#endif
