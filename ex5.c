/* An include and it is the way to import the contents of one file into this source file. C has a convention of using .h extensions for "header" files, which then contain lists of functions you want to use in your program. */
#include <stdio.h>

/* This is a comment */

/* A more complex version of the main function you've been using blindly so far. How C programs work is the operating system loads your program, and then runs the function named main. For the function to be totally complete it needs to return an int and take two parameters, an int for the argument count, and an array of char * strings for the arguments. Did that just fly over your head? Do not worry, we'll cover this soon. */
int main(int argc, char *argv[])
{
	int distance = 100;

	//this is also a comment

/* A call to your old friend printf. Like in many languages function calls work with the syntax name(arg1, arg2); and can have no arguments, or any number. The printf function is actually kind of weird and can take multiple arguments. We'll see that later. */
	printf("You are %d miles away.\n",distance);
	
/*A return from the main function, which gives the OS your exit value. You may not be familiar with how Unix software uses return codes, so we'll cover that as well. */	
	return 0;
}
