#ifndef UNIT_H
#define UNIT_H

typedef enum{
    East,
    South,
    West,
    North
} Direction;

typedef enum{
    Best,
    Pmin,
    Threshold,
    Entropy,
    Mine
} Policy;

typedef struct{
    double x;
    double y;
} Location;

typedef struct{
    Location loc;
    Direction dir;
    int id;
    int base;
    int base_best;
    int base_pmin;
} Car;

typedef struct NODE{
    Car car;
    struct NODE *next;
} List_cars;

#endif