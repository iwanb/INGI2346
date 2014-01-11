#include "response.h"

void print_response_error(int errorno, char * msg) {
	printf("%s %d\n", msg, errorno);
}
