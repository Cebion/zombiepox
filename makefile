###########################################
#
# makefile for LD48 2004 project
# theme: infection
# game: zombiepox
#
###########################################
#
###########################################


# compiler
CC = g++

# compiler flags
CFLAGS = \
	-O2\
	-fomit-frame-pointer\
	-ffast-math\
	-funroll-loops\
	-W\
	-Wall\
	-pedantic

# linker flags
LFLAGS = \
        `allegro-config --libs`\
	-s


# o-files
OBJ_DIR = ./obj

ALL_OBJ =       main.o \
	        CItem.o \
	        CTile.o \
	        CMap.o \
		CActor.o \
		hisc.o

# merge o-files and prepend path
OBJS = $(addprefix $(OBJ_DIR)/, $(ALL_OBJ))

# compile cpp to o
$(OBJ_DIR)/%.o: ./src/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@


# output filename
OUTPUT = zombiepox

# output location
OUTPUT_DIR = .

# main rule
$(OUTPUT): $(OBJS)
	$(CC) $(OBJS) -o $(OUTPUT_DIR)/$@ $(LFLAGS)


# resource file
RES_DIR = res
RES_FILE = $(RES_DIR)/ld48.rc

# resource rule
res.o : $(RES_FILE)
	windres $(RES_FILE) -o $@

# UPX rule
upx : $(OUTPUT)
	upx $(OUTPUT)



