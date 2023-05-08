/*
** talker.c -- a datagram "client" demo
 * code snippits taken from Beej's guide
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

#define MAXBUFLEN 100
#define PACKETLENGTH 1000
#define MAX_NUMBER_STRINGS 2
#define MAX_STRING_SIZE 10

typedef struct {
      unsigned int total_frag;
      unsigned int frag_no;
      unsigned int size;
      char* filename;
      char filedata[PACKETLENGTH];
 } PACKET ;


int recvtimeout(int s, char (*buf)[100], int len, struct sockaddr *from,int *fromlen, float timeout) {
  fd_set fds;
  int n;
  struct timeval tv;
  int numbytes;

  FD_ZERO(&fds);
  FD_SET(s, &fds);

  tv.tv_usec = floor(fmod(timeout,1)*1000000);
  tv.tv_sec= floor(timeout);

  n = select(s+1,&fds,NULL,NULL,&tv);
  //printf("this is n %d\n",n);
  if(n == 0) return -2;
  if(n == -1) return -1;

  numbytes = recvfrom(s, buf, len, 0,from,fromlen);

  return numbytes;

}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
//Function to send packets
void sendPacket(FILE *file, char* filename, char* buf3, unsigned int total_frag,unsigned int frag_no, int sockfd,struct addrinfo* p, bool * resendPacket) {
      PACKET packet;


   // guard so we don't reread packet on timeout
  if (*resendPacket == 0) {

  memset(packet.filedata,0,PACKETLENGTH);
  fread(packet.filedata,sizeof(char), PACKETLENGTH, file);

  packet.total_frag = total_frag;
  packet.frag_no = frag_no;

// Shorten last packet
  if (packet.frag_no == packet.total_frag) {
    fseek(file, 0, SEEK_END);
    packet.size = (ftell(file) - 1) % 1000 + 1;
  }
  else {packet.size = PACKETLENGTH;}
  packet.filename = filename;


  memset(buf3, 0, sizeof buf3);
  


   int j = 0;
   /* Format and print various data */
   // Convert to string
   j = sprintf(buf3, "%d", packet.total_frag);
   j += sprintf(buf3+j, "%s", ":");
   j += sprintf(buf3+j, "%d", packet.frag_no);
   j += sprintf(buf3+j, "%s", ":");
   j += sprintf(buf3+j, "%d", packet.size);
   j += sprintf(buf3+j, "%s", ":");
   j += sprintf(buf3+j, "%s", packet.filename);
   j += sprintf(buf3+j, "%s", ":");
   memcpy(buf3+j, packet.filedata, sizeof(char)*packet.size);//changed this

   //printf("%s\n", packet.filedata);
   //printf("%s\n", buf3);
  }
  

  int numbytes;

  if ((numbytes = sendto(sockfd, buf3, PACKETLENGTH+100, 0,
			 p->ai_addr, p->ai_addrlen)) == -1) {
		perror("deliver: sendto");
		exit(1);
	}

  printf("deliver: sent packet %d\n", frag_no);
}



int main(int argc, char *argv[])

