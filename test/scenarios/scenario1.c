#include <stdio.h>
#include <stdbool.h>

bool scenario1(char** msg) {
	asprintf(msg, "This is an example scenario.");
	return true;
}
