#include "../mylibrary.h"

int main(int argc, char** argv) {

	char buf[BUFLEN];									// buffer for message passing
	SOCKET s;  			  								// socket
	struct sockaddr_in saddr, caddr;  // server and client address structures
	uint16_t lport_n, lport_h; 				// server port number by htons()
	socklen_t caddr_len;							// client address length
	int n;
	
	/* Check number of arguments */
	checkArg(argc, 2);

	/* Set port number  */
	lport_n = setPortarg(argv[1], &lport_h);

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
	
		/* Wait for data from client */
		fprintf(stdout, "Waiting for data...\n");
	
		/* Receive a message */
		caddr_len = sizeof(struct sockaddr_in);
	  n = Recvfrom(s, buf, BUFLEN-1, 0, (struct sockaddr *) &caddr, (socklen_t*) &caddr_len);
	  buf[n] = '\0';
	  fprintf(stdout, "- Received message: %s\n", buf);

		/* Send a message */
	  Sendto(s, buf, n, 0, (struct sockaddr *) &caddr, caddr_len);
		fprintf(stdout, "- Reply sent\n");
	}
	
	return 0;
}
