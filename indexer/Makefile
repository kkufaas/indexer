## Authors: 
## Steffen Viken Valvaag <steffenv@cs.uit.no>
## Morten Grønnesby <morten.gronnesby@uit.no>

# Common sources to build both binaries
# You can change these values to test with different implementations (for example chained hashmap vs open addressing) 
LIST_SRC=linkedlist.c
MAP_SRC=hashmap.c
SET_SRC=treeset.c

# TODO: Add your index implementation here
INDEX_SRC=index.c

# Directories
INCLUDE_DIR=include
SRC_DIR=src

# Binary targets
INDEXER=indexer
ASSERT_INDEX=assert_index

# Target source files
INDEXER_SRC=indexer.c common.c httpd.c $(LIST_SRC) $(MAP_SRC) $(SET_SRC) $(INDEX_SRC)
ASSERT_SRC=assert_index.c common.c $(LIST_SRC) $(MAP_SRC) $(SET_SRC) $(INDEX_SRC)

# Prefix the files with the src folder
INDEXER_SRC := $(patsubst %.c, $(SRC_DIR)/%.c, $(INDEXER_SRC))
ASSERT_SRC := $(patsubst %.c, $(SRC_DIR)/%.c, $(ASSERT_SRC))

# Find all header files
HEADERS = $(wildcard $(INCLUDE_DIR)/*.h)

# Override this to remove all printing
DEBUG_FLAG = -DDEBUG

# # Add -pg flag to CFLAGS for profiling
# CFLAGS = -Wall -g -I$(INCLUDE_DIR) -lm -lpthread -pg $(DEBUG_FLAG)

.PHONY=all

all: indexer assert_index

indexer: $(INDEXER_SRC) $(HEADERS) Makefile
	gcc -Wall -o $@ -D_GNU_SOURCE -D_REENTRANT $(INDEXER_SRC) -I$(INCLUDE_DIR) -g -lm -lpthread $(DEBUG_FLAG) -pg

assert_index: $(ASSERT_SRC) $(HEADERS) Makefile
	gcc -o $@ $(ASSERT_SRC) -I$(INCLUDE_DIR) -g -lm $(DEBUG_FLAG)

clean:
	rm -f *~ *.o *.exe *.out *.prof *.stackdump indexer assert_index
	rm -rf *.dSYM
