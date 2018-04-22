CC = clang++
CFLAGS = -std=c++14 -O3 -Wall -Wextra
LDFLAGS = -std=c++14 -O3 -Wall -Wextra -lGLEW -lGLU -lGL -lglfw3 -lX11 -lXrandr -lXxf86vm -lXinerama -lXcursor -pthread -ldl
NAME = OpenglTest
BIN_FILE_PATH = ./bin/
CPP = main.cpp lodepng.cpp

OBJ = $(CPP:%.cpp=%.o)
OBJ_DEST = $(CPP:%.cpp=$(BIN_FILE_PATH)%.o)

all: $(OBJ)
	$(CC) -o $(BIN_FILE_PATH)$(NAME) $(OBJ_DEST) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $(BIN_FILE_PATH)$@

test: all
	$(BIN_FILE_PATH)$(NAME)

clean:
	rm $(OBJ_DEST) $(BIN_FILE_PATH)$(NAME)
