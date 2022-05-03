all: visplay

%: %.cc
	g++ -std=c++20 -Wall -Wextra -o $@ $< $(shell pkg-config --cflags --libs sfml-all)
