#pragma once

static Arena<0x000fffff> arena;

class Op {
public:
  virtual void evaluate() = 0;
  virtual void backwards() = 0;
};

struct Literal : Op {
  void evaluate() override { return; }
  void backwards() override { return; }
};

struct Scalar_ {
  Op *op;
  float value;
  float delta_root;
  Scalar_(float value_)
      : op(arena.alloc<Literal>()), value(value_), delta_root(0) {}
};

struct Scalar {
  Scalar_ *scalar;
  Scalar() : scalar(arena.alloc<Scalar_>()) {}
  Scalar(float value) : scalar(arena.construct<Scalar_>(value)) {}
  Scalar(Scalar_ *scalar_) : scalar(scalar_) {}
  float value() {
    ASSERT(scalar);
    return scalar->value;
  }
  float delta_root() {
    ASSERT(scalar);
    return scalar->delta_root;
  }
  float evaluate() {
    ASSERT(scalar);
    scalar->op->evaluate();
    return scalar->value;
  }
  void backwards() {
    scalar->delta_root = 1.f;
    scalar->op->backwards();
  }
};

Scalar operator"" _r(long double f) { return Scalar(static_cast<float>(f)); }

Scalar operator"" _r(unsigned long long f) {
  return Scalar(static_cast<float>(f));
}

struct Sum : Op {
  Scalar_ *scalar;
  Scalar_ *operands[2];
  void evaluate() override {
    scalar->value = operands[0]->value + operands[1]->value;
  }
  void backwards() override {
    for (int i = 0; i < 2; ++i) {
      operands[i]->delta_root += scalar->delta_root;
    }
  }
};

static void sum(Scalar intermediate, Scalar a, Scalar b) {
  Sum *op = arena.construct<Sum>();
  op->scalar = intermediate.scalar;
  op->operands[0] = a.scalar;
  op->operands[1] = b.scalar;
  op->scalar->op = op;
}

struct Multiply : Op {
  Scalar_ *scalar;
  Scalar_ *operands[2];
  void evaluate() override {
    scalar->value = operands[0]->value * operands[1]->value;
  }
  void backwards() override {
    operands[0]->delta_root += (scalar->delta_root * operands[1]->value);
    operands[0]->op->backwards();
    operands[1]->delta_root += (scalar->delta_root * operands[0]->value);
    operands[1]->op->backwards();
  }
};

static void multiply(Scalar intermediate, Scalar a, Scalar b) {
  Multiply *op = arena.construct<Multiply>();
  op->scalar = intermediate.scalar;
  op->operands[0] = a.scalar;
  op->operands[1] = b.scalar;
  op->scalar->op = op;
}

static Scalar operator+(Scalar a, Scalar b) {
  Scalar ret;
  sum(ret, a, b);
  ret.evaluate();
  return ret;
}

static Scalar operator*(Scalar a, Scalar b) {
  Scalar ret;
  multiply(ret, a, b);
  ret.evaluate();
  return ret;
}

void test_scalars(bool print) {
  // Computational graph of:
  //    c = a + b
  //    d = b + f
  //    e = c * d
  //    a = 2
  //    b = 1
  //    f = 1
  Scalar b = 1_r;
  Scalar e = (2_r + b) * (b + 1_r);

  if (print)
    fprintf(stdout, "Final: %f\n", e.value());
  ASSERT(floateq(e.value(), 6.f));
  e.backwards();

  Scalar c(reinterpret_cast<Multiply *>(e.scalar->op)->operands[0]);
  Scalar d(reinterpret_cast<Multiply *>(e.scalar->op)->operands[1]);
  Scalar a(reinterpret_cast<Sum *>(c.scalar->op)->operands[0]);
  Scalar f(reinterpret_cast<Sum *>(d.scalar->op)->operands[1]);
  if (print) {
    fprintf(stdout, "de/de: %f\n", e.delta_root());
    fprintf(stdout, "de/dc: %f\n", c.delta_root());
    fprintf(stdout, "de/dd: %f\n", d.delta_root());
    fprintf(stdout, "de/da: %f\n", a.delta_root());
    fprintf(stdout, "de/db: %f\n", b.delta_root());
    fprintf(stdout, "de/df: %f\n", f.delta_root());
  }
  ASSERT(floateq(e.delta_root(), 1.f));
  ASSERT(floateq(c.delta_root(), 2.f));
  ASSERT(floateq(d.delta_root(), 3.f));
  ASSERT(floateq(a.delta_root(), 2.f));
  ASSERT(floateq(b.delta_root(), 5.f));
  ASSERT(floateq(f.delta_root(), 3.f));
}
