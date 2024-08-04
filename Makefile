# Directory where the .cpp files are
CPP_DIR = ./src

#libraries to include (use -l<libraryname>)
LIBS = # -lnlopt -lm

# name of the executable file to generate
EXECUTABLE = a.out

# directories to search when resolving #includes
INCLUDE_DIRS = ./include

# Optimisation flag 
# OPTFLAG = -fexternal-templates
OPTFLAG = -O3

# Language standard to use
STD = c++20

# compiler to use
COMPILER = g++

# all the cpp files recursively below CPP_DIR
CPP_FILES = $(wildcard $(CPP_DIR)/*.cpp) $(wildcard $(CPP_DIR)/**/*.cpp)

# all object files to generate
OBJ_FILES = $(CPP_FILES:.cpp=.o)

all: $(OBJ_FILES)
	$(COMPILER) $(OBJ_FILES) $(LIBS) -o $(EXECUTABLE)

clean:
	find $(CPP_DIR) -type f -name '*.o' -print -delete

check:
	$(info CPP_FILES = $(CPP_FILES))
	$(info OBJ_FILES = $(OBJ_FILES))
	$(info compile command = $(COMPILER) $(OPTFLAG) -std=$(STD) -I$(INCLUDE_DIRS) -c -o file.o file.cpp)
	$(info link command    = $(COMPILER) $(OBJ_FILES) $(LIBS) -o $(EXECUTABLE))

# rule to compile .cpp files to .o files
%.o: %.cpp
	$(COMPILER) $(OPTFLAG) -std=$(STD) -I$(INCLUDE_DIRS) -c -o $@ $<
