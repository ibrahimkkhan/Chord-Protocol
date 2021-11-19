#include "hash.h"
#include <openssl/evp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <argp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <unistd.h>
#include <string>
// #include "node.h" // header in local directory

// using namespace N;
using std::cout;
using std::endl;
using std::string;

int check = 0;
struct chord_arguments
{
  struct sockaddr_in my_iport; //self ip and port, used in create to make new ring
  struct sockaddr_in peer_iport; //Chord client will join this node’s ring
  
  int ts; //time between invocations of ‘stabilize’
  int tff; //time between invocations of 'fix fingers'
  int tcp; //time between invocations of ‘check predecessor'
  int r_succ; //number of successors maintained by the Chord client
  string id_opt;

};

class Local_Node {

  public:
    Local_Node* successor;
    Local_Node* predecessor;
    struct sockaddr_in my_iport; //self ip and port, used in create to make new ring
    struct sockaddr_in peer_iport; //Chord client will join this node’s ring
    
    int t_stb; //time between invocations of ‘stabilize’
    int t_fix_fing; //time between invocations of 'fix fingers'
    int t_chck_pred; //time between invocations of ‘check predecessor'
    int r_succ; //number of successors maintained by the Chord client
    string id_opt;
    int sock_fd;
    // string node_id;
    
    Local_Node(struct sockaddr_in m_port, struct sockaddr_in p_port, int ts, int tff, int t_cp, int r_s, string id_o){
      my_iport.sin_port = htons(m_port.sin_port);
      my_iport.sin_addr.s_addr = INADDR_ANY;
      my_iport.sin_family = AF_INET;

      peer_iport.sin_port = htons(p_port.sin_port);
      peer_iport.sin_family = AF_INET;
      predecessor = this;
      successor = this;
      t_stb = ts;
      t_fix_fing = tff;
      t_chck_pred = t_cp;
      r_succ = r_s;
      id_opt = id_o;
    }

    //connects node to port and returns file descriptor
    void node_socket_initialize(){
      if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
      }else{
		    printf("Socket created\n");
        node->sock_fd = sock_fd;
	    }

      if (bind(sock_fd, (const struct sockaddr *)&my_iport, sizeof(my_iport)) < 0 ){
        perror("bind failed");
        exit(EXIT_FAILURE);
      }else{
        printf("Server binded to Port %d and ADDR %s ..\n", my_iport.sin_port, inet_ntoa(my_iport.sin_addr));
      }

      if(listen(sock_fd, INT_MAX)){
        perror("listen failed");
        exit(EXIT_FAILURE);
      }else{
        printf("Node is listening...");
      }
    }
};

Local_Node *node;

uint8_t *hash(const char *lookup) {
	uint8_t checksum[40];
	uint8_t *ret;

  struct sha1sum_ctx *ctx = sha1sum_create(NULL, 0);
	if (!ctx) {
		fprintf(stderr, "Error creating checksum\n");
		return 0;
	}


	int error = sha1sum_finish(ctx, (const uint8_t*)lookup, strlen(lookup), checksum);
	if (!error) {
		printf("%s ", lookup);
		for(size_t i = 0; i < 20; ++i) {
			printf("%02x", checksum[i]);
		}
		putchar('\n');
	}


	sha1sum_reset(ctx);
    sha1sum_destroy(ctx);

    ret = &checksum[0];
	return ret;
}

uint8_t *create_id(sockaddr_in ipp){

  struct in_addr ip = ipp.sin_addr;
  string ip_s = inet_ntoa(ip);
  string str = ip_s + std::to_string(ipp.sin_port);

  const char * c = str.c_str();
  uint8_t *id = hash(c);
  return id;
};


