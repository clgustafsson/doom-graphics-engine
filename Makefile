CC = gcc
TARGET = target/main
SRC = main.c

MAC_LIBS = -framework OpenGL -framework GLUT
LINUX_LIBS = -lGL -lglut

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Darwin)
    LIBS = $(MAC_LIBS)
else
    LIBS = $(LINUX_LIBS)
endif

all: $(TARGET) run

$(TARGET): $(SRC)
	@mkdir -p target
	$(CC) -o $(TARGET) $(SRC) $(LIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf target
