#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>


double sigf(double x){
    return 1.f/(1.f+expf(-x));
}

typedef struct Network{
    double inp1_w1;
    double inp1_w2;
    double inp1_b;

    double inp2_w1;
    double inp2_w2;
    double inp2_b;

    double inner_w1;
    double inner_w2;
    double inner_b;

}Network;

double forward(Network m, double x, double y){
    double a = sigf(m.inp1_w1*x+m.inp1_w2*y+m.inp1_b);
    double b = sigf(m.inp2_w1*x+m.inp2_w2*y+m.inp2_b);
    return sigf(m.inner_w1*a+m.inner_w2*b+m.inner_b);
}


double TrainXor[][3]={
    {0,0,0},
    {1,0,1},
    {0,1,1},
    {1,1,0},
};

double TrainAnd[][3]={
    {0,0,0},
    {1,0,0},
    {0,1,0},
    {1,1,1},
};

double TrainOr[][3]={
    {0,0,0},
    {1,0,1},
    {0,1,1},
    {1,1,1},
};

#define train_count (int)(sizeof(TrainXor)/sizeof(TrainXor[0]))



double cost(Network m, double data [][3]){
    double res = 0.0f;
    for (int i = 0; i < train_count ; ++i){
        double x1 = data[i][0];
        double x2 = data[i][1];
        double y = forward(m,x1,x2);

        res += (y-data[i][2])*(y-data[i][2]);
    }

    return res/train_count;
}

double rand_float(void){
    return (double) rand() / (double) RAND_MAX;
}

Network rand_network(){
    Network m;
    m.inp1_w1 = rand_float();
    m.inp1_w2 = rand_float();
    m.inp1_b = rand_float();
    m.inp2_w1 = rand_float();
    m.inp2_w2 = rand_float();
    m.inp2_b = rand_float();
    m.inner_w1 = rand_float();
    m.inner_w2 = rand_float();
    m.inner_b = rand_float();
    return m;
}

void print_network(Network m){

    printf("inp1_w1 = %f\n",m.inp1_w1);
    printf("inp1_w2 = %f\n",m.inp1_w2);
    printf("inp1_b = %f\n",m.inp1_b);
    printf("inp2_w1 = %f\n",m.inp2_w1);
    printf("inp2_w2 = %f\n",m.inp2_w2);
    printf("inp2_b = %f\n",m.inp2_b);
    printf("inner_w1 = %f\n",m.inner_w1);
    printf("inner_w2 = %f\n",m.inner_w2);
    printf("inner_b = %f\n",m.inner_b);

}

Network finite_difference(Network m, double eps, double data[][3]){

    Network g;
    double c = cost(m,data);
    double saved;

    saved = m.inp1_w1;
    m.inp1_w1 += eps;
    g.inp1_w1 = (cost(m,data) -c)/eps;
    m.inp1_w1 = saved;

    saved = m.inp1_w2;
    m.inp1_w2 += eps;
    g.inp1_w2 = (cost(m,data) -c)/eps;
    m.inp1_w2 = saved;

    saved = m.inp1_b;
    m.inp1_b += eps;
    g.inp1_b = (cost(m,data) -c)/eps;
    m.inp1_b = saved;

    saved = m.inp2_w1;
    m.inp2_w1 += eps;
    g.inp2_w1 = (cost(m,data) -c)/eps;
    m.inp2_w1 = saved;

    saved = m.inp2_w2;
    m.inp2_w2 += eps;
    g.inp2_w2 = (cost(m,data) -c)/eps;
    m.inp2_w2 = saved;

    saved = m.inp2_b;
    m.inp2_b += eps;
    g.inp2_b = (cost(m,data) -c)/eps;
    m.inp2_b = saved;

    saved = m.inner_w1;
    m.inner_w1 += eps;
    g.inner_w1 = (cost(m,data) -c)/eps;
    m.inner_w1 = saved;

    saved = m.inner_w2;
    m.inner_w2 += eps;
    g.inner_w2 = (cost(m,data) -c)/eps;
    m.inner_w2 = saved;

    saved = m.inner_b;
    m.inner_b += eps;
    g.inner_b = (cost(m,data) -c)/eps;
    m.inner_b = saved;

    return g;
}


Network train(Network m, Network g, double rate){
    m.inp1_w1 -= rate*g.inp1_w1;
    m.inp1_w2 -= rate*g.inp1_w2;
    m.inp1_b -= rate*g.inp1_b;
    m.inp2_w1 -= rate*g.inp2_w1;
    m.inp2_w2 -= rate*g.inp2_w2;
    m.inp2_b -= rate*g.inp2_b;
    m.inner_w1 -= rate*g.inner_w1;
    m.inner_w2 -= rate*g.inner_w2;
    m.inner_b -= rate*g.inner_b;

    return m;
}

void bit_full_adder(Network xor, Network and, Network or, double bit1, double bit2, double carry_in, double* sum, double* carry_out) {
    double sum1 = forward(xor, bit1, bit2);
    *sum = forward(xor, sum1, carry_in);
    double carry1 = forward(and, bit1, bit2);
    double carry2 = forward(and, sum1, carry_in);
    *carry_out = forward(or, carry1, carry2);
}

char* full_adder(Network xor, Network and, Network or, char* num1, char* num2){
    int n = 4; 
    char* result = (char*)malloc(n + 2); 
    result[n + 1] = '\0';

    double carry_in = 0;
    for (int i = n - 1; i >= 0; --i) {
        double bit1 = num1[i] - '0'; 
        double bit2 = num2[i] - '0';
        double sum, carry_out;

        bit_full_adder(xor, and, or, bit1, bit2, carry_in, &sum, &carry_out);

        result[i + 1] = (sum >= 0.5) ? '1' : '0'; 
        carry_in = carry_out;
    }

    
    result[0] = (carry_in >= 0.5) ? '1' : '0';

    return result;
}


int main(){
    srand(time(NULL));

    Network xor = rand_network();
    Network and = rand_network();
    Network or  = rand_network();

    printf("Training Xor");
    printf("---------------------\n");
    for (int i=0;i<1000000;++i){
        xor=train(xor,finite_difference(xor,1e-2,TrainXor),1e-2);
    }
    printf("cost: %f\n", cost(xor,TrainXor));

    for (int i=0; i< train_count; ++i){
        int x= TrainXor[i][0];
        int y= TrainXor[i][1];
        printf("%d | %d => %f\n",x,y,forward(xor,x,y));
    }

    printf("Training And");
    printf("---------------------\n");
    for (int i=0;i<1000000;++i){
        and=train(and,finite_difference(and,1e-2,TrainAnd),1e-2);
    }
    printf("cost: %f\n", cost(and,TrainAnd));

    for (int i=0; i< train_count; ++i){
        int x= TrainAnd[i][0];
        int y= TrainAnd[i][1];
        printf("%d | %d => %f\n",x,y,forward(and,x,y));
    }

    printf("Training Or");
    printf("---------------------\n");
    for (int i=0;i<1000000;++i){
        or=train(or,finite_difference(or,1e-2,TrainOr),1e-2);
    }
    printf("cost: %f\n", cost(or,TrainOr));

    for (int i=0; i< train_count; ++i){
        int x= TrainOr[i][0];
        int y= TrainOr[i][1];
        printf("%d | %d => %f\n",x,y,forward(or,x,y));
    }    


    char* num1 = "1101";
    char* num2 = "1011";
    printf("%s + %s = ",num1,num2);
    char* result = full_adder(xor, and, or, num1, num2);

    printf(" %s\n", result);

    free(result);

    return 0;
}