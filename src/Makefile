# Marked below in the "files" section are the lines you need to
# change. Also, to make your code run as fast as possible, change the
# flags option from debugging to optimizing below and do a clean make.

####### FLAGS

CC      = g++
#LIBS = -lbugle -lglut -lGLEW
LIBS    = -lGLEW -lglut #-lGLU
BASE_DIR = .
INCLUDES = -I$(BASE_DIR)

# Optimizing flags, remove '#' to activate.
#CFLAGS	= -Wall -O6 -DNDEBUG $(INCLUDES)

# Profiling flags, remove '#' to activate.
#CFLAGS	= -Wall -pg -O6 -DNDEBUG $(INCLUDES)

# Debugging flags, insert '#' to deactivate.
CFLAGS	= -Wall -g -U_FORTIFY_SOURCE $(INCLUDES)

####### Files

# The name of the target file
TARGET  = radianceSplat

# All headers in the project. Add your new header files here.
# Note that template .cpp files that are #include'd somewhere should go here
HEADERS = SceneObject.h Helpers.h GLCommand.h Scene.h Light.h Matrix.h stopwatch.h

# All source files in the project. Add your new source files here.
SOURCES = radianceSplat.cpp SceneObject.cpp Helpers.cpp Scene.cpp Light.cpp stopwatch.cpp

# All object files in the project. Add .o versions of all your
# _untemplated_ .cpp files (probably no new files here for this project).
OBJECTS = radianceSplat.o SceneObject.o Helpers.o Scene.o Light.o stopwatch.o

###### 
# Uncomment this version to make test rather than word_count (and
# comment the corresponding lines above (TARGET and OBJECTS).

# The name of the target file
#TARGET  = test

# All object files in the project. Add .o versions of all your
# _untemplated_ .cpp files (probably no new files here for this project).
#OBJECTS = test.o WordCounter.o

####### Implicit rules
.SUFFIXES:

.SUFFIXES: .c .cc .cpp .o

.c.o:; $(CC) -c $(CFLAGS) $<
.cpp.o:; $(CC) -c $(CFLAGS) $<
.cc.o:; $(CC) -c $(CFLAGS) $<

####### Build rules
# No changes should be necessary in this section.

all: $(TARGET)

$(TARGET): $(OBJECTS) $(HEADERS)
	$(CC) $(CFLAGS) $(LIBS) -o $(TARGET) $(OBJECTS)
purip: $(OBJECTS)
	purify $(CC) -o purip $(OBJECTS) $(CFLAGS) $(LIBS)

$(OBJECTS): $(HEADERS)

show:
	@echo $(SOURCES) $(HEADERS)

clean:
	rm -f *.o *~ 

full_clean: clean
	rm -f $(TARGET)







#CC = g++
#OPT = -Wall -O2 -g 

#all: radianceSplat

#COMPILE = $(CC) $(OPT) -c

#OBJFILES := $(pathsubst %.cpp, %.o, $(wildcard *.cpp))

#radianceSplat: $(OBJFILES)
#	$(CC) -o radianceSplat $(OBJFILES)

#%.o: %.cpp
#	$(COMPILE) -o $@ $<