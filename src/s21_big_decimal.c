#include "s21_decimal.h"

s21_big_decimal decimal_to_big_decimal(s21_decimal value) {
  s21_big_decimal result = {0};
  result.bits[0] = value.bits[0];
  result.bits[1] = value.bits[1];
  result.bits[2] = value.bits[2];
  result.bits[7] = value.bits[3];

  return result;
}

s21_decimal big_decimal_to_decimal(s21_big_decimal value) {
  s21_decimal result = {0};
  result.bits[0] = value.bits[0];
  result.bits[1] = value.bits[1];
  result.bits[2] = value.bits[2];
  result.bits[3] = value.bits[7];
  return result;
}

sign_and_pow parsing_sign_and_pow_big(s21_big_decimal value) {
  sign_and_pow result_scale = {0};
  result_scale.sign = (value.bits[7] >> 31) & 1;
  result_scale.pow =
      (value.bits[7] >> 16) & 0xFF;  // вытаскиваем 8 битов с 16 до 23
  return result_scale;
}

int32_t s21_is_less_big(s21_big_decimal value_1, s21_big_decimal value_2) {
  int flag = 0;
  int sign_1 = get_sign_big(value_1);  // какой знак
  int sign_2 = get_sign_big(value_2);
  int count_1 = get_power_big(value_1);  // степень
  int count_2 = get_power_big(value_2);

  // проверка знака
  if (sign_1 == 1 && sign_2 == 0) {
    flag = 1;
  } else if (sign_1 == 0 && sign_2 == 1) {
    flag = 0;
  } else {
    // проверка если не равны степени
    // выравнивание степени
    if (count_1 > count_2) {
      apply_pow(&value_2, count_1);
    } else {
      apply_pow(&value_1, count_2);
    }

    int i = 5;
    int exit = 0;
    while (exit == 0 && i >= 0) {
      if (value_1.bits[i] < value_2.bits[i]) {
        exit = 1;
        flag = 1;
      } else if (value_1.bits[i] > value_2.bits[i]) {
        exit = 1;
      }

      i--;
    }
  }
  // если оба отрицательные менять знак
  if (sign_1 == 1 && sign_2 == 1 && !s21_is_equal_big(value_1, value_2)) {
    flag = flag ^ 1;
  }
  return flag;
}

/* Больше > */
int32_t s21_is_greater_big(s21_big_decimal value_1, s21_big_decimal value_2) {
  int32_t flag = 0;
  if (!s21_is_less_big(value_1, value_2) && !s21_is_equal_big(value_1, value_2))
    flag = 1;
  return flag;
}

/* Больше или равно >= */
int32_t s21_is_greater_or_equal_big(s21_big_decimal value_1,
                                    s21_big_decimal value_2) {
  int32_t flag = 0;
  if (s21_is_equal_big(value_1, value_2) ||
      s21_is_greater_big(value_1, value_2))
    flag = 1;
  return flag;
}

/* Равно == */
int32_t s21_is_equal_big(s21_big_decimal value_1, s21_big_decimal value_2) {
  int32_t flag = 0;
  if (value_1.bits[0] == value_2.bits[0] &&
      value_1.bits[1] == value_2.bits[1] &&
      value_1.bits[2] == value_2.bits[2] &&
      value_1.bits[3] == value_2.bits[3] &&
      value_1.bits[4] == value_2.bits[4] &&
      value_1.bits[5] == value_2.bits[5] &&
      value_1.bits[6] == value_2.bits[6] && value_1.bits[7] == value_2.bits[7])
    flag = 1;
  return flag;
}

/* Не равно != */
int32_t s21_is_not_equal_big(s21_big_decimal value_1, s21_big_decimal value_2) {
  int32_t flag = 1;
  if (s21_is_equal_big(value_1, value_2)) flag = 0;
  return flag;
}

/* из децимала в строку десятичного числа для анализа переполнения по степен и в
 * принтдек */
