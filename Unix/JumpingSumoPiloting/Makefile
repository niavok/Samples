EXEC_NAME=JumpingSumoPiloting #output filename

SDK_DIR=../../../ARSDKBuildUtils
IDIR=./
CC=gcc
CFLAGS=-I$(IDIR) -I $(SDK_DIR)/Targets/Windows/Install/include

OBJDIR=obj
LDIR = $(SDK_DIR)/Targets/Windows/Install/lib

EXTERNAL_LIB=-lncurses

#LIBS=-L$(SDK_DIR)/Targets/Windows/Install/lib -larsal -larcommands -larnetwork -larnetworkal -lardiscovery $(EXTERNAL_LIB)
LIBS=-L$(SDK_DIR)/Targets/Windows/Install/lib  -larnetworkal -larnetwork -lardiscovery -larcommands  -larsal $(EXTERNAL_LIB)
#LIBS=-L$(SDK_DIR)/Targets/Windows/Install/lib -larsal -larcommands  $(EXTERNAL_LIB)

LIBS_DBG=-L$(SDK_DIR)/Targets/Windows/Install/lib -larsal_dbg -larcommands_dbg -larnetwork_dbg -larnetworkal_dbg -lardiscovery_dbg $(EXTERNAL_LIB)

_DEPS = JumpingSumoPiloting.h ihm.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = JumpingSumoPiloting.o ihm.o
#_OBJ = JumpingSumoPiloting.o

OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

$(OBJDIR)/%.o: %.c $(DEPS)
	@ [ -d $(OBJDIR) ] || mkdir $(OBJDIR)
	@ $(CC) -c -o $@ $< $(CFLAGS)

$(EXEC_NAME): $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	@ rm -f $(OBJDIR)/*.o *~ core $(INCDIR)/*~
	@ rm -rf $(OBJDIR)
	@ rm -r $(EXEC_NAME)

run:
	LD_LIBRARY_PATH=$(LDIR) ./$(EXEC_NAME)
