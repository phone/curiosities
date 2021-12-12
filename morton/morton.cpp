#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

static long long now() {
  struct timeval tp;
  gettimeofday(&tp, NULL);
  long long mslong = (long long)tp.tv_sec * 1000L + tp.tv_usec / 1000;
  return mslong;
}

static const uint32_t morton_lut_3d[256] = {
    0x0,      0x1,      0x8,      0x9,      0x40,     0x41,     0x48,
    0x49,     0x200,    0x201,    0x208,    0x209,    0x240,    0x241,
    0x248,    0x249,    0x1000,   0x1001,   0x1008,   0x1009,   0x1040,
    0x1041,   0x1048,   0x1049,   0x1200,   0x1201,   0x1208,   0x1209,
    0x1240,   0x1241,   0x1248,   0x1249,   0x8000,   0x8001,   0x8008,
    0x8009,   0x8040,   0x8041,   0x8048,   0x8049,   0x8200,   0x8201,
    0x8208,   0x8209,   0x8240,   0x8241,   0x8248,   0x8249,   0x9000,
    0x9001,   0x9008,   0x9009,   0x9040,   0x9041,   0x9048,   0x9049,
    0x9200,   0x9201,   0x9208,   0x9209,   0x9240,   0x9241,   0x9248,
    0x9249,   0x40000,  0x40001,  0x40008,  0x40009,  0x40040,  0x40041,
    0x40048,  0x40049,  0x40200,  0x40201,  0x40208,  0x40209,  0x40240,
    0x40241,  0x40248,  0x40249,  0x41000,  0x41001,  0x41008,  0x41009,
    0x41040,  0x41041,  0x41048,  0x41049,  0x41200,  0x41201,  0x41208,
    0x41209,  0x41240,  0x41241,  0x41248,  0x41249,  0x48000,  0x48001,
    0x48008,  0x48009,  0x48040,  0x48041,  0x48048,  0x48049,  0x48200,
    0x48201,  0x48208,  0x48209,  0x48240,  0x48241,  0x48248,  0x48249,
    0x49000,  0x49001,  0x49008,  0x49009,  0x49040,  0x49041,  0x49048,
    0x49049,  0x49200,  0x49201,  0x49208,  0x49209,  0x49240,  0x49241,
    0x49248,  0x49249,  0x200000, 0x200001, 0x200008, 0x200009, 0x200040,
    0x200041, 0x200048, 0x200049, 0x200200, 0x200201, 0x200208, 0x200209,
    0x200240, 0x200241, 0x200248, 0x200249, 0x201000, 0x201001, 0x201008,
    0x201009, 0x201040, 0x201041, 0x201048, 0x201049, 0x201200, 0x201201,
    0x201208, 0x201209, 0x201240, 0x201241, 0x201248, 0x201249, 0x208000,
    0x208001, 0x208008, 0x208009, 0x208040, 0x208041, 0x208048, 0x208049,
    0x208200, 0x208201, 0x208208, 0x208209, 0x208240, 0x208241, 0x208248,
    0x208249, 0x209000, 0x209001, 0x209008, 0x209009, 0x209040, 0x209041,
    0x209048, 0x209049, 0x209200, 0x209201, 0x209208, 0x209209, 0x209240,
    0x209241, 0x209248, 0x209249, 0x240000, 0x240001, 0x240008, 0x240009,
    0x240040, 0x240041, 0x240048, 0x240049, 0x240200, 0x240201, 0x240208,
    0x240209, 0x240240, 0x240241, 0x240248, 0x240249, 0x241000, 0x241001,
    0x241008, 0x241009, 0x241040, 0x241041, 0x241048, 0x241049, 0x241200,
    0x241201, 0x241208, 0x241209, 0x241240, 0x241241, 0x241248, 0x241249,
    0x248000, 0x248001, 0x248008, 0x248009, 0x248040, 0x248041, 0x248048,
    0x248049, 0x248200, 0x248201, 0x248208, 0x248209, 0x248240, 0x248241,
    0x248248, 0x248249, 0x249000, 0x249001, 0x249008, 0x249009, 0x249040,
    0x249041, 0x249048, 0x249049, 0x249200, 0x249201, 0x249208, 0x249209,
    0x249240, 0x249241, 0x249248, 0x249249,
};

/*
 * Calculate the morton number for a 3d coordinate, only defined for positive
 * coordinates.
 *
 * Semantically, this works by interleaving the bits of each coordinate. Maybe
 * best explained with an example:
 *
 * Given,
 *
 *   x = 5 = 0b101, y = 3 = 0b011, z = 6 = 0b110:
 *
 * we create a large output integer and set it to 0:
 *
 *   out = 0;
 *
 * but, think of the output like a list of bits. it begins empty. then we take
 * the msb of x, then y, then z, and append those bits to the list.
 *
 *   x y z
 *   | | |
 *   1 0 1
 *
 * and we keep doing this along each coordinate word.
 *
 *   x2 y2 z2 x1 y1 z1 ...
 *   1  0  1  0 ..
 *
 * til finally we get
 *
 *   1 0 1 0 1 1 1 1 0 => 0x15e
 *
 */
