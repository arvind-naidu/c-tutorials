#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define MAX_DATA 512
#define MAX_ROWS 100

/* The Address struct then uses these constants to create
 a piece of data that is fixed in size making it less efficient, 
 but easier to store and read. */
struct Address
{
	 int id;
	 int set;
	 char name[MAX_DATA];
	 char email[MAX_DATA];
};

// Fixed Sized Structs

/* The Database struct is then also fixed size because 
   it is a fixed length array of Address structs. That lets you 
   write the whole thing to disk in one move later on. */
struct Database
{
	struct Address rows[MAX_ROWS];
};

struct Connection
{
	FILE *file;
	struct Database *db;
};

// die function to abort with an error

/* I call this die, and it's used after 
   any failed function calls or bad inputs to 
   exit with an error using exit. */
void die(const char *message)
{
	// errno and perror() for error reporting

	/* When you have an error return from a function, it will usually set 
	   an "external" variable called errno to say exactly what error happened. 
	   These are just numbers, so you can use perror to "print the error message". */
	if(errno){
		perror(message);
	} else {
		printf("ERROR: %s\n", message);
	}

	exit(1);
}

void Address_print(struct Address *addr)
{
	printf("%d %s %s\n", addr->id, addr->name, addr->email);
}

void Database_load(struct Connection *conn)
{
	int rc = fread(conn->db, sizeof(struct Database), 1, conn->file);
	if (rc != 1) die("Failed to load database.");
}

// FILE functions

/* I'm using all new functions like fopen, fread, fclose, and rewind to work with files. 
   Each of these functions works on a FILE struct that's just like your structs, 
   but it's defined by the C standard library. */
struct Connection *Database_open(const char *filename, char mode)
{
	struct Connection *conn = malloc(sizeof(struct Connection));
	// NULL is 0 so boolean works

	/* In many of the checks I'm testing that a pointer is not NULL 
	   by simply doing if(!ptr) die("fail!") this is valid because NULL 
	   will evaluate to false. You could be explicit and say if(ptr == NULL) die("fail!") 
	   as well. On some rare systems NULL will be stored in the computer (represented) as 
	   something not 0, but the C standard says you should still be able to write code as 
	   if it has a 0 value. From now on when I say "NULL is 0" I mean its value for anyone 
	   who is overly pedantic. */
	if(!conn) die("Memory Error");

	conn->db = malloc(sizeof(struct Database));
	if (!conn->db) die("Memory Error");

	if(mode == 'c'){
		conn->file = fopen(filename, "w");
	} else {
		conn->file = fopen(filename, "r+");

		if (conn->file) {
			Database_load(conn);
		}
	}

	if(!conn->file) die("Failed to open the file");

	return conn;
}

void Database_close(struct Connection *conn)
{
	if(conn) {
		if(conn->file) fclose(conn->file);
		if(conn->db) free(conn->db);
		free(conn);
	}
}

void Database_write(struct Connection *conn)
{
	rewind(conn->file);

	int rc = fwrite(conn->db, sizeof(struct Database), 1, conn->file);
	if(rc != 1) die("Failed to write database.");

	rc = fflush(conn->file);
	if(rc == -1) die("Cannot flush database.");
}

void Database_create(struct Connection *conn)
{
	int i = 0;

	for (i = 0; i < MAX_ROWS; i++){
		//make a prototype to initialize it
		struct Address addr = {.id = i, .set = 0};
		// then just assign it
		conn->db->rows[i] = addr;
	}
}

void Database_set(struct Connection *conn, int id, const char *name, const char *email)
{
	// nested struct pointers

	/* There's use of nested structures and getting the address of array elements 
	   that you should study. Specifically code like &conn->db->rows[i] which reads 
	   "get the i element of rows, which is in db, which is in conn, then get the address 
	   of (&) it". */
	struct Address *addr = &conn->db->rows[id];
	if(addr->set) die("Already set, delete it first.");

	addr->set = 1;
	// WARNING: bug, read the "How To Break It" and fix this
	char *res = strncpy(addr->name, name, MAX_DATA);
	// demonstrate the strncpy bug
	if(!res) die("Name copy failed");

	res = strncpy(addr->email, email, MAX_DATA);
	if(!res) die("Name copy failed");
}

void Database_get(struct Connection *conn, int id)
{
	struct Address *addr = &conn->db->rows[id];

	if(addr->set){
		Address_print(addr);
	} else {
		die("ID is not set");
	}
}

void Database_delete(struct Connection *conn, int id)
{
	// copying struct prototypes

	/* best shown in Database_delete, you can see I'm using a temporary local Address, 
	initializing its id and set fields, and then simply copying it into the rows array 
	by assigning it to the element I want. This trick makes sure that all fields but set 
	and id are initialized to 0s and is actually easier to write. Incidentally, you shouldn't 
	be using memcpy to do these kinds of struct copying operations. Modern C allows you to simply 
	assign one struct to another and it'll handle the copying for you. */
	struct Address addr = {.id = id, .set = 0};
	conn->db->rows[id] = addr;
}

void Database_list(struct Connection *conn)
{
	int i = 0;
	struct Database *db = conn->db;

	for(i = 0; i < MAX_ROWS; i++) {
		struct Address *cur = &db->rows[i];

		if(cur->set) {
			Address_print(cur);
		}
	}
}

int main(int argc, char *argv[])
{
	if(argc < 3) die("USAGE: ex17 <dbfile> <action> [action params]");

	char *filename = argv[1];
	char action = argv[2][0];
	struct Connection *conn = Database_open(filename, action);
	int id = 0;

	// converting strings to ints

	/* I use the atoi function to take the string for the id on the command line 
	   and convert it to the int id variable. Read up on this function and similar ones. */
	if(argc > 3) id = atoi(argv[3]);
	if(id >= MAX_ROWS) die("There's not that many records.");

	switch(action){
		case 'c':
			Database_create(conn);
			Database_write(conn);
			break;

		case 'g':
			if(argc != 4) die("Need an id to get");

			Database_get(conn, id);
			break;

		case 's':
			if(argc != 6) die("Need id, name, email to set");

			Database_set(conn, id, argv[4], argv[5]);
			Database_write(conn);
			break;

		case 'd':
			if(argc != 4) die("Need id to delete");

			Database_delete(conn, id);
			Database_write(conn);
			break;

		case 'l':
			Database_list(conn);
			break;

		default: die("Invalid action, only: c=create, g=get, s=set, d=del, l=list");
	}

	Database_close(conn);

	return 0;
}