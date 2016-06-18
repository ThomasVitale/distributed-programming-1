#include "../mylibrary.h"

#define MAX_UINT16T 0xffff
#define MSG_ERR "incorrect operands\r\n"
#define MSG_OVF "overflow\r\n"

int receiver(SOCKET s);

int main(int argc, char** argv) {

	SOCKET s, s_acceptor;  			  		// socket and acceptor socket
	struct sockaddr_in saddr, caddr;  // server and client address structures
	uint16_t lport_n, lport_h; 				// server port number by htons()
	int backlog = 2;									// pending requests queue length
	socklen_t caddr_len;							// client address length
	int retValue;											// if the connection was closed by client or server
	
	/* Check number of arguments */
	checkArg(argc, 2);

	/* Initialize the socket API (only for Windows) */
	SockStartup();

	/* Set port number  */
	lport_n = setPortarg(argv[1], &lport_h);

	/* Create the socket */
	fprintf(stdout, "Creating the socket...\n");
	s = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	fprintf(stdout, "- OK. Socket fd: %d\n", s);
	
	/* Prepare server address structure */
	saddr.sin_family = AF_INET;
	saddr.sin_port = lport_n;
	saddr.sin_addr.s_addr = INADDR_ANY;

	/* Bind the socket to a local network address */
	fprintf(stdout, "Binding the socket...\n");
	Bind(s, (struct sockaddr*) &saddr, sizeof(saddr));
	fprintf(stdout, "- OK. Socket bound.\n");

	/* Listen to connection requests */
	fprintf(stdout, "Listen at socket %d with backlog = %d...\n", s, backlog);
	Listen(s, backlog);
	fprintf(stdout, "- OK. Socket is listening on ");
	showAddress(&saddr);
	
	while(1) {
		/* Accept connection requests */
		br();
		caddr_len = sizeof(struct sockaddr_in);
		fprintf(stdout, "Waiting for connection requests...\n");
		s_acceptor = Accept(s, (struct sockaddr*) &caddr, &caddr_len);
		fprintf(stdout, "- New connection from client ");
		showAddress(&caddr);
		
		retValue = receiver(s_acceptor);
		
		closesocket(s_acceptor);
		fprintf(stdout, "- Connection closed by %s", (retValue == 0) ? "client" : "server");
	}
	
	return 0;
}

int receiver(SOCKET s) {

	char buf[BUFLEN+1]; 
	int nread, nwritten, len; 		
	uint16_t op1, op2;
	uint32_t sum;  
	
	while(1) {
	
		/* Wait for data from client */
		fprintf(stdout, "- Waiting for operands...\n");
	
		/* Read data from client */
		if ((nread = readlineS(s, buf, BUFLEN)) < 0) {
			// Error
			fprintf(stderr, "--- ERROR. readlineS() failed.\n");
			return 0;
		} else if (nread == 0) {
			// Client disconnected
			return 0;
		}
		
		/* Append the string terminator and print the received message */
		buf[nread] = '\0';
		fprintf(stdout, "--- Received string: %s", buf);
		
		/* Get the two operands */
		if ((sscanf(buf, "%hu %hu", &op1, &op2) != 2)) {
			fprintf(stdout, "--- ERROR. wrong operands.\n");
			len = strlen(MSG_ERR);
			nwritten = write(s, MSG_ERR, len);
			if (nwritten != len) {
				fprintf(stderr, "--- ERROR. write() MSG_ERR failed.\n");
				return 0;
			}
			continue;
		}
		
		/* Print the operands */
		fprintf(stdout, "--- Operands: %hu %hu\n", op1, op2);
		
		/* Compute the sum */
		sum = op1 + op2;
		if (sum > MAX_UINT16T) {
			fprintf(stdout, "--- ERROR. sum overflow.\n");
			len = strlen(MSG_OVF);
			nwritten = write(s, MSG_OVF, len);
			if (nwritten != len) {
				fprintf(stderr, "--- ERROR. write() MSG_OVF failed.\n");
				return 0;
			}
			continue;
		} else {
			fprintf(stdout, "--- Sum: %u\n", sum);
		}
		
		/* Generate the result string to send back */
		snprintf(buf, BUFLEN, "%d\r\n", sum);
		
		/* Send back the result */
		len = strlen(buf);
		nwritten = write(s, buf, len);
		if (nwritten != len) {
			fprintf(stderr, "--- ERROR. write() answer failed.\n");
			return 0;
		} else {
			fprintf(stdout, "--- Result sent back.\n");
		}
		
	}
	
}
