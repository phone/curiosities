#include <string>
#include <vector>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

#define CRASH() (((void (*)())0)())

// #define PART1
#define PART2

int walk(std::vector<std::string> const &map, int width, int row_inc = 1,
         int col_inc = 3) {
  int trees = 0, row = 0, col = 0;
  while (row < map.size()) {
    char ch = map[row][col % width];
    if (ch == '#') {
      trees++;
    }
    row += row_inc;
    col += col_inc;
  }
  return trees;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s map-file\n", argv[0]);
    exit(1);
  }

  std::ifstream map_file(argv[1]);
  int width = 0;
  std::vector<std::string> map;
  for (std::string line; std::getline(map_file, line);) {
    if (!width) {
      width = line.size();
    } else {
      if (line.size() != width) {
        CRASH();
      }
    }
    map.push_back(line);
  }

#ifdef PART1
  int trees = walk(map, width);
  printf("trees: %d\n", trees);
#elif defined(PART2)
  const int num_incrs = 5;
  const int incrs[num_incrs][2] = {
      {1, 1}, {3, 1}, {5, 1}, {7, 1}, {1, 2},
  };
  int64_t num_trees_mult = 1;
  for (int i = 0; i < num_incrs; i++) {
    int col_incr = incrs[i][0];
    int row_incr = incrs[i][1];
    int trees = walk(map, width, row_incr, col_incr);
    fprintf(stderr, "trees: %d, ri: %d, ci: %d\n", trees, row_incr, col_incr);
    num_trees_mult *= trees;
  }
  printf("num_trees_mult: %lld\n", num_trees_mult);

#endif
}
