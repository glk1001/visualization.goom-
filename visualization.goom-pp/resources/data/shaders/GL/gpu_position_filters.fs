vec2 GetAmuletVelocity(const vec2 position, const float time)
{
  vec2 p = position;

  const float sqDistFromZero = p.x * p.x + p.y * p.y;

  const float sinT = sin(0.01 * time);
  const float cosT = cos(0.01 * time);
  const float x = p.x;
  p.x = p.x*cosT - p.y*sinT;
  p.y = p.y*cosT + x*sinT;

  const float Ax = 1.0;
  const float Ay = 1.0;
  const float baseX = 0.25;
  const float baseY = 0.25;
  const vec2 v = vec2(baseX + (Ax * sqDistFromZero),
                      baseY + (Ay * sqDistFromZero));

  return -p * v;
}

vec2 GetWaveVelocity(const vec2 position)
{
  const vec2 p = position;

  const float sqDistFromZero = p.x * p.x + p.y * p.y;
  const float reducerCoeff = 0.0;
  const float reducer      = exp(-reducerCoeff * sqDistFromZero);

  const float freqFactor = 10.5;
  //const float angle          = atan(p.y, p.x);
  const float angle = pow(sqDistFromZero, 1.1);

  const float sinAngle = sin(freqFactor * angle);
  const float cosAngle = cos(freqFactor * angle);

  const float Ax = 0.2;
  const float Ay = 0.2;
  const float baseX = 0.1;
  const float baseY = 0.1;

  const vec2 v = vec2(baseX + reducer * Ax * cosAngle,
                      baseY + reducer * Ay * sinAngle);

  return -p * v;
}

vec2 GetVortexVelocity(const vec2 position, const float time)
{
  const vec2 p = position;

  const float r = length(p);
  const float theta = atan(p.y, p.x);
  const float t = sqrt(10.0 * r) + theta + (0.02 * time);

  vec2 v = vec2(p.y, -p.x) / r;
  v *= sin(t);
  v *= 2.0 * length(v);
  v += 0.2 * p;

  return v;
}

vec2 GetReflectingPoolVelocity(const vec2 position, const float time)
{
  const vec2 p = position;

  const float timeFreq = 0.05;
  const float t = 5.0 * sin(timeFreq * time);
  const float A = 1.5;

  const vec2 v = vec2(A * sin((t * p.y) + p.x),
                      A * cos((t * p.x) - p.y));

  return v;
}

vec2 GetBeautifulFieldVelocity(const vec2 position, const float time)
{
  const vec2 p = position;

  const float frame = 150.0 * sin(0.01 * time);

  const float PI = 3.14;
  const float dt = 0.01;

  const float t = frame * dt;
  const float w = 2.0 * (PI / 5.0);
  const float A = 5.0;

  const float d = sqrt((p.x * p.x) + (p.y * p.y));

  const vec2 v = vec2(A * cos((w * t) / d), A * sin((w * t) / d));

  return v;
}

// Following assumes HEIGHT <= WIDTH.
const float RATIO_DEV_TO_NORMALIZED_COORD = FILTER_POS_COORD_WIDTH / float(WIDTH - 1);
const vec2 MID_POINT = vec2(FILTER_POS_MIN_COORD + (RATIO_DEV_TO_NORMALIZED_COORD * float(WIDTH/2)),
                            FILTER_POS_MIN_COORD + (RATIO_DEV_TO_NORMALIZED_COORD * float(HEIGHT/2))
           );

vec2 GetGPUFilteredPosition(const ivec2 deviceXY, const float time)
{
  const vec2 pos = vec2(FILTER_POS_MIN_COORD + (RATIO_DEV_TO_NORMALIZED_COORD * float(deviceXY.x)),
                        FILTER_POS_MIN_COORD + (RATIO_DEV_TO_NORMALIZED_COORD * float(deviceXY.y)));

  const vec2 centredPos = pos - MID_POINT;

  //const vec2 v = GetAmuletVelocity(centredPos, time);
  //const vec2 v = GetWaveVelocity(centredPos);
  //const vec2 v = GetVortexVelocity(centredPos, time);
  const vec2 v = GetReflectingPoolVelocity(centredPos, time);
  //const vec2 v = GetBeautifulFieldVelocity(centredPos, time);

  return (centredPos + v) + MID_POINT;
}
