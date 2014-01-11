#include <stdio.h>

/* Same format as requests 
 * char response
 * Arguments ...
 * An error occured if response < 0
 */

#define NICKNAME_TOO_LONG -1
#define NICKNAME_ALREADY_IN_USE -2
#define UNKNOWN_NICKNAME -3
#define TOO_MANY_CONNECTIONS -4

// string from
// string msg
#define ASYNC_MSG 1

void print_response_error(int errorno, char * msg);
