SRC = vk_db_count_intersection_test.cpp
EXE = vk_db_count_intersection_test

CFLAGS = -std=c++14 -Wall -Wextra -Wshadow -O3
$(EXE) :: $(SRC)
	g++ $(CFLAGS) $< -o $@

.PHONY: all clean
