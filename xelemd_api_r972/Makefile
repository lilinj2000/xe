CC=g++
INCL=-I.
LDFLAGS:= -pthread -lrt

TARGET:=userdemo userdemo_shfe

all: $(TARGET)


userdemo: userdemo.cpp
	$(CC) userdemo.cpp -g -O3 $(INCL) ./libXeleMdAPI64.a  $(LDFLAGS) -o $@

userdemo_shfe: userdemo_shfe.cpp
	$(CC) userdemo_shfe.cpp -g -O3 $(INCL) ./libXeleMdAPI64.a  $(LDFLAGS) -o $@




clean:
	@rm -f $(TARGET)

.PHONY:all clean $(TARGET)