{
    
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
        clock_t senTime;  //clock_t is a datatype for number of clock ticks and is counted by the time a process has taken
                           // to be completed in a CPU since an arbitrary event
        clock_t recTime; //clock_t is long int and CLOCKS_PER_SEC is of integer data type
        clock_t sampleStart = 0;
        clock_t sampleEnd = 0;
        float timeout;
        float DevRTT; 
        float SampleRTT;
        float EstimatedRTT;
        float beta;
        float alpha;
        float TimeoutInterval;
        bool sampleTaken = false;


        

        
 

	if (argc != 3) {
		fprintf(stderr,"usage: deliver hostname message\n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;
        
	if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

               
	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("deliver: socket");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "deliver: failed to create socket\n");
		return 2;
	}
        
  //New code
  
  char buf[MAXBUFLEN];
  
  // Take input string and split words
  printf("deliver: enter command\n");

  fgets(buf, MAXBUFLEN, stdin);

  char* command = strtok(buf, " ");

  char* filename;
  
  if (filename = strtok(NULL, " ")) {}
  else {
    printf("deliver: missing filename\n");
    exit(1);
    }
  
  filename[strlen(filename)-1] = '\0';

  //check if command is ftp
  if(strcmp(command,"ftp") != 0) {
      printf("talker: ftp command or file name missing\n");
      exit(1);
  } 
  
  //check if file exists
  if(access(filename, F_OK) == -1) {
      fprintf(stderr,"deliver: the file %s does not exist \n", filename);
      exit(1);
  } 
  
  //if command is ftp, reply with ftp

  //start timer
	

        senTime = clock();
  
	if ((numbytes = sendto(sockfd, command, strlen(command), 0,
			 p->ai_addr, p->ai_addrlen)) == -1) {
		perror("deliver: sendto");
		exit(1);
	}

	freeaddrinfo(servinfo);

	printf("deliver: sent %d bytes to %s\n", numbytes, argv[1]);
        
        char buf2[MAXBUFLEN];
        //clear buffer
 


        struct sockaddr_storage their_addr;
	    socklen_t addr_len;
        addr_len = sizeof their_addr;
        char s[INET6_ADDRSTRLEN];
        
        
        //listen for server reply
        printf("deliver: waiting to recvfrom...\n");
        
        if ((numbytes = recvfrom(sockfd, buf2, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) { 
            perror("recvfrom");
            exit(1);
	}
        
        recTime = clock();
        
        EstimatedRTT = ((double) (recTime - senTime)) / CLOCKS_PER_SEC;
        printf("Round trip time taken is: %f seconds\n", EstimatedRTT);
        EstimatedRTT = 2;
	
        printf("deliver: got packet from %s\n", inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));
	printf("deliver: packet is %d bytes long\n", numbytes);
	buf2[numbytes] = '\0';
	printf("deliver: packet contains \"%s\"\n", buf2);
        
        // If reply is yes, then say A file transfer can start
        
        if(strcmp(buf2,"yes") == 0) {
            printf("deliver: A file transfer can start.\n");
   
        } 
        else {
            printf("deliver: No reply received from server.\n");
            exit(1);
        }

  //Lab 2 code




  FILE *file;
  file = fopen(filename,"r");
  fseek(file, 0, SEEK_END);
  unsigned int total_frag = ftell(file)/1000 + 1;
  rewind(file);

  bool resendPacket = 0;

  //fix later when in loop
  char buf3[PACKETLENGTH+100];
  



  sampleTaken = false;
  TimeoutInterval = EstimatedRTT;
  for (int i = 1; i <= total_frag; i++)
  {
    
    retry:
    sampleStart = clock();
    sendPacket(file, filename, buf3, total_frag,i, sockfd, p, &resendPacket);
    char buf4[MAXBUFLEN];
           //listen for server ACK PACKET reply
     printf("deliver: waiting to recvfrom...\n");

     //Lab3 

     //float timeout = rtt*200000;
     if (sampleTaken){
         alpha = 0.125;
         beta = 0.25;
        EstimatedRTT = (1 - alpha)*EstimatedRTT + alpha*SampleRTT;   
        float difference;
        if (SampleRTT - EstimatedRTT < 0){
            difference = -1*(SampleRTT - EstimatedRTT);
        }else{
            difference = SampleRTT - EstimatedRTT;
        }
     
        DevRTT = (1 - beta) * DevRTT + beta * difference;
        TimeoutInterval = EstimatedRTT + 4 * DevRTT;
     }
    
    
     printf("TimeoutInterval is %f\n", TimeoutInterval);
     printf("EstimatedRTT is %f\n", EstimatedRTT);
        if ((numbytes = recvtimeout(sockfd, &buf4, MAXBUFLEN-1 ,(struct sockaddr *)&their_addr, &addr_len,TimeoutInterval)) == -1){ 
            perror("recvfrom");
            exit(1);
        }
        else if (numbytes == -2) {
        printf("deliver: timeout resend packet %d\n",i);
        resendPacket = 1;
        sampleTaken = false;
        goto retry;
        }
        else {resendPacket = 0;
            sampleTaken = true;
            sampleEnd = clock();
            SampleRTT = ((double) (sampleEnd - sampleStart)) / CLOCKS_PER_SEC;
        }

    if (atoi(buf4) == i) {printf("deliver: ACK packet %d received \n", i);
    
    }
    

  }
  
       
  close(sockfd);

  printf("file transfer complete \n");
 

	return 0;
}