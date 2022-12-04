static char winners[3] = {
    'Y', // paper beats rock
    'Z', // scissors beats paper
    'X', // rock beats scissors
};

static int64_t score_choice(char choice) { return (choice - 'X') + 1; }

enum WinState { LOSS, TIE, WIN };

static WinState outcome(char opponent, char me) {
  if ((opponent - 'A') == (me - 'X'))
    return TIE;
  if (me == winners[opponent - 'A'])
    return WIN;
  return LOSS;
}

template <typename F> int64_t _score_strategy(const char *path, F mepicker) {
  int64_t score = 0;
  std::string data = readfile(path);
  for (size_t i = 0; i < data.size(); i += 4) {
    char opponent = data[i];
    char me = mepicker(data[i + 2], opponent);
    int outcome_score = outcome(opponent, me) * 3;
    int choice_score = score_choice(me);
    score += outcome_score + choice_score;
  }

  return score;
}

static int64_t score_strategy(const char *path) {
  return _score_strategy(path, [](char me, char opponent) { return me; });
}

static char moves[3][3] = {
    {'Z', 'X', 'Y'}, // Losers
    {'X', 'Y', 'Z'}, // Ties
    {'Y', 'Z', 'X'}, // Winners
};

static int64_t score_strategy2(const char *path) {
  return _score_strategy(path, [](char me, char opponent) {
    return moves[me - 'X'][opponent - 'A'];
  });
}
