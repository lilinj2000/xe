%:%.cpp
	g++ -Wall -O3 -g $< -lpthread -lrt -L. -l:libXeleTdAPI64.so -Iinclude -o $@
all:userdemo

.PHONY:clean
clean:
	rm -f userdemo
