#include "s21_decimal.h"

int s21_is_equal(s21_decimal value_1, s21_decimal value_2) {
  int flag = 0;
  int have_res = 0;
  int sign_1 = get_sign(value_1);
  int sign_2 = get_sign(value_2);
  int count_1 = get_power(value_1);
  int count_2 = get_power(value_2);
  s21_decimal buffer = {0};

  if (s21_is_equal_null(value_1) && s21_is_equal_null(value_2)) {
    flag = 1;
  }
  if (sign_1 == sign_2) {
    if (count_1 < count_2) {
      int sum = count_2 - count_1;
      for (int i = 0; i < sum && !have_res; i++) {
        int result_mul_1 = s21_mul_equal_pow(value_1, DECIMAL_10, &buffer);
        if (result_mul_1 != 0) {
          flag = 0;
          have_res = 1;
        } else {
          value_1 = buffer;
          decimal_to_null(&buffer);
        }
      }
    } else if (count_1 > count_2) {
      int sum = count_1 - count_2;
      for (int i = 0; i < sum && !have_res; i++) {
        int result_mul_2 = s21_mul_equal_pow(value_2, DECIMAL_10, &buffer);
        if (result_mul_2 != 0) {
          flag = 0;
          have_res = 1;
        } else {
          value_2 = buffer;
          decimal_to_null(&buffer);
        }
      }
    }

    if (have_res == 0) {
      int i = 2;
      int exit = 0;
      while (exit == 0 && i >= 0) {
        if (value_1.bits[i] == value_2.bits[i] && exit == 0 && i == 0) {
          exit = 1;
          flag = 1;
        }

        i--;
      }
    }
  }

  return flag;
}

int s21_is_less(s21_decimal value_1, s21_decimal value_2) {
  int flag = 0;
  int have_res = 0;
  int sign_1 = get_sign(value_1);  // какой знак
  int sign_2 = get_sign(value_2);
  int count_1 = get_power(value_1);  // степень
  int count_2 = get_power(value_2);
  s21_decimal buffer = {{0}};

  // проверка знака
  if (sign_1 == 1 && sign_2 == 0) {
    flag = 1;
  } else if (sign_1 == 0 && sign_2 == 1) {
    flag = 0;
  } else {
    //  проверка если не равны степени домножать на 10
    if (count_1 < count_2) {
      int sum = count_2 - count_1;
      for (int i = 0; i < sum && !have_res; i++) {
        int mul_res = s21_mul_equal_pow(value_1, DECIMAL_10, &buffer);
        if (mul_res) {
          flag = 0;
          have_res = 1;
        } else {
          value_1 = buffer;
          decimal_to_null(&buffer);
        }
      }
    } else if (count_1 > count_2) {
      int sum = count_1 - count_2;
      for (int i = 0; i < sum && !have_res; i++) {
        int result_mul_2 = s21_mul_equal_pow(value_2, DECIMAL_10, &buffer);
        if (result_mul_2 != 0) {
          flag = 1;
          have_res = 1;
        } else {
          value_2 = buffer;
          decimal_to_null(&buffer);
        }
      }
    }
    if (have_res == 0) {
      int i = 2;
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
  }
  //  если оба отрицательные менять знак !s21_is_equal(value_1, value_2)
  if (sign_1 == 1 && sign_2 == 1 && !s21_is_equal(value_1, value_2)) {
    flag = flag ^ 1;
  }
  return flag;
}

int s21_is_less_or_equal(s21_decimal value_1, s21_decimal value_2) {
  int flag = 0;
  if (s21_is_equal(value_1, value_2)) {
    flag = 1;
  } else {
    flag = s21_is_less(value_1, value_2);
  }

  return flag;
}

int s21_is_greater(s21_decimal value_1, s21_decimal value_2) {
  int flag = 0;
  if (!s21_is_less(value_1, value_2) && !s21_is_equal(value_1, value_2)) {
    flag = 1;
  }
  return flag;
}

int s21_is_greater_or_equal(s21_decimal value_1, s21_decimal value_2) {
  int flag = 0;
  if (s21_is_equal(value_1, value_2)) {
    flag = 1;
  } else {
    flag = s21_is_greater(value_1, value_2);
  }
  return flag;
}

int s21_is_not_equal(s21_decimal value_1, s21_decimal value_2) {
  int flag = 0;
  if (!s21_is_equal(value_1, value_2)) {
    flag = 1;
  }
  return flag;
}

int32_t s21_is_equal_null(s21_decimal value) {
  int32_t flag = 0;

  if (value.bits[0] == DECIMAL_0.bits[0] &&
      value.bits[1] == DECIMAL_0.bits[1] && value.bits[2] == DECIMAL_0.bits[2])
    flag = 1;
  return flag;
}

int32_t s21_is_equal_null_big(s21_big_decimal value) {
  int32_t flag = 1;

  for (int i = 6; i >= 0; i--) {
    if (value.bits[i] != DECIMAL_0_BIG.bits[i]) flag = 0;
  }

  return flag;
}