inline uint64_t morton_3d(uint16_t x, uint16_t y, uint16_t z) {
  uint64_t out =
      (((uint64_t)morton_lut_3d[x >> CHAR_BIT]) << (CHAR_BIT * 3 + 2)) |
      (((uint64_t)morton_lut_3d[y >> CHAR_BIT]) << (CHAR_BIT * 3 + 1)) |
      (((uint64_t)morton_lut_3d[z >> CHAR_BIT]) << (CHAR_BIT * 3)) |
      (morton_lut_3d[x & 0xFF] << 2) | (morton_lut_3d[y & 0xFF] << 1) |
      (morton_lut_3d[z & 0xFF]);
  return out;
}

union V3 {
  struct {
    uint16_t x, y, z;
  };
  uint16_t v[3];
};

union Morton3DMinMax {
  struct {
    uint64_t min, max;
  };
  uint64_t v[2];
};

#define NCORNERS 8
union Corners {
  struct {
    V3 fxfybz;
    V3 fxbybz;
    V3 fxfyfz;
    V3 fxbyfz;
    V3 bxfybz;
    V3 bxbybz;
    V3 bxfyfz;
    V3 bxbyfz;
  };
  V3 v[NCORNERS];
};

inline Corners corners_from_coord_3d(V3 coord, uint16_t kernel_size) {
  uint16_t dist = kernel_size / 2;
  uint16_t fx = coord.x < (((uint16_t)-1) - dist) ? coord.x + dist : coord.x;
  uint16_t fy = coord.y < (((uint16_t)-1) - dist) ? coord.y + dist : coord.y;
  uint16_t fz = coord.z < (((uint16_t)-1) - dist) ? coord.z + dist : coord.z;
  uint16_t bx = coord.x > (((uint16_t)0) + dist) ? coord.x - dist : coord.x;
  uint16_t by = coord.y > (((uint16_t)0) + dist) ? coord.y - dist : coord.y;
  uint16_t bz = coord.z > (((uint16_t)0) + dist) ? coord.z - dist : coord.z;
  Corners corners = {};
  corners.fxfybz = {fx, fy, bz};
  corners.fxbybz = {fx, by, bz};
  corners.fxfyfz = {fx, fy, fz};
  corners.fxbyfz = {fx, by, fz};
  corners.bxfybz = {bx, fy, bz};
  corners.bxbybz = {bx, by, bz};
  corners.bxfyfz = {bx, fy, fz};
  corners.bxbyfz = {bx, by, fz};
  return corners;
}

inline Morton3DMinMax min_max_corners_of_corners(Corners corners) {
  Morton3DMinMax minmax = {(uint64_t)-1, 0};
  for (int i = 0; i < NCORNERS; ++i) {
    uint64_t morton = morton_3d(corners.v[i].x, corners.v[i].y, corners.v[i].z);
    if (morton > minmax.max) {
      minmax.max = morton;
    }
    if (morton < minmax.min) {
      minmax.min = morton;
    }
  }
  return minmax;
}

void genpoints(int numpoints, uint16_t maxval, V3 *dst) {
  for (int i = 0; i < numpoints; ++i) {
    for (int j = 0; j < 3; ++j) {
      dst[i].v[j] = rand() % maxval;
    }
  }
}

#define NPOINTS 10000
#define MAXPOINT 220
static uint64_t mortons[NPOINTS];
static V3 points[NPOINTS];

struct MortonPoint {
  uint64_t morton;
  V3 point;
};

void mortonswap(uint64_t *mortons, V3 *points, int a, int b) {
  uint64_t morton_tmp = mortons[a];
  V3 point_tmp = points[a];

  mortons[a] = mortons[b];
  points[a] = points[b];

  mortons[b] = morton_tmp;
  points[b] = point_tmp;
}

int partition(uint64_t *mortons, V3 *points, int lo, int hi) {
  uint64_t pivot = mortons[lo + (hi - lo) / 2];
  int i = lo - 1;
  int j = hi + 1;
  for (;;) {
    do {
      ++i;
    } while (mortons[i] < pivot);
    do {
      --j;
    } while (mortons[j] > pivot);
    if (i >= j) {
      return j;
    }
    mortonswap(mortons, points, i, j);
  }
}

