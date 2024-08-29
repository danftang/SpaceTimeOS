# Directory where the .cpp files are to be found
CPP_DIR = src

# Directory where .o files should be put
OBJ_DIR = obj
RELEASE_DIR = release
DEBUG_DIR = debug

# libraries to include (use -l<libraryname>) #-lnlopt -lm
LIBS = -pthread

# name of the executable file to generate
EXECUTABLE = a.out

# directories to search when resolving #includes
INC_DIRS = include

# Optimisation flag: use -ggdb for debugging and -O3 for release
# OPTFLAG = -fexternal-templates
RESEASE_FLAGS = -O3
DEBUG_FLAGS = -ggdb

# Language standard to use
STD = c++20

# compiler executable
COMPILER = /usr/bin/g++

# all the cpp files recursively below CPP_DIR
CPP_FILES = $(wildcard $(CPP_DIR)/*.cpp) $(wildcard $(CPP_DIR)/**/*.cpp)

# all object files to generate
RELEASE_OBJ_FILES =	$(patsubst $(CPP_DIR)/%, $(RELEASE_DIR)/%, $(CPP_FILES:.cpp=.o))
DEBUG_OBJ_FILES =	$(patsubst $(CPP_DIR)/%, $(DEBUG_DIR)/%, $(CPP_FILES:.cpp=.o))

# rules for compilation
all: release

release: $(RELEASE_OBJ_FILES)
	$(COMPILER) $(RELEASE_OBJ_FILES)  $(LIBS) -o $(RELEASE_DIR)/$(EXECUTABLE)

debug: ${DEBUG_OBJ_FILES}
	$(COMPILER) $(DEBUG_OBJ_FILES)  $(LIBS) -o $(DEBUG_DIR)/$(EXECUTABLE)

clean:
	find $(RELEASE_DIR) -type f -name '*.o' -print -delete
	find $(DEBUG_DIR) -type f -name '*.o' -print -delete

check:
	$(info CPP_FILES = $(CPP_FILES))
	$(info OBJ_FILES = $(OBJ_FILES))
	$(info compile command = $(COMPILER) $(OPTFLAG) -std=$(STD) -I$(INC_DIRS) -c -o $(OBJ_DIR)/file.o $(SRC_DIR)/file.cpp)
	$(info link command    = $(COMPILER) $(OBJ_FILES) $(LIBS) -o $(EXECUTABLE))

# rule to compile .cpp files to .o files
$(RELEASE_DIR)/%.o: $(CPP_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(COMPILER) $(RELEASE_FLAGS) -std=$(STD) -I$(INC_DIRS) -c -o $@ $<

$(DEBUG_DIR)/%.o: $(CPP_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(COMPILER) $(DEBUG_FLAGS) -std=$(STD) -I$(INC_DIRS) -c -o $@ $<
