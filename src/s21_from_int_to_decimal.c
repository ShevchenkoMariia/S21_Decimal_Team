#include "s21_decimal.h"

int s21_from_int_to_decimal(int src, s21_decimal *dst) {
  int res = 0;

  if (dst == NULL)
    res = 1;
  else {
    // заполняем нулями
    for (int i = 3; i >= 0; i--) dst->bits[i] = 0;

    // проверяем и устанавливаем знак
    if (src < 0) {
      src *= -1;
      dst->bits[3] |= HIGH_BIT;
    } else
      dst->bits[3] = 0;

    dst->bits[0] = src;
    dst->bits[1] = 0;
    dst->bits[2] = 0;
  }
  return res;
}
