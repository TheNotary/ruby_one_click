EXENAME ?= ruby-oneclick-2.0.0
CXX = g++
MAINFILE = main.cpp
DEBUGFLAG = -DDEBUG_INFO

SYS := $(shell gcc -dumpmachine)
ifneq (,$(findstring mingw, $(SYS)))
 COMPILEFLAG = -static-libgcc -static-libstdc++ -mwindows $(MAINFILE) -static -lcurl -lz -lwldap32 -lws2_32 -o
 OUTPUTEXENAME = $(EXENAME).exe
else ifneq (, $(findstring cygwin, $(SYS)))
 COMPILEFLAG = -mwindows $(MAINFILE) -lcurl -o
 OUTPUTEXENAME = $(EXENAME).exe
else
 GETPACKAGE = ubuntupackage
 COMPILEFLAG = $(MAINFILE) -lcurl -o
 OUTPUTEXENAME = $(EXENAME)
endif

all: rsi

ubuntupackage: 
	if ! dpkg -l | grep libcurl4-gnutls-dev -c >>/dev/null; then sudo apt-get install libcurl4-gnutls-dev; fi
rsi: $(MAINFILE) $(GETPACKAGE)
	$(CXX) $(COMPILEFLAG) $(OUTPUTEXENAME)
debug: $(MAINFILE) $(GETPACKAGE)
	$(CXX) $(DEBUGFLAG) $(COMPILEFLAG) $(OUTPUTEXENAME)