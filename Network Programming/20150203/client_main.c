#include "../mylibrary.h"
#include "../utils.h"

#define MB_SIZE 1048576

char *prog_name;

int main (int argc, char *argv[]) {
	
	char buf[MB_SIZE], *tbuf;
	SOCKET s; 										 	// socket
	struct in_addr sIPaddr;					// server IP address structure
	struct sockaddr_in saddr; 			// server address structure
	uint16_t tport_n, tport_h;			// server port number by htons()
	int nMB, i;												
	char* fileName;									
	FILE* inputFile;
	uint32_t hc, hcn;
	long int fileSize;
	uint32_t nReadBytes, nLeftBytes, nNextBytes;

	/* Check number of arguments */
	checkArg(argc, 5);
	
	/* Save the program name */
	prog_name = argv[0];
	
	/* Set IP address and port number of Server */
	setIParg(argv[1], &sIPaddr);
	tport_n = setPortarg(argv[2], &tport_h);
	
	/* Save the number of MB to send */
	nMB = atoi(argv[3]);
	
	/* The file to read */
	fileName = argv[4];
	
	/* Open the file */
	inputFile = fopen(fileName, "rb");
	if (inputFile == NULL) {
		fprintf(stderr, "ERROR. The file does not exist.\n");
		return 1;
	}
	
	/* Check the file size */
	fseek(inputFile, 0L, SEEK_END);
	fileSize = ftell(inputFile);
	fseek(inputFile, 0L, SEEK_SET);
	if (fileSize < MB_SIZE) {
		fprintf(stderr, "ERROR. The file must be at least 1MB.\n");
		return 1;
	}
	
	/* Prepare the buffer to send */
	nReadBytes = fread(buf, 1, MB_SIZE, inputFile);
	if (nReadBytes != MB_SIZE) {
		fprintf(stderr, "ERROR. fread() failed.\n");
		return 1;
	}
	
	if (fclose(inputFile) != 0) {
		fprintf(stderr, "ERROR. fclose() failed.\n");
	}

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
	
	/* Main */
	for (i = 0; i < nMB; i++) {
		tbuf = buf;
		nLeftBytes = (uint32_t) MB_SIZE;

		while(nLeftBytes > 0) {
		  if (nLeftBytes > MAXBUF) {
				nNextBytes = MAXBUF;
			} else {
				nNextBytes = nLeftBytes;
			}
	
			Writen(s, tbuf, nNextBytes);
			
			nLeftBytes -= nNextBytes;
			tbuf += nNextBytes;
		}
	}
	
	/* Read and show the final hash code */
	Read(s, &hcn, sizeof(uint32_t));
	hc = ntohl(hcn);
	fprintf(stdout, "%u\n", hc);

	/* Close the socket connection */
	closesocket(s);
	
	return 0;
}
