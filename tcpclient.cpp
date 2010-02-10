/*
 * tcpclient.cpp
 *
 *  Created on: Feb 2, 2010
 *      Author: sears
 */

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <assert.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "tcpclient.h"
#include "datatuple.h"
#include "network.h"

//	const char *appid;
//	const char *region;
struct logstore_handle_t {
	char *host;
	int portnum;
	int timeout;
	struct sockaddr_in serveraddr;
	struct hostent* server;
	int server_socket;
};

//LogStoreDBImpl::LogStoreDBImpl(const TestSettings & testSettings):
//	const char *appid, int timeout, const char *region, int portnum){
//    host_(testSettings.host()),
//    appid_(testSettings.appID()),
//    timeout_(testSettings.timeout()),
//    region_(testSettings.myRegion()),
//    routerLatency_(0.0),
//    suLatency_(0.0)
//    const std::string& appid_;
//    const int timeout_;
//    const std::string& region_;
//
//    int portnum;
//
//    int server_socket;
//
//    struct sockaddr_in serveraddr;
//    struct hostent *server;
//    ret->server_socket = -1;
//    portnum = 32432; //this should be an argument.

logstore_handle_t * logstore_client_open(const char *host, int portnum, int timeout) {
	logstore_handle_t *ret = (logstore_handle_t*) malloc(sizeof(*ret));
	ret->host = strdup(host);
	ret->portnum = portnum;
	if(ret->portnum == 0) { ret->portnum = 32432; }
	ret->timeout = timeout;
	ret->server_socket = -1;

    ret->server = gethostbyname(ret->host);
    if (ret->server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", ret->host);
        free(ret->host); free(ret); return 0;
    }

    /* build the server's Internet address */
    bzero((char *) &ret->serveraddr, sizeof(ret->serveraddr));
    ret->serveraddr.sin_family = AF_INET;
    bcopy((char *)ret->server->h_addr,
	  (char *)&ret->serveraddr.sin_addr.s_addr, ret->server->h_length);
    ret->serveraddr.sin_port = htons(ret->portnum);

    printf("LogStore start\n");

    return ret;
}

static inline void close_conn(logstore_handle_t *l) {
	printf("read/write err.. conn closed.\n");
	close(l->server_socket); //close the connection
    l->server_socket = -1;
}
datatuple *
logstore_client_op(logstore_handle_t *l,
//		  int *server_socket,
//          struct sockaddr_in serveraddr,
//          struct hostent *server,
          uint8_t opcode,  datatuple * tuple)
{

    if(l->server_socket < 0)
    {
        l->server_socket = socket(AF_INET, SOCK_STREAM, 0);

        if (l->server_socket < 0)
        {
            printf("ERROR opening socket.\n");
        return 0;
        }


        int flag = 1;
        int result = setsockopt(l->server_socket,            /* socket affected */
                                IPPROTO_TCP,     /* set option at TCP level */
                                TCP_NODELAY,     /* name of option */
                                (char *) &flag,  /* the cast is historical
                                                    cruft */
                                sizeof(int));    /* length of option value */
        if (result < 0)
        {
            printf("ERROR on setting socket option TCP_NODELAY.\n");
            return 0;
        }


        /* connect: create a connection with the server */
        if (connect(l->server_socket, (sockaddr*) &(l->serveraddr), sizeof(l->serveraddr)) < 0)
        {
            printf("ERROR connecting\n");
            return 0;
        }

        printf("sock opened %d\n", l->server_socket);
    }



    //send the opcode
    if( writetosocket(l->server_socket, &opcode, sizeof(opcode))  ) { close_conn(l); return 0; }

    //send the tuple
    if( writetupletosocket(l->server_socket, tuple)               ) { close_conn(l); return 0; }

    network_op_t rcode = readopfromsocket(l->server_socket,LOGSTORE_SERVER_RESPONSE);

    if( opiserror(rcode)                                          ) { close_conn(l); return 0; }

    datatuple * ret;

    if(rcode == LOGSTORE_RESPONSE_SENDING_TUPLES)
    {
    	ret = readtuplefromsocket(l->server_socket);

    } else if(rcode == LOGSTORE_RESPONSE_SUCCESS) {
    	ret = tuple;
    } else {
    	assert(rcode == LOGSTORE_RESPONSE_FAIL); // if this is an invalid response, we should have noticed above
    	ret = 0;
    }

    return ret;
}

int logstore_client_close(logstore_handle_t* l) {
    if(l->server_socket > 0)
    {
        writetosocket(l->server_socket, (char*) &OP_DONE, sizeof(uint8_t));

        close(l->server_socket);
        printf("socket closed %d\n.", l->server_socket);
    }
    free(l->host);
    free(l);
    return 0;
}
