#include <stdio.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include "hash.h"
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
#include <string>
#include <unistd.h>
#include "./chord.pb.h"

using namespace protocol;
using namespace std;
using std::cout;
using std::endl;

struct call_args
{
    uint ip_address[16];
    int port;
    int timereqs;
    int timout;
} __attribute__((packed));

int check = 0;
struct chord_arguments
{
    struct sockaddr_in my_iport;   //self ip and port, used in create to make new ring
    struct sockaddr_in peer_iport; //Chord client will join this node’s ring

    int ts;     //time between invocations of ‘stabilize’
    int tff;    //time between invocations of 'fix fingers'
    int tcp;    //time between invocations of ‘check predecessor'
    int r_succ; //number of successors maintained by the Chord client
    int id_opt;
};

void print_data(uint8_t *data, int size)
{
    int i;

    for (i = 0; i < size; i++)
    {
        printf("%02x ", data[i]);
    }

    cout << "\nPRINTED" << endl;
}

// class Function{

// }

class Local_Node
{

public:
    Local_Node *successor;
    Local_Node *predecessor;
    struct sockaddr_in my_iport;   //self ip and port, used in create to make new ring
    struct sockaddr_in peer_iport; //Chord client will join this node’s ring

    int t_stb;       //time between invocations of ‘stabilize’
    int t_fix_fing;  //time between invocations of 'fix fingers'
    int t_chck_pred; //time between invocations of ‘check predecessor'
    int r_succ;      //number of successors maintained by the Chord client
    int id_opt;
    int sock_fd;

    Local_Node(struct sockaddr_in m_port, struct sockaddr_in p_port, int ts, int tff, int t_cp, int r_s, int id_o)
    {
        my_iport.sin_port = htons(m_port.sin_port);
        my_iport.sin_addr = m_port.sin_addr;
        my_iport.sin_family = AF_INET;

        // printf(" Binded to Port %d and ADDR %s ..\n", ntohs(my_iport.sin_port), inet_ntoa(my_iport.sin_addr));

        peer_iport.sin_port = htons(p_port.sin_port);
        peer_iport.sin_addr = p_port.sin_addr;
        peer_iport.sin_family = AF_INET;

        t_stb = ts;
        t_fix_fing = tff;
        t_chck_pred = t_cp;
        r_succ = r_s;
        id_opt = id_o;
    }

    //connects node to port and returns file descriptor
    //
    void node_socket_initialize()
    {
        if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }
        else
        {
            printf("Socket created\n");
        }

