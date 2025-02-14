#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <cstring>
#include <ostream>
#include <vector>
#include <array>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <omp.h>
#include <climits>

int si, smax, t, d;
int current_stamina, current_turn;

struct Demon {
    bool enfrentado;
    int index;
    int sc, tr, sr; // stamina batalha, turnos para recuperar, stamina recuperada
    int na;
    int score;
    std::vector<int> fragments;
};

struct
{
    bool operator()(Demon a, Demon b) const { return a.sc < b.sc; }
}compStamina;

struct
{
    bool operator()(Demon a, Demon b) const { return a.score < b.score; }
}compScore;

std::array<Demon, 500000> demons;
std::array<int, 500000> output;
std::array<int, 100000> fragments_in_turn;
std::array<int, 1000000> stamina_charge_in_turn;
int outputPos = 0;

void readInput();
int calculateFragments(int demon_index, int turn);
void solve();

int main() 
{
    readInput();

    solve();

    
    #pragma omp for
    for(int i = 0; i< d; i += 1)
    {
        if(demons[i].enfrentado == true )
            continue;
        demons[i].enfrentado = true;
        //set output
        output[outputPos] = demons[i].index;
        outputPos += 1;
    }

    
    for( int i = 0; i< d; i++)
    {
        std::cout << output[i] << std::endl;
    }

    return 0;
}

void readInput() 
{
    std::cin >> si >> smax >> t >> d;
    current_stamina = si;
    for (int i = 0; i < d; i++) 
    {
        Demon &demon = demons[i];
        demon.index = i;
        demon.enfrentado = false;

        std::cin >> demon.sc >> demon.tr >> demon.sr >> demon.na;

        demon.fragments.resize(demon.na);
        
        for (int j = 0; j < demon.na; j++) 
        {
            std::cin >> demon.fragments[j];
        }
    }

    //Sort by stamina
    std::sort(demons.begin(), demons.begin() + d, compStamina);
    
    /*
    for (int i = 0; i<d; i++) 
    {
        
        Demon &demon = demons[i];
        std::cout << demon.sc << " " << demon.tr << " " << demon.sr << " "<< demon.fragments.size() << " ";
        for (int &fragment : demon.fragments) {
            std::cout << fragment << " ";
        }
        std::cout<<std::endl;
    }*/
    
}

int calculateFragments(int demon_index, int turn) 
{
    int N = demons[demon_index].na;
    int soma = 0;

    if( N  == 0)
        return 0;

    int upper_limit = (t - turn) < N ? (t - turn) : N;

    for(int i = 0; i < upper_limit; i++ )
    {
        soma += demons[demon_index].fragments[i];
    }

    return soma;
}

int calculateFragmentsParallel(int demon_index, int turn) 
{
    int N = demons[demon_index].na;
    int soma = 0;

    if( N  == 0)
        return 0;

    int upper_limit = (t - turn) < N ? (t - turn) : N;

    #pragma omp parallel for reduction(+:soma)
    for(int i = 0; i < upper_limit; i++ )
    {
        soma += demons[demon_index].fragments[i];
    }

    return soma;
}

int selectDemon(int turn)
{
    int num_frags = 0;
    int greater_frags = -1;
    int selected_index = -1;
    int index = 0;

    for( index = 0; index < d; index += 1)
    {
        if( demons[index].enfrentado == true)
            continue;
        
        num_frags = calculateFragments(index, turn);
        if((demons[index].sc <= current_stamina) &&  (num_frags > greater_frags))
        {
            greater_frags = num_frags;
            selected_index = index;
        }
        
    }

    return selected_index;
}

int projectNumfrags(int demon_index, int current_turn, int steps_in_future)
{
    int n = demons[demon_index].na;
    int sum = 0;

    int upper_limit = steps_in_future < n ? steps_in_future : n;
    int turn_index = current_turn;
    for( int j = 0; j<upper_limit; j++)
    {
        sum += fragments_in_turn[turn_index] + demons[demon_index].fragments[j];
        turn_index += 1;
    }
    
    //printf("**%d**\n", sum);
    return sum;
}

