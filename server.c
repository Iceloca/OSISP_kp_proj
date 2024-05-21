#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include "server_structures.h"
#define PORT 12345
#define BUFFER_SIZE 1024
#define BORDER_MAX_SIZE_Y 1028
#define BORDER_MAX_SIZE_X 1920
#define UPDATE_INTERVAL 6 // Интервал обновления
#define MAX_BOSS_SPEED 3.0 // Максимальная скорость босса
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

long get_current_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void move_boss(entity_t* boss) {
    static float angle = 0.0f;
    static float speed = 0.0f;
    static int delay_random = 0;
    static int update_interval = 0;
    static long last_rand_update_time = 0;
    // Получение текущего времени в миллисекундах
    long current_time = time(NULL);
    static long last_update_time_ms = 0;
    // Получение текущего времени в миллисекундах
    long current_time_ms = get_current_time_ms();
    // Обновление угла и скорости через определённые промежутки времени
    if (current_time - last_rand_update_time> update_interval) {
        srand(time(NULL));
        angle = ((float)rand() / RAND_MAX) * 2 * M_PI;
        speed = ((float)rand() / RAND_MAX) * MAX_BOSS_SPEED;
        update_interval = rand() % UPDATE_INTERVAL;
        printf("speed :%f \n", speed);
        printf("%ld \n", last_rand_update_time);
        last_rand_update_time = current_time;
    }
    // Вычисление новых координат босса
    if(current_time_ms - last_update_time_ms > 50) {
        boss->coordinates.x += speed * cos(angle);
        boss->coordinates.y += speed * sin(angle);
        last_update_time_ms = current_time_ms;
        // Ограничения для движения босса по границам поля
        if (boss->coordinates.x < 0) boss->coordinates.x = 0;
        if (boss->coordinates.x > BORDER_MAX_SIZE_X) boss->coordinates.x = BORDER_MAX_SIZE_X;
        if (boss->coordinates.y < 0) boss->coordinates.y = 0;
        if (boss->coordinates.y > BORDER_MAX_SIZE_Y) boss->coordinates.y = BORDER_MAX_SIZE_Y;
        printf("x: %f  y: %f \n", boss->coordinates.x, boss->coordinates.x);
    }
}


game_data_t initialise(void) {
    game_data_t gamedata = {0};
    gamedata.boss.type = 'b';
    gamedata.boss.coordinates = (struct  coordinate){256,256};
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
    if (bullet->coordinates.x < 0 || bullet->coordinates.x < BORDER_MAX_SIZE_X )
        return 1;
    if (bullet->coordinates.y < 0 || bullet->coordinates.y < BORDER_MAX_SIZE_Y )
        return 1;
    return  0;
}
void process_bullets(game_data_t* gamedata) {
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

void procces_client_data (game_data_t* gamedata, client_data_t clientdata) {
    if (clientdata.player.type == '1')
        gamedata->player1 = clientdata.player;
    if (clientdata.player.type == '2')
        gamedata->player2 = clientdata.player;
    if(!bullet_empty(clientdata.bullet))
        push_bullet(gamedata->bullets,clientdata.bullet);
}

// Функция обработки снарядов

int init_server_socket(){
    int sockfd;
    struct sockaddr_in server_addr;
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
    return sockfd;
}


void start_lobby(int sockfd,struct sockaddr_in* client_addr_1,struct sockaddr_in* client_addr_2) {
    struct sockaddr_in new_client_addr;
    socklen_t client_addr_len;
    char player = '0';
    char signal = 0;
    client_addr_len = sizeof (new_client_addr);
    do {
        if (recvfrom(sockfd, &signal, sizeof(signal), 0, (struct sockaddr *) &new_client_addr, &client_addr_len) ==
            -1) {
            perror("Receive error");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        printf("Signal :%c \n", signal);
        switch (signal) {
            case '0':
                break;
            case '1':
                if (player != '1' && player < '3') {
                    player += 1;

                }
                *client_addr_1 = new_client_addr;
                break;
            case '2':
                if (player != '2' && player < '3') {
                    player += 2;

                }
                *client_addr_2 = new_client_addr;
                break;
            case 'r':
                player = '0';
                break;
            default:
                break;
        }


        if (player == '3') {
            signal = 's';
            if (sendto(sockfd, &signal, sizeof(signal), 0, (struct sockaddr *) client_addr_1,
                       sizeof(*client_addr_1)) == -1) {
                perror("Sendto failed");
                printf("%d", errno);
                close(sockfd);
                exit(EXIT_FAILURE);
            }
            if (sendto(sockfd, &signal, sizeof(signal), 0, (struct sockaddr *) client_addr_2,
                       sizeof(*client_addr_2)) == -1) {
                perror("Sendto failed");
                printf("%d", errno);
                close(sockfd);
                exit(EXIT_FAILURE);
            }
            return;
        }else{
            if (sendto(sockfd, &player, sizeof(player), 0, (struct sockaddr *) &new_client_addr,
                       sizeof(new_client_addr)) == -1) {
                printf("%d", errno);
                perror("Sendto failed");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
            printf("Player :%c \n", player);
            signal = '0';
        }
    }while (signal != 's');

}

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr, client_addr_1, client_addr_2;
    socklen_t client_addr_len = sizeof(client_addr);
    game_data_t gamedata;
    client_data_t clientdata;
    size_t size = 0;
    sockfd = init_server_socket();
    gamedata = initialise();
    while (1){
        move_boss(&gamedata.boss);
    }
    start_lobby(sockfd, &client_addr_1, &client_addr_2);
    //if(fcntl(sockfd,F_SETFL, O_NONBLOCK) == -1)
    //    perror("NON_BLOCK error");
    printf("Server listening on port %d...\n", PORT);
    while(1) {
        // Receive number from client

        size = recvfrom(sockfd, &clientdata, sizeof(clientdata), 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (size == -1) {
            if( errno != EWOULDBLOCK) {
                perror("Receive error");
                //close(sockfd);
                //exit(EXIT_FAILURE);
            }
        }else if(size >0) {
            int success_signal = 0;
            if (sendto(sockfd, &success_signal, sizeof(success_signal), 0, (struct sockaddr *) &client_addr,
                       client_addr_len) == -1) {
                perror("Sendto failed check");
                printf("%d", errno);
                //exit(EXIT_FAILURE);
            }
        }
        procces_client_data(&gamedata,clientdata);
        process_bullets(&gamedata);
//        if (sendto(sockfd, &gamedata, sizeof(gamedata), 0, (struct sockaddr *) &client_addr, client_addr_len) ==
//            -1) {
//            perror("Sendto failed player 1");
//            printf("%d", errno);
//        }

        if (sendto(sockfd, &gamedata, sizeof(gamedata), 0, (struct sockaddr *) &client_addr_1, sizeof(client_addr_1)) ==
            -1) {
            perror("Sendto failed player 1");
            printf("%d", errno);
        }

        printf(" %d , %d \n", gamedata.player1.coordinates.x, gamedata.player1.coordinates.y);

        if (sendto(sockfd, &gamedata, sizeof(gamedata), 0, (struct sockaddr *) &client_addr_2,
                   sizeof(client_addr_2)) == -1) {
            perror("Sendto failed player 2");
            printf("%d", errno);
        }
        printf(" %d , %d \n", gamedata.player2.coordinates.x, gamedata.player2.coordinates.y);


    }

    // Close socket
    close(sockfd);

    return 0;
}
