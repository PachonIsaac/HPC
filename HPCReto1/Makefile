CC=gcc
CFLAGS=-O2 -Wall -pthread
SRC=src
BIN=bin

all: pi_needles_serial pi_needles_thread pi_needles_fork pi_dartboard_serial pi_dartboard_thread pi_dartboard_fork pi_needles_serial_opt pi_dartboard_serial_opt
pi_dartboard_serial_opt: $(SRC)/pi_dartboard_serial_opt.c
	$(CC) $(CFLAGS) -o $(BIN)/pi_dartboard_serial_opt $(SRC)/pi_dartboard_serial_opt.c -lm

pi_needles_serial_opt: $(SRC)/pi_needles_serial_opt.c
	$(CC) $(CFLAGS) -o $(BIN)/pi_needles_serial_opt $(SRC)/pi_needles_serial_opt.c -lm

pi_needles_serial: $(SRC)/pi_needles_serial.c
	$(CC) $(CFLAGS) -o $(BIN)/pi_needles_serial $(SRC)/pi_needles_serial.c -lm

pi_needles_thread: $(SRC)/pi_needles_thread.c
	$(CC) $(CFLAGS) -o $(BIN)/pi_needles_thread $(SRC)/pi_needles_thread.c -lm

pi_needles_fork: $(SRC)/pi_needles_fork.c
	$(CC) $(CFLAGS) -o $(BIN)/pi_needles_fork $(SRC)/pi_needles_fork.c -lm

pi_dartboard_serial: $(SRC)/pi_dartboard_serial.c
	$(CC) $(CFLAGS) -o $(BIN)/pi_dartboard_serial $(SRC)/pi_dartboard_serial.c -lm

pi_dartboard_thread: $(SRC)/pi_dartboard_thread.c
	$(CC) $(CFLAGS) -o $(BIN)/pi_dartboard_thread $(SRC)/pi_dartboard_thread.c -lm

pi_dartboard_fork: $(SRC)/pi_dartboard_fork.c
	$(CC) $(CFLAGS) -o $(BIN)/pi_dartboard_fork $(SRC)/pi_dartboard_fork.c -lm

clean:
	rm -rf $(BIN)/*
