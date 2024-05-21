//
// Created by iceloca on 28.03.24.
//

#ifndef STRUCTURES_H
#define STRUCTURES_H
#define MAX_BULLETS 128
struct  coordinate {
    float x;
    float y;
};

struct bulleta {
    struct coordinate coordinates;
    struct coordinate vector;
    char owner;
} typedef bullet_t;

struct entity {
    struct coordinate coordinates;
    int8_t hp;
    char type;
} typedef entity_t;

struct gamedata {
    entity_t boss;
    entity_t player1;
    entity_t player2;
    bullet_t bullets[MAX_BULLETS];
}typedef  gamedata_t;

struct clientdata {
    entity_t player;
    bullet_t bullet;
}typedef  clientdata_t;

#endif //STRUCTURES_H
