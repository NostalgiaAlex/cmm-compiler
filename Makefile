CC       = gcc
LD       = ld
CFLAGS   = -ggdb -Wall -Werror -fno-strict-aliasing -I./include -O2
DEPFLAGS = -MM

FLEX_SOURCE   = src/lexical.l
BISON_SOURCE  = src/syntax.y
SRC_DIR       = src
TARGET_DIR    = bin
GEN_DIR       = generate
BISON_C_FILE  = $(GEN_DIR)/syntax.tab.c
BISON_HEADER  = $(GEN_DIR)/syntax.tab.h
BISON_OBJ     = $(GEN_DIR)/syntax.tab.o
BISON_DEP     = $(GEN_DIR)/syntax.tab.d
FLEX_C_FILE   = $(GEN_DIR)/lex.yy.c
FLEX_OBJ      = $(GEN_DIR)/lex.yy.o
FLEX_DEP      = $(GEN_DIR)/lex.yy.d
OBJ_DIR       = $(TARGET_DIR)/objs
DEP_DIR       = $(TARGET_DIR)/dep
TARGET        = $(TARGET_DIR)/parse
CFILES        = $(shell find $(SRC_DIR) -name "*.c")
DFILES        = $(shell find . -name "*.d")
TESTFILES     = $(shell find testcase -name "*.c")
OUTPUTFILES   = $(shell find testcase -name "*.output")
OBJS          = $(CFILES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

$(TARGET): $(OBJS) $(FLEX_OBJ) $(BISON_OBJ)
	$(CC) $(OBJS) $(FLEX_OBJ) $(BISON_OBJ) $(CFLAGS) -lfl -ly -o $(TARGET)

-include $(DFILES)

$(OBJS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEP_DIR)/%.d
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -c -o $@

$(DEP_DIR)/%.d: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(DEPFLAGS) -I./include $< > $(DEP_DIR)/$*.d.tmp
	@sed -e 's|.*:|$(OBJ_DIR)/$*.o:|' < $(DEP_DIR)/$*.d.tmp > $(DEP_DIR)/$*.d
	@rm -f $(DEP_DIR)/$*.d.tmp

$(FLEX_C_FILE): $(FLEX_SOURCE) $(BISON_HEADER)
	flex $(FLEX_SOURCE)
	@mkdir -p $(GEN_DIR)
	@mv lex.yy.c $(FLEX_C_FILE)
$(FLEX_OBJ): $(FLEX_C_FILE)
	@mkdir -p $(@D)
	$(CC) -I./include $< -c -o $@
	$(CC) $(DEPFLAGS) -I./include $< > $(FLEX_DEP).tmp
	@sed -e 's|lex.yy.o|$(FLEX_OBJ)|' < $(FLEX_DEP).tmp > $(FLEX_DEP)
	@rm -f $(FLEX_DEP).tmp

$(BISON_C_FILE) $(BISON_HEADER): $(BISON_SOURCE) 
	bison -d $(BISON_SOURCE)
	@mkdir -p $(GEN_DIR)
	@mv syntax.tab.c $(BISON_C_FILE)
	@mv syntax.tab.h $(BISON_HEADER)
$(BISON_OBJ): $(BISON_C_FILE)
	@mkdir -p $(@D)
	$(CC) -I./include $< -c -o $@
	$(CC) $(DEPFLAGS) -I./include $< > $(BISON_DEP).tmp
	@sed -e 's|syntax.tab.o|$(BISON_OBJ)|' < $(BISON_DEP).tmp > $(BISON_DEP)
	@rm -f $(BISON_DEP).tmp

test: $(TARGET) $(TESTFILES)
	./test.sh $(TESTFILES)

clean:
	rm -rf parse $(TARGET_DIR) $(GEN_DIR) $(OUTPUTFILES)
