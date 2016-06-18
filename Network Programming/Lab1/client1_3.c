#include "../mylibrary.h"

int main(int argc, char** argv) {

	char buf[BUFLEN], rbuf[BUFLEN];	// transmitter and receiver buffers
	SOCKET s; 											// socket
	struct in_addr sIPaddr;					// server IP address structure
	struct sockaddr_in saddr; 			// server address structure
	uint16_t tport_n, tport_h;			// server port number by htons()
	uint16_t op1, op2;
	int len, ret1, ret2;

	/* Check number of arguments */
	checkArg(argc, 3);
	
	/* Set IP address and port number of Server */
	setIParg(argv[1], &sIPaddr);
	tport_n = setPortarg(argv[2], &tport_h);

	/* Create the socket */
	fprintf(stdout, "Creating the socket...\n");
	s = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	fprintf(stdout, "- OK. Socket fd: %d\n", s);
	
	/* Prepare server address structure */
	saddr.sin_family = AF_INET;
	saddr.sin_port = tport_n;
	saddr.sin_addr = sIPaddr;

	/* Send connection request */
	fprintf(stdout, "Connecting to target address...\n");
	Connect(s, (struct sockaddr*) &saddr, sizeof(saddr));
	fprintf(stdout, "- OK. Connected to ");
	showAddress(&saddr);
	
	br();
	
	/* Get two integers */
	fprintf(stdout, "Input the first unsigned integer:\n");
	ret1 = fscanf(stdin, "%hu", &op1);
	fprintf(stdout, "Input the second unsigned integer:\n");
	ret2 = fscanf(stdin, "%hu", &op2);
	if (ret1 != 1 || ret2 != 1) {
		fprintf(stderr, "ERROR. Input values must be unsigned integers.\n");
		/* Close the socket connection */
		fprintf(stdout, "Closing the socket connection\n");
		closesocket(s);
		fprintf(stdout, "- OK.\n");
		return 0;
	}
	
	
	sprintf(buf, "%hu %hu \n", op1, op2);
	
	/* Send them to the server */
	len = strlen(buf);
	Write(s, buf, len);
	
	/* Data sent, now wait */
	fprintf(stdout, "- Data sent, waiting for response...\n");
	
	/* Get a response from the server */
	ReadlineS(s, rbuf, BUFLEN);
	
	/* Print the response */	
	switch(rbuf[0]) {
		case 'o':
			fprintf(stderr, "ERROR. Overflow.\n");
			break;
		case 'i':
			fprintf(stderr, "ERROR. Incorrect operands.\n");
			break;
		default:
			fprintf(stdout, "Sum: %s", rbuf);
	}
	
	br();

	/* Close the socket connection */
	fprintf(stdout, "Closing the socket connection...\n");
	closesocket(s);
	fprintf(stdout, "- OK. Closed.\n");
	
	return 0;
}
