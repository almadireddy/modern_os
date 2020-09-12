// Simple inline assembly example
// 
// For JOS lab 1 exercise 1
// By: Aahlad Madireddy

#include <stdio.h>

int main(int argc, char **argv) {
	int x = 1;
	printf("Hello x = %d\n", x);
	
	asm("add %0, %1" : "=r" (x) : "r" (1));
	printf("Hello x = %d after increment\n", x);

	if(x == 2){
		printf("OK\n");
	}
	else{
		printf("ERROR\n");
	}
}