void quicksort(uint64_t *mortons, V3 *points, int lo, int hi) {
  if (lo < hi) {
    int p = partition(mortons, points, lo, hi);
    quicksort(mortons, points, lo, p);
    quicksort(mortons, points, p + 1, hi);
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s KERNEL_SIZE\n", argv[0]);
    exit(1);
  }
  int kernel = atoi(argv[1]);

  genpoints(NPOINTS, MAXPOINT, points);

  MortonPoint mps[NPOINTS] = {};

  int min_morton = 0;
  int max_morton = 0;
  for (int i = 0; i < NPOINTS; ++i) {
    V3 pt = points[i];
    mortons[i] = morton_3d(pt.x, pt.y, pt.z);

    // Save the pair for error checking later
    mps[i].point = pt;
    mps[i].morton = mortons[i];

    // Record the min and max
    if (mortons[i] < mortons[min_morton]) {
      min_morton = i;
    }
    if (mortons[i] > mortons[max_morton]) {
      max_morton = i;
    }
  }
  // printf("Mortons range: %lu -> %lu\n", mortons[min_morton],
  //        mortons[max_morton]);

  long int ranges[NPOINTS] = {};
  Corners corners[NPOINTS] = {};
  // For each point, fill in all eight corners of its kernel window
  for (int i = 0; i < NPOINTS; ++i) {
    corners[i] = corners_from_coord_3d(points[i], kernel);
    // record the min and max morton numbers among the kernel window corners.
    Morton3DMinMax minmax = min_max_corners_of_corners(corners[i]);

    // Start each at our index
    int hi = i;
    int lo = i;

    // Walk our points, going up along the morton index. At each step, check if
    // the morton number is greater than the max morton number we could have in
    // our kernel window. If so, stop the loop. Otherwise, increment the index.
    for (; hi < NPOINTS && mortons[hi] <= minmax.max; ++hi) {
    }

    // Walk our points, going down along the morton index. At each step, check
    // if the morton number is less than the min morton number we could have in
    // our kernel window. If so, stop the loop. Otherwise, decrement the index.
    for (; lo >= 0 && mortons[lo] >= minmax.min; --lo) {
    }

    // This number represents the total number of points we'd have to check in
    // order to search our kernel for other points within it.
    ranges[i] = (hi - lo);
  }

  long int running_hist_total = 0;
  // Bin the ranges exponentially
  for (int i = 1; i < (CHAR_BIT * (sizeof(*ranges) - 1)); ++i) {
    long int lower = (long int)1L << (i - 1);
    long int upper = (long int)(1L << i);
    long int total = 0;
    // Check each point. If the range for it falls into this bin, count it.
    for (int j = 0; j < NPOINTS; ++j) {
      if (ranges[j] >= lower && ranges[j] < upper) {
        ++total;
      }
    }
    running_hist_total += total;
    // printf("Range for %ld -> %ld:\t%ld\t%ld\n", lower, upper, total,
    //        running_hist_total);
    if (running_hist_total >= NPOINTS)
      break;
  }

  // How many total vertex comparison operations would it take to evaluate every
  // kernel of this set of points?
  int total_operations = 0;
  for (int i = 0; i < NPOINTS; ++i) {
    total_operations += ranges[i];
  }
  int hashtable_operations = powf(kernel, 3) * NPOINTS;
  printf("kernel size: %d: total vertex comparisons: %d, vs hashtable: %d, "
         "better: %d, by: "
         "%10.2lf%%\n",
         kernel, total_operations, hashtable_operations,
         total_operations <= hashtable_operations,
         100.0d * ((double)hashtable_operations / (double)total_operations));

  quicksort(mortons, points, 0, NPOINTS - 1);

  // Error check the quicksort
  for (int i = 0; i < NPOINTS; ++i) {
    int morton = mortons[i];
    V3 pt = points[i];
    for (int j = 0; j < NPOINTS; ++j) {
      if (mps[j].morton == morton) {
        if ((mps[j].point.x != pt.x) || (mps[j].point.y != pt.y) ||
            (mps[j].point.z != pt.z)) {
          printf("morton mismatch\n");
        }
      }
    }
  }

  /*
  uint16_t x, y, z;
  uint64_t m;

  x = 0, y = 0, z = 0;
  m = morton_3d(x, y, z);
  printf("x: %hd, y: %hd, z: %hd: morton: %lx\n", x, y, z, m);

  x = 2, y = 4, z = 3;
  m = morton_3d(x, y, z);
  printf("x: %hd, y: %hd, z: %hd: morton: %lx\n", x, y, z, m);

  x = 5, y = 3, z = 6;
  m = morton_3d(x, y, z);
  printf("x: %hd, y: %hd, z: %hd: morton: %lx\n", x, y, z, m);

  x = 1520, y = 1530, z = 1525;
  m = morton_3d(x, y, z);
  printf("x: %hd, y: %hd, z: %hd: morton: %lx\n", x, y, z, m);
  */
}
