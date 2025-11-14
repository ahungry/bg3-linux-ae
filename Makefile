all: build

build:
	g++ -shared -fPIC -o bg3_linux_ae.so bg3_linux_ae.cpp -ldl -std=c++11
