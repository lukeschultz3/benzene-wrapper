main:
	make clean
	make wrapper
	./wrapper

wrapper:
	g++ wrapper.cpp -o wrapper

clean:
	rm -f wrapper
