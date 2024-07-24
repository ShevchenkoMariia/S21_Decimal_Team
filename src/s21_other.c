#include "s21_decimal.h"

// Возвращает результат умножения указанного Decimal на -1.
int s21_negate(s21_decimal value, s21_decimal *result) {
  int flag = 0;
  if (result == NULL)
    flag = 1;
  else {
    if (!flag) {
      *result = value;
      int bit = get_sign(value);
      if (!s21_is_equal_null(value)) set_sign(result, bit == 0 ? 1 : 0);
    }
  }
  return flag;
}

int s21_truncate(s21_decimal value, s21_decimal *result) {
  int error = 0;
  error = validation_check(value);
  if (result == NULL) error = 1;

  if (!error) {
    // получаем знак
    int sign = (value.bits[3] >> 31) & 1;

    // 0xFF = 11111111, изолируем первые 8 бит, содерщащие степень
    // определяем наличие степени и саму степень
    int scale = (value.bits[3] >> 16) & 0xFF;

    s21_decimal result_my_num = {0};

    int count_scale = scale;

    if (scale == 0)
      result_my_num = value;
    else {
      for (int i = scale; i > 0 && !(value.bits[0] < 10 && value.bits[1] == 0 &&
                                     value.bits[2] == 0);
           i--) {
        result_my_num = div_to_ten(value);
        value = result_my_num;
        count_scale--;
      }
    }

    s21_decimal decimal_zero = {0};

    if (count_scale != 0)
      *result = decimal_zero;
    else {
      result_my_num.bits[3] = 0;
      if (sign) result_my_num.bits[3] |= HIGH_BIT;
      *result = result_my_num;
    }
  }
  return error;
}

// Округляет Decimal до ближайшего целого числа.
int s21_round(s21_decimal value, s21_decimal *result) {
  int flag = 0;
  if (result == NULL)
    flag = 1;
  else {
    int sign = get_sign(value);
    s21_decimal ostatok = {0};
    s21_decimal bufer = {0};
    // на выходе value - целая часть, ostatok - одна первая цифра дробной части
    // десяточного децимала
    for (int i = get_power(value); i > 0; i--) {
      decimal_to_null(&ostatok);
      ostatok = s21_div_integer_part(value, DECIMAL_10, &bufer);
      value = bufer;
      decimal_to_null(&bufer);
    }

    set_sign(&value, 0);
    // увеличение последней цифры, если остаток(1 цифра) больше 5
    add_two_positiv(&bufer,
                    s21_is_greater_or_equal(ostatok, DECIMAL_5) == 1
                        ? DECIMAL_1
                        : DECIMAL_0,
                    value);
    set_sign(&bufer, sign);

    *result = bufer;
  }
  return flag;
}

// Округляет указанное Decimal число до ближайшего целого числа в сторону
// отрицательной бесконечности.
int s21_floor(s21_decimal value, s21_decimal *result) {
  int flag = 0;
  if (result == NULL)
    flag = 1;
  else {
    int sign = get_sign(value);
    s21_decimal bufer = {0};

    s21_truncate(value, &bufer);
    s21_decimal ostatok = {0};
    s21_sub(value, bufer, &ostatok);
    value = bufer;
    decimal_to_null(&bufer);

    set_sign(&value, 0);
    // // увеличение последней цифры, если остаток (1 цифра) больше 5
    if (sign == 1)
      add_two_positiv(&bufer,
                      s21_is_equal_null(ostatok) == 0 ? DECIMAL_1 : DECIMAL_0,
                      value);
    else
      bufer = value;
    set_sign(&bufer, sign);

    *result = bufer;
  }
  return flag;
}
