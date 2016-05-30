#include "mylibrary.h"

int main(int argc, char** argv) {

	char inbuf[BUFLEN], outbuf[BUFLEN]; // transmitter and receiver buffers
	SOCKET s; 													// socket
	struct in_addr sIPaddr;							// server IP address structure
	struct sockaddr_in saddr; 					// server address structure
	uint16_t tport_n, tport_h;					// server port number by htons()
	uint32_t taddr_n; 									// server IP address by htonl()
	int stop, len, result;
	
	/* Check arguments */
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
	
	/* Actual communication with the server */
	stop = 0;
	while(!stop) {
		fprintf(stdout, "Input two integers. 'end' or 'stop' to finish.\n");
		fgets(inbuf, BUFLEN, stdin);
		
		if (iscloseorstop(inbuf)) {
			stop = 1;
		} else {
			len = strlen(inbuf);
			Write(s, inbuf, len);
			
			fprintf(stdout, "Waiting for response...\n");
			result = Read(s, outbuf, BUFLEN);
			
			if (result) {
				outbuf[result] = '\0';
				fprintf(stdout, "Result: %s", outbuf);
			}
		}
	}

	/* Close the socket connection */
	fprintf(stdout, "Closing the socket connection\n");
	closesocket(s);
	fprintf(stdout, "OK.\n");
	
	return 0;
}
