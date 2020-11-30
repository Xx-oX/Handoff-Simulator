#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "unit.h"

/*global variables*/
double arrival_rate = 0.5;
double velocity = 10;
double T;
double E;
int nHandoff = 0;
long long int total = 0;
long double total_power_max = 0;
long double total_power_min = 0;
long double total_power = 0;
int list_handoff[86400];

/*function declaration*/
int poisson(double, double);
double getDistanceToBase(Location, int);
double getPower(double);
int getMaxPowerBase(Location);
int getBase(Car*, Policy);
int updateCar(Car*, double);
void run(int, int, Policy);


/*poisson arrival generator*/
int poisson(double lambda, double delta)
{
    double r = (double) rand() / (RAND_MAX);
    return r < (1 - exp(-lambda * delta));
}

/*get distance to base station (in meter)*/
double getDistanceToBase(Location loc, int base)
{
    double bx, by;
    switch(base){
        case 1:
            bx = 330;
            by = 350;
            break;
        case 2:
            bx = 640;
            by = 310;
            break;
        case 3:
            bx = 360;
            by = 680;
            break;
        case 4:
            bx = 660;
            by = 658;
            break;
    }
    return sqrt(pow(loc.x-bx, 2) + pow(loc.y-by, 2));
}

/*get power by distance with Pt = 100dBm (in dBm)*/
double getPower(double distance)
{
    double Pt = 100;
    return Pt - 33 - 20 * log10(distance);
}

/*get base of any arriving car (first time)*/
int getMaxPowerBase(Location loc)
{
    double p[5];
    p[0] = 0;
    p[1] = getPower(getDistanceToBase(loc, 1));
    p[2] = getPower(getDistanceToBase(loc, 2));
    p[3] = getPower(getDistanceToBase(loc, 3));
    p[4] = getPower(getDistanceToBase(loc, 4));
    int I_max = 1;
    double P_max = p[1];
    for(int i=1; i<5; ++i){
        if(p[i] > P_max){
            P_max = p[i];
            I_max = i;
        }
    }
    return I_max;
}

/*update base of a car by policy*/
int getBase(Car *c, Policy policy)
{
    double P_min = 10;
    double p[5];
    p[0] = getPower(getDistanceToBase(c->loc, c->base));
    p[1] = getPower(getDistanceToBase(c->loc, 1));
    p[2] = getPower(getDistanceToBase(c->loc, 2));
    p[3] = getPower(getDistanceToBase(c->loc, 3));
    p[4] = getPower(getDistanceToBase(c->loc, 4));

    int I_max = 1;
    double P_max = p[1];
    for(int i=1; i<5; ++i){
        if(p[i] > P_max){
            P_max = p[i];
            I_max = i;
        }
    }

    /*for AvgMax*/
    c->base_best = I_max;
    total_power_max += P_max;
    
    /*for AvgMin*/
    if(getPower(getDistanceToBase(c->loc, c->base_pmin)) < P_min){
        c->base_pmin = I_max;
    }
    total_power_min += getPower(getDistanceToBase(c->loc, c->base_pmin));

    int isChanged = 0;
    if(c->base != I_max) isChanged = 1; 

    /*for deciding base*/
    switch(policy){
        case Best:
            c->base = c->base_best;
            if(isChanged == 1) nHandoff++;
            break;
        case Pmin:
            // won't be used
            c->base = c->base_pmin;
            break;
        case Threshold:
            if(p[0] < T){
                c->base = I_max;
                if(isChanged == 1) nHandoff++;
            }
            break;
        case Entropy:
            if(P_max > p[0] + E){
                c->base = I_max;
                if(isChanged == 1) nHandoff++;
            }
            break;
        case Mine:
            if(P_max > p[0] + E && p[0] < T){
                c->base = I_max;
                if(isChanged == 1) nHandoff++;
            }
            break;
    }
    total_power += getPower(getDistanceToBase(c->loc, c->base));
}

/*updates state of a car*/
int updateCar(Car *c, double v)
{
    /*change direction*/
    if((int)c->loc.x % 100 == 0 && (int)c->loc.y % 100 == 0 
        && (int)c->loc.x != 0 && (int)c->loc.x != 1000
        && (int)c->loc.y != 0 && (int)c->loc.y != 1000
    ){
        int r = rand()%5 + 1;
        if(r == 1){
            /*turn right*/
            c->dir = (c->dir + 1) % 4;
        } 
        if(r == 2){
            /*turn left*/
            c->dir = (c->dir - 1) % 4;
        } 
    }

    /*move*/
    switch(c->dir){
        case East:
            c->loc.x += v;
            break;
        case South:
            c->loc.y -= v;
            break;
        case West:
            c->loc.x -= v;
            break;
        case North:
            c->loc.y += v;
            break;
    }

    int ret = 0;
    /*check if is out of bounds*/
    if(c->loc.x < 0 || c->loc.x > 1000 || c->loc.y < 0 || c->loc.y > 1000){
        ret = 1;
    }
    return ret;
}

