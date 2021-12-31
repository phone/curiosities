#pragma once
#include <stdint.h>
#include <stdio.h>
#include <type_traits>

namespace std {
// This lets us avoid including <utility>
template <class T>
inline T &&forward(typename std::remove_reference<T>::type &t) noexcept {
  return static_cast<T &&>(t);
}

template <class T>
inline T &&forward(typename std::remove_reference<T>::type &&t) noexcept {
  static_assert(!std::is_lvalue_reference<T>::value,
                "Can not forward an rvalue as an lvalue.");
  return static_cast<T &&>(t);
}
} // namespace std

// Lets us avoid including <new>
inline void *operator new(std::size_t, void *_p) noexcept { return _p; }

#define CRASH() (((void (*)())0)())
#define ASSERT(expr)                                                           \
  do {                                                                         \
    if (!(expr)) {                                                             \
      fprintf(stderr, "assert failed: %s\n", #expr);                           \
      CRASH();                                                                 \
    }                                                                          \
  } while (0)
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

template <size_t N> class Arena {
  static const int num_arena_marks = 32;
  char *ptr = nullptr;
  int next_arena_mark = 0;
  char *arena_marks[num_arena_marks] = {};
  char base[N];

public:
  Arena() : ptr(base) {}
  Arena(Arena const &Other) = delete;
  Arena(Arena &&Other) = delete;

  template <typename T>
  T *alloc(size_t number = 1, size_t alignment = alignof(T)) {
    size_t padding = alignment - ((uintptr_t)ptr % alignment);
    size_t size = padding + (sizeof(T) * number);
    if (size > capacity())
      return nullptr;
    T *ret = reinterpret_cast<T *>(ptr + padding);
    ptr += size;
    return ret;
  }
  template <typename T, typename... Args> T *construct(Args &&...args) {
    T *ret = new (alloc<T>()) T(std::forward<Args>(args)...);
    return ret;
  }
  size_t capacity() {
    size_t used = (ptr - base);
    return N - used;
  }
  void clear_to(const char *to) {
    ASSERT(to >= base && to < ptr);
    size_t used_size = ptr - to;
    ptr = to;
    for (int i = 0; i < used_size; ++i)
      ptr[i] = 0;
  }
  void clear() { clear_to(base); }
  int push_mark() {
    ASSERT(next_arena_mark < num_arena_marks);
    int ret = next_arena_mark;
    arena_marks[next_arena_mark++] = ptr;
    return ret;
  }
  void pop_mark(int mark = -1) {
    if (mark >= 0) {
      ASSERT(mark < next_arena_mark);
      clear_to(arena_marks[mark]);
      for (int i = mark; i < next_arena_mark; ++i)
        arena_marks[i] = nullptr;
      next_arena_mark = mark;
    } else {
      ASSERT(next_arena_mark > 0);
      clear_to(arena_marks[--next_arena_mark]);
      arena_marks[next_arena_mark] = nullptr;
    }
  }
};

static bool floateq(float a, float b, float epsilon = 0.01f) {
  return (b <= (a + epsilon) && b >= (a - epsilon));
}
