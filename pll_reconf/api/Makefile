
include $(PWD)/../Settings.mak
CC=gcc
#CROSS_COMPILE defined in Settings.mak
DEFINES=-DDEBUG_PLL_CTL=$(DEBUG_PLL_CTL)

LIBRARY_NAME=libpll.a
INC=-I../driver -I../include
CFLAGS=-Wall -O3
ARFLAGS=rcs
OBJ=obj/pll_api.o



all: $(OBJ)
	$(CROSS_COMPILE)$(AR) $(ARFLAGS) $(LIBRARY_NAME) $(OBJ)

obj/%.o:src/%.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) $(DEFINES) $(INC) -c $< -o $@

clean:
	$(RM) $(LIBRARY_NAME)
	$(RM) obj/*

