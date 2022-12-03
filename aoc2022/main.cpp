#include <chrono>
#include <iostream>
#include <stdlib.h>

struct Timer {
  const char *_name;
  std::chrono::time_point<std::chrono::high_resolution_clock> _start;
  Timer(const char *name)
      : _name(name), _start(std::chrono::high_resolution_clock::now()) {}
  ~Timer() {
    std::chrono::time_point<std::chrono::high_resolution_clock> end =
        std::chrono::high_resolution_clock::now();
    std::cout << _name << ": "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end -
                                                                      _start)
                     .count()
              << '\n';
  }
};

#include "day1.cpp"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s day section\n", argv[0]);
    return -1;
  }
  int day = atoi(argv[1]);
  int section = atoi(argv[2]);
  switch (day) {
  case 1: {
    int64_t calories = 0;
    {
      Timer("day 1");
      if (section & 1)
        calories = most_calories("1.1.txt");
      else
        calories = most_3_calories("1.1.txt");
    }
    fprintf(stdout, "%lld\n", calories);
  } break;
  }
  return 0;
}
