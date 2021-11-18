#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
//#include <argp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <unistd.h>

using std::cout;
using std::endl;

struct chord_arguments
{
  struct sockaddr_in my_iport; //self ip and port, used in create to make new ring
  struct sockaddr_in host_iport; //Chord client will join this node’s ring
  
  int ts; //time between invocations of ‘stabilize’
  int tff; //time between invocations of 'fix fingers'
  int tcp; //time between invocations of ‘check predecessor'
  int r_succ; //number of successors maintained by the Chord client
  int id_opt;

};


int main(int argc, char *argv[])
{

  chord_arguments client = client_parseopt(argc, argv);
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in serv_addr = client.ip_port;
}