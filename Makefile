CXX = clang++
CXXFLAGS = -std=c++17 -Iinclude -I$(shell brew --prefix raylib)/include
LIBS = -L$(shell brew --prefix raylib)/lib -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:.cpp=.o)

all: TerraForge_Pro

TerraForge_Pro: $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o TerraForge_Pro $(LIBS)

clean:
	rm -f TerraForge_Pro $(OBJS)
