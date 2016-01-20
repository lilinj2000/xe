%:%.cpp
	g++ -g $< -lpthread -lrt -L. -l:libXeleTdAPI64.so -Iinclude -o $@
all:userdemo

.PHONY:clean
clean:
	rm -f userdemo