void s21_from_big_decimal_to_str(s21_big_decimal value, char *dec_str) {
  sign_and_pow scale = parsing_sign_and_pow_big(value);

  char binary_number[1000] = {'\0'};
  char dec_number[1000] = {'\0'};  // под \0 и под точку(пока не придумал как)

  // делаем строку двоичного числа
  str_bin_big(binary_number, &value);

  // делаем строку десятичного числа без степени
  // проходим по двоичной строке и преобразуем в десятичное число
  str_dec_without_pow(binary_number, dec_number);

  // добавление в строку десятичного числа точки и добавление нуля в целую часть
  // и нулей до первой значащей цифры, если число меньше 0
  pow_to_str_dec(dec_number, dec_str, scale);

  if (strlen(dec_number) == 0) dec_number[0] = '0';

  strcpy(dec_str, dec_number);
}

void str_bin_big(char *binary_number, s21_big_decimal *value) {
  int32_t bit = 0, k = 0;
  for (int32_t j = 5; j >= 0; j--) {
    for (int32_t i = 0; i < 32; i++) {
      bit = 1 & (value->bits[j] >> 31);
      binary_number[k] = bit + '0';
      value->bits[j] = value->bits[j] << 1;
      k++;
    }
  }
}

// определеяет количество битов в биг десимал числе
int count_bits_big(s21_big_decimal value) {
  int res = 192;
  int esc = 0;
  for (int i = 5; i >= 0; i--) {
    for (int j = 31; j >= 0; j--) {
      if ((1 & (value.bits[i] >> j)) == 0)
        res--;
      else {
        esc = 1;
        break;
      }
    }
    if (esc) break;
  }

  return res;
}

int set_bit_big(s21_big_decimal *value, uint8_t bit_idx, uint8_t bit_val) {
  int res = 0;
  if (bit_idx > 192 || bit_val > 1) {
    res = 1;
  } else {
    if (bit_val != get_bit_big(*value, bit_idx)) {
      int int_idx = bit_idx / 32;
      bit_idx = bit_idx % 32;
      (*value).bits[int_idx] ^= (1 << bit_idx);
    }
  }
  return res;
}

int get_bit_big(s21_big_decimal value, uint8_t bit_idx) {
  int res = 0;
  int int_idx = bit_idx / 32;
  bit_idx = bit_idx % 32;
  if (value.bits[int_idx] & (1 << bit_idx)) {
    res = 1;
  }
  return res;
}

s21_big_decimal left_shift_big(s21_big_decimal value, int n) {
  s21_big_decimal res = value;
  int higth_bit = count_bits_big(res);

  for (; n > 0; n--) {
    higth_bit = count_bits_big(res);
    for (int i = higth_bit - 1; i >= 0; i--) {
      // вытащили бит i записали его в i+1 место
      int bit = get_bit_big(res, i);
      set_bit_big(&res, i + 1, bit);
    }
    set_bit_big(&res, 0, 0);
  }

  return res;
}

s21_big_decimal right_shift_big(s21_big_decimal value, int n) {
  s21_big_decimal res = value;
  int higth_bit = count_bits_big(res);

  for (int j = 0; j < n; j++) {
    higth_bit = count_bits_big(res);
    for (int i = 1; i <= higth_bit - 1; i++) {
      // вытащили бит i записали его в i-1 место
      int bit = get_bit_big(res, i);
      set_bit_big(&res, i - 1, bit);
    }
    set_bit_big(&res, higth_bit - 1, 0);
  }

  return res;
}

