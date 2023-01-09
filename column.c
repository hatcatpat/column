#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FREE(x)                                                                \
  if (x) {                                                                     \
    free(x);                                                                   \
    x = NULL;                                                                  \
  }

#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a) ((a) < 0 ? -(a) : (a))

#define DEBUG 0

#define PRINT                                                                  \
  if (DEBUG)                                                                   \
  printf

#define BLANK_CHAR ' '
#define LINE_CHAR '='

int char2int(char x) { return x - '0'; }
char int2char(int x) { return x + '0'; }

int close_enough(double a, double b) { return fabs(a - b) < FLT_EPSILON * 10; }

double rand_double() { return (double)rand() / (double)RAND_MAX; }
double rand_double_range(double lo, double hi) {
  return rand_double() * (hi - lo) + lo;
}
int rand_int_range(int lo, int hi) { return floor(rand_double_range(lo, hi)); }

double round2dp(double a, int dp) {
  double p = pow(10, dp);
  return roundf(a * p) / p;
}

double digi2double(uint8_t *digi, int len, int dp) {
  double sum = 0;

  for (int i = 0; i < len; ++i)
    sum += digi[len - 1 - i] * pow(10, i - dp);

  return sum;
}

void trim0(char *str, int len) {
  for (int i = 0; i < len; ++i)
    if (str[len - 1 - i] == '0')
      str[len - 1 - i] = '\0';
    else
      return;
}

struct digits {
  uint8_t *digi;
  int d, dp;
} digits;
void digits_free(struct digits *D);
void string2digits(struct digits *D, const char *_str);
void double2digits(struct digits *D, double x);
int digits_getl(struct digits *D, int i);
int digits_getr(struct digits *D, int i);

int digits_getl(struct digits *D, int i) { return D->digi[i]; }

int digits_getr(struct digits *D, int i) {
  return D->digi[D->d + D->dp - 1 - i];
}

#define MAX_DOUBLE_STRLEN 50
void double2digits(struct digits *D, double x) {
  char str[MAX_DOUBLE_STRLEN];
  snprintf(str, MAX_DOUBLE_STRLEN, "%f", x);
  trim0(str, strlen(str));
  string2digits(D, str);
}

void string2digits(struct digits *D, const char *_str) {
  static const char *delim = ".";

  memset(D, 0, sizeof(struct digits));

  unsigned long _len = strlen(_str);

  char *str = malloc(_len);
  strcpy(str, _str);

  char *token = strtok(str, delim);

  while (token != NULL) {
    unsigned long len = strlen(token);

    if (D->d)
      D->dp = len;
    else
      D->d = len;

    token = strtok(NULL, delim);
  }

  D->digi = malloc((D->d + D->dp) * sizeof(uint8_t));

  for (int i = 0; i < D->d; ++i)
    D->digi[i] = char2int(_str[i]);

  for (int i = 0; i < D->dp; ++i)
    D->digi[D->d + D->dp - 1 - i] = char2int(_str[_len - 1 - i]);

  free(str);
}

void red() { printf("\033[31m"); }
void white() { printf("\033[37m"); }
void reset() { printf("\033[0m"); }

void digits_add_print1(struct digits *D, int d_max, int dp_max) {
  int diff = d_max - D->d;
  int len = D->d + D->dp;
  int max_len = d_max + dp_max;

  for (int i = 0; i < max_len; ++i) {
    if (D->dp && i == max_len - dp_max)
      putchar('.');

    int j = i - diff;
    if (0 <= j && j < len)
      putchar(int2char(digits_getl(D, j)));
    else
      putchar(BLANK_CHAR);
  }

  putchar('\n');
}

void digits_add_print(struct digits *A, struct digits *B, uint8_t *out,
                      uint8_t *rems, int len, int d_max, int dp_max) {
  // 1st line (A)
  putchar(BLANK_CHAR);
  digits_add_print1(A, d_max + (out[0] ? 1 : 0), dp_max);

  // 2nd line (B)
  putchar('+');
  digits_add_print1(B, d_max + (out[0] ? 1 : 0), dp_max);

  // 3rd line (LINE_CHAR)
  white();
  for (int i = 0; i < len + (out[0] ? 2 : 1); ++i)
    putchar(LINE_CHAR);
  putchar('\n');
  reset();

  // 4th line (out)
  putchar(BLANK_CHAR);
  uint8_t trail = 1;
  for (int i = 0; i < len; ++i) {
    if (i == len - dp_max)
      putchar('.');

    if (trail && out[i])
      trail = 0;

    if (!trail)
      putchar(int2char(out[i]));
  }
  putchar('\n');

  // 5th line (rems)
  red();
  if (out[0])
    putchar(BLANK_CHAR);

  for (int i = 0; i < len; ++i) {
    if (i == len - dp_max)
      putchar(BLANK_CHAR);

    putchar(rems[i + 1] ? int2char(rems[i + 1]) : BLANK_CHAR);
  }
  reset();

  putchar('\n'), putchar('\n');
}