        if (bind(sock_fd, (const struct sockaddr *)&my_iport, sizeof(my_iport)) < 0)
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }
        else
        {
            printf(" Binded to Port %d and ADDR %s ..\n", ntohs(my_iport.sin_port), inet_ntoa(my_iport.sin_addr));
        }
    }

    void send_call(string id, int sizee)
    {

        // convert()
        //convert string to unint_8 byte array of 40 bytes

        //! ARGS
        int port = ntohs(peer_iport.sin_port);
        // uint8_t buffer[40];
        protocol::FindSuccessorArgs successor_args = FindSuccessorArgs();

        successor_args.set_id(&id[0], id.length());
        cout << successor_args.DebugString() << endl;

        int size = successor_args.ByteSize();

        char successor_args_bytes[size + 1];
        //string serialized_bytes;

        successor_args_bytes[0] = (char)id.length();
        // uint8_t ptr_s_a_byes = &successor_args_bytes[0];
        // int num_bytes = sizeof(id);
        cout << "SIZE " << size << endl;
        successor_args.SerializeToArray(&successor_args_bytes[1], size);

        string call_name = "find_successor";
        // uint8_t buffer[call_name.length() + size + 1];

        //!packet call

        protocol::Call call = Call();
        call.set_name(&call_name[0]);
        call.set_args(&successor_args_bytes[0]);

        int call_size = call.ByteSize();
        cout << "call size is " << call_size << endl;
        cout << "call size is" << (char)call_size << endl;

        char final_buffer[call_size + 1];
        final_buffer[0] = (char)call_size;
        call.SerializeToArray(&(final_buffer[1]), call_size);

        sendto(sock_fd, &(final_buffer[0]), sizeof(final_buffer), 0, (struct sockaddr *)&peer_iport, sizeof(peer_iport));
    }

    void recv_call()
    {
        Call call = Call();
        int port = ntohs(peer_iport.sin_port);
        // char buffer[sizeof(call)];
        // char *ptr = &(buffer[0]);

        char call_size[500];
        recvfrom(sock_fd, &(call_size[0]), 500, 0, NULL, 0);

        cout << "call size is" << (int)call_size[0] << endl;
        cout << "call size is" << call_size[0] << endl;

        int bytes_to_read = (int)call_size[0];

        // char buffer[(int)call_size[0]];

        char *ptr = &(call_size[1]);
        // recvfrom(sock_fd, ptr, sizeof(buffer), 0,  NULL, 0);

        call.ParsePartialFromArray(ptr, bytes_to_read);

        cout << "Debug " << call.DebugString() << endl;

        cout << "name " << call.name() << endl;

        protocol::FindSuccessorArgs successor_args = FindSuccessorArgs();

        string parsed_args = call.args();

        int arg_size = (int)parsed_args[0];

        successor_args.ParseFromString(&parsed_args[1]);

        cout << "ARGUMENT SIZE" << arg_size << endl;

        cout << "id is " << successor_args.id() << endl;
    }
};
uint8_t *hash_find(const char *lookup)
{
    uint8_t checksum[40];
    uint8_t *ret;

    struct sha1sum_ctx *ctx = sha1sum_create(NULL, 0);
    if (!ctx)
    {
        fprintf(stderr, "Error creating checksum\n");
        return 0;
    }

    int error = sha1sum_finish(ctx, (const uint8_t *)lookup, strlen(lookup), checksum);
    if (!error)
    {
        printf("%s ", lookup);
        for (size_t i = 0; i < 20; ++i)
        {
            printf("%02x", checksum[i]);
        }
        putchar('\n');
    }

    sha1sum_reset(ctx);
    sha1sum_destroy(ctx);

    ret = &checksum[0];
    return ret;
};
uint8_t *create_id(sockaddr_in ipp)
{

    struct in_addr ip = ipp.sin_addr;
    string ip_s = inet_ntoa(ip);
    string str = ip_s + std::to_string(ipp.sin_port);

    const char *c = str.c_str();
    uint8_t *id = hash_find(c);
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
        if (args->ts < 0)
        {
            argp_error(state, "Negative number for time stabalize");
        }
        break;
    case 7628390:
        args->tff = atoi(arg);
        if (args->tff < 0)
        {
            argp_error(state, "Negative number for time fix finger");
        }
        break;
    case 7627632:
        args->tcp = atoi(arg);
        if (args->tcp < 0)
        {
            argp_error(state, "Negative number for time check predecessor");
        }
        break;
    case 'r':
        args->r_succ = atoi(arg);
        if (args->r_succ < 0)
        {
            argp_error(state, "Negative number for no. of successors");
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
        {"jport", 27248, "jport", 0, "The port of the peer whose ring is joined", 0},

        {"ts", 29811, "ts", 0, "time between invocations of stabilize", 0},
        {"tff", 7628390, "tff", 0, "time between invocations of fix fingers", 0},
        {"tcp", 7627632, "tcp", 0, "time between invocations of check predecessor", 0},
        {"r", 'r', "r", 0, "number of successors maintained", 0},
        {"i", 'i', "i", 0, "the identifier (ID) assigned to the Chord client", 0},
        {0}};

    struct argp argp_settings = {options, chord_parser, 0, 0, 0, 0, 0};
    struct chord_arguments args;

    bzero(&args, sizeof(args));

    if (argp_parse(&argp_settings, argc, argv, 0, NULL, &args) != 0)
    {
        perror("Got error in parse\n");
        exit(0);
    }
    else
    {
        if (check % 2 == 1)
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
    //   int port = 4000;
    //   struct sockaddr_in servaddr;
    //   servaddr.sin_family = AF_INET;
    //   servaddr.sin_addr.s_addr = INADDR_ANY;
    //   servaddr.sin_port = htons(port);

    GOOGLE_PROTOBUF_VERIFY_VERSION;

    chord_arguments chord = chord_parseopt(argc, argv);
    printf("Got %s port %d with ts=%d tff=%d tcp=%d r=%d \n", inet_ntoa(chord.my_iport.sin_addr), chord.my_iport.sin_port, chord.ts, chord.tff, chord.tcp, chord.r_succ);
    printf("Got %s port %d for peer \n", inet_ntoa(chord.peer_iport.sin_addr), chord.peer_iport.sin_port);

    Local_Node node(chord.my_iport, chord.peer_iport, chord.ts, chord.tff, chord.tcp, chord.r_succ, chord.id_opt);
    if (check == 0)
    {
        node.node_socket_initialize();
        //! Nistha to add hashing here
        //!

        //Create a new ring
    }
    else if (check == 2)
    {
        node.node_socket_initialize();
        printf("join ring \n");
        //join existing ring
    }

    string input;
    while (1)
    {
        cout << "send or recv: ";
        getline(cin, input);

        if (input.compare("send") == 0)
        {
            string myString = "hagatwg8836";
            // std::vector<uint8_t> myVector(myString.begin(), myString.end());
            // uint8_t *p = &myVector[0];
            node.send_call(&myString[0], 40);
        }
        else if (input.compare("recv") == 0)
        {
            node.recv_call();
        }
    }
}

// void create_ring()
// {
//     return;
// }
