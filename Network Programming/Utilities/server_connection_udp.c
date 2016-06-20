#include "mylibrary.h"

int main(int argc, char** argv) {

	char buf[BUFLEN];									// buffer for message passing
	SOCKET s, s_acceptor;  			  		// socket and acceptor socket
	struct sockaddr_in saddr, caddr;  // server and client address structures
	uint16_t lport_n, lport_h; 				// server port number by htons()
	socklen_t caddr_len;							// client address length
	
	/* Check number of arguments */
	checkArg(argc, 2);

	/* Initialize the socket API (only for Windows) */
	SockStartup();

	/* Set port number  */
	lport_n = setPortarg(argv[1], &lport_h);
		//tport_n = setPortin(&lport_h);

	/* Create the socket */
	fprintf(stdout, "Creating the socket...\n");
	s = Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	fprintf(stdout, "- OK. Socket fd: %d\n", s);
	
	/* Prepare server address structure */
	saddr.sin_family = AF_INET;
	saddr.sin_port = lport_n;
	saddr.sin_addr.s_addr = INADDR_ANY;

	/* Bind the socket to any available local network address */
	fprintf(stdout, "Binding the socket to address ");
	showAddress(&saddr);
	Bind(s, (struct sockaddr*) &saddr, sizeof(saddr));
	fprintf(stdout, "- OK. Socket bound.\n");
	
	br();
	
	/* Main loop */
	while(1) {
		doSomething();
	}
	
	return 0;
}
