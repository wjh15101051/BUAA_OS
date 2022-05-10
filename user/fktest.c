#include "lib.h"


void umain()
{
	int a = 0;
	int id = 0;

	if ((id = fork()) == 0) {
		if ((id = fork()) == 0) {
			a += 3;

			for (;;) {
				writef("\t\tthis is child2 :a:%d\n", a);
			}
		}
//		writef("child2 : %d\n", id);

		a += 2;

		for (;;) {
			writef("\tthis is child :a:%d\n", a);
		}
	}
//	writef("child1 : %d\n", id);

	a++;

	for (;;) {
		writef("this is father: a:%d\n", a);
	}
}