/*run simulation*/
void run(int times, int subtimes, Policy policy)
{
    /*initial setup*/
    List_cars *head = (List_cars*)malloc(sizeof(List_cars));
    List_cars *tail = head;
    int car_id = 0;

    /*set start points*/
    double start_point_x[36];
    double start_point_y[36];
    int start_point_base[36];
    int tmp_index = 0;

    for(int a=0; a<4; ++a){
        for(int b=1; b<10; ++b){
            double x, y;
            switch(a){
                case 0:
                    x = 0;
                    y = 100 * b;
                    break;
                case 1:
                    x = 100 * b;
                    y = 1000;
                    break;
                case 2:
                    x = 1000;
                    y = 100 * b;
                    break;
                case 3:
                    x = 100 * b;
                    y = 0;
                    break;
            }
            start_point_x[tmp_index] = x;
            start_point_y[tmp_index] = y;
            Location l = {.x = x, .y = y};
            start_point_base[tmp_index++] = getMaxPowerBase(l);
        }
    }

    /*run 'times * subtimes' times*/
    for(int i=0; i<times; ++i){
        int cnt = 0;
        int tmpHandoff = nHandoff;
        for(int j=0; j<subtimes; ++j){
            /*36 start points*/
            for(int k=0; k<36; ++k){
                if(poisson(arrival_rate, 1.2) == 1){
                    /*a car arrives*/
                    //printf("Car %d come @ %d dir:%d loc:%f, %f\n", car_id, i, a, x, y);
                    
                    /*set up new car*/
                    Car c = {
                        .loc.x = start_point_x[k],
                        .loc.y = start_point_y[k], 
                        .dir = k/9,
                        .id = car_id++,
                        .base = start_point_base[k],
                        .base_best  = start_point_base[k],
                        .base_pmin = start_point_base[k]
                    };
                
                    /*add new car to the list*/
                    List_cars *new_car = (List_cars*)malloc(sizeof(List_cars));
                    new_car->car = c;
                    new_car->next = NULL;

                    tail->next = new_car;
                    tail = tail->next;
                }
            }

            List_cars *previous = head;
            List_cars *current = head->next;
            
            while(current != NULL){
                /*iterate the list to update all cars*/

                if(j == subtimes-1){
                    /*record car number per sec*/
                    cnt++;
                }

                if(updateCar(&current->car, velocity) == 0){
                    // printf("Car %d ~%d :%d %f,%f\n", current->car.id, current->car.base, current->car.dir, current->car.loc.x, current->car.loc.y);
                
                    /*handle handoff*/
                    getBase(&current->car, policy);
                    current = current->next;
                    previous = previous->next;
                }
                else{
                    /*a car leaves*/
                    //printf("Car %d leaves @ %d\n", current->car.id, i);
                    List_cars *tmp = current;
                    previous->next = current->next;
                    current = current->next;
                    free(tmp);
                }
            }
        }
        /*record car number per sec*/
        total += cnt;
        /*record handoff time per sec*/
        list_handoff[i] = nHandoff - tmpHandoff;
    }
}

void print_info(int s)
{
    printf("Arrival rate: %lf\n", arrival_rate);
    printf("T = %lf, E = %lf\n", T, E);
    printf("Total_power_max: %Lf, Avg_max: %Lf dBm\n", total_power_max, total_power_max/total);
    printf("Total_power_min: %Lf, Avg_min: %Lf dBm\n", total_power_min, total_power_min/total);
    printf("Condition: %Lf dBm\n",(total_power_max/total + total_power_min/total)/2);
    printf("Total_power: %Lf, Avg power: %Lf dBm\n", total_power, total_power/total);
    printf("Total cars: %lld, Avg cars per sec: %lld cars\n", total, total/s);
    printf("Handoffs: %d times\n", nHandoff);

    FILE *ptrf;
    if((ptrf=fopen("log_cver.csv", "w")) == NULL){
        printf("[ERR]Fail to open log file!\n");
    }
    else{
        for(int i=0; i<86400-1; ++i){
            fprintf(ptrf, "%d,", list_handoff[i]);
        }
        fprintf(ptrf, "%d", list_handoff[86400-1]);
    }
}

int main(int argc, char* argv[])
{
    double start, end;
    start = clock();
    srand(time(NULL));
    //printf("%ld\n%ld\n", sizeof(List_cars), sizeof(Car));
    int s = 86400;
    arrival_rate = (double)1/2;
    E = 11;
    T = 17;
    run(s, 1, Best);
    print_info(s);
    end = clock();
    printf("Total time:%lf sec\n", (end-start)/CLOCKS_PER_SEC);
    return 0;
}
