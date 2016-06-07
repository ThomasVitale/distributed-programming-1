#include "mylibrary.h"

int main(int argc, char** argv) {

	char buf[BUFLEN];									// receiver buffer
	SOCKET s;						  			  		// socket
	struct sockaddr_in saddr, caddr;  // server and client address structures
	uint16_t lport_n, lport_h; 				// server port number by htons()
	int caddrlen, n, i, j, counter;
	unsigned long historyIP[10];				// FIFO containing the last 10 IP addresses served
	
	/* Check number of arguments */
	checkArg(argc, 2);

	/* Initialize the socket API (only for Windows) */
	SockStartup();

	/* Set port number  */
	lport_n = setPortarg(argv[1], &lport_h);

	/* Create the socket */
	fprintf(stdout, "Creating the socket\n");
	s = Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	fprintf(stdout, "OK. Socket fd: %d\n", s);
	
	/* Prepare server address structure */
	saddr.sin_family = AF_INET;
	saddr.sin_port = lport_n;
	saddr.sin_addr.s_addr = INADDR_ANY;

	/* Bind the socket to any available local network address */
	fprintf(stdout, "Binding the socket\n");
	Bind(s, (struct sockaddr*) &saddr, sizeof(saddr));
	fprintf(stdout, "OK.\n");
	
	i=0;
	
	/* Main loop */
	while(1) {
	
		counter=1;
	
		// Receive a message
		caddrlen = sizeof(struct sockaddr_in);
	  n = Recvfrom(s, buf, BUFLEN-1, 0, (struct sockaddr *) &caddr, (socklen_t*) &caddrlen);
	  buf[n] = '\0';
	  fprintf(stdout, "Received message: %s\n", buf);
	  
	  // If the client IP address was not served more than 3 times, serves it
	  historyIP[i%10] = caddr.sin_addr.s_addr;
	  
	  if (i>=10) {
	  	for (j=0; j<10; j++) {
	  		if (historyIP[j] == caddr.sin_addr.s_addr) {
	  			counter++;
	  		}
	  	}
	  } else {
	  	for (j=0; j<i; j++) {
	  		if (historyIP[j] == caddr.sin_addr.s_addr) {
	  			counter++;
	  		}
	  	}
	  }
	  
	  if (counter <= 3) {
			// Send a message
			Sendto(s, buf, n, 0, (struct sockaddr *) &caddr, caddrlen);
			fprintf(stdout, "Reply sent\n");
		} else {
			fprintf(stdout, "Not replied\n");
		}
		
		i++;
	}
	
	return 0;
}
