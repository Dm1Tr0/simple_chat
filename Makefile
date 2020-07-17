MAIN_DIR=$(PWD)
CC = gcc
CFLAGS = -std=gnu18 -g
IN_PROG_DEFS = -DEXLOG -DDEBUG
SILENT = -DEXLOG -DDEBUG=0
WARNINGS = -Wall -Wextra -Wpedantic
LDFL= -lpthread

#logig is on ese to debug
all : clean
	mkdir $(MAIN_DIR)/target
	mkdir $(MAIN_DIR)/target/inmsg
	cd $(MAIN_DIR)/target
	$(CC) $(IN_PROG_DEFS) $(CFLAGS) $(WARNINGS) $(MAIN_DIR)/src/soc_client.c $(LDFL) -o $(MAIN_DIR)/target/client
	$(CC) $(IN_PROG_DEFS) $(CFLAGS) $(WARNINGS) $(MAIN_DIR)/src/soc_server.c $(LDFL) -o $(MAIN_DIR)/target/server

# output data only in case of fatal errors
silent : clean
	mkdir $(MAIN_DIR)/target
	mkdir $(MAIN_DIR)/target/inmsg
	cd $(MAIN_DIR)/target
	$(CC) $(IN_PROG_DEFS) $(CFLAGS) $(WARNINGS) $(MAIN_DIR)/src/soc_client.c $(LDFL) -o $(MAIN_DIR)/target/client
	$(CC) $(SILENT) $(CFLAGS) $(WARNINGS) $(MAIN_DIR)/src/soc_server.c $(LDFL) -o $(MAIN_DIR)/target/server

clean :
	rm -rf $(MAIN_DIR)/target
