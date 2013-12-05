CC := gcc
#SOURCES := main.cpp parser.cpp bitstream.cpp params.cpp slice.cpp sliceheader.cpp prediction.cpp \
#	macroblock.cpp macroblocktype.cpp codedblock.cpp residuals.cpp scanorder.cpp
SOURCES := $(wildcard *.cpp)
INCLUDES := $(wildcard *.h)
OBJS := $(patsubst %.cpp,%.o,$(SOURCES))
#CFLAGS := -Wall -O3 -msse -ffast-math
CPPFLAGS := -I. -g 
LDFLAGS = 
INCLUDES = 
LIBS = 

all: parser

clean:
	rm -rf *.o
	rm -rf parser

parser: $(OBJS)
	$(LINK.cpp) $(LIBS) -o $@ $^ 

%.o: %.cpp %.h
	$(COMPILE.cpp) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<

.PHONY:	all parser
