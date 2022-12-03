#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string>

std::string readfile(const char *path) {
  std::string ret;
  auto f = fopen(path, "r");
  if (f) {
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    ret.resize(size, '\0');
    fread(ret.data(), 1, size, f);
    fclose(f);
  }
  return ret;
}

int64_t most_calories(const char *path) {
  int64_t max = INT64_MIN;
  std::string data = readfile(path);
  enum { NUMBER, NEWCAL } state = NUMBER;
  int64_t total = 0;
  int64_t current = 0;
  for (size_t i = 0; i < data.size(); ++i) {
    switch (state) {
    case NEWCAL: {
      state = NUMBER;
      if (data[i] == '\n') {
        if (total > max) {
          max = total;
        }
        total = 0;
        break;
      }
    }
    case NUMBER: {
      if (isdigit(data[i])) {
        current *= 10;
        current += (data[i] - '0');
      } else {
        state = NEWCAL;
        total += current;
        current = 0;
      }
    } break;
    }
  }
  return max;
}

int64_t most_3_calories(const char *path) {
  int64_t max1 = INT64_MIN, max2 = INT64_MIN, max3 = INT64_MIN;
  std::string data = readfile(path);
  enum { NUMBER, NEWCAL } state = NUMBER;
  int64_t total = 0;
  int64_t current = 0;
  for (size_t i = 0; i < data.size(); ++i) {
    switch (state) {
    case NEWCAL: {
      state = NUMBER;
      if (data[i] == '\n') {
        if (total > max1) {
          max3 = max2;
          max2 = max1;
          max1 = total;
        } else if (total > max2) {
          max3 = max2;
          max2 = total;
        } else if (total > max3) {
          max3 = total;
        }
        total = 0;
        break;
      }
    }
    case NUMBER: {
      if (isdigit(data[i])) {
        current *= 10;
        current += (data[i] - '0');
      } else {
        state = NEWCAL;
        total += current;
        current = 0;
      }
    } break;
    }
  }
  return max1 + max2 + max3;
}
