#include <chrono>
#include <iostream>
#include <stdlib.h>

#define CRASH() (((void (*)())0)())
#define ASSERT(expr)                                                           \
  do {                                                                         \
    if (!(expr)) {                                                             \
      fprintf(stderr, "assert failed: %s\n", #expr);                           \
      CRASH();                                                                 \
    }                                                                          \
  } while (0)

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
              << "ns" << '\n';
  }
};

#include "day1.cpp"
#include "day2.cpp"

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
  case 2: {
    int64_t score = 0;
    {
      Timer("day 2");
      if (section & 1)
        score = score_strategy("2.1.txt");
      else
        score = score_strategy2("2.1.txt");
    }
    fprintf(stdout, "%lld\n", score);
  } break;
  }
  return 0;
}
