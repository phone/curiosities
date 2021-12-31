#include <fstream>
#include <iostream>
#include <limits.h>
#include <optional>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <vector>

// #define TWOSUM
#define THREESUM

struct TwoSum {
  int a;
  int b;
};

struct ThreeSum {
  int a;
  int b;
  int c;
};

std::optional<TwoSum> twosum(std::vector<int> &numbers, int target) {
  std::optional<TwoSum> ret;
  std::unordered_map<int64_t, int> delts;
  for (auto &number : numbers) {
    delts[target - number] = number;
  }

  for (auto &number : numbers) {
    if (delts.find(number) != delts.end()) {
      TwoSum ts = {};
      ts.a = number;
      ts.b = delts[number];
      ret = ts;
      break;
    }
  }
  return ret;
}

int swap_out(std::vector<int> &numbers, int index) {
  int tmp = numbers[index];
  numbers[index] = numbers.back();
  numbers.pop_back();
  return tmp;
}

void swap_back_in(std::vector<int> &numbers, int value, int index) {
  int tmp = numbers[index];
  numbers[index] = value;
  numbers.push_back(tmp);
}

std::optional<ThreeSum> threesum(std::vector<int> &numbers, int target) {
  std::optional<ThreeSum> ret;
  for (int i = 0; i < (int)numbers.size(); ++i) {
    int delta = target - numbers[i];
    int saved = swap_out(numbers, i);
    auto two_sum = twosum(numbers, delta);
    swap_back_in(numbers, saved, i);
    if (two_sum) {
      ThreeSum ts = {};
      ts.a = numbers[i];
      ts.b = two_sum->a;
      ts.c = two_sum->b;
      ret = ts;
      break;
    }
  }
  return ret;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s target ledger\n", argv[0]);
    exit(1);
  }

  const int target = atoi(argv[1]);
  std::ifstream ledger(argv[2]);
  if (!ledger) {
    fprintf(stderr, "bad file: %s\n", argv[2]);
  }

  std::vector<int> numbers;
  for (std::string line; std::getline(ledger, line);) {
    numbers.push_back(std::stoi(line));
  }

#ifdef TWOSUM
  auto result = twosum(numbers, target);
  if (result) {
    printf("%d + %d = %d\n", result->a, result->b, target);
  } else {
    printf("no valid sum to %d\n", target);
  }
  return 0;
#elif defined(THREESUM)
  auto result = threesum(numbers, target);
  if (result) {
    int64_t mult = result->a * result->b * result->c;
    printf("%d + %d + %d = %d\nmult: %lld\n", result->a, result->b, result->c,
           target, mult);
  } else {
    printf("no valid sum to %d\n", target);
  }
#endif
}
