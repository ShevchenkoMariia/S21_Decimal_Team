#include <stdlib.h>

#include "s21_decimal.h"

int s21_from_float_to_decimal(float src, s21_decimal *dst) {
  int error = 0;

  if (dst == NULL || isnan(src) || src == INFINITY ||
      src > 79228162514264337593543950335.0f ||
      src < -79228162514264337593543950335.0f)
    error = 1;
  else {
    // обнуляем dst
    for (int i = 3; i >= 0; i--) dst->bits[i] = 0;

    // переводим float для прямого доступа к битам
    unsigned int num = *((unsigned int *)&src);

    // вытаскиваем знак
    int sign = (num >> 31) & 1;

    // узнаем степень и проверяем на переполнение
    int scale = 0;

    // домножаем, чтобы дальше работать с положительным числом
    if (sign) {
      src *= -1;
    }

    int help = 0;
    while (src != (unsigned int)src && scale <= 40) {
      src *= 10;
      scale++;
      if (src == 0.999999940395355224609375) src = 1.0f;
      if ((unsigned int)src > 0 && help == 0) {
        help = 1;
        char str[50];
        sprintf(str, "%f", src);
        src = atof(str);
      }
    }

    if (src != (unsigned int)src || scale > 28) {
      error = 1;
      dst->bits[0] = 0;
    } else {
      // записываем знак
      if (sign) dst->bits[3] |= HIGH_BIT;

      // записываем степень
      dst->bits[3] |= (scale << 16);

      int my_num = 0;
      my_num = (unsigned int)src;
      dst->bits[0] = my_num;
    }
  }

  return error;
}
