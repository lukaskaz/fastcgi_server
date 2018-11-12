#Output files
APP_NAME=fcgi

#======================================================================#
#Directories
SRC_DIR=src/
INC_DIR=inc/
OBJ_DIR=obj/
MAP_DIR=out/
EXEC_DIR=out/

#======================================================================#
#Cross Compiler
CC=g++
READELF=readelf

#======================================================================#
#Flags

CFLAGS=-I $(INC_DIR) -Wall -std=c++14

LFLAGS=

#ELF interpreter options
RELFFLAGS=-aW
#======================================================================#
#Libraries

LIBS+=-lpthread
LIBS+=
#======================================================================#
#add sources
SRC+=main.cpp
SRC+=server.cpp
SRC+=connection.cpp
SRC+=responder.cpp
SRC+=request.cpp
SRC+=support.cpp
SRC+=logging.cpp

#prepare objects
OBJ=$(SRC:.cpp=.o)

#prepare groups
SRC_GROUP=$(addprefix $(SRC_DIR), $(SRC))
OBJ_GROUP=$(addprefix $(OBJ_DIR), $(OBJ))

#prepare outputs
EXEC_FILE=$(EXEC_DIR)$(APP_NAME)
MAP_FILE=$(MAP_DIR)$(APP_NAME).map
#======================================================================#
#Prepare rules
release: CFLAGS+=-O3
build: CFLAGS+=-O2
debug: CFLAGS+=-O0 -g3

#Makefile rules
build: $(MAP_FILE)
rebuild:
	$(MAKE) clean
	$(MAKE) build

release: clean $(MAP_FILE)
debug: clean $(MAP_FILE)


$(EXEC_FILE):$(OBJ_GROUP)
	@echo "Linking project..."
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)

#$(OBJ_GROUP):$(SRC_GROUP)
#	@echo "Building project..."
#	$(CC) -c $^ -o $@ $(CFLAGS) $(LIBS)

$(OBJ_DIR)%.o:$(SRC_DIR)%.cpp
	@echo "Creating object" $@
	$(CC) -c $^ -o $@ $(CFLAGS) $(LIBS)

$(MAP_FILE): $(EXEC_FILE)
	@echo "Creating map file..."
	$(READELF) $(RELFFLAGS) $^ > $@


#Make clean
clean:
	@echo "Cleaning the project..."
	rm -rf $(EXEC_FILE) $(MAP_FILE) $(OBJ_DIR)*

#======================================================================
.PHONY: build rebuild release debug clean

