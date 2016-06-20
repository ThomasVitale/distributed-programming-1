#include "../mylibrary.h"
#include "../utils.h"

#define MB_SIZE 1048576

/* Function executed to serve each client */
int receiver(SOCKET);

char *prog_name; // the program name
uint32_t nAcceptedBytes;

int main (int argc, char *argv[]) {

	SOCKET s, s_acceptor;  			  		// socket and acceptor socket
	struct sockaddr_in saddr, caddr;  // server and client address structures
	uint16_t lport_n, lport_h; 				// server port number by htons()
	int backlog = 2;									// pending requests queue length
	socklen_t caddr_len;							// client address length
	int retValue;											// if the connection was closed by client or server
	
	/* Check number of arguments */
	checkArg(argc, 3);
	
	/* Save the program name */
	prog_name = argv[0];

	/* Set port number  */
	lport_n = setPortarg(argv[1], &lport_h);
	
	/* Save the number of MB to receive upon each established connection */
	if (sscanf(argv[2], "%" SCNu32, &nAcceptedBytes) != 1) {
		fprintf(stderr, "Number of megabytes must be an integer.\n");
		return 1;
	}
	nAcceptedBytes *= MB_SIZE;

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
		caddr_len = sizeof(struct sockaddr_in);
		fprintf(stdout, "Waiting for connection requests...\n");
		s_acceptor = Accept(s, (struct sockaddr*) &caddr, &caddr_len);
		fprintf(stdout, "- New connection from client ");
		showAddress(&caddr);
		
		retValue = receiver(s_acceptor);
		close(s_acceptor);
		//fprintf(stdout, "--- Connection closed by %s", (retValue == 0) ? "client" : "server");
	}

	return 0;
}

int receiver(SOCKET s) {

	char *buf; // buffer for message passing
	uint32_t nReadBytes, nLeftBytes, nNextBytes; // number of bytes already read and still to read
	uint32_t hc = 1, hcn;
	int finished = 0;
	
	buf = (char*) malloc(MAXBUF*sizeof(void));
	
	nLeftBytes = nAcceptedBytes;
	
	while(nLeftBytes > 0 && !finished) {
	
		if (nLeftBytes > MAXBUF) {
			nNextBytes = MAXBUF;
		} else {
			nNextBytes = nLeftBytes;
		}
	
		nReadBytes = readn(s, buf, nNextBytes);
		
		if (nReadBytes > 0) {
			hc = hashCode(buf, nReadBytes, hc);
			nLeftBytes -= nReadBytes;
		} else {
			// Error or EOF
			finished = 1;
		}
	}
	
	if (nLeftBytes == 0) {
		fprintf(stdout, "%u\n", hc);
		hcn = htonl(hc);
		if (write(s, &hcn, sizeof(uint32_t)) <= 0) {
			// Error
			fprintf(stderr, "--- ERROR. write() failed.\n");
			free(buf);
			return 0;
		}
		return 1;
	} else { 
		// Error
		fprintf(stderr, "--- ERROR. The number of bytes received is less than expected.\n");
		free(buf);
		return 0;
	} 
	free(buf);
	return 1;
	
}
