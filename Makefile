CC = gcc
AR = ar rc
RANLIB = ranlib
RM_F = rm -rf
CWARNS = -Wall -Wextra
CFLAGS = $(CWARNS)
TRASH = *.o *.a

MPARSER_T = libmparser.a
MPARSER_O = mparser.o
TEST_T = test
TEST_O = test.o

.PHONY: clean

debug: $(MPARSER_T)
release: $(MPARSER_T)

$(MPARSER_T): $(MPARSER_O)
	$(AR) $@ $(MPARSER_O)
	$(RANLIB) $@

$(TEST_T): $(TEST_O) $(MPARSER_T)
	$(CC) -o $@ $(TEST_O) $(MPARSER_T)

clean:
	$(RM_F) $(TRASH) $(MPARSER_T) $(TEST_T)

