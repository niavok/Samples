DLL_NAME=Arsdk3Plugin.dll #output filename

SDK_DIR=../../../ARSDKBuildUtils
IDIR=./
#CC=x86_64-w64-mingw32-gcc
CC=/usr/bin/x86_64-w64-mingw32-gcc
CFLAGS= -g -I$(IDIR) -I $(SDK_DIR)/Targets/Windows/Install/include -I ffmpeg-2.7.2 -Dlocaltime_r=localtime

OBJDIR=obj
LDIR = $(SDK_DIR)/Targets/Windows/Install/lib

#EXTERNAL_LIB=-lncurses -ljson
EXTERNAL_LIB= -ljson -lwsock32 -lws2_32 -Lffmpeg-2.7.2/libavcodec -lavcodec -Lffmpeg-2.7.2/libavutil -lavutil -Lffmpeg-2.7.2/libswscale -lswscale

#LIBS= -g -L$(SDK_DIR)/Targets/Windows/Install/lib  -larcontroller -larcommands -lardiscovery -larstream  -larnetworkal -larnetwork  $(EXTERNAL_LIB)
LIBS=-L$(SDK_DIR)/Targets/Windows/Install/lib -larcontroller -larcommands -lardiscovery -larstream -larnetwork -larnetworkal -larsal $(EXTERNAL_LIB)
LIBS_DBG=-L$(SDK_DIR)/Targets/Windows/Install/lib -larsal_dbg -larcontroller_dbg -lardiscovery_dbg $(EXTERNAL_LIB)

_DEPS = 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = Arsdk3WindowsPluginC.o unityWindowsSendMessage.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

all: $(DLL_NAME)
	@[ -p $(FIFO) ] || mkfifo $(FIFO)

$(OBJDIR)/%.o: %.c $(DEPS)
	@ [ -d $(OBJDIR) ] || mkdir $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(DLL_NAME): $(OBJ)
	$(CC) -shared -o $@ $^ $(CFLAGS) $(LIBS)

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
