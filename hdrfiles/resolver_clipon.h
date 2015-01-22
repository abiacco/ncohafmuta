#ifndef _RESOLVER_CLIPON_H
#define _RESOLVER_CLIPON_H

/* How many secs select should wait for a socket to become readable	*/
/* A pending connection is not guaranteed to return a success or	*/
/* fail result to the talker in this amount of time. This is due	*/
/* to the fact that select() may return before the timeout.		*/
/* The longest time the talker will possibly have to wait for a		*/
/* is (RESOLVE_SELECT_TIMEOUT - 1) + RESOLVE_SELECT_TIMEOUT		*/
/*	i.e  if it's set to 5, it may take up to 9 seconds		*/
#define RESOLVE_SELECT_TIMEOUT         5
/* how many resolves we can do at a time */
#define MAX_RESOLVES_IN_PROGRESS       5
/* The ip address of a DNS server, in case we can't get it from a	*/
/* unix config file, which is usually /etc/resolv.conf		 	*/
/* PLEASE specify one of your local Internet Provider, if possible	*/
/* This will probably be used mostly on windows systems			*/
#define FALLBACK_DNS_SERVER		"63.205.114.67"
/* set to 1 if you want to use the fallback server even if we find a    */
/* local DNS server.. else leave at 0                                   */
#define USE_FALLBACK_AS_DEFAULT         0


/* ANYTHING BELOW HERE YOU SHOULDN'T NEED TO TOUCH! */

/* Our pipes we will set up between talker and clipon */
#define FROM_SERVER_READ resolver_toserver_pipes[0]
#define FROM_SERVER_WRITE resolver_toclient_pipes[1]
#define FROM_CLIENT_READ resolver_toclient_pipes[0]
#define FROM_CLIENT_WRITE resolver_toserver_pipes[1]

/* What kind of UDP socket sending do we want to do */
/* 43 and above will do connect() and send() */
/* < 43 will do sendto() */
/* I will use lowest common denominator for now */
#define RES_BSD 42

#define RESOLVER_SERVER_CONNECT_MSG "Server Connect:"
#define RESOLVER_CLIENT_CONNECT_MSG "Client Connect:"
#define RESOLVER_CLIENT_SEND_REQUEST "A"
#define RESOLVER_CLIENT_SEND_SUSPEND "S"
#define RESOLVER_CLIENT_SEND_RESUME "R"
#define RESOLVER_CLIENT_CANCEL_REQUEST "B"

/* checkpoint gets sent when select() exits in clipon, to make sure	*/
/* talker is still there. Talker disregards this string when reading	*/
/* replies. If the write() of checkpoint returns bad, we exit clipon.	*/
/* This should never happen unless someone kill -9's the talker		*/
/* MAKE SURE THE LAST CHAR HERE IS A SPACE, SSCANF DEPENDS ON IT!!	*/
#define RESOLVER_SERVER_SEND_CHECKPOINT ": "
#define RESOLVER_SERVER_SEND_REPLY ":" /* not used yet */
#define RESOLVER_BINARY			"resolver_clipon"

/* final resolve statuses */
#define  STA_WAITING                0
#define  STA_PENDING                1
#define  STA_DONE                   2

/* system resolve statuses */
#define  RES_SUCCESS                0
#define  RES_TIME_OUT               -1
#define  RES_NO_INFO                -2
#define  RES_ERROR                  -3
#define  RES_NONAUTH                -4
#define  RES_NO_RESPONSE            -5


#endif /* _RESOLVER_CLIPON_H */

