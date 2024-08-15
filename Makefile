# Directory where the .cpp files are to be found
CPP_DIR = src

# Directory where .o files should be put
OBJ_DIR = obj

# libraries to include (use -l<libraryname>) #-lnlopt -lm
LIBS = -pthread

# name of the executable file to generate
EXECUTABLE = a.out

# directories to search when resolving #includes
INC_DIRS = include

# Optimisation flag  # OPTFLAG = -fexternal-templates
OPTFLAG = -O3

# Language standard to use
STD = c++20

# compiler executable
COMPILER = /usr/bin/g++

# all the cpp files recursively below CPP_DIR
CPP_FILES = $(wildcard $(CPP_DIR)/*.cpp) $(wildcard $(CPP_DIR)/**/*.cpp)

# all object files to generate
OBJ_FILES =	$(patsubst $(CPP_DIR)/%, $(OBJ_DIR)/%, $(CPP_FILES:.cpp=.o))

# rules for compilation
all: $(OBJ_FILES)
	$(COMPILER) $(OBJ_FILES)  $(LIBS) -o $(EXECUTABLE)

clean:
	find $(OBJ_DIR) -type f -name '*.o' -print -delete

check:
	$(info CPP_FILES = $(CPP_FILES))
	$(info OBJ_FILES = $(OBJ_FILES))
	$(info compile command = $(COMPILER) $(OPTFLAG) -std=$(STD) -I$(INC_DIRS) -c -o $(OBJ_DIR)/file.o $(SRC_DIR)/file.cpp)
	$(info link command    = $(COMPILER) $(OBJ_FILES) $(LIBS) -o $(EXECUTABLE))

# rule to compile .cpp files to .o files
$(OBJ_DIR)/%.o: $(CPP_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(COMPILER) $(OPTFLAG) -std=$(STD) -I$(INC_DIRS) -c -o $@ $<
