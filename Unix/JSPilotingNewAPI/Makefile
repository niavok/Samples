EXEC_NAME=JumpingSumoPiloting #output filename

SDK_DIR=../../../ARSDKBuildUtils
IDIR=./
CC=/usr/bin/x86_64-w64-mingw32-gcc -fstack-protector
CFLAGS=-I$(IDIR) -I $(SDK_DIR)/Targets/Windows/Install/include

OBJDIR=obj
LDIR = $(SDK_DIR)/Targets/Windows/Install/lib

EXTERNAL_LIB= -ljson

LIBS=-L$(SDK_DIR)/Targets/Windows/Install/lib  -larcontroller -larcommands -lardiscovery -larstream  -larnetworkal -larnetwork -larsal  $(EXTERNAL_LIB)
LIBS_DBG=-L$(SDK_DIR)/Targets/Windows/Install/lib -larsal_dbg -larcontroller_dbg -lardiscovery_dbg $(EXTERNAL_LIB)

_DEPS = JumpingSumoPiloting.h ihm.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = JumpingSumoPiloting.o ihm.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

FIFO = video_fifo.mjpg

all: $(EXEC_NAME)
	@[ -p $(FIFO) ] || mkfifo $(FIFO)

$(OBJDIR)/%.o: %.c $(DEPS)
	@ [ -d $(OBJDIR) ] || mkdir $(OBJDIR)
	@ $(CC) -c -o $@ $< $(CFLAGS)

$(EXEC_NAME): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	@ rm -f $(OBJDIR)/*.o *~ core $(INCDIR)/*~
	@ rm -rf $(OBJDIR)
	@ rm -r $(EXEC_NAME)
	@ rm -f $(FIFO)

run:
	LD_LIBRARY_PATH=$(LDIR) ./$(EXEC_NAME)

valgrind:
	LD_LIBRARY_PATH=$(LDIR) valgrind ./$(EXEC_NAME)
