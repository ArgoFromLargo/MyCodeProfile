all: sender.o receiver.o network.o
	gcc -Wall sender.o -o sender
	gcc -Wall receiver.o -o receiver
	gcc -Wall network.o -o network

sender.o: rdtSender.h
	gcc -Wall -c sender.c

receiver.o: rdtReceiver.h
	gcc -Wall -c receiver.c

network.o:
	gcc -Wall -c network.c

clean:
	rm sender receiver network sender.o receiver.o network.o
