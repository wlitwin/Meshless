CXXFLAGS=-Wall -Wextra -DGL_GLEXT_PROTOTYPES -std=c++03 -O2 -flto -pedantic -L/usr/lib/
LFLAGS=-lGL -lglfw -Ldlib -lGLEW 
CXX=g++

OBJ_DIR=obj

OBJ_FILES=$(shell find src/ -name '*.cpp' | sed -e 's/^\(.*\).cpp$$/obj\/\1.o/g')

#==============================================================================
# RULES
#==============================================================================

$(OBJ_DIR)/%.o : %.cpp
	@echo Compiling $^
	$(shell mkdir -p $(dir $@))
	@$(CXX) $(CXXFLAGS) -c $^ -o $@

build: $(OBJ_FILES)
	@$(CXX) $(OBJ_FILES) $(CXXFLAGS) $(LFLAGS) -o meshless
	@echo Finished

slowmo: CXXFLAGS += -DSLOW_MO
slowmo: build

run: build
	./meshless

clean:
	/bin/rm -f meshless
	/bin/rm -rf `find $(OBJ_DIR)/ -name '*.o'`
