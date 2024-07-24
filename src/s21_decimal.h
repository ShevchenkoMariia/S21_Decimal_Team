#ifndef S21_DECIMAL
#define S21_DECIMAL

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define DECIMAL_10 ((s21_decimal){{10, 0, 0, 0}})
#define DECIMAL_10_BIG ((s21_big_decimal){{10, 0, 0, 0, 0, 0, 0, 0}})
#define DECIMAL_5 ((s21_decimal){{5, 0, 0, 0}})
#define DECIMAL_5_BIG ((s21_big_decimal){{5, 0, 0, 0, 0, 0, 0, 0}})
#define DECIMAL_1 ((s21_decimal){{1, 0, 0, 0}})
#define DECIMAL_1_BIG ((s21_big_decimal){{1, 0, 0, 0, 0, 0, 0, 0}})

#define DECIMAL_0 ((s21_decimal){{0, 0, 0, 0}})
#define DECIMAL_0_BIG ((s21_big_decimal){{0, 0, 0, 0, 0, 0, 0, 0}})
#define DECIMAL_0_minus \
  ((s21_decimal){{0, 0, 0, 0b10000000000000000000000000000000}})

#define DECIMAL_BIG_MAX_DECIMAL \
  ((s21_big_decimal){{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0, 0, 0, 0}})

#define HIGH_BIT ((unsigned int)1 << 31)

typedef struct {
  uint32_t bits[4];
} s21_decimal;

typedef struct {
  uint32_t bits[8];
} s21_big_decimal;

typedef struct {
  int32_t sign;
  int32_t pow;
} sign_and_pow;

// Arifmetic
int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);

// Comparison
int32_t s21_is_less(s21_decimal, s21_decimal);
int32_t s21_is_less_or_equal(s21_decimal, s21_decimal);
int32_t s21_is_greater(s21_decimal, s21_decimal);
int32_t s21_is_greater_or_equal(s21_decimal, s21_decimal);
int32_t s21_is_equal(s21_decimal, s21_decimal);
int32_t s21_is_not_equal(s21_decimal, s21_decimal);

// Convertors/Parsers
int s21_from_int_to_decimal(int src, s21_decimal *dst);
int s21_from_float_to_decimal(float src, s21_decimal *dst);
int s21_from_decimal_to_int(s21_decimal src, int *dst);
int s21_from_decimal_to_float(s21_decimal src, float *dst);

// Other
int s21_floor(s21_decimal value, s21_decimal *result);
int s21_round(s21_decimal value, s21_decimal *result);
int s21_truncate(s21_decimal value, s21_decimal *result);
int s21_negate(s21_decimal value, s21_decimal *result);

// ****************************** support_func поддерживаающие фукнции -
// используются в разных математических операциях несколько раз
// ******************************
//  зануление децимал
void decimal_to_null(s21_decimal *value);
// из децимала в строку десятичного числа для анализа переполнения по степени
void s21_from_decimal_to_str(s21_decimal value, char *dec_str);
// формирование строки двоичного числа для декомпозиции
void str_bin(char *binary_number, s21_decimal *value);
// формирование строки десятичного числа из строки двоичного числа для
// декомпозиции
void str_dec_without_pow(char *binary_number, char *dec_number);
// добавление в строку десятичного числа точки и добавление нуля в целую часть и
// нулей до первой значащей цифры. если число меньше 0 (есть степень в децимале)
void pow_to_str_dec(char *dec_number, char *dec_str, sign_and_pow scale);
//  изменение децималов под единую степень или ошибка если переполнение
void add_multi_pow(s21_decimal *value_1, s21_decimal *value_2);
void add_pow_to_mul(s21_decimal value_1, s21_decimal value_2,
                    s21_decimal *result);
int get_last_dec_digit(s21_big_decimal value);
// установка флага переполнения на бесконечно большое или бесконечно малое
void apply_flag_to_mauch_or_litle(int *flag, s21_big_decimal res);
// установка степени в биг децимал
void apply_pow(s21_big_decimal *value, int32_t pow);
//  умножение двух децималов одинаковой степени
int s21_mul_equal_pow(s21_decimal value_1, s21_decimal value_2,
                      s21_decimal *result);

