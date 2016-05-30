#include "mylibrary.h"

int main(int argc, char** argv) {

	char buf[BUFLEN], rbuf[BUFLEN];	// transmitter and receiver buffers
	SOCKET s; 											// socket
	struct in_addr sIPaddr;					// server IP address structure
	struct sockaddr_in saddr, caddr;// server address structure
	uint16_t tport_n, tport_h;			// server port number by htons()
	uint32_t taddr_n; 							// server IP address by htonl()
	struct timeval tval;
	fd_set cset;
	int len, fromlen, n;
	struct sockaddr_in from;				

	/* Check number of arguments */
	checkArg(argc, 4);

	/* Initialize the socket API (only for Windows) */
	SockStartup();
	
	/* Set IP address and port number of Server */
	setIParg(argv[1], &sIPaddr);
	tport_n = setPortarg(argv[2], &tport_h);

	/* Create the socket */
	fprintf(stdout, "Creating the socket\n");
	s = Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	fprintf(stdout, "OK. Socket fd: %d\n", s);
	
	/* Prepare client address structure */
	caddr.sin_family = AF_INET;
	caddr.sin_port = htons(0); // Ask for an unused port number
	caddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	/* Bind the socket to a local network address */
	fprintf(stdout, "Binding the socket\n");
	Bind(s, (struct sockaddr*) &caddr, sizeof(caddr));
	fprintf(stdout, "OK.\n");
	
	/* Prepare server address structure */
	saddr.sin_family = AF_INET;
	saddr.sin_port = tport_n;
	saddr.sin_addr = sIPaddr;

	/* Send a string */
	strcpy(buf, argv[3]);
	len = strlen(buf);
	Sendto(s, buf, len, 0, (struct sockaddr *) &saddr, sizeof(saddr));
	fprintf(stdout, "Waiting for response...\n");
	
	/* Set timeout */
	FD_ZERO(&cset);
	FD_SET(s, &cset);
	tval.tv_sec = TIMEOUT;
	tval.tv_usec = 0;
	n = Select(FD_SETSIZE, &cset, NULL, NULL, &tval);
	
	/* Receive a datagram */
	if (n > 0) {
		fromlen = sizeof(struct sockaddr_in);
	  n = Recvfrom(s, rbuf, BUFLEN-1, 0, (struct sockaddr *) &from, (socklen_t*) &fromlen);
    rbuf[n] = '\0';
		fprintf(stdout, "Received response: %s\n", rbuf);
	} else {
		fprintf(stdout, "No response received after %d seconds\n", TIMEOUT);
	}
	
	/* Close the socket connection */
	fprintf(stdout, "Closing the socket connection\n");
	closesocket(s);
	fprintf(stdout, "OK.\n");
	
	/* Release resources (only for Windows) */
	SockCleanup();

	return 0;
}
