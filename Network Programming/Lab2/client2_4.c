#include "mylibrary.h"

int main(int argc, char** argv) {

	char inbuf[BUFLEN], outbuf[BUFLEN]; // transmitter and receiver buffers
	SOCKET s; 													// socket
	struct in_addr sIPaddr;							// server IP address structure
	struct sockaddr_in saddr; 					// server address structure
	uint16_t tport_n, tport_h;					// server port number by htons()
	uint32_t taddr_n; 									// server IP address by htonl()
	XDR xdrs_in, xdrs_out;
	int xdr_len, addend1, addend2, sum;
	
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
	while(1) {
	
		/* Input the two addends */
		fprintf(stdout, "Insert the first integer:\n");
		fscanf(stdin, "%d", &addend1);
		
		fprintf(stdout, "Insert the second integer:\n");
		fscanf(stdin, "%d", &addend2);
		
		/* Create an XDR stream for output */
		xdrmem_create(&xdrs_out, outbuf, BUFLEN, XDR_ENCODE);
		
		/* Encode the two addends */
		xdr_int(&xdrs_out, &addend1);
		xdr_int(&xdrs_out, &addend2);
		
		/* Send the message and destroy the XDR stream */
		xdr_len = xdr_getpos(&xdrs_out);
		Write(s, outbuf, xdr_len);
		xdr_destroy(&xdrs_out);
		
		/* Get for the response */
		fprintf(stdout, "Waiting for response...\n");
		Read(s, inbuf, BUFLEN);
		
		/* Create an XDR stream for input */
		xdrmem_create(&xdrs_in, inbuf, BUFLEN, XDR_DECODE);
		
		/* Decode the result and destroy the XDR stream */
		xdr_int(&xdrs_in, &sum);
		xdr_destroy(&xdrs_in);
		
		/* Print the result */
		fprintf(stdout, "The sum is: %d\n", sum);

	}

	/* Close the socket connection */
	fprintf(stdout, "Closing the socket connection\n");
	closesocket(s);
	fprintf(stdout, "OK.\n");
	
	return 0;
}
