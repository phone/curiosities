#include <ctype.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

// #define PART1
#define PART2

struct PasswordPolicy {
  int min;
  int max;
  char ch;
};

struct Password {
  PasswordPolicy policy;
  std::string password;
  static Password from_string(std::string const &input);
  bool is_ok1();
  bool is_ok2();
};

bool Password::is_ok1() {
  int found_of_ch = 0;
  for (int i = 0; i < (int)password.size(); ++i) {
    if (password[i] == policy.ch) {
      found_of_ch++;
    }
  }
  return found_of_ch >= policy.min && found_of_ch <= policy.max;
}

bool Password::is_ok2() {
  if (password.size() < (policy.max - 1) || password.size() < (policy.min - 1))
    return false;
  bool has_min = password[policy.min - 1] == policy.ch;
  bool has_max = password[policy.max - 1] == policy.ch;
  return has_min != has_max;
}

Password Password::from_string(std::string const &input) {
  enum States { S_MIN, S_MAX, S_CHAR, S_POSTCHAR1, S_POSTCHAR2, S_PASSWORD };
  Password ret = {};
  States state = S_MIN;
  for (int i = 0; i < (int)input.size(); ++i) {
    switch (state) {
    case S_MIN:
      if (isdigit(input[i])) {
        ret.policy.min *= 10;
        ret.policy.min += (input[i] - '0');
      } else if (input[i] == '-') {
        state = S_MAX;
      }
      break;
    case S_MAX:
      if (isdigit(input[i])) {
        ret.policy.max *= 10;
        ret.policy.max += (input[i] - '0');
      } else if (isspace(input[i])) {
        state = S_CHAR;
      }
      break;
    case S_CHAR:
      if (isalpha(input[i])) {
        ret.policy.ch = input[i];
        state = S_POSTCHAR1;
      }
      break;
    case S_POSTCHAR1:
      if (input[i] == ':') {
        state = S_POSTCHAR2;
      }
      break;
    case S_POSTCHAR2:
      if (input[i] == ' ') {
        state = S_PASSWORD;
      }
      break;
    case S_PASSWORD:
      ret.password += input[i];
    }
  }
  return ret;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s password-file\n", argv[0]);
    exit(1);
  }

  std::ifstream password_file(argv[1]);
  std::vector<Password> passwords;
  for (std::string line; std::getline(password_file, line);) {
    passwords.push_back(Password::from_string(line));
  }
#ifdef PRINTALL
  for (auto &password : passwords) {
    fprintf(stderr, "min: %d, max: %d, ch: %hhd, password: %s\n",
            password.policy.min, password.policy.max, password.policy.ch,
            password.password.c_str());
  }
#endif
  int ok = 0;
  for (auto &password : passwords) {
#ifdef PART1
    if (password.is_ok1()) {
#elif defined(PART2)
    if (password.is_ok2()) {
#endif
      ok++;
    }
  }
  printf("ok: %d\n", ok);
  return 0;
}
