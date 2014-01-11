/* Requests from the client are in the form:
 * char request_id
 * followed by a number of arguments depending on the request id
 */

/* One argument, the nickname, a null terminated string with a maximum size */
#define REQUEST_CONNECT 0
/* No argument */
#define REQUEST_QUIT 1
/* Two arguments, nickname and the message, null terminated strings */
#define REQUEST_MSG 2
/* No argument */
#define REQUEST_WHO 3
