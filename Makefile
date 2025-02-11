CC = gcc
CFLAGS = -Wall -I./include
LIBS = -lncurses -pthread

SRCDIR = src
INCDIR = include

CLIENT_EXEC = client
SERVEUR_EXEC = serveur

all: $(CLIENT_EXEC) $(SERVEUR_EXEC)

$(CLIENT_EXEC): $(SRCDIR)/client.c $(SRCDIR)/grille.c $(SRCDIR)/init.c $(SRCDIR)/convert.c $(SRCDIR)/utils.c
	$(CC) $(CFLAGS) -o $@ $^ -I$(INCDIR) $(LIBS)

$(SERVEUR_EXEC): $(SRCDIR)/serveur.c $(SRCDIR)/grille.c $(SRCDIR)/init.c $(SRCDIR)/convert.c $(SRCDIR)/utils.c
	$(CC) $(CFLAGS) -o $@ $^ -I$(INCDIR) $(LIBS)

clean:
	rm -f $(CLIENT_EXEC) $(SERVEUR_EXEC)
