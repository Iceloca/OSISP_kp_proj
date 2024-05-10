//
// Created by iceloca on 7.05.24.
//

#ifndef SFML_CLIENT_H
#define SFML_CLIENT_H
#include "server_structures.h"
//Для доплнительного приложения менять PORT на 12346
#define PORT 12344
#define SERVER_PORT 12345
int initialize_client(int port);
void initialize_server(int port,const char* server_ip, struct sockaddr_in* server_addr);
void send_client_data(client_data_t data, int sockfd, struct sockaddr_in server_addr);
void receive_game_data(game_data_t * data, int sockfd, struct sockaddr_in server_addr);
void make_nonblock(int sockfd);
#endif //SFML_CLIENT_H
