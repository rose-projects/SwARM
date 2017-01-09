CXXFLAGS += -std=c++14 -Wall -Wextra -pedantic `pkg-config --cflags opencv`
LDLIBS += `pkg-config --libs opencv`

CPPSRC = cam.cpp

EXE = cam

$(EXE) : $(CPPSRC)