// ****************************** s21_add сопутствующие функции
// ****************************** декомпозиция набора функций по сложению двух
// положительных
void math_add(s21_big_decimal val_1, s21_big_decimal val_2,
              s21_big_decimal *res, sign_and_pow scale_1, sign_and_pow scale_2,
              int *flag);
// сумма двух децималов - повторяется два раза, декомпозиция
int32_t add_two_positiv(s21_decimal *result, s21_decimal value_1,
                        s21_decimal value_2);
// оценка переполнения
int add_overflow(uint32_t x, uint32_t y);

// увеличиваем степень числа. для декомпозиции
void pow_up(int32_t *change_pow, s21_decimal *value_1, s21_decimal *value_2,
            sign_and_pow scale_2);
// суммирование биг дицималов. декомпозиция
void plus_plus_big(s21_big_decimal value_1, s21_big_decimal value_2,
                   sign_and_pow scale_1, sign_and_pow scale_2,
                   s21_big_decimal *result, int32_t *flag);
// шизгарим нули в конец децимала если степень есть и длина меньше 29
void add_zero_to_end_result(s21_decimal *result, int len_res);

// ****************************** s21_sub сопутствующие функции
// ****************************** декомпозиция набора функций по вычитанию двух
// чисел val2 БОЛЬШЕЕ
void math_sub_small_big(s21_big_decimal *val_1, s21_big_decimal *val_2,
                        s21_big_decimal *res, sign_and_pow scale_1,
                        sign_and_pow scale_2, s21_decimal value_1,
                        s21_decimal value_2, int *flag);
// декомпозиция набора функций по вычитанию двух чисел val1 БОЛЬШЕЕ
void math_sub_big_small(s21_big_decimal *val_1, s21_big_decimal *val_2,
                        s21_big_decimal *res, sign_and_pow scale_1,
                        sign_and_pow scale_2, s21_decimal value_1,
                        s21_decimal value_2, int *flag);
// тоже но биг децимал
void s21_sub_big_small_big_decimal(s21_big_decimal value_1,
                                   s21_big_decimal value_2,
                                   s21_big_decimal *result);
// выравнинваниие степени у биг децимал
void pow1_pow2_align(s21_big_decimal *val_1, s21_big_decimal *val_2,
                     s21_decimal value_1, s21_decimal value_2);

// ****************************** s21_mul
int s21_mul_big(s21_decimal value_1, s21_decimal value_2,
                s21_big_decimal *result);
void set_pow_and_mul(s21_decimal value_1, s21_decimal value_2,
                     s21_big_decimal *result);
void remove_nonsignificant_zeros(s21_big_decimal *value);
void round_to_dec_precision(s21_big_decimal *value);
void bank_rounding(s21_big_decimal *value, int *scale, int *len);

// ****************************** s21_div
// функция деления биг децималов на цело. выход функции остаток от деления
s21_big_decimal s21_div_integer_part_big(s21_big_decimal value_1,
                                         s21_big_decimal value_2,
                                         s21_big_decimal *result);
// деление двух биг децималов одинаковой степени. резалт - просто децимал
int div_zero_pow_big(s21_big_decimal value_1, s21_big_decimal value_2,
                     s21_big_decimal *res_temp);
// установка знака в резал декомпозиция
void sing_for_res(sign_and_pow sc_1, sign_and_pow sc_2, s21_big_decimal *res);
s21_big_decimal s21_div_float_part_big(s21_big_decimal value_1,
                                       s21_big_decimal value_2,
                                       s21_big_decimal *result);

// ****************************** s21_comparision сопутствующие функции
// ****************************** сравнение с нулем без знака и степени - защита
// от дурного ввода
int32_t s21_is_equal_null(s21_decimal value);
int32_t s21_is_equal_null_big(s21_big_decimal value);

// // ****************************** check_correct и соптствующие функции
// // ****************************** проверка децимал числа на корректность
// данных функция деления децималов только ЦЕЛАЯ ЧАСТЬ. выход фукнции остаток от
// деления
s21_decimal s21_div_integer_part(s21_decimal value_1, s21_decimal value_2,
                                 s21_decimal *result);

