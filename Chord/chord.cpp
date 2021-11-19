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
// #include "node.h" // header in local directory

// using namespace N;
using std::cout;
using std::endl;

int check = 0;
struct chord_arguments
{
  struct sockaddr_in my_iport; //self ip and port, used in create to make new ring
  struct sockaddr_in peer_iport; //Chord client will join this node’s ring
  
  int ts; //time between invocations of ‘stabilize’
  int tff; //time between invocations of 'fix fingers'
  int tcp; //time between invocations of ‘check predecessor'
  int r_succ; //number of successors maintained by the Chord client
  int id_opt;

};

class Node {

  public:
    Node* successor;
    Node* predecessor;
    struct sockaddr_in my_iport; //self ip and port, used in create to make new ring
    struct sockaddr_in peer_iport; //Chord client will join this node’s ring
    
    int t_stb; //time between invocations of ‘stabilize’
    int t_fix_fing; //time between invocations of 'fix fingers'
    int t_chck_pred; //time between invocations of ‘check predecessor'
    int r_succ; //number of successors maintained by the Chord client
    int id_opt;
    int sock_fd;
    
    Node(struct sockaddr_in m_port, struct sockaddr_in p_port, int ts, int tff, int t_cp, int r_s, int id_o){
      my_iport.sin_port = htons(m_port.sin_port);
      my_iport.sin_addr.s_addr = INADDR_ANY;
      my_iport.sin_family = AF_INET;

      peer_iport.sin_port = htons(p_port.sin_port);
      peer_iport.sin_family = AF_INET;

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

  case 'ja':
        check++;
        inet_pton(AF_INET, arg, &(args->peer_iport.sin_addr));
    break;

  case 'jp':
        check++;
        args->peer_iport.sin_port = atoi(arg);
        if (args->peer_iport.sin_port <= 1024)
        {
            argp_error(state, "Invalid option for a port");
        } 
    break;

  case 'ts':
    args->ts = atoi(arg);
    if(args->ts < 0)
    {
      argp_error(state,"Negative number for time stabalize");
    }
    break;
  case 'tff':
    args->tff = atoi(arg);
    if(args->tff < 0)
    {
      argp_error(state,"Negative number for time fix finger");
    }
    break;
  case 'tcp':
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
      {"jaddr", 'ja', "jaddr", 0, "The IP address of the peer whose ring is joined ", 0},
      {"jport", 'jp', "jport", 0, "The port of the peer whose ring is joined" , 0},

      {"ts", 'ts', "ts", 0, "time between invocations of stabilize", 0},
      {"tff", 'tff', "tff", 0, "time between invocations of fix fingers", 0},
      {"tcp", 'tcp', "tcp", 0, "time between invocations of check predecessor", 0},
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

int main(int argc, char *argv[])
{

  // chord_arguments client = client_parseopt(argc, argv);
  // int sock = socket(AF_INET, SOCK_DGRAM, 0);
  // struct sockaddr_in serv_addr = client.ip_port;
  int port = 4000;
  struct sockaddr_in servaddr;
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(port);

  Node node(servaddr, servaddr, 0, 0, 0, 0, 0);
  chord_arguments chord = chord_parseopt(argc, argv);
  printf("Got %s port %d with ts=%d tff=%d tcp=%d r=%d \n", inet_ntoa(chord.my_iport.sin_addr),chord.my_iport.sin_port, chord.ts, chord.tff , chord.tcp, chord.r_succ);
  printf("Got %s port %d for peer \n", inet_ntoa(chord.peer_iport.sin_addr),chord.peer_iport.sin_port);

  if(check == 0)
    {
        printf("create ring \n");
        //Create a new ring
    }
    else if (check == 2){
        printf("join ring \n");
        //join existing ring
    }

}

void create_ring(Node *node){
  return;
}
  

