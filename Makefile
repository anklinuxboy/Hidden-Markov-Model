CPP=g++
CFLAGS=-Wall -pedantic -std=c++11 -g
OBJS=HiddenMarkovModel.o Utils.o

all: recognize statepath optimize

recognize: $(OBJS) recognize.cpp
	$(CPP) $(CFLAGS) -o $@ $^

statepath: $(OBJS) statepath.cpp
	$(CPP) $(CFLAGS) -o $@ $^

optimize: $(OBJS) optimize.cpp
	$(CPP) $(CFLAGS) -o $@ $^

%.o: %.cpp
	$(CPP) $(CFLAGS) -c $<

clean:
	rm -f *.o recognize statepath optimize
