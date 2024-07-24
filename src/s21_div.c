#include "s21_decimal.h"

int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  if (result == NULL) {
    return 0;
  }
  int32_t flag = 0;
  s21_decimal check_zero = value_2;
  check_zero.bits[3] = 0;
  s21_decimal check_equal_1 = value_1;
  set_sign(&check_equal_1, 0);
  s21_decimal check_equal_2 = value_2;
  set_sign(&check_equal_2, 0);
  sign_and_pow sc_1 = parsing_sign_and_pow(value_1);
  sign_and_pow sc_2 = parsing_sign_and_pow(value_2);

  if (s21_is_equal_null(check_zero))
    flag = 3;
  else if (s21_is_equal(check_equal_1, check_equal_2)) {
    *result = DECIMAL_1;
    s21_big_decimal res = decimal_to_big_decimal(*result);
    sing_for_res(sc_1, sc_2, &res);
    *result = big_decimal_to_decimal(res);
  } else if (s21_is_equal_null(value_1))
    *result = DECIMAL_0;
  else {
    s21_big_decimal val_1 = decimal_to_big_decimal(value_1);
    s21_big_decimal val_2 = decimal_to_big_decimal(value_2);
    s21_big_decimal res = {0};
    sign_and_pow scale_1 = parsing_sign_and_pow_big(val_1);
    sign_and_pow scale_2 = parsing_sign_and_pow_big(val_2);

    if (scale_1.pow == scale_2.pow && scale_1.pow == 0) {
      div_zero_pow_big(val_1, val_2, &res);

      if (res.bits[3] != 0 || res.bits[4] != 0 || res.bits[5] != 0 ||
          get_power_big(res) > 28)
        round_to_dec_precision(&res);
      else
        remove_nonsignificant_zeros(&res);
      // устанавливаем знак в резалт
      sing_for_res(sc_1, sc_2, &res);
      // установка флага переполнения на бесконечно большое или бесконечно малое
      apply_flag_to_mauch_or_litle(&flag, res);
    } else {
      // 1. сделать оба числа с 0 степенью
      // 2. сделать деление биг децималов одинаковой степени
      pow1_pow2_align(&val_1, &val_2, value_1, value_2);
      flag = div_zero_pow_big(val_1, val_2, &res);

      char str_result[1000] = "";
      s21_from_big_decimal_to_str(res, str_result);

      if (res.bits[3] != 0 || res.bits[4] != 0 || res.bits[5] != 0 ||
          get_power_big(res) > 28)
        round_to_dec_precision(&res);
      else
        remove_nonsignificant_zeros(&res);

      // устанавливаем знак в резалт
      sing_for_res(sc_1, sc_2, &res);
      // установка флага переполнения на бесконечно большое или бесконечно малое
      apply_flag_to_mauch_or_litle(&flag, res);
    }
    *result = big_decimal_to_decimal(res);

    char str_result[1000] = "";
    s21_from_decimal_to_str(*result, str_result);
    if (s21_is_equal_null(*result) && str_result[0] == '0') flag = 2;
  }
  if (flag) decimal_to_null(result);
  return flag;
}

int div_zero_pow_big(s21_big_decimal value_1, s21_big_decimal value_2,
                     s21_big_decimal *res_temp) {
  int flag = 0;  // чтобы функцию не плодить новую
  s21_big_decimal res_left_half = {0};
  s21_big_decimal res_right_half = {0};

  char str_value_1[1000] = "";
  s21_from_big_decimal_to_str(value_1, str_value_1);
  char str_value_2[1000] = "";
  s21_from_big_decimal_to_str(value_2, str_value_2);

  // получаем целую часть числа
  s21_big_decimal ostatok =
      s21_div_integer_part_big(value_1, value_2, &res_left_half);

  if (s21_is_greater_big(res_left_half, DECIMAL_BIG_MAX_DECIMAL)) {
    flag = 1;
    *res_temp = res_left_half;
  }

  if (!flag) {
    char str_left_half[1000] = "";
    s21_from_big_decimal_to_str(res_left_half, str_left_half);

    s21_div_float_part_big(ostatok, value_2, &res_right_half);

    char str_right_half[1000] = "";
    s21_from_big_decimal_to_str(res_right_half, str_right_half);

    sign_and_pow scale_1 = parsing_sign_and_pow_big(res_left_half);
    sign_and_pow scale_2 = parsing_sign_and_pow_big(res_right_half);
    plus_plus_big(res_left_half, res_right_half, scale_1, scale_2, res_temp,
                  &flag);
    set_power_big(res_temp, get_power_big(res_right_half));
  }
  return flag;
}

