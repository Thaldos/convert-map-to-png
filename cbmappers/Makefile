CPP=g++
LDFLAGS=-lpng -lz
CFLAGS=-Wall

C3_OBJECTS=pkwareinputstream.o pngimage.o caesar3colours.o c3file.o
PHARAOH_OBJECTS=pkwareinputstream.o pngimage.o pharaohcolours.o pharaohfile.o
ZEUS_OBJECTS=pkwareinputstream.o pngimage.o zeuscolours.o zeusfile.o

all: c3 pharaoh zeus

c3: $(C3_OBJECTS)
	$(CPP) $(LDFLAGS) $(C3_OBJECTS) -o c3mapper

pharaoh: $(PHARAOH_OBJECTS)
	$(CPP) $(LDFLAGS) $(PHARAOH_OBJECTS) -o pharaohmapper

zeus: $(ZEUS_OBJECTS)
	$(CPP) $(LDFLAGS) $(ZEUS_OBJECTS) -o zeusmapper

pkwareinputstream.o: pkwareinputstream.h pkwareinputstream.cpp
	$(CPP) $(CFLAGS) -c pkwareinputstream.cpp

pngimage.o: pngimage.h pngimage.cpp
	$(CPP) $(CFLAGS) -c pngimage.cpp

# C3 stuff
caesar3colours.o: caesar3colours.h caesar3colours.cpp
	$(CPP) $(CFLAGS) -c caesar3colours.cpp

c3file.o: c3file.h c3file.cpp caesar3colours.h pkwareinputstream.h pngimage.h grid.h
	$(CPP) $(CFLAGS) -c c3file.cpp

# Pharaoh stuff
pharaohcolours.o: pharaohcolours.h pharaohcolours.cpp
	$(CPP) $(CFLAGS) -c pharaohcolours.cpp

pharaohfile.o: pharaohfile.h pharaohfile.cpp pharaohcolours.h pkwareinputstream.h pngimage.h grid.h
	$(CPP) $(CFLAGS) -c pharaohfile.cpp

# Zeus stuff
zeuscolours.o: zeuscolours.h zeuscolours.cpp
	$(CPP) $(CFLAGS) -c zeuscolours.cpp

zeusfile.o: zeusfile.h zeusfile.cpp zeuscolours.h pkwareinputstream.h pngimage.h grid.h
	$(CPP) $(CFLAGS) -c zeusfile.cpp

clean:
	rm -f *.o
	rm -f c3mapper pharaohmapper zeusmapper
