CC      = gcc
LD      = ld
CFLAGS  = -ggdb -MD -Wall -Werror -fno-strict-aliasing -I./include -O2

FLEX_SOURCE   = src/lexical.l
FLEX_C_FILE   = lex.yy.c
BISON_SOURCE  = src/syntax.y
BISON_C_FILE  = syntax.tab.c
BISON_HEADER  = syntax.tab.h
SRC_DIR       = src
TARGET_DIR    = bin
OBJ_DIR       = $(TARGET_DIR)/objs
TARGET        = $(TARGET_DIR)/main
CFILES        = $(shell find $(SRC_DIR) -name "*.c")
OBJS          = $(CFILES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
FLEX_OBJ      = $(OBJ_DIR)/lex.yy.o
BISON_OBJ     = $(OBJ_DIR)/syntax.tab.o

$(TARGET): $(BISON_OBJ) $(FLEX_OBJ) $(OBJS) 
	$(CC) $(OBJS) $(BISON_OBJ) $(FLEX_OBJ) $(CFLAGS) -lfl -ly -o $(TARGET)

$(OBJS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p `dirname $@`
	$(CC) $(CFLAGS) -c $< -o $@

$(FLEX_C_FILE): $(FLEX_SOURCE) $(BISON_HEADER)
	flex $(FLEX_SOURCE)

$(FLEX_OBJ): $(FLEX_C_FILE)
	mkdir -p `dirname $(FLEX_OBJ)`
	$(CC) $(FLEX_C_FILE) -c -ggdb -O2 -I./include -o $(FLEX_OBJ)

$(BISON_C_FILE) $(BISON_HEADER): $(BISON_SOURCE) 
	bison -d $(BISON_SOURCE)

$(BISON_OBJ): $(BISON_C_FILE)
	mkdir -p `dirname $(BISON_OBJ)`
	$(CC) $(BISON_C_FILE) -c -ggdb -O2 -I./include -o $(BISON_OBJ)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf parse $(TARGET_DIR) $(FLEX_C_FILE) $(BISON_C_FILE) $(BISON_HEADER)
