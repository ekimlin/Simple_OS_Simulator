CXXFLAGS=-Wall -g -std=c++11

OBJS = main.o Scheduler.o Memory.o PCB.o

.PHONY: clean

run.me : $(OBJS)
	g++ $(CXXFLAGS) $(OBJS) -o run.me

main.o : Scheduler.h

Scheduler.o : PCB.h Memory.h

Memory.o : PCB.h

clean: 
	(\rm -f *.o*; rm -f run.me)
