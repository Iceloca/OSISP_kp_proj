#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>
#include "structures.h"
#define PORT 12345
#define BUFFER_SIZE 1024
#define BORDER_MAX_SIZE 1028

gamedata_t initialise(void) {
    gamedata_t gamedata = {0};
    gamedata.boss.type = 'b';
    gamedata.boss.coordinates = (struct  coordinate){256,0};
    gamedata.player1.type = '1';
    gamedata.player1.coordinates = (struct coordinate){0,0};
    gamedata.player2.type = '1';
    gamedata.player2.coordinates = (struct coordinate){0,0};
    return  gamedata;
}

int bullet_empty(bullet_t bullet) {
    return  bullet.owner == 0;
}
int check_hit(bullet_t bullet, entity_t entity, int radius) {
    float distanse = 0;
    int dif_x = bullet.coordinates.x - entity.coordinates.x;
    int dif_y = bullet.coordinates.y - entity.coordinates.y;
    distanse = sqrt(pow(dif_x,2) + pow(dif_y,2));
    return distanse < radius;
}
int process_bullet_hit(entity_t* entity, bullet_t bullet) {
    if ((entity->hp>0) && (bullet.owner != entity->type) && (check_hit(bullet,* entity,   15))) {
        entity->hp--;
        return 1;
    }
    return 0;
}
int move_bullet(bullet_t* bullet) {
    bullet->coordinates.x += bullet->vector.x;
    bullet->coordinates.y += bullet->vector.y;
    if (bullet->coordinates.x < 0 || bullet->coordinates.x < BORDER_MAX_SIZE )
        return 1;
    if (bullet->coordinates.y < 0 || bullet->coordinates.y < BORDER_MAX_SIZE )
        return 1;
    return  0;
}
void process_bullets(gamedata_t* gamedata) {
    int step = 0;
    int i ;
    for(i = 0; (i + step < MAX_BULLETS) && bullet_empty(gamedata->bullets[i + step]); i ++) {
        gamedata->bullets[i] = gamedata->bullets[i+step];
        if (gamedata->bullets[i].owner != 'b') {
            step += process_bullet_hit(&gamedata->boss, gamedata->bullets[i]);
            i--;
        }
        else if(process_bullet_hit(&gamedata->player1,gamedata->bullets[i]) ||
                   process_bullet_hit(&gamedata->player2,gamedata->bullets[i]) ||
                        move_bullet(&gamedata->bullets[i])){
                step++;
                i--;
            }
    }
    for(i = 0; (i < MAX_BULLETS) && bullet_empty(gamedata->bullets[i]); i ++)
        gamedata->bullets[i] = (bullet_t){0};
}

void push_bullet(bullet_t* bullets,bullet_t bullet) {
    int i = 0;
    while(!bullet_empty(bullets[i]))
        i++;
    if(i < MAX_BULLETS)
        bullets[i] = bullet;
}

void procces_client_data (gamedata_t* gamedata, clientdata_t clientdata) {
    if (clientdata.player.type == '1')
        gamedata->player1 = clientdata.player;
    if (clientdata.player.type == '2')
        gamedata->player2 = clientdata.player;
    if(!bullet_empty(clientdata.bullet))
        push_bullet(gamedata->bullets,clientdata.bullet);
}

// Функция обработки снарядов


int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    gamedata_t gamedata;
    clientdata_t clientdata;

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket to port
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Binding failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);
    while(1) {
        // Receive number from client
        if (recvfrom(sockfd, &clientdata, sizeof(clientdata), 0, (struct sockaddr *)&client_addr, &client_addr_len) == -1) {
            perror("Receive error");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        int success_signal = 0;
        if (sendto(sockfd, &success_signal, sizeof(success_signal), 0, (struct sockaddr *)&client_addr, client_addr_len) == -1) {
            perror("Sendto failed");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        procces_client_data(&gamedata,clientdata);
        process_bullets(&gamedata);
    }
    // Close socket
    close(sockfd);

    return 0;
}
