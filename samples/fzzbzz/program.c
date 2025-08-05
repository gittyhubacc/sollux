#include <stdio.h>
int main(int argc, char **argv)
{
	for (int i =1; i <= 20; i++) {
		int state = 0;
               int checks = 0;
               int checked = 0;
		state |= (i % 3 == 0) << checks;
		if (state & (1 << (checks++))) {
			printf("fizz");
                       checked = 1;
		}
		state |= (i % 5 == 0) << checks;
		if (state & (1 << (checks++))) {
			printf("buzz");
                       checked = 1;
		}
               if (!checked) {
			printf("%i", i);
               }
		printf("\n");
	}
       return 0;
}
