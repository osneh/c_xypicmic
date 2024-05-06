


CC = gcc
PROG = xypicmic
CFLAGS =  -W -Wall -Wextra -g
CPPFLAGS = -DLOG_ENABLE=4 -Iutil
LDFLAGS =


SRC = main.c       \
	  xypicmic.c   \
	  util/mlog.c  \
	  util/color.c \
	  util/getline.c
OBJ = $(SRC:.c=.o)
DEP = $(SRC:.c=.d)

.SUFFIXES:
.PHONY: clean

$(PROG): $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)
   
-include $(DEP)

%.o: %.c
	$(CC) -c  $< -o $@ $(CFLAGS) $(CPPFLAGS)
	@ $(CC) -MM  $< > $(@:.o=.d) $(CPPFLAGS)
	
clean:
	del $(OBJ)
	del $(DEP)
	del $(PROG).exe
