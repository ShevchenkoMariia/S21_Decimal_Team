#include "s21_decimal.h"

int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  if (result == NULL) {
    return 0;
  }
  decimal_to_null(result);
  int32_t flag = 0;
  // если чисвла одинковые, то результат ноль, а резалт у нас и так ноль уже.
  if (!s21_is_equal(value_1, value_2)) {
    s21_big_decimal val_1 = decimal_to_big_decimal(value_1);
    s21_big_decimal val_2 = decimal_to_big_decimal(value_2);
    s21_big_decimal res = {0};
    sign_and_pow scale_1 = parsing_sign_and_pow_big(val_1);
    sign_and_pow scale_2 = parsing_sign_and_pow_big(val_2);

    /*
    I.    (+1) - (+2)
    если 2 больше 1 - знак минус и (+2) - (+1)
    если 1 больше 2 - знак плюс и (+1) - (+2)

    II.   (+1) - (-2)
    меняем знак у 2 и делаем (+1) + (+2)

    III.  (-1) - (+2)
    меняем знак у 2 и делаем (-1) + (-2)

    IV.   (-1) - (-2)
    меняем знак у обоих
    (+2) - (+1)
    если 2 больше 1 - знак плюс и (+2) - (+1)
    если 1 больше 2 - знак минус и (+1) - (+2)
     */
    if (!scale_1.sign && !scale_2.sign) {
      // I.(+1) - (+2)
      if (s21_is_greater_or_equal_big(val_2, val_1)) {
        // набор функций по вычитанию двух положитлеьных чисел val2 БОЛЬШЕЕ
        math_sub_small_big(&val_1, &val_2, &res, scale_1, scale_2, value_1,
                           value_2, &flag);
        // установили знак в резуал
        set_sign_big(&res, 1);
        // записали результат
        *result = big_decimal_to_decimal(res);
      } else {
        // набор функций по вычитанию двух положитлеьных чисел val1 БОЛЬШЕЕ
        math_sub_big_small(&val_1, &val_2, &res, scale_1, scale_2, value_1,
                           value_2, &flag);

        set_sign_big(&res, 0);
        // записали результат
        *result = big_decimal_to_decimal(res);
      }
    } else if (!scale_1.sign && scale_2.sign) {
      // II. (+1) - (-2)
      set_sign(&value_2, 0);
      flag = s21_add(value_1, value_2, result);
    } else if (scale_1.sign && !scale_2.sign) {
      // III. (-1) - (+2)
      set_sign(&value_2, 1);
      flag = s21_add(value_1, value_2, result);
      if (flag) flag = 2;
    } else if (scale_1.sign && scale_2.sign) {
      // IV. (-1) - (-2)
      set_sign_big(&val_1, 0);
      set_sign_big(&val_2, 0);

      if (s21_is_greater_or_equal_big(val_2, val_1)) {
        // набор функций по вычитанию двух положитлеьных чисел val2 БОЛЬШЕЕ
        math_sub_small_big(&val_1, &val_2, &res, scale_1, scale_2, value_1,
                           value_2, &flag);
        // записали результат
        *result = big_decimal_to_decimal(res);
      } else {
        // набор функций по вычитанию двух положитлеьных чисел val1 БОЛЬШЕЕ
        math_sub_big_small(&val_1, &val_2, &res, scale_1, scale_2, value_1,
                           value_2, &flag);

        set_sign_big(&res, 1);
        // записали результат
        *result = big_decimal_to_decimal(res);
      }
    }
  }

  if (flag) decimal_to_null(result);
  return flag;
}

// декомпозиция набора функций по вычитанию двух чисел val2 БОЛЬШЕЕ
void math_sub_small_big(s21_big_decimal *val_1, s21_big_decimal *val_2,
                        s21_big_decimal *res, sign_and_pow scale_1,
                        sign_and_pow scale_2, s21_decimal value_1,
                        s21_decimal value_2, int *flag) {
  // выровняли степень
  pow1_pow2_align(val_1, val_2, value_1, value_2);
  s21_sub_big_small_big(*val_2, *val_1, res);
  // применили большую степень к резул биг децимал
  set_power_big(res, scale_1.pow > scale_2.pow ? scale_1.pow : scale_2.pow);

  // начали округление
  round_to_dec_precision(res);

  // установка флага переполнения на бесконечно большое или бесконечно малое
  apply_flag_to_mauch_or_litle(flag, *res);
}

// декомпозиция набора функций по вычитанию двух чисел val1 БОЛЬШЕЕ
void math_sub_big_small(s21_big_decimal *val_1, s21_big_decimal *val_2,
                        s21_big_decimal *res, sign_and_pow scale_1,
                        sign_and_pow scale_2, s21_decimal value_1,
                        s21_decimal value_2, int *flag) {
  // выровняли степень
  pow1_pow2_align(val_1, val_2, value_1, value_2);
  s21_sub_big_small_big(*val_1, *val_2, res);
  // применили большую степень к резул биг децимал
  set_power_big(res, scale_1.pow > scale_2.pow ? scale_1.pow : scale_2.pow);

  // начали округление
  round_to_dec_precision(res);

  // установка флага переполнения на бесконечно большое или бесконечно малое
  apply_flag_to_mauch_or_litle(flag, *res);
}
// выравнинваниие степени у биг децимал
void pow1_pow2_align(s21_big_decimal *val_1, s21_big_decimal *val_2,
                     s21_decimal value_1, s21_decimal value_2) {
  // выравнивание степени
  if (get_power(value_1) > get_power(value_2)) {
    apply_pow(val_2, get_power(value_1));
  } else {
    apply_pow(val_1, get_power(value_2));
  }

  // установили степень в ноль
  set_power_big(val_1, 0);
  set_power_big(val_2, 0);
}