s21_big_decimal s21_div_float_part_big(s21_big_decimal value_1,
                                       s21_big_decimal value_2,
                                       s21_big_decimal *result) {
  // обнунилил служебный бит
  value_1.bits[7] = value_2.bits[7] = 0;
  s21_big_decimal bufer = {0};

  char str_value_1[1000] = "";
  s21_from_big_decimal_to_str(value_1, str_value_1);
  int pow = 0;

  s21_big_decimal divider = value_2;  // делитель
  s21_big_decimal float_part = value_1;

  int32_t i = 0;

  while (1) {
    // вытащили бит из делимого в флоат

    s21_mul_equal_pow_big(float_part, DECIMAL_10_BIG, &bufer);
    float_part = bufer;
    big_decimal_to_null(&bufer);
    pow++;

    // смотрим на флоат, если хватает числа - делам, если нет - еще раз
    // вытаскиваем бит
    if (s21_is_greater_or_equal_big(float_part, divider)) {
      s21_mul_equal_pow_big(*result, DECIMAL_10_BIG, &bufer);
      *result = bufer;
      big_decimal_to_null(&bufer);
      i++;

      while (s21_is_greater_or_equal_big(float_part, divider)) {
        s21_sub_big_small_big(float_part, divider, &bufer);
        float_part = bufer;
        big_decimal_to_null(&bufer);

        add_two_positiv_big(&bufer, *result, DECIMAL_1_BIG);
        *result = bufer;
        big_decimal_to_null(&bufer);
      }
    } else {
      s21_mul_equal_pow_big(*result, DECIMAL_10_BIG, &bufer);
      *result = bufer;
      big_decimal_to_null(&bufer);
      i++;
    }
    if (s21_is_equal_null_big(float_part) || i >= 29) break;
  }
  set_power_big(result, pow);
  return float_part;
}

s21_big_decimal s21_div_integer_part_big(s21_big_decimal value_1,
                                         s21_big_decimal value_2,
                                         s21_big_decimal *result) {
  // обнунилил служебный бит
  value_1.bits[7] = value_2.bits[7] = 0;

  s21_big_decimal divider = value_2;  // делитель
  s21_big_decimal float_part = {0};

  int32_t bits_divisible = count_bits_big(value_1);
  int32_t temp_bit = 0, i = 0;

  if (s21_is_greater_big(
          value_2,
          value_1))  // резалт не трогаем, будет 0, все остальное в остаток
    float_part = value_1;
  else {
    while (1) {
      // вытащили бит из делимого в флоат
      temp_bit = get_bit_big(value_1, bits_divisible - 1 - i);
      float_part = left_shift_big(float_part, 1);
      set_bit_big(&float_part, 0, temp_bit);
      i++;

      // смотрим на флоат, если хватает числа - делам, если нет - еще раз
      // вытаскиваем бит
      if (s21_is_greater_or_equal_big(float_part, divider)) {
        // сделали деление (вычитание)
        *result = left_shift_big(*result, 1);
        set_bit_big(result, 0, 1);
        s21_big_decimal bufer = {0};
        s21_sub_big_small_big(float_part, divider, &bufer);
        big_decimal_to_null(&float_part);
        float_part = bufer;
      } else {
        *result = left_shift_big(*result, 1);
        set_bit_big(result, 0, 0);
      }

      if (bits_divisible - i == 0) break;
    }
  }
  return float_part;
}

// установка знака в резал декомпозиция
void sing_for_res(sign_and_pow sc_1, sign_and_pow sc_2, s21_big_decimal *res) {
  // устанавливаем знак в резалт
  if ((!sc_1.sign && !sc_2.sign) || (sc_1.sign && sc_2.sign))
    set_sign_big(res, 0);
  else
    set_sign_big(res, 1);
}