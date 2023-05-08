/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include "helper.h"
#include "client.h"


#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void handle_kill (int sig) {
  (void) sig;
  logout();
  exit(0);
  printf("killed");
}



void* recieve () {


  int numbytes;
  MESSAGE_PACKET new_message;

  while (running) { 

    if ((numbytes = recv(sockfd, &new_message, sizeof(MESSAGE_PACKET), 0)) == -1) {
      perror("recv");
      exit(1);}

    else if (numbytes == 0) {
        close(sockfd);
        continue;
    }

    if (new_message.type == MESSAGE) {
      printf("%s:%s \n",new_message.source,new_message.data);
    }
    if (new_message.type == P_ACK) {
      printf("private message from %s:%s \n",new_message.source,new_message.data);
    }
    else if (new_message.type == LO_ACK) {
      printf("client: login succesful \n");
    }
    else if (new_message.type == LO_NAK) {
      printf("client: login failed due to %s \n",new_message.data);
      //will have to login again
      close(sockfd);
    }
    else if (new_message.type == JN_ACK) {
      printf("client: joined session succesfully \n");
    }
    else if (new_message.type == JN_NAK) {
      printf("client: failed as already in session \n");
    }
    else if (new_message.type == BOOT_ACK) {
      printf("client: boot succesful \n");
    }
    else if (new_message.type == BOOT_NACK) {
      printf("client: boot failed \n");
    }
    else if (new_message.type == BOOT_WARN) {
      printf("client: you were booted from session \n");
    }
    else if (new_message.type == PROMOTE_ACK) {
      printf("client: promote was succesful \n");
    }
     else if (new_message.type == PROMOTE_NACK) {
      printf("client: promote failed \n");
    }
    else if (new_message.type == PROMOTE_WARN) {
      printf("client: you were promoted to admin \n");
    }
    else if (new_message.type == QU_ACK) {

      CLIENT client_array[CLIENT_ARRAY_LENGTH];
        
      if ((numbytes = recv(sockfd, &client_array, sizeof(client_array), 0)) == -1) {
        perror("recv");
        exit(1);
      }

      printf("client: recieved list \n");


      for (int i =0; i<CLIENT_ARRAY_LENGTH; i++) {
        //Print list
        if (*client_array[i].clientid != '\0') {
        printf("client: %s, session: %d, admin: %d, sockfd: %d \n",client_array[i].clientid,client_array[i].sessionid,client_array[i].admin,client_array[i].sockfd);
        }
      }
    }

  }
  return 0;
}


  

int login (char* clientid,char* password,char* serverIP, char* server_port) {

  //setup client information

  strncpy(client.clientid,clientid,MAX_NAME);
  strncpy(client.password, password,MAX_DATA);

  int numbytes;
	//char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(serverIP, server_port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if (((sockfd) = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect((sockfd), p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close((sockfd));
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

  





  //Prepare login message

  MESSAGE_PACKET message_login;
  message_login.type = LOGIN;
  message_login.size = strlen(password);
  strncpy((char*)message_login.source,clientid,MAX_NAME);
  strncpy((char*)message_login.data, password,MAX_DATA);


  //send login message
  if ((numbytes = sendto((sockfd), &message_login, sizeof(MESSAGE_PACKET), 0,
			 p->ai_addr, p->ai_addrlen)) == -1) {
		perror("client: sendto");
		exit(1);
	}

  printf("client: attempted to login \n");
  //printf("sockid %d \n",sockfd);

   //create recieve thread

  
  pthread_t recieve_thread;
  //globalserverIP = serverIP;
  //globalserver_port =server_port;
  pthread_create(&recieve_thread, NULL, recieve, NULL);


	freeaddrinfo(servinfo); // all done with this structure

  /*

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}

	buf[numbytes] = '\0';

	printf("client: received '%s'\n",buf);

  */


  return 0;

}

int create_session (char* sessionid) {

  MESSAGE_PACKET message_session;
  message_session.type = NEW_SESS;
  message_session.size = strlen(sessionid);
  strncpy((char*)message_session.source,client.clientid,MAX_NAME);
  strncpy((char*)message_session.data,sessionid,MAX_DATA);
  int numbytes;

  //send login message
  if ((numbytes = send((sockfd), &message_session, sizeof(MESSAGE_PACKET), 0)) == -1) {
		perror("client: sendto");
		exit(1);
	}

  printf("client: Asked for new session with id %s\n",sessionid);

  return 0;

}

int join_session (char* sessionid) {

  MESSAGE_PACKET message_session;
  message_session.type = JOIN;
  message_session.size = strlen(sessionid);
  strncpy((char*)message_session.source,client.clientid,MAX_NAME);
  strncpy((char*)message_session.data,sessionid,MAX_DATA);
  int numbytes;

  //send login message
  if ((numbytes = send((sockfd), &message_session, sizeof(MESSAGE_PACKET), 0)) == -1) {
		perror("client: sendto");
		exit(1);
	}

  printf("client: Asked to join new session with id %s\n",sessionid);

  return 0;

}

int leave_session () {

  MESSAGE_PACKET message_session;
  message_session.type = LEAVE_SESS;
  strncpy((char*)message_session.source,client.clientid,MAX_NAME);

  int numbytes;
  //send login message
  if ((numbytes = send((sockfd), &message_session, sizeof(MESSAGE_PACKET), 0)) == -1) {
		perror("client: sendto");
		exit(1);
	}

  printf("client: leaving session \n");

  return 0;

}


