// Generates data for the Splay Tree homework

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>

#define MAX_LEN 10000000

/*
 * This is the xoroshiro128+ random generator, designed in 2016 by David Blackman
 * and Sebastiano Vigna, distributed under the CC-0 license. For more details,
 * see http://vigna.di.unimi.it/xorshift/.
 */
static uint64_t rng_state[2];

static uint64_t rng_rotl(const uint64_t x, int k)
{
  return (x << k) | (x >> (64 - k));
}

static uint64_t rng_next_u64(void)
{
  uint64_t s0 = rng_state[0], s1 = rng_state[1];
  uint64_t result = s0 + s1;
  s1 ^= s0;
  rng_state[0] = rng_rotl(s0, 55) ^ s1 ^ (s1 << 14);
  rng_state[1] = rng_rotl(s1, 36);
  return result;
}

static uint32_t rng_next_u32(void)
{
  return rng_next_u64() >> 11;
}

static void rng_setup(unsigned int seed)
{
  rng_state[0] = seed * 0xdeadbeef;
  rng_state[1] = seed ^ 0xc0de1234;
  for (int i=0; i<100; i++)
    rng_next_u64();
}

/*** The RNG ends here ***/

int a[MAX_LEN+1];

static void usage(void)
{
  fprintf(stderr, "Usage: gen [-s <student-id>] [-t <size-of-subset>] [-b] [-l] [-u] [-w]\n");
  exit(1);
}

static int nextrandom2(unsigned int A, unsigned int B)
{
  if (B <= A)
    return A;
  int offset = rng_next_u32() % (1+B-A);
  if (offset < 0)
    return A - offset;
  else
    return A + offset;
}

static void swapa(int i, int j)
{
  int t = a[i];
  a[i] = a[j];
  a[j] = t;
}

static void randomize(int len)
{
  for(int i=0; i<len; i++)
    swapa(i, nextrandom2(0, len-1));
}

static void makeprogression(int A, int B, int s, int inc, int len)
{
  for(int i=0; i<len; i++) {
    if (i == s + inc*(a[i]-A))
      ;
    else if (A <= a[i] && a[i] <= B) {
      swapa(i, s + inc*(a[i]-A));
      i--;
    }
  }
}

static void subset_generator(int subset_size, int last)
{
  for (int length = 1000; length <= 1000000; length += 5000) {
    for (int i=0; i<length; i++)
      a[i] = i;

    randomize(length);
    makeprogression(length/4, length/4 + length/20, length/10, 1, length);
    makeprogression(length/2, length/2 + length/20, length/10, -1, length);
    makeprogression(3*length/4, 3*length/4 + length/20, length/2, -4, length);
    makeprogression(17*length/20, 17*length/20 + length/20, 2*length/5, 5, length);

    printf("# %d\n",length);
    for (int i=length-1; i>=0; i--)
      printf("I %d\n", a[i]);

    if (!last)
      randomize(length);

    int search_size = subset_size < length ? subset_size : length, repeat;

    if (search_size >= 500000)
      repeat = 1;
    else if (search_size >= 200000)
      repeat = 3;
    else if (search_size >= 100000)
      repeat = 10;
    else if (search_size >= 10000)
      repeat = 20;
    else if (search_size >= 1000)
      repeat = 50;
    else if (search_size >= 100)
      repeat = 100;
    else
      repeat = 1000;

    for (int j=0; j < repeat*search_size; j++)
      printf("F %d\n",a[nextrandom2(0, search_size-1)]);
  }
}

static void sequential_generator(void)
{
  for (int elements = 10; elements <= 5000; elements *= 1.3) {
    printf("# %d\n",elements);

    for(int i = 1; i <= elements; ++i)
      printf("I %d\n", i);

    for(int j = 0; j < 2; ++j)
      for(int i = 1; i <= elements/2; ++i)
        printf("F %d\n", i);
  }
}


int d[MAX_LEN+1];
int psd[MAX_LEN+1];
int weight[MAX_LEN+1];

static void pref_sum(int n) {
  psd[0] = d[0];
  for (int i = 0; i < n; ++i)
    psd[i] = psd[i - 1] + d[i];
}

// returns i such that psd[i] >= x and psd[i - 1] < x
static int ind(int n, int x) {
  int b = 0;
  int e = n - 1;
  while (e - b > 3) {
    int h = (b + e) / 2;
    if (psd[h] < x) {
      b = h;
    } else {
      e = h;
    }
  }
  for (int i = b; i <= e; ++i)
    if (psd[i] >= x)
      return i;
  return n;
}

static void set_d(int n, int value) {
  for (int i = 0; i < n; ++i) {
    d[i] = value;
  }
  pref_sum(n);
}

static void in_d(int begin, int end, int what) {
  if (what == 0) return;
  if (end - 1 <= begin) {
    d[begin] = what;
    return;
  }
  int med = nextrandom2(begin + 1, end - 1);
  int part = nextrandom2(0, what);
  if (med < (end - begin) / 2) {
    in_d(begin, med, part);
    in_d(med, end, what - part);
  } else {
    in_d(med, end, what - part);
    in_d(begin, med, part);
  }
}

static void nonunif_d(int n) {
  set_d(n, 1);
  int s = n * n;
  in_d(0, n, s);
  pref_sum(n);
}

void static_optimal_tree_generator(int uniform) {
  for (int elements = 10; elements <= 11000; elements *= 1.3) {
    printf("# %d\n",elements);

    if (uniform)
      set_d(elements, 1);
    else
      nonunif_d(elements);

    for(int i = 1; i <= elements; ++i)
      printf("I %d\n", i);

    int n_finds = 42 * elements;
    for (int i = 0; i < n_finds; ++i)
      weight[i] = 0;
    for (int i = 1; i <= elements; ++i) {
      weight[i]++;
      printf("F %d\n", i);
    }
    for (int i = 0; i < n_finds; ++i) {
      int find = ind(elements, nextrandom2(0, psd[elements - 1]));
      weight[find]++;
      printf("F %d\n", 1 + find);
    }
  }
}

int main(int argc, char* argv[])
{
  int opt, student_id = -1, subset_size = -1, sequential = 0, last = 0, nonuniform = 0, uniform = 0;

  if (argc > 1 && !strcmp(argv[1], "--help"))
    usage();

  while ((opt = getopt(argc, argv, "bls:t:uwi")) >= 0)
    switch (opt) {
      case 's': student_id = atoi(optarg); break;
      case 't': subset_size = atoi(optarg); break;
      case 'b': sequential = 1; break;
      case 'l': last = 1; break;
      case 'u': uniform = 1; break;
      case 'w': nonuniform = 1; break;
      default: usage();
    }

  if ((subset_size != -1 ? 1 : 0) + sequential + nonuniform + uniform != 1) {
    fprintf(stderr, "Exactly one test to generate must be set.\n");
    return 1;
  }

  if (sequential) {
    sequential_generator();
    return 0;
  }

  if (student_id < 0) {
    fprintf(stderr, "WARNING: Student ID not given, defaulting to 42.\n");
    student_id = 42;
  }

  rng_setup(student_id);

  if (subset_size != -1) {
    if (subset_size < 10 || subset_size > 1000000) {
      fprintf(stderr, "The size of sought subset must be between 10 and 1000000.\n");
      return 1;
    }
    subset_generator(subset_size, last);
    return 0;
  }

  static_optimal_tree_generator(uniform);
  return 0;
}
