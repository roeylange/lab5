#format is target-name: target dependencies
#{-tab-}actions

# All Targets
all: looper

# Tool invocations
# Executable "hello" depends on the files numbers.o and main.o and add.s.
looper: looper.o looper.o
	gcc -g -m32 -Wall -o looper looper.o

looper.o: looper.c
	gcc -g -m32 -Wall -c -o looper.o looper.c

#tell make that "clean" is not a file name!
.PHONY: clean

#Clean the build directory
clean:
	rm -f *.o looper