clean:
	rm -f a.out

build: clean
	g++ -std=c++11 src/*.cpp test/main.cpp
