CCFLAGS = -O2 -std=c++11 -Wall -Werror
CC = g++
AR = ar
ROOT_BIN = ./bin/
ROOT_SRC = ./src/
LIB_SRC = $(ROOT_SRC)lib/
LINUX_BIN = $(ROOT_BIN)linux/
WINDOWS_BIN = $(ROOT_BIN)windows/

all: lib a exe

a_64: o_64
	if not exist "$(LINUX_BIN)x64" mkdir "$(LINUX_BIN)x64"
	$(AR) rvs "$(LINUX_BIN)x64/GexLZSS.a" "$(ROOT_BIN)GexLZSS64.o"

a_32: o_32
	if not exist "$(LINUX_BIN)x86" mkdir "$(LINUX_BIN)x86"
	$(AR) rvs "$(LINUX_BIN)x86/GexLZSS.a" "$(ROOT_BIN)GexLZSS32.o"

lib_64: o_64
	if not exist "$(WINDOWS_BIN)x64" mkdir "$(WINDOWS_BIN)x64"
	$(AR) rcs "$(WINDOWS_BIN)x64/GexLZSS.lib" "$(ROOT_BIN)GexLZSS64.o"

lib_32: o_32
	if not exist "$(WINDOWS_BIN)x86" mkdir "$(WINDOWS_BIN)x86"
	$(AR) rcs "$(WINDOWS_BIN)x86/GexLZSS.lib" "$(ROOT_BIN)GexLZSS32.o"

a: a_64 a_32

lib: lib_64 lib_32

o_64:
	if not exist "./bin" mkdir "./bin"
	$(CC) $(CCFLAGS) -m64 -o "$(ROOT_BIN)GexLZSS64.o" -c "$(LIB_SRC)GexLZSS.cpp"

o_32:
	if not exist "./bin" mkdir "./bin"
	$(CC) $(CCFLAGS) -m32 -o "$(ROOT_BIN)GexLZSS32.o" -c "$(LIB_SRC)GexLZSS.cpp"

windows-x64: lib_64
windows-x86: lib_32

linux-x64: a_64
linux-x86: a_32

exe: lib_32
	g++ $(CCFLAGS) -m32 -o $(ROOT_BIN)GexLZSS.exe $(ROOT_SRC)GexLZSSProgram.cpp -L$(WINDOWS_BIN)x86 -lGexLZSS

test: lib_64
	g++ $(CCFLAGS) -m64 -o $(ROOT_BIN)test.exe $(ROOT_SRC)test.cpp -L$(WINDOWS_BIN)x64 -lGexLZSS

clean:
	rm -rf "./bin"