#include "goom_tools.h"

#include <stdlib.h>

GoomRandom* goom_random_init()
{
  GoomRandom* grandom = (GoomRandom*)malloc(sizeof(GoomRandom));
  grandom->pos = 1;
  goom_random_update_array(grandom, GOOM_NB_RAND);
  return grandom;
}

void goom_random_free(GoomRandom* grandom)
{
  free(grandom);
}

void goom_random_update_array(GoomRandom* grandom, int numberOfValuesToChange)
{
  while (numberOfValuesToChange > 0) {
#if RAND_MAX < 0x10000
    grandom->array[grandom->pos++] = ((pcg32_rand() << 16) + pcg32_rand()) / 127;
#else
    grandom->array[grandom->pos++] = pcg32_rand() / 127;
#endif
    numberOfValuesToChange--;
  }
}

/* https://en.wikipedia.org/wiki/Permuted_congruential_generator#Example_code */

static uint64_t seed = 0;
static uint64_t state = 0x4d595df4d0f33173; // Or something seed-dependent
static uint64_t last_state = 0;
static const uint64_t multiplier = 6364136223846793005u;
static const uint64_t increment = 1442695040888963407u; // Or an arbitrary odd constant

static inline uint32_t rotr32(uint32_t x, unsigned r)
{
  return x >> r | x << (-r & 31);
}

uint32_t pcg32_rand(void)
{
  last_state = state;
  uint64_t x = state;
  unsigned count = (unsigned)(x >> 59); // 59 = 64 - 5

  state = x * multiplier + increment;
  x ^= x >> 18;                              // 18 = (64 - 27)/2
  return rotr32((uint32_t)(x >> 27), count); // 27 = 32 - 5
}

uint64_t pcg32_get_seed()
{
  return seed;
}

uint64_t pcg32_get_last_state()
{
  return last_state;
}

void pcg32_set_state(uint64_t s)
{
  state = s;
  last_state = state;
}

void pcg32_init(uint64_t the_seed)
{
  seed = the_seed;
  state = seed + increment;
  (void)pcg32_rand();
}
