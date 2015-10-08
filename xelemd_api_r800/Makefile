#
#                          Copyright (c) 2010 - 2012
#                                  AcceleCom
#
#                             All Rights Reserved
#
# File: Makefile
# $Id $
#
CC_RELEASE= -O3

CC_FLAGS= $(CC_RELEASE)

CC=g++ 
INCL=-I.
LDFLAGS:= -pthread -lrt

TARGET:=userdemo

$(TARGET): userdemo.cpp
	$(CC) userdemo.cpp $(INCL) ./libXeleMdAPI64.a  $(LDFLAGS) -o $(TARGET)

clean:
	@rm -f $(TARGET)



.PHONY:all clean $(TARGET)

