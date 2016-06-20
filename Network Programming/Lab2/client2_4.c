#include "../mylibrary.h"

int main(int argc, char** argv) {

	char buf[BUFLEN], rbuf[BUFLEN];	// transmitter and receiver buffers
	SOCKET s; 											// socket
	struct in_addr sIPaddr;					// server IP address structure
	struct sockaddr_in saddr; 			// server address structure
	uint16_t tport_n, tport_h;			// server port number by htons()
	XDR xdrs_in, xdrs_out;
	int xdr_len, addend1, addend2, sum, ret1, ret2;
	
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
	
	/* Actual communication with the server */
	while(1) {
	
		br();
	
		/* Get two integers */
		fprintf(stdout, "Input the first unsigned integer:\n");
		ret1 = fscanf(stdin, "%d", &addend1);
		fprintf(stdout, "Input the second unsigned integer:\n");
		ret2 = fscanf(stdin, "%d", &addend2);
		if (ret1 != 1 || ret2 != 1) {
			fprintf(stderr, "ERROR. Input values must be unsigned integers.\n");
			/* Close the socket connection */
			fprintf(stdout, "Closing the socket connection\n");
			closesocket(s);
			fprintf(stdout, "- OK.\n");
			return 0;
		}
	
		/* Create an XDR stream for output */
		xdrmem_create(&xdrs_out, rbuf, BUFLEN, XDR_ENCODE);
		
		/* Encode the two addends */
		xdr_int(&xdrs_out, &addend1);
		xdr_int(&xdrs_out, &addend2);
		
		/* Send the message and destroy the XDR stream */
		xdr_len = xdr_getpos(&xdrs_out);
		Write(s, rbuf, xdr_len);
		xdr_destroy(&xdrs_out);
		
		/* Get the response */
		fprintf(stdout, "Waiting for response...\n");
		Read(s, buf, BUFLEN);
		
		/* Create an XDR stream for input */
		xdrmem_create(&xdrs_in, buf, BUFLEN, XDR_DECODE);
		
		/* Decode the result and destroy the XDR stream */
		xdr_int(&xdrs_in, &sum);
		xdr_destroy(&xdrs_in);
		
		/* Print the result */
		fprintf(stdout, "--- The sum is: %d\n", sum);

	}

	/* Close the socket connection */
	fprintf(stdout, "Closing the socket connection...\n");
	closesocket(s);
	fprintf(stdout, "- OK. Closed.\n");
	
	return 0;
}