error_t chord_parser(int key, char *arg, struct argp_state *state)
{
  struct chord_arguments *args = (chord_arguments *)state->input;
  error_t ret = 0;
  switch (key)
  {
  case 'a':
    if (arg == NULL)
        perror("IP address not given");
    else
        inet_pton(AF_INET, arg, &(args->my_iport.sin_addr));
    break;
  case 'p':
    args->my_iport.sin_port = atoi(arg);
    if (args->my_iport.sin_port <= 1024)
    {
      argp_error(state, "Invalid option for a port");
    }
    break;

  case 27233:
        check++;
        inet_pton(AF_INET, arg, &(args->peer_iport.sin_addr));
    break;

  case 27248:
        check++;
        args->peer_iport.sin_port = atoi(arg);
        if (args->peer_iport.sin_port <= 1024)
        {
            argp_error(state, "Invalid option for a port");
        } 
    break;

  case 29811:
    args->ts = atoi(arg);
    if(args->ts < 0)
    {
      argp_error(state,"Negative number for time stabalize");
    }
    break;
  case 7628390:
    args->tff = atoi(arg);
    if(args->tff < 0)
    {
      argp_error(state,"Negative number for time fix finger");
    }
    break;
  case 7627632:
    args->tcp = atoi(arg);
    if(args->tcp < 0)
    {
      argp_error(state,"Negative number for time check predecessor");
    }
    break;
  case 'r':
    args->r_succ = atoi(arg);
    if(args->r_succ < 0)
    {
      argp_error(state,"Negative number for no. of successors");
    }
    break;
  default:
    ret = ARGP_ERR_UNKNOWN;
    break;
  }
  return ret;
}


chord_arguments chord_parseopt(int argc, char *argv[])
{

  //check all possible options and parse them

  struct argp_option options[] = {
      {"myaddr", 'a', "myaddr", 0, "The IP address of the current node ", 0},
      {"myport", 'p', "myport", 0, "The port of the current node ", 0},
      {"jaddr", 27233, "jaddr", 0, "The IP address of the peer whose ring is joined ", 0},
      {"jport", 27248, "jport", 0, "The port of the peer whose ring is joined" , 0},

      {"ts", 29811, "ts", 0, "time between invocations of stabilize", 0},
      {"tff", 7628390, "tff", 0, "time between invocations of fix fingers", 0},
      {"tcp", 7627632, "tcp", 0, "time between invocations of check predecessor", 0},
      {"r", 'r', "r", 0, "number of successors maintained", 0},
      { "i", 'i', "i", 0, "the identifier (ID) assigned to the Chord client", 0},
      {0}};

  struct argp argp_settings = {options, chord_parser, 0, 0, 0, 0, 0};
  struct chord_arguments args;

  bzero(&args, sizeof(args));

  if (argp_parse(&argp_settings, argc, argv, 0, NULL, &args) != 0)
  {
    perror("Got error in parse\n");
    exit(0);
  }
  else{
    if(check%2 == 1)
      {
        perror("Got error in parse\n");
        exit(0);
      }
  }

    return args;
}

void create (Local_Node *node){
  //check for ID
  if(node->id_opt.empty()){
    memcpy(&node->id_opt,create_id(node->my_iport),40);
  }

  //Successor points to self
  node->successor = node;

  //Predecessor point to null
  node->predecessor = NULL;
}

void notify(Local_Node *succ){
// if (predecessor of succ is nil or n′ ∈ (predecessor, n)) predecessor = n′;
if(succ->predecessor == NULL || node != succ->predecessor){
  succ->predecessor = node;
}
}


// successor, and tells the successor about n. 
void stabilize(){
Local_Node *temp = node->successor->predecessor;
if(temp != node)
{
  node->successor = temp;
}
notify(node->successor);
}


Local_Node *find_successor(sockaddr_in ipp, string id){

  //connect
  //ask for successors
  //recieve successors

  //close socket
  //return successor
}

void join (Local_Node *node){
  //findsuccessor in the new ring
  node->successor = find_successor(node->peer_iport, node->id_opt);
}


//if recieved process command
void processCommand(int clientSock, uint8_t command)
{
	 switch (command) {
		//join room
		case 'Lookup': 
			break;

	default:
		ARGP_ERR_UNKNOWN;
		break;
	}

}


int main(int argc, char *argv[])
{
    chord_arguments chord = chord_parseopt(argc, argv);
    printf("Got %s port %d with ts=%d tff=%d tcp=%d r=%d \n", inet_ntoa(chord.my_iport.sin_addr), chord.my_iport.sin_port, chord.ts, chord.tff, chord.tcp, chord.r_succ);
    printf("Got %s port %d for peer \n", inet_ntoa(chord.peer_iport.sin_addr), chord.peer_iport.sin_port);

    Local_Node new_node(chord.my_iport, chord.peer_iport, chord.ts, chord.tff, chord.tcp, chord.r_succ, chord.id_opt);
    node = &new_node;

    if (check == 0)
    {
        node->node_socket_initialize();
    
        //Create a new ring
        create (node);
    }
    else if (check == 2)
    {
        node->node_socket_initialize();
        printf("join ring \n");
        //join existing ring
    }

}