// ****************************** shift битовый сдвиг влево и вправо
// ******************************
s21_decimal left_shift(s21_decimal value, int n);
s21_decimal right_shift(s21_decimal value, int n);

// ****************************** sing_and_pow функции работы с степенью и
// знаком ****************************** парсинг знака и разряда для точки
sign_and_pow parsing_sign_and_pow(s21_decimal value);
// получить (get) и записать (set) степень (power) и знак (sign)
uint8_t get_power(s21_decimal value);
int set_power(s21_decimal *value, uint8_t power);
int get_sign(s21_decimal value);
void set_sign(s21_decimal *value, int sign);

// ****************************** count_bit ******************************
// определяет старший бит децимал числа
int count_bits(s21_decimal value);

// ****************************** bit работа с конкретным бит
// ****************************** получить (get) и записать (set) бит
int get_bit(s21_decimal value, uint8_t bit_idx);
int set_bit(s21_decimal *value, uint8_t bit_idx, uint8_t bit_val);

// ****************************** big_decimal сопутствующие функции по работе с
// биг децимал ****************************** преобразование обычного децимал в
// биг и биг в обычный
s21_big_decimal decimal_to_big_decimal(s21_decimal value);
s21_decimal big_decimal_to_decimal(s21_big_decimal value);
// парсинг знака и разряда для точки
sign_and_pow parsing_sign_and_pow_big(s21_big_decimal value);
// сравнение двух биг децималов
int32_t s21_is_less_big(s21_big_decimal value_1, s21_big_decimal value_2);
int32_t s21_is_greater_big(s21_big_decimal value_1, s21_big_decimal value_2);
int32_t s21_is_greater_or_equal_big(s21_big_decimal value_1,
                                    s21_big_decimal value_2);
int32_t s21_is_equal_big(s21_big_decimal value_1, s21_big_decimal value_2);
int32_t s21_is_not_equal_big(s21_big_decimal value_1, s21_big_decimal value_2);
// биг децимал в строку, остальные функции из децимала - там разницы нет
void s21_from_big_decimal_to_str(s21_big_decimal value, char *dec_str);
void str_bin_big(char *binary_number, s21_big_decimal *value);
// определеяет количество битов в биг десимал числе
int count_bits_big(s21_big_decimal value);
// получить (get) и записать (set) бит для биг децимал
int get_bit_big(s21_big_decimal value, uint8_t bit_idx);
int set_bit_big(s21_big_decimal *value, uint8_t bit_idx, uint8_t bit_val);
//   сдвиг налево на n бит
s21_big_decimal left_shift_big(s21_big_decimal value, int n);
s21_big_decimal right_shift_big(s21_big_decimal value, int n);
// вычитание из большого маленькое для биг децимал
void s21_sub_big_small_big(s21_big_decimal value_1, s21_big_decimal value_2,
                           s21_big_decimal *result);
void big_decimal_to_null(s21_big_decimal *value);
// сложение двух биг децималов одинаковой степени
int32_t add_two_positiv_big(s21_big_decimal *result, s21_big_decimal value_1,
                            s21_big_decimal value_2);
//  умножение двух биг децималов одинаковой степени
int s21_mul_equal_pow_big(s21_big_decimal value_1, s21_big_decimal value_2,
                          s21_big_decimal *result);
// получить (get) и записать (set) степень (power) и знак (sign)
uint8_t get_power_big(s21_big_decimal value);
int set_power_big(s21_big_decimal *value, uint8_t power);
int get_sign_big(s21_big_decimal value);
void set_sign_big(s21_big_decimal *value, int sign);

// проверка на валидность s21_decimal для функции s21_from_decimal_to_int
int validation_check(s21_decimal src);
int validation_check_for_float(s21_decimal src);
int count_bits(s21_decimal src);
s21_decimal div_to_ten(s21_decimal src);
int beginning(int arr[]);
int first_5_bits(int arr[], int index);
int finding_the_first_divisible(int first_num, int *index);
void filling_in_the_array(s21_decimal src, int *arr);

#endif
