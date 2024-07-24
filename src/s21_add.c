#include <stdio.h>

#include "s21_decimal.h"

// без пустых строк и комментариев 42 строки
int32_t s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  if (result == NULL) {
    return 0;
  }
  /*
     I.    (+1) + (+2)
     выравниваем степени
     делаем (+1) + (+2)

     II.   (+1) + (-2)
     меняем знак у 2
     делаем (+1) - (+2)

     III.  (-1) + (-2)
     меняем знак у 1 и у 2 на плюс
     делаем (+1) + (+2)
     менем знак у 1 и у 2 на минус

     IV.   (-1) + (+2)
     меняем знак у 1
     делаем (+2) - (+1)
      */
  decimal_to_null(result);
  s21_big_decimal val_1 = decimal_to_big_decimal(value_1);
  s21_big_decimal val_2 = decimal_to_big_decimal(value_2);
  s21_big_decimal res = {0};

  sign_and_pow scale_1 = parsing_sign_and_pow_big(val_1);
  sign_and_pow scale_2 = parsing_sign_and_pow_big(val_2);
  int32_t flag = 0;

  if (!scale_1.sign && !scale_2.sign) {
    // I.    (+1) + (+2)
    // набор функций по сложению двух положительных
    math_add(val_1, val_2, &res, scale_1, scale_2, &flag);

    if ((s21_is_equal_big(val_1, DECIMAL_BIG_MAX_DECIMAL) &&
         !s21_is_equal_big(val_2, DECIMAL_0_BIG)) ||
        (s21_is_equal_big(val_2, DECIMAL_BIG_MAX_DECIMAL) &&
         !s21_is_equal_big(val_1, DECIMAL_0_BIG)))
      flag = 1;
    // записали результат
    *result = big_decimal_to_decimal(res);
  } else if (!scale_1.sign && scale_2.sign) {
    // II.   (+1) + (-2)
    set_sign(&value_2, 0);
    s21_sub(value_1, value_2, result);
  } else if (scale_1.sign && scale_2.sign) {
    // III.  (-1) + (-2)
    if ((s21_is_equal_big(val_1, DECIMAL_BIG_MAX_DECIMAL) &&
         !s21_is_equal_big(val_2, DECIMAL_0_BIG)) ||
        (s21_is_equal_big(val_2, DECIMAL_BIG_MAX_DECIMAL) &&
         !s21_is_equal_big(val_1, DECIMAL_0_BIG)))
      flag = 1;
    else {
      // набор функций по сложению двух положительных
      math_add(val_1, val_2, &res, scale_1, scale_2, &flag);

      // установка знака в резал
      set_sign_big(&res, 1);
      // записали результат
      *result = big_decimal_to_decimal(res);
    }
  } else if (scale_1.sign && !scale_2.sign) {
    // IV.   (-1) + (+2)
    set_sign(&value_1, 0);
    s21_sub(value_2, value_1, result);
  }

  if (flag) decimal_to_null(result);
  return flag;
}

// декомпозиция набора функций по сложению двух положительных
void math_add(s21_big_decimal val_1, s21_big_decimal val_2,
              s21_big_decimal *res, sign_and_pow scale_1, sign_and_pow scale_2,
              int *flag) {
  // сложили жва биг децимал
  plus_plus_big(val_1, val_2, scale_1, scale_2, res, flag);
  // применили большую степень к резул биг децимал
  set_power_big(res, scale_1.pow > scale_2.pow ? scale_1.pow : scale_2.pow);
  // начали округление
  round_to_dec_precision(res);
  // установка флага переполнения на бесконечно большое или бесконечно малое
  apply_flag_to_mauch_or_litle(flag, *res);
}

// суммирование биг дицималов. декомпозиция
void plus_plus_big(s21_big_decimal value_1, s21_big_decimal value_2,
                   sign_and_pow scale_1, sign_and_pow scale_2,
                   s21_big_decimal *result, int32_t *flag) {
  set_sign_big(&value_1, 0);
  set_sign_big(&value_2, 0);
  if (scale_1.pow == scale_2.pow) {
    set_power_big(result, scale_1.pow);
    *flag = add_two_positiv_big(result, value_1, value_2);
  } else {
    // выравнивание степени
    if (scale_1.pow > scale_2.pow) {
      apply_pow(&value_2, scale_1.pow);
    } else {
      apply_pow(&value_1, scale_2.pow);
    }
    *flag = add_two_positiv_big(result, value_1, value_2);
  }
}

// установка степени в биг децимал
void apply_pow(s21_big_decimal *value, int32_t pow) {
  s21_big_decimal temp = {0};
  int now_pow = get_power_big(*value);
  if (pow > now_pow) {
    for (int32_t i = pow; i > now_pow; i--) {
      s21_mul_equal_pow_big(*value, DECIMAL_10_BIG, &temp);
      *value = temp;
      big_decimal_to_null(&temp);
    }
  } else {
    for (int32_t i = now_pow - pow; i > 0; i--) {
      s21_div_integer_part_big(*value, DECIMAL_10_BIG, &temp);
      *value = temp;
      big_decimal_to_null(&temp);
    }
  }
  set_power_big(value, pow);
}

int32_t add_two_positiv(s21_decimal *result, s21_decimal value_1,
                        s21_decimal value_2) {
  int flag = 0;
  int overflow_0 = 0;
  int overflow_1 = 0;

  (*result).bits[0] = value_1.bits[0] + value_2.bits[0];
  overflow_0 = add_overflow(value_1.bits[0], value_2.bits[0]);

  for (int i = 1; i <= 2; i++) {
    (*result).bits[i] = value_1.bits[i] + value_2.bits[i];
    overflow_1 = add_overflow(value_1.bits[i], value_2.bits[i]);
    if (overflow_0) {
      (*result).bits[i] += 1;
      if ((*result).bits[i] == 0) overflow_1 = 1;
    }
    overflow_0 = overflow_1;
  }

  // проверка на переполнение (больше макс децимал)
  int pow_1 = get_power(value_1);
  int pow_2 = get_power(value_2);
  int bufer_pow = pow_1 <= pow_2 ? pow_1 : pow_2;
  s21_decimal bufer = *result;
  set_power(&bufer, bufer_pow);

  if (overflow_1) flag = 1;

  return flag;
}

int32_t add_overflow(uint32_t x, uint32_t y) {
  int32_t res = 0;
  uint32_t max = x > y ? x : y;
  if (x + y < max) {
    res = 1;
  }
  return res;
}