int list () {
  MESSAGE_PACKET message_list;
  message_list.type = QUERY;
  message_list.size = 0;
  strncpy((char*)message_list.source,client.clientid,MAX_NAME);

  int numbytes;

  //send login message
  if ((numbytes = send((sockfd), &message_list, sizeof(MESSAGE_PACKET), 0)) == -1) {
		perror("client: sendto");
		exit(1);
	}
  printf("client: asked for list \n");



  /*
  if ((numbytes = recv(sockfd, &client_array, sizeof(client_array), 0)) == -1) {
    perror("recv");
    exit(1);
  }

  printf("client: recieved list \n");

  for (int i =0; i<CLIENT_ARRAY_LENGTH; i++) {
    //Print list
    if (*client_array[i].clientid != '\0') {
    printf("client: %s, session: %d, sockfd: %d \n",client_array[i].clientid,client_array[i].sessionid,client_array[i].sockfd);
    }
  }

  */
  return 0;


}

int logout() {
  MESSAGE_PACKET message_logout;
  message_logout.type = EXIT;
  strncpy((char*) message_logout.source,client.clientid,MAX_NAME);

  int numbytes;

  if ((numbytes = send((sockfd), &message_logout, sizeof(MESSAGE_PACKET), 0)) == -1) {
		perror("client: sendto");
		exit(1);
	}

  close((sockfd));
  return 0;
} 

int message(char* line) {
  MESSAGE_PACKET message_text;
  message_text.type = MESSAGE;
  message_text.size = strlen(line);
  strncpy((char*) message_text.source,client.clientid,MAX_NAME);
  strncpy((char*) message_text.data,line,MAX_DATA);
  int numbytes;

  //printf("sockfd %d, \n" ,sockfd);

  //send login message
  if ((numbytes = send((sockfd), &message_text, sizeof(MESSAGE_PACKET), 0)) == -1) {
		perror("client: sendto");
		exit(1);
	}

  return 0;
} 

int private(char* line, char* recipient) {
  MESSAGE_PACKET message_private;
  message_private.type = PRIVATE;
  //strncpy((char*) message_private.size,recipient,MAX_NAME);
  strncpy((char*) message_private.source,recipient,MAX_NAME);

  //remove beginning of line 
  char *pos = strstr(line, recipient);
  if (pos != NULL) {
      memmove(line, pos + strlen(recipient), strlen(pos + strlen(recipient)) + 1);}

  /*
  if (*(line-1) == ' ') {
      memmove(line-1, line, strlen(line) + 1);
  }
  */

  //printf("%s \n",line );
  //printf("%s \n",recipient);

  strncpy((char*) message_private.data,line,MAX_DATA);
  int numbytes;

  //send private message
  if ((numbytes = send((sockfd), &message_private, sizeof(MESSAGE_PACKET), 0)) == -1) {
		perror("client: sendto");
		exit(1);
	}
  return 0;
} 

int boot (char* name) {

  MESSAGE_PACKET message_boot;
  message_boot.type = BOOT;
  strncpy((char*)message_boot.source,client.clientid,MAX_NAME);
  strncpy((char*)message_boot.data,name,MAX_DATA);
  int numbytes;

  //send boot message
  if ((numbytes = send((sockfd), &message_boot, sizeof(MESSAGE_PACKET), 0)) == -1) {
		perror("client: sendto");
		exit(1);
	}

  printf("client: Asked to boot %s\n",name);

  return 0;

}

int promote (char* name) {

  MESSAGE_PACKET message_promote;
  message_promote.type = PROMOTE;
  strncpy((char*)message_promote.source,client.clientid,MAX_NAME);
  strncpy((char*)message_promote.data,name,MAX_DATA);
  int numbytes;

  //send boot message
  if ((numbytes = send((sockfd), &message_promote, sizeof(MESSAGE_PACKET), 0)) == -1) {
		perror("client: sendto");
		exit(1);
	}

  printf("client: Asked to promote %s\n",name);

  return 0;

}





int main() {

  struct sigaction sa;
  sa.sa_handler = handle_kill;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  if (sigaction(SIGINT,&sa,NULL) == -1) {
    perror("cannot handle");
    exit(1);
  }

  //signal(SIGINT,handle_kill);
  printf("Enter a line of text: \n");

  char line [MAX_LINE_LENGTH];
  char cleanline [MAX_LINE_LENGTH];

  running = 1;
  
  while (running) {

  char** words;

  fgets(line, MAX_LINE_LENGTH -1, stdin);
  line[strcspn(line, "\n")] = 0; // Remove newline character from end of line

  strcpy(cleanline,line);
  words = split_input(line);

  //Isolate cmd
  char* cmd = words[0];

  if(strcmp(cmd,"login") == 0) {
    login (words[1],words[2],words[3],words[4]);
  }
  else if(strcmp(cmd,"logout") == 0) {
    logout();
  }
  else if(strcmp(cmd,"createsession") == 0) {
    create_session(words[1]);
  }
  else if(strcmp(cmd,"joinsession") == 0) {
    join_session(words[1]);
  }
  else if(strcmp(cmd,"leavesession") == 0) {
    leave_session();
  }
  else if(strcmp(cmd,"list") == 0) {
    list();
  }
  else if(strcmp(cmd,"exit") == 0) {
    exit(1);
  }
  else if(strcmp(cmd,"private") == 0) {
    private(cleanline,words[1]);
  }
  else if(strcmp(cmd,"boot") == 0) {
    boot(words[1]);
  }
  else if(strcmp(cmd,"boot") == 0) {
    boot(words[1]);
  }
  else if(strcmp(cmd,"promote") == 0) {
    promote(words[1]);
  }
  else {
    message(cleanline);
  }


  free(words);
  }

  /*
  // Print the array of words
  int i;
  for (i = 0; words[i] != NULL; i++) {
      printf("%s\n", words[i]);
  }
  */


	return 0;
}
