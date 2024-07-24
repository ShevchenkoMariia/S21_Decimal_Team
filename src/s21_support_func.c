#include "s21_decimal.h"

void decimal_to_null(s21_decimal *value) {
  value->bits[0] = 0;
  value->bits[1] = 0;
  value->bits[2] = 0;
  value->bits[3] = 0;
}

/* из децимала в строку десятичного числа для анализа переполнения по степен и в
 * принтдек */
void s21_from_decimal_to_str(s21_decimal value, char *dec_str) {
  sign_and_pow scale = parsing_sign_and_pow(value);

  char binary_number[97] = {'\0'};
  char dec_number[31] = {'\0'};
  // делаем строку двоичного числа
  str_bin(binary_number, &value);

  // делаем строку десятичного числа без степени
  // проходим по двоичной строке и преобразуем в десятичное число
  str_dec_without_pow(binary_number, dec_number);

  // добавление в строку десятичного числа точки и добавление нуля в целую часть
  // и нулей до первой значащей цифры, если число меньше 0
  pow_to_str_dec(dec_number, dec_str, scale);

  if (strlen(dec_number) == 0) dec_number[0] = '0';

  strcpy(dec_str, dec_number);
}

void str_bin(char *binary_number, s21_decimal *value) {
  int32_t bit = 0, k = 0;
  for (int32_t j = 2; j >= 0; j--) {
    for (int32_t i = 0; i < 32; i++) {
      bit = 1 & (value->bits[j] >> 31);
      binary_number[k] = bit + '0';
      value->bits[j] = value->bits[j] << 1;
      k++;
    }
  }
}

void str_dec_without_pow(char *binary_number, char *dec_number) {
  // делаем строку десятичного числа без степени
  // проходим по двоичной строке и преобразуем в десятичное число
  for (int32_t i = 0; binary_number[i] != '\0'; i++) {
    // Умножаем текущее десятичное число на 2
    int32_t carry = 0;
    int32_t len = strlen(dec_number);
    for (int32_t i = len - 1; i >= 0; i--) {
      int32_t num = (dec_number[i] - '0') * 2 + carry;
      dec_number[i] = (num % 10) + '0';
      carry = num / 10;
    }
    if (carry > 0) {
      memmove(dec_number + 1, dec_number, len + 1);
      dec_number[0] = carry + '0';
    }

    // Если текущий бит равен '1', прибавляем 1 к десятичному числу
    if (binary_number[i] == '1') {
      int32_t carry = 1;
      int32_t len = strlen(dec_number);
      for (int32_t i = len - 1; i >= 0; i--) {
        int32_t num = (dec_number[i] - '0') + carry;
        dec_number[i] = (num % 10) + '0';
        carry = num / 10;
        if (carry == 0) break;
      }
      if (carry > 0) {
        memmove(dec_number + 1, dec_number, len + 1);
        dec_number[0] = carry + '0';
      }
    }
  }
}

void pow_to_str_dec(char *dec_number, char *dec_str, sign_and_pow scale) {
  int32_t len = strlen(dec_number);
  if (len > scale.pow && scale.pow != 0) {
    memmove(dec_number + (len - scale.pow + 1), dec_number + (len - scale.pow),
            scale.pow + 1);
    dec_number[len - scale.pow] = '.';
  } else if (len == scale.pow && scale.pow != 0) {
    // Перемещаем строку вправо на один символ, включая '\0'
    memmove(dec_number + 2, dec_number, len + 1);
    // Вставляем '0' в начало
    dec_number[0] = '0';
    // Вставляем точку после '0'
    dec_number[1] = '.';
  } else if (len < scale.pow && scale.pow != 0) {
    // Количество нулей перед числом
    int32_t num_zero = scale.pow - len;
    // Перемещаем строку вправо на num_zeros + 2 (для "0."), включая '\0'
    memmove(dec_number + num_zero + 2, dec_number, len + 1);
    // Вставляем '0' в начало
    dec_number[0] = '0';
    // Вставляем точку после '0'
    dec_number[1] = '.';
    // Заполняем необходимое количество нулей
    for (int32_t i = 0; i < num_zero; i++) {
      dec_number[2 + i] = '0';
    }
  }
  strcpy(dec_str, dec_number);
}

int get_last_dec_digit(s21_big_decimal value) {
  int last_digit = 0;
  int pow = 0;
  int bit = 0;
  for (int j = 0; j <= 6; j++) {
    for (int i = 0; i < 32; i++) {
      bit = 1 & value.bits[j];
      if (bit) {
        if (pow == 0) {
          last_digit += 1;
        } else {
          switch (pow % 4) {
            case 0: {
              last_digit += 6;
              break;
            }
            case 1: {
              last_digit += 2;
              break;
            }
            case 2: {
              last_digit += 4;
              break;
            }
            case 3: {
              last_digit += 8;
              break;
            }
            default:
              break;
          }
          last_digit = last_digit % 10;
        }
      }
      value.bits[j] = value.bits[j] >> 1;
      pow++;
    }
  }
  return last_digit;
}

