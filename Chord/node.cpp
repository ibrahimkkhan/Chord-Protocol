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


// class Node {

//   public:
//     Node* successor;
//     Node* predecessor;
//     struct sockaddr_in my_iport; //self ip and port, used in create to make new ring
//     struct sockaddr_in peer_iport; //Chord client will join this node’s ring
    
//     int t_stb; //time between invocations of ‘stabilize’
//     int t_fix_fing; //time between invocations of 'fix fingers'
//     int t_chck_pred; //time between invocations of ‘check predecessor'
//     int r_succ; //number of successors maintained by the Chord client
//     int id_opt;
//     int sock_fd;
    
//     Node(struct sockaddr_in m_port, struct sockaddr_in p_port, int ts, int tff, int t_cp, int r_s, int id_o){
//       my_iport.sin_port = htons(m_port.sin_port);
//       my_iport.sin_addr.s_addr = INADDR_ANY;
//       my_iport.sin_family = AF_INET;

//       peer_iport.sin_port = htons(p_port.sin_port);
//       peer_iport.sin_family = AF_INET;

//       t_stb = ts;
//       t_fix_fing = tff;
//       t_chck_pred = t_cp;
//       r_succ = r_s;
//       id_opt = id_o;
//     }

//     //connects node to port and returns file descriptor
//     void node_socket_initialize(){
//       if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
//         perror("socket creation failed");
//         exit(EXIT_FAILURE);
//       }else{
// 		    printf("Socket created\n");
// 	    }

//       if (bind(sock_fd, (const struct sockaddr *)&my_iport, sizeof(my_iport)) < 0 ){
//         perror("bind failed");
//         exit(EXIT_FAILURE);
//       }else{
//         printf("Server binded to Port %d and ADDR %s ..\n", my_iport.sin_port, inet_ntoa(my_iport.sin_addr));
//       }

//       if(listen(sock_fd, 4294967296)){
//         perror("listen failed");
//         exit(EXIT_FAILURE);
//       }else{
//         printf("Node is listening...");
//       }
//     }
// };