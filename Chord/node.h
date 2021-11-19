// // #include <netinet/in.h>
// #include <sys/socket.h>

// namespace N{
//     class Node{

//         public:
//             Node* successor;
//             Node* predecessor;
//             struct sockaddr_in my_iport; //self ip and port, used in create to make new ring
//             struct sockaddr_in peer_iport; //Chord client will join this node’s ring
//             int t_stb; //time between invocations of ‘stabilize’
//             int t_fix_fing; //time between invocations of 'fix fingers'
//             int t_chck_pred; //time between invocations of ‘check predecessor'
//             int r_succ; //number of successors maintained by the Chord client
//             int id_opt;
//             int sock_fd;
//             Node(struct sockaddr_in m_port, struct sockaddr_in p_port, int ts, int tff, int t_cp, int r_s, int id_o);
//             void node_socket_initialize();
//     };
// }