s21_decimal s21_div_integer_part(s21_decimal value_1, s21_decimal value_2,
                                 s21_decimal *result) {
  // обнунилил служебный бит
  value_1.bits[3] = value_2.bits[3] = 0;

  s21_decimal divider = value_2;  // делитель
  s21_decimal float_part = {0};

  int32_t bits_divisible = count_bits(value_1);
  int32_t temp_bit = 0;

  if (s21_is_greater(
          value_2,
          value_1))  // резалт не трогаем, будет 0, все остальное в остаток
    float_part = value_1;
  else {
    int i = 0;
    while (1) {
      // вытащили бит из делимого в флоат
      temp_bit = get_bit(value_1, bits_divisible - 1 - i);
      float_part = left_shift(float_part, 1);
      set_bit(&float_part, 0, temp_bit);
      i++;

      // смотрим на флоат, если хватает числа - делам, если нет - еще раз
      // вытаскиваем бит
      if (s21_is_greater_or_equal(float_part, divider)) {
        // сделали деление (вычитание)

        *result = left_shift(*result, 1);
        set_bit(result, 0, 1);
        s21_decimal bufer = {0};
        s21_sub(float_part, divider, &bufer);
        decimal_to_null(&float_part);
        float_part = bufer;
      } else {
        *result = left_shift(*result, 1);
        set_bit(result, 0, 0);
      }

      if (bits_divisible - i == 0) {
        break;
      }
    }
  }

  return float_part;
}

int count_bits(s21_decimal value) {
  int res = 96;
  while (get_bit(value, res - 1) == 0 && res > 0) {
    res--;
  }

  return res;
}

s21_decimal right_shift(s21_decimal value, int n) {
  s21_decimal res = value;
  int higth_bit = count_bits(res);

  for (int j = 0; j < n; j++) {
    higth_bit = count_bits(res);
    for (int i = 1; i <= higth_bit - 1; i++) {
      // вытащили бит i записали его в i-1 место
      int bit = get_bit(res, i);
      set_bit(&res, i - 1, bit);
    }
    set_bit(&res, higth_bit - 1, 0);
  }
  // s21_decimal res = value;
  // int last_high, last_mid;
  // for (int i = 0; i < n; i++) {
  //   last_high = 1 & res.bits[2];
  //   res.bits[2] = res.bits[2] >> 1;
  //   last_mid = 1 & res.bits[1];
  //   res.bits[1] = res.bits[1] >> 1;
  //   if (res.bits[1] < 0) res.bits[1] += (HIGH_BIT);
  //   if (last_high) {
  //     res.bits[1] += (HIGH_BIT);
  //   }
  //   res.bits[0] = res.bits[0] >> 1;
  //   if (res.bits[0] < 0) res.bits[0] += (HIGH_BIT);
  //   if (last_mid) {
  //     res.bits[0] += (HIGH_BIT);
  //   }
  // }

  return res;
}

s21_decimal left_shift(s21_decimal value, int n) {
  s21_decimal res = value;
  int higth_bit = count_bits(res);

  for (; n > 0; n--) {
    higth_bit = count_bits(res);
    for (int i = higth_bit - 1; i >= 0; i--) {
      // вытащили бит i записали его в i+1 место
      int bit = get_bit(res, i);
      set_bit(&res, i + 1, bit);
    }
    set_bit(&res, 0, 0);
  }
  // s21_decimal res = {value.bits[0], value.bits[1], value.bits[2],
  //                    value.bits[3]};
  // int first_low, first_mid;
  // for (int i = 0; i < n; i++) {
  //   first_low = 1 & (res.bits[0] >> 31);
  //   res.bits[0] = res.bits[0] << 1;
  //   first_mid = 1 & (res.bits[1] >> 31);
  //   res.bits[1] = res.bits[1] << 1;
  //   if (first_low) {
  //     res.bits[1] += 1;
  //   }
  //   res.bits[2] = res.bits[2] << 1;
  //   if (first_mid) {
  //     res.bits[2] += 1;
  //   }
  // }
  return res;
}

int set_bit(s21_decimal *value, uint8_t bit_idx, uint8_t bit_val) {
  int res = 0;
  if (bit_idx > 127 || bit_val > 1) {
    res = 1;
  } else {
    if (bit_val != get_bit(*value, bit_idx)) {
      int int_idx = bit_idx / 32;
      bit_idx = bit_idx % 32;
      (*value).bits[int_idx] ^= (1 << bit_idx);
    }
  }
  return res;
}

int get_bit(s21_decimal value, uint8_t bit_idx) {
  int res = 0;
  int int_idx = bit_idx / 32;
  bit_idx = bit_idx % 32;
  if (value.bits[int_idx] & (1 << bit_idx)) {
    res = 1;
  }
  return res;
}

// установка флага переполнения на бесконечно большое или бесконечно малое
void apply_flag_to_mauch_or_litle(int *flag, s21_big_decimal res) {
  for (int i = 3; i < 7; i++) {
    if (res.bits[i] != 0) *flag = 1;
  }
  if (*flag && get_sign_big(res)) {
    *flag = 2;
  }
}

// проверяем s21_decimal на валидность
int validation_check(s21_decimal src) {
  int res = 0;

  // проверка для первых 16 бит на заполненность нулями
  for (int i = 0; i <= 15; i++)
    if ((src.bits[3] >> i) & 1) res = 1;

  // проверка битов 17-23, если содержимое больше 28 или меньше 0, то ошибка
  int degree = (src.bits[3] >> 16) & 0xFF;
  if (degree > 28 || degree < 0) res = 1;

  // проверка битов 24-30 на заполненость нулями, 31 - знаковый, не трогаем
  for (int i = 24; i <= 30; i++)
    if ((src.bits[3] >> i) & 1) res = 1;

  return res;
}