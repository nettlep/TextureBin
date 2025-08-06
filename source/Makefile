#
# Generic (very simple) makefile
#

#
# What to build
#

PROG = texturebin
OBJS = 3ds.o clip.o jpeg.o render.o texturebin.o tmap.o
INCS = 3ds.h clip.h jpeg.h render.h texturebin.h tmap.h primitive.h rayplaneline.h vertext.h vmath

#
# Make stuff happen
#

%.o : %.cpp
	g++ -c -O3 -fomit-frame-pointer -fstrength-reduce -ffast-math -Wall $<

$(PROG) : $(OBJS)
	g++ -ljpeg -o $@ $^

#
# Clean things up...
#

clean:
	@rm -f $(OBJS) $(PROG)
