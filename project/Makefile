PROJECT = project
SUBMIT_FILES = *.cpp *.h Makefile
OS = $(shell uname)
CC = gcc
CPP = g++
MOC = $(QTDIR)/bin/moc
INCFLAGS = -I $(QTDIR)/include
LDFLAGS =
LIBS = -L $(QTDIR)/lib -lqt-mt -lGL -lavformat
CFLAGS = -g -Wall -c $(INCFLAGS)
CPP_OBJS = Window.o VideoWidget.o HandWidget.o Hand.o Cylinder.o WRLModel.o ImageRoutines.o
MOC_OBJS = mWindow.o mVideoWidget.o mHandWidget.o
OBJS = $(CPP_OBJS) $(MOC_OBJS)

all: $(PROJECT)

clean:
	rm -f $(PROJECT) *.o

$(PROJECT): $(OBJS)
	$(CPP) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

hsvextract: hsvextract.o
	$(CPP) $(LDFLAGS) -o $@ hsvextract.o $(LIBS)

yiqextract: yiqextract.o
	$(CPP) $(LDFLAGS) -o $@ yiqextract.o $(LIBS)

blockextract: blockextract.o ImageRoutines.o
	$(CPP) $(LDFLAGS) -o $@ blockextract.o ImageRoutines.o $(LIBS)

imagesubtract: imagesubtract.o ImageRoutines.o
	$(CPP) $(LDFLAGS) -o $@ imagesubtract.o ImageRoutines.o $(LIBS)

submit:
	tar -zcvf $(PROJECT).tgz $(SUBMIT_FILES)

dep:
	@ echo "Generating dependencies"
	@ $(CPP) $(INCFLAGS) -MM  $(CPP_OBJS:.o=.cpp) > .depends
	@ echo "Finished generating dependencies"

.depends:
	@ touch .depends
	@ $(MAKE) dep

.SUFFIXES : .o .cpp .h

.cpp.o:
	$(CPP) $(CFLAGS) $<

m%.cpp: %.h
	$(MOC) $< -o $@

include .depends
