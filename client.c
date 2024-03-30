#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "structures.h"
#define SERVER_IP "127.0.0.1"
#define PORT 12345

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    clientdata_t data = {0};
    int received_number;

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Get number from user
    printf("Enter a number to send to the server: ");
    scanf("%d", &data.player.coordinates.x);
    printf("Enter a number to send to the server: ");
    scanf("%d", &data.player.coordinates.y);

    do{
    // Send number to server
            received_number = - 1;
        if (sendto(sockfd, &data, sizeof(data), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
            perror("Sendto failed");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        if (recv(sockfd, &received_number, sizeof(received_number), 0) == -1) {
            perror("Receive error");
            close(sockfd);
            exit(EXIT_FAILURE);
        } else {
            printf("Received response from server: %d\n", received_number);
        }
    }while(received_number != 0 );
    printf("Number sent to server.\n");

    // Close socket
    close(sockfd);

    return 0;
}
