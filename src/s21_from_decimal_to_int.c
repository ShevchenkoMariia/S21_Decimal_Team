#include "s21_decimal.h"

int s21_from_decimal_to_int(s21_decimal src, int *dst) {
  // переменная res возвращает результат функции: 1 - fail, 0 - ok
  int res = 0;
  // переменная, в которую будем записывать число из s21_decimal
  int num = 0;

  // проверка s21_decimal на корректность
  int validity = validation_check(src);

  if (validity || dst == NULL)
    res = 1;
  else {
    // получаем знак
    int sign = (src.bits[3] >> 31) & 1;

    // 0xFF = 11111111, изолируем первые 8 бит, содерщащие степень
    // определяем наличие степени и саму степень
    int scale = (src.bits[3] >> 16) & 0xFF;

    if (scale != 0) {
      for (int i = 0; i < scale; i++) {
        s21_decimal my_num = div_to_ten(src);
        src = my_num;
      }
      if (src.bits[1] != 0 || src.bits[2] != 0)
        res = 1;
      else
        num = src.bits[0];
    }
    if (scale == 0 && ((src.bits[3] >> 31) & 1) == 0 &&
        src.bits[0] == 0b10000000000000000000000000000000)
      res = 1;
    else if (scale == 0 && src.bits[1] == 0 && src.bits[2] == 0 &&
             src.bits[0] != 0xffffffff)
      num = src.bits[0];

    // проверка на переполнение
    if (src.bits[0] == 0xffffffff) res = 1;

    // ставим знак
    if (sign && !res) num = -1 * num;

    if (!res)
      *dst = num;
    else
      *dst = 0;
  }
  return res;
}

// деление на 10
s21_decimal div_to_ten(s21_decimal src) {
  // создаем массив, в котором будет храниться полное число
  int arr[96] = {0};

  // заполняем массив 0 и 1 из src
  int j = 2;
  for (int i = 95; i >= 0; i--) {
    if (i <= 63 && i >= 32) j = 1;
    if (i <= 31 && i >= 0) j = 0;
    arr[i] = (src.bits[j] >> i) & 1;
  }

  int index =
      beginning(arr);  // index - первое вхождение 1 (откуда начинается число)

  int first = first_5_bits(
      arr,
      index);  // берем первый 5 битов числа и записываем в переменную first

  int divisible = finding_the_first_divisible(
      first, &index);  // находим первое делимое (4 или 5 бит) и смещаем index

  divisible -= 10;  // делимое дальше всегда будет равно остатку деления на 10

  int res[96] = {0},
      res_index = 1;  // массив для результата и индекс для этого массива
  res[0] = 1;  // первый раз уже поделили, результат записали

  // начинаем цикл с делением
  for (int i = index; i >= 0; i--) {
    divisible =
        divisible
        << 1;  // делимое сдвигаем на 1, чтобы освободить место новому биту
    if (arr[i]) divisible |= 1;  // если новый бит 1, то добавляем его в делимое
    if (divisible < 10) res[res_index++] = 0;
    if (divisible >= 10) {
      divisible -= 10;  // отнимаем 10, обновляя делимое
      res[res_index++] = 1;
    }
  }

  int num_div_to_ten[96] = {
      0};  // создаем массив, куда запишем 0 и 1 в правильном порядке
  int help = 0;  // вспомогательная переменная

  for (int i = res_index - 1; i >= 0; i--)  // индекс с которого начнется запись
                                            // числа (результат деления на 10)
    num_div_to_ten[help++] = res[i];

  // склеиваем в decimal 0 и 1 из массива с конечным результатом
  s21_decimal my_num = {0};

  int g = 2;
  for (int i = 95; i >= 0; i--) {
    if (i <= 63 && i >= 32) g = 1;
    if (i <= 31 && i >= 0) g = 0;
    if (num_div_to_ten[i] == 1) my_num.bits[g] |= 1;
    if (i != 64 && i != 32 && i != 0) my_num.bits[g] = my_num.bits[g] << 1;
  }

  return my_num;
}

int beginning(int arr[]) {
  int index = 0;
  for (int i = 95; i >= 0; i--) {
    if (arr[i] == 1) {
      index = i;
      break;
    }
  }
  return index;
}

int first_5_bits(int arr[], int index) {
  int num = 0;
  for (int i = index; i > index - 5; i--) {
    if (arr[i]) num |= 1;
    if (i != index - 4) num = num << 1;
  }
  return num;
}

int finding_the_first_divisible(int first_num, int *index) {
  int divisible = 0;
  if (((first_num >> 1) & 0b1111) >= 10) {
    divisible = ((first_num >> 1) & 0b1111);  // делимое = первые 4 бита
    *index -= 4;
  } else {
    divisible = first_num & 0b11111;  // делимое = 5 битов
    *index -= 5;
  }
  return divisible;
}
