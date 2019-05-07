SRC = vk_db_count_intersection_test.cpp
LIB = catch.hpp
EXE = ./out/vk_db_count_intersection_test

all: $(EXE)
CFLAGS = -std=c++14 -Wall -Wextra -Wshadow -O3
$(EXE) :: $(SRC) $(LIB)
	mkdir -p out
	g++ $(CFLAGS) $< -o $@


clean:
	rm -rf ./out

.PHONY: all clean