double digits_add(struct digits *A, struct digits *B) {
  struct digits *d_max, *d_min;
  if (A->d > B->d)
    d_max = A, d_min = B;
  else
    d_max = B, d_min = A;
  int d_diff = d_max->d - d_min->d;

  struct digits *dp_max, *dp_min;
  if (A->dp > B->dp)
    dp_max = A, dp_min = B;
  else
    dp_max = B, dp_min = A;
  int dp_diff = dp_max->dp - dp_min->dp;

  int len = d_max->d + dp_max->dp + 1;
  uint8_t *out = calloc(len, sizeof(uint8_t));
  uint8_t *rems = calloc(len, sizeof(uint8_t));

  uint8_t rem = 0, ones = 0;

  for (int i = 0; i < d_max->d + dp_max->dp; ++i) {
    // initial dp
    if (i < dp_diff) {
      ones = digits_getr(dp_max, i);
      PRINT("[inital dp]\t%i\n", ones);
    }
    // overlap d and dp
    else if (i - dp_diff < d_min->d + dp_min->dp) {
      int x = digits_getr(dp_max, i);
      int y = digits_getr(dp_min, i - dp_diff);
      int sum = x + y + rem;
      ones = sum % 10;
      PRINT("[overlap]\t%i + %i + (%i) = %i", x, y, rem, ones);
      rem = (sum - ones) / 10;
      PRINT("\t\trem = %i\n", rem);
    }
    // final d
    else {
      int j = i - (dp_diff + d_min->d + dp_min->dp);
      int x = digits_getl(d_max, (d_diff - 1) - j);
      int sum = x + rem;
      ones = sum % 10;
      PRINT("[final d]\t%i + (%i) = %i", x, rem, sum);
      rem = (sum - ones) / 10;
      PRINT("\t\trem  = %i\n", rem);
    }

    out[len - 1 - i] = ones;
    rems[len - 1 - i] = rem;
  }

  if (rem) {
    PRINT("[final rem]\t%i\n", rem);
    out[0] = rem;
  }

  double fin = digi2double(out, len, dp_max->dp);
  PRINT("out: %.*f\n", dp_max->dp, fin);

  digits_add_print(A, B, out, rems, len, d_max->d, dp_max->dp);

  free(out), free(rems);

  return fin;
}

void digits_free(struct digits *D) { FREE(D->digi); }

int test_add(double a, double b) {
  PRINT("\nTESTING: %f + %f\n", a, b);

  struct digits A, B;
  double2digits(&A, a), double2digits(&B, b);

  PRINT("%.*f, d=%i, dp=%i\n", A.dp, digi2double(A.digi, A.d + A.dp, A.dp), A.d,
        A.dp);
  PRINT("%.*f, d=%i, dp=%i\n", B.dp, digi2double(B.digi, B.d + B.dp, B.dp), B.d,
        B.dp);

  PRINT("\n");
  double x = digits_add(&A, &B);
  double correct = a + b;

  digits_free(&A), digits_free(&B);

  if (close_enough(x, correct)) {
    PRINT("%lf + %lf == %lf, RIGHT\n", a, b, x);
    return 0;
  } else {
    PRINT("%lf + %lf != %lf, WRONG, should be %f\n", a, b, x, correct);
    return -1;
  }
}

int main(void) {
  if (0) {
    srand(time(NULL));

#define TEST_NUM 100
#define TEST_MAG 100000
    int i;

    for (i = 0; i < TEST_NUM; ++i) {
      double a = ((double)rand() / (double)RAND_MAX) * TEST_MAG;
      a = round2dp(a, rand_int_range(0, 10));
      double b = ((double)rand() / (double)RAND_MAX) * TEST_MAG;
      b = round2dp(b, rand_int_range(0, 10));
      if (test_add(a, b))
        break;
    }

    PRINT("total successes = %i / %i\n", i, TEST_NUM);
  }

  double a, b;
  scanf("%lf + %lf", &a, &b);
  test_add(a, b);

  return 0;
}
