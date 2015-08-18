DLL_NAME=Arsdk3Plugin.dll #output filename

SDK_DIR=../../../ARSDKBuildUtils
IDIR=./
#CC=x86_64-w64-mingw32-gcc
CC=gcc
CFLAGS= -g -I$(IDIR) -I $(SDK_DIR)/Targets/Windows/Install/include

OBJDIR=obj
LDIR = $(SDK_DIR)/Targets/Windows/Install/lib

EXTERNAL_LIB=-lncurses -ljson

#LIBS= -g -L$(SDK_DIR)/Targets/Windows/Install/lib  -larcontroller -larcommands -lardiscovery -larstream  -larnetworkal -larnetwork -larsal  $(EXTERNAL_LIB)
LIBS=
LIBS_DBG=-L$(SDK_DIR)/Targets/Windows/Install/lib -larsal_dbg -larcontroller_dbg -lardiscovery_dbg $(EXTERNAL_LIB)

_DEPS = 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = Arsdk3WindowsPluginC.o unityWindowsSendMessage.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

all: $(DLL_NAME)
	@[ -p $(FIFO) ] || mkfifo $(FIFO)

$(OBJDIR)/%.o: %.c $(DEPS)
	@ [ -d $(OBJDIR) ] || mkdir $(OBJDIR)
	@ $(CC) -c -o $@ $< $(CFLAGS)

$(DLL_NAME): $(OBJ)
	gcc -shared -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	@ rm -f $(OBJDIR)/*.o *~ core $(INCDIR)/*~
	@ rm -rf $(OBJDIR)
	@ rm -r $(DLL_NAME)
	@ rm -f $(FIFO)

run:
	LD_LIBRARY_PATH=$(LDIR) ./$(DLL_NAME)

valgrind:
	LD_LIBRARY_PATH=$(LDIR) valgrind ./$(DLL_NAME)
