
static int64_t prioritize_branchless(char x) {
  // Lowercase letters will always have bit 5 set. The ranges are:
  // A(hex: 41, binary: 1000001) - Z(hex: 5a, binary: 1011010)
  // a(hex: 61, binary: 1100001) - z(hex: 7a, binary: 1111010)
  //
  // Test the 5th bit. This gives us an unsigned quantity either 0 or 1
  size_t test_lower = (x >> 5) & 1;
  int64_t ret = 0;
  // If the test passed, unsigned 0 minus the test will underflow and produce a
  // 64 bit word filled with 1s, which can be safely bitwise ANDed with whatever
  // you want
  ret += (0UL - test_lower) & ((x - 'a') + 1);
  ret += (0UL - (!test_lower) & ((x - 'A') + 27));
  return ret;
}

[[maybe_unused]] static int64_t prioritize(char x) {
  if (x < 'a') {
    return (x - 'A') + 27;
  }
  return (x - 'a') + 1;
}

static int64_t score_rucksack_weirdness(const char *path) {
  int64_t score = 0;
  std::string data = readfile(path);
  enum { FIRSTHALF, SECONDHALF } state = FIRSTHALF;
  size_t linecount = 0;
  int set[256] = {};
  for (size_t i = 0, j = 0; i < data.size(); ++i, j += 2) {
    // In this loop, we advance j twice as much as i. During the first half, we
    // look to see if data[j] is a newline. If so, that starts the 2nd half
    // parse. During the second half, data[j] is never read, so we never
    // overrun the buffer.
    switch (state) {
    case FIRSTHALF: {
      if (data[j] == '\n') {
        state = SECONDHALF;
        // record the end of the line. when i gets here in the 2nd half, we'll
        // have to swap back to the first half.
        linecount = j;
        // fall through here
      } else {
        set[data[i]] += 1;
        break;
      };
    }
    case SECONDHALF: {
      if (i == linecount) {
        state = FIRSTHALF;
        // if we're here, i is at the newline. set j to one less, since it'll
        // advance twice. then when we hit the first character of the next
        // line, i and j will be the same.
        j = linecount - 1;
        // clear the set.
        memset(&set, 0, sizeof(set));
      } else {
        size_t found = !!set[data[i]];
        score += (0UL - found) & prioritize_branchless(data[i]);
        // this is basically a break to the end of the line.
        // looks underflow-sketchy, but it'll be fine the way this is executed
        size_t diff = ((linecount - 1) - i);
        i += (0UL - found) & diff;
      }
    } break;
    }
  }
  return score;
}

static int64_t score_rucksack_weirdness2(const char *path) {
  int64_t score = 0;
  std::string data = readfile(path);
  size_t linecount = 0;
  int sets[2][256] = {};
  size_t skip = 0;
  for (size_t i = 0; i < data.size(); ++i) {
    // This loop processes a character from either the first two lines of a
    // group, or the third line. In the first, case, we accumulate our sets, and
    // tick our linecount each newline. In the second case, we check each letter
    // against both sets to find if we need to add to our score, and if we get a
    // newline, we zero all our state.
    if (linecount == 2) {
      if (data[i] == '\n') {
        linecount = 0;
        memset(sets, 0, sizeof(sets));
        skip = 0;
      } else {
        size_t found = 1;
        for (int j = 0; j < 2; ++j)
          found &= (!!sets[j][data[i]]);
        score +=
            ((0UL - (found - (skip & found))) & prioritize_branchless(data[i]));
        skip |= found;
      }
    } else {
      if (data[i] == '\n') {
        linecount++;
      } else {
        sets[linecount][data[i]]++;
      }
    }
  }
  return score;
}
