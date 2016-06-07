#include     "mylibrary.h"

int main(int argc, char** argv) {

	SOCKET s; 											// socket
	struct in_addr sIPaddr;					// server IP address structure
	struct sockaddr_in saddr; 			// server address structure
	uint16_t tport_n, tport_h;			// server port number by htons()
	uint32_t taddr_n; 							// server IP address by htonl()
	
	/* Check number of arguments */
	if (argc != 3) {
		fprintf(stderr, "Input: %s ip_address port_number:\n", argv[0]);
		return -1;
	}
	
	/* Set IP address and port number of Server */
	setIParg(argv[1], &sIPaddr);
	tport_n = setPortarg(argv[2], &tport_h);

	/* Create the socket */
	fprintf(stdout, "Creating the socket\n");
	s = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	fprintf(stdout, "OK. Socket fd: %d\n", s);

	/* Prepare server address structure */
	saddr.sin_family = AF_INET;
	saddr.sin_port = tport_n;
	saddr.sin_addr = sIPaddr;

	/* Send connection request */
	fprintf(stdout, "Connecting to target address\n");
	Connect(s, (struct sockaddr*) &saddr, sizeof(saddr));
	fprintf(stdout, "OK.\n");

	/* Close the socket connection */
	fprintf(stdout, "Closing the socket connection\n");
	closesocket(s);
	fprintf(stdout, "OK.\n");
	
	return 0;
}