int selectDemonOptimization(int turn)
{
    int num_frags = 0;
    int greater_frags = -1;
    int selected_index = -1;
    int less_stamina_lost = INT_MAX;
    int less_turn_wait_for_recover = INT_MAX;
    int greater_stamina_recover = -1;
    int less_tr = INT_MAX;
    int index = 0;
    int greater_rank = -1;
    int greater_score = -1;
    
    for( index = 0; index < d; index += 1)
    {
        if( demons[index].enfrentado == true)
            continue;

        //num_frags = calculateFragments(index, turn);
        //int rank = 0.5*num_frags + (0.25/demons[index].tr) + 0.25 * demons[index].sr;
        num_frags = projectNumfrags(index, turn, t-turn);
        int score = (num_frags * demons[index].sr) / demons[index].tr - demons[index].sc;
        if( (demons[index].sc <= current_stamina) && ( score > greater_score) )
        {
            //greater_frags = num_frags;
            greater_score = score;
            selected_index = index;
        }
    }
    
    /*for( index = 0; index < d; index += 1)
    {
        if( demons[index].enfrentado == true)
            continue;
        
        num_frags = calculateFragments(index, turn);
        if( turn <= (t/2) && (demons[index].sc <= current_stamina) && (demons[index].sr > greater_stamina_recover) )
        {
            greater_stamina_recover = demons[index].sr;
            selected_index = index;
        }
        else if( turn > (t/2) && (demons[index].sc <= current_stamina) && (demons[index].tr < less_turn_wait_for_recover) )
        {
            less_turn_wait_for_recover = demons[index].tr;
            selected_index = index;
        }
        
        
    }*/

    return selected_index;
}

int selectDemonParallel(int turn)
{
    int greater_frags = -1;
    int selected_index = -1;

    #pragma omp parallel
    {
        int thread_num_frags = 0;
        int thread_greater_frags = -1;
        int thread_selected_index = -1;

        #pragma omp for
        for (int index = 0; index < d; index++)
        {
            if (demons[index].enfrentado == true)
                continue;

            thread_num_frags = calculateFragments(index, turn);
            if ((demons[index].sc <= current_stamina) && (thread_num_frags > thread_greater_frags))
            {
                thread_greater_frags =  thread_num_frags;
                thread_selected_index = index;
            }
        }

        #pragma omp critical
        {
            if (thread_greater_frags > greater_frags)
            {
                greater_frags = thread_num_frags;
                selected_index = thread_selected_index;
            }
        }
    }

    return selected_index;
}

void solve() 
{
    for(int turn = 0 ; turn<t; turn++)
    {
        int demon_index_current_turn = -1;
        //Recover stamina
        current_stamina += stamina_charge_in_turn[turn];
        if( current_stamina > smax)
            current_stamina = smax;
            
        //select the demon
        //demon_index_current_turn = selectDemonParallel(turn);

        demon_index_current_turn = selectDemonOptimization(turn);
       
        //printf("%d %d %d\n", stamina_charge_in_turn[turn], current_stamina, turn);
        //
        if( demon_index_current_turn == -1)
        {
            //printf("turn: %d# -> X \tstamina: %d# \tdemon: %d#, index: %d#\n", turn, current_stamina, demons[demon_index_current_turn].index, demon_index_current_turn);
            continue;
        }
        else
        {
            //printf("turn: %d# -> \tstamina: %d#\tdemon: %d#, index: %d#\n", turn, current_stamina, demons[demon_index_current_turn].index, demon_index_current_turn);
        }
            

        demons[demon_index_current_turn].enfrentado = true;

        //set fragments in turns
        int index_in_fragments = turn;
        for(int j=0; j < demons[demon_index_current_turn].na; j++){

            fragments_in_turn[index_in_fragments] += demons[demon_index_current_turn].fragments[j];
            //printf("%d ", fragments_in_turn[index_in_fragments]);
            index_in_fragments += 1; 
        }
        //printf("\n");

        //set output
        output[outputPos] = demons[demon_index_current_turn].index;
        outputPos += 1;


        //subtract stamina
        current_stamina -= demons[demon_index_current_turn].sc;

        //Set stamina charge for a  specific turn(future)
        int stamina_charge_index = turn + demons[demon_index_current_turn].tr;

        stamina_charge_in_turn[stamina_charge_index] = demons[demon_index_current_turn].sr;
    }
}