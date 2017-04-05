CC=gcc

# Additional settings
include $(PWD)/../Settings.mak

DEFINES=-DMSGDMA_DEBUG=$(MSGDMA_DEBUG)
LIBRARY=libmsgdma.a
CFLAGS=
ARFLAGS=rcs
INCLUDES=-I../driver -I../include
OBJ=obj/msgdma_api.o

all: $(OBJ)
	$(CROSS_COMPILE)$(AR) $(ARFLAGS) -o $(LIBRARY) $(OBJ)

obj/%.o:src/%.c
	$(CROSS_COMPILE)$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(LIBRARY)
	$(RM) obj/*


