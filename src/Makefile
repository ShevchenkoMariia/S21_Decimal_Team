CC = gcc
FLAGS = -Wall -Werror -Wextra -std=c11
OS = $(shell uname)
ifeq ($(OS), Linux)
	CHECK_FLAGS = -lcheck -pthread -lrt -lm -lsubunit
	OPEN_CMD = xdg-open
else
	CHECK_FLAGS = -lcheck
	OPEN_CMD = open
endif

SRC = $(wildcard s21_*.c)

all: s21_decimal.a test clean 

clean:
	@rm -rf *.o *.g* *.info *.out report *.a *.dSYM debug

rebuild: clean all

objects:
	@$(CC) $(FLAGS) $(SRC) -c $<

s21_decimal.a: objects
	@ar rcs s21_decimal.a *.o
	@ranlib s21_decimal.a

test: s21_decimal.a
	@$(CC) $(FLAGS) *.c s21_decimal.a $(CHECK_FLAGS) -L.
	@./a.out

gcov_report: s21_decimal.a
	@rm -f *.g*
	@$(CC) $(FLAGS) *.c $(CHECK_FLAGS) -L. --coverage
	@./a.out
	@lcov -t "test" -o rep.info -c -d .
	@genhtml -o report rep.info
	@$(OPEN_CMD) ./report/index.html
	@rm -rf *.gcda *.gcno s21_tests s21_tests.info *.o

check:
	@clang-format -style=google -n *.c *.h
	@make test
ifeq ($(OS), Darwin)
	CK_FORK=no leaks --atExit -- ./a.out
else
	valgrind -q --tool=memcheck --leak-check=summary ./a.out
endif
	make clean

check_style :
	cp ../materials/linters/.clang-format .
	clang-format -n *.c *.h
	rm -rf .clang-format


