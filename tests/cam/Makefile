export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/

CXXFLAGS += -std=c++11 -Wall -Wextra -pedantic `pkg-config --cflags opencv`
LDLIBS += `pkg-config --libs opencv`

CPPSRC = cam.cpp

EXE = cam

$(EXE) : $(CPPSRC)
