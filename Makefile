main:
	make clean
	make wrapper
	./wrapper

wrapper:
	g++ -std=c++11 wrapper.cpp main.cpp -o wrapper

clean:
	rm -f wrapper