void s21_sub_big_small_big(s21_big_decimal value_1, s21_big_decimal value_2,
                           s21_big_decimal *result) {
  int32_t bit_1 = 0;
  int32_t bit_2 = 0;
  int32_t bit_res = 0;
  int32_t borrow = 0;

  for (int32_t j = 0; j <= 5; j++) {
    for (int32_t i = 31; i >= 0; i--) {
      bit_1 = (value_1.bits[j] >> (31 - i)) & 1;
      bit_2 = (value_2.bits[j] >> (31 - i)) & 1;

      bit_res = bit_1 - bit_2 - borrow;

      if (bit_res < 0) {
        bit_res += 2;
        borrow = 1;
      } else {
        borrow = 0;
      }
      set_bit_big(result, j * 32 + (31 - i), bit_res);
    }
  }
}

void big_decimal_to_null(s21_big_decimal *value) {
  value->bits[0] = 0;
  value->bits[1] = 0;
  value->bits[2] = 0;
  value->bits[3] = 0;
  value->bits[5] = 0;
  value->bits[6] = 0;
  value->bits[7] = 0;
}

// умножение двух биг децималов одинаковой степени
int s21_mul_equal_pow_big(s21_big_decimal value_1, s21_big_decimal value_2,
                          s21_big_decimal *result) {
  int flag = 0;
  s21_big_decimal res = {0};
  s21_big_decimal tmp = value_1;
  int add_res;
  int bits = count_bits_big(value_2);
  for (int i = 0; i < bits; i++) {
    int current_bit = 1 & right_shift_big(value_2, i).bits[0];
    if (current_bit) {
      tmp = left_shift_big(value_1, i);
      add_res = add_two_positiv_big(&res, tmp, res);
      if (add_res) {
        flag = add_res;
      }
    }
  }

  *result = res;
  return flag;
}

int32_t add_two_positiv_big(s21_big_decimal *result, s21_big_decimal value_1,
                            s21_big_decimal value_2) {
  int flag = 0;
  int overflow_0 = 0;
  int overflow_1 = 0;

  (*result).bits[0] = value_1.bits[0] + value_2.bits[0];
  overflow_0 = add_overflow(value_1.bits[0], value_2.bits[0]);

  for (int i = 1; i <= 5; i++) {
    (*result).bits[i] = value_1.bits[i] + value_2.bits[i];
    overflow_1 = add_overflow(value_1.bits[i], value_2.bits[i]);
    if (overflow_0) {
      (*result).bits[i] += 1;
      if ((*result).bits[i] == 0) {
        overflow_1 = 1;
      }
    }
    overflow_0 = overflow_1;
  }

  // проверка на переполнение (больше макс децимал)
  int pow_1 = get_power_big(value_1);
  int pow_2 = get_power_big(value_2);
  int bufer_pow = pow_1 <= pow_2 ? pow_1 : pow_2;
  s21_big_decimal bufer = *result;
  s21_big_decimal temp = {0};
  set_power_big(&bufer, bufer_pow);
  // отбросили дробную часть - не стал переписывать функцию транкате для биг
  // децимал
  for (int i = bufer_pow; i > 0; i--) {
    s21_div_integer_part_big(bufer, DECIMAL_10_BIG, &temp);
    bufer = temp;
    big_decimal_to_null(&temp);
  }

  if (s21_is_greater_big(bufer, DECIMAL_BIG_MAX_DECIMAL)) flag = 1;

  return flag;
}

uint8_t get_power_big(s21_big_decimal value) {
  uint8_t res = 0;
  res = (uint8_t)(511 & (value.bits[7] >> 16));
  return res;
}

int set_power_big(s21_big_decimal *value, uint8_t power) {
  int sign = get_sign_big(*value);
  (*value).bits[7] = (uint32_t)(power << 16);
  set_sign_big(value, sign);
  return 0;
}

int get_sign_big(s21_big_decimal value) { return (value.bits[7] >> 31) & 1; }

void set_sign_big(s21_big_decimal *value, int sign) {
  if (sign != get_sign_big(*value)) {
    if (sign == 1) {
      (*value).bits[7] += HIGH_BIT;
    } else {
      (*value).bits[7] -= HIGH_BIT;
    }
  }
}
