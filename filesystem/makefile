# Source: https://www.cs.swarthmore.edu/~newhall/unixhelp/howto_makefiles.html
# This is a Makefile for the filesystem program
# Typing 'make' or 'make DRIVER' will create the executable file
#
#
# To create the executable file DRIVER we need the object files
# fsStructures.o, inputParser.o, fsImplementation.o, fsLow.o, fsdriver3.o

BUILD: inputParser.o fsImplementation.o fsLow.o fsdriver3.o
	gcc -o fsdriver3 inputParser.o fsImplementation.o fsLow.o fsdriver3.o


inputParser.o: inputParser.c inputParser.h 
	gcc -c inputParser.c


fsImplementation.o: fsImplementation.c fsImplementation.h 
	gcc -c fsImplementation.c


fsLow.o: fsLow.c fsLow.h 
	gcc -c fsLow.c


fsdriver3.o: fsdriver3.c 
	gcc -c fsdriver3.c



# To start over from scratch, type 'make clean'.  This
# removes the executable file, as well as old .o object
# files and *~ backup files:
#
clean: 
	$(RM) count *.o *~