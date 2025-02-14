#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <climits>
#include <cmath>
#include <random>
#include <omp.h>
using namespace std;


#define MaxN  40000
#define MAX_ANTENNAS 60000
#define MAX_BUILDS 350000

struct Coordinate{
    int x;
    int y;
};

struct Quadrant{
    Coordinate leftExtrem;
    Coordinate rightExtrem;
};

struct Build {
    Coordinate coordinate;
    int l;
    int c;
};

struct Antenna {
    int id;
    int r;
    int c;
    Coordinate coordinate;
};

long long W, H, N, M, R;
int center_x;
int center_y;
Build builds[MAX_BUILDS];
Antenna antennas[MAX_ANTENNAS];
bool there_is_antenna[MaxN][MaxN];
Quadrant quadrants[5];

long long contConectedAntennas = 0;
long long current_antenna = 0;
bool allBuildAreConnectedd = false;

long long resolveQuadrantAntenaScoreZero(Quadrant, Antenna *, Coordinate* );
long long resolveQuadrant(Quadrant, Antenna *, Coordinate* );
void readInput();
void divideGridSetQuadrants();
void ajustInput();
long long distAB(Coordinate coordinateA, Coordinate coordinateB);
void ajustAntenna(int buildIndex, Antenna *antenna);
void ajustAntennaRandom(int nearestBuildIndex, Antenna *antenna);
bool thereIsAntennaIncoordinate(Coordinate coordinate);

int main() 
{
    const int  MAX_QUADRANTS = 4;
    int aproximateReacheBuild = 0;
    long int total = 0;

    //Quadrant absoluteQuadrante;
    //Coordinate coordinateOfAntennaInAbsoluteQuadrante;

    readInput();
    divideGridSetQuadrants();
    ajustInput();

    omp_set_num_threads(4);

    /*
    absoluteQuadrante.leftExtrem.x = 0;
    absoluteQuadrante.leftExtrem.y = 0;
    absoluteQuadrante.rightExtrem.x = W;
    absoluteQuadrante.rightExtrem.y = H;
    //Posicionar a primeira antena
    if( antennas[current_antenna].r == 0 )
    {
        long int score = resolveQuadrantAntenaScoreZero(absoluteQuadrante, &antennas[current_antenna], &coordinateOfAntennaInAbsoluteQuadrante);
        total += score;
    }else{
            long int score = resolveQuadrant(absoluteQuadrante, &antennas[current_antenna], &coordinateOfAntennaInAbsoluteQuadrante);

            //printf("Antena %d# posicionada na coordenada: (%d, %d)/ score: %d\n", antennas[current_antenna].id, 
            //antennas[current_antenna].coordinate.x, antennas[current_antenna].coordinate.y, score);
            total += score;
    }
    current_antenna += 1;
    */

    for(long long i = current_antenna; !allBuildAreConnectedd && i<M; i++ )
    {
        Coordinate selectedAntennaCoordinate;
        Coordinate coordinate;
        long int greaterScore = -1;
        long int quadrantScore = -1;
        #pragma omp parallel
        {
            Coordinate thread_selectedAntennaCoordinate = selectedAntennaCoordinate;
            Coordinate thread_coordinate = coordinate;
            long long thread_greaterScore = greaterScore;
            long long thread_quadrantScore = quadrantScore;

            #pragma omp for
            for( int quadrant_index = 0; quadrant_index<MAX_QUADRANTS; quadrant_index++ )
            {
                thread_coordinate.x = -1;
                thread_coordinate.y = -1;
                thread_selectedAntennaCoordinate = thread_coordinate;
                thread_greaterScore = -1;

                if( antennas[current_antenna].r == 0 )
                {
                    thread_quadrantScore = resolveQuadrantAntenaScoreZero(quadrants[quadrant_index], &antennas[current_antenna], &thread_coordinate);
                    thread_selectedAntennaCoordinate = thread_coordinate;
                    thread_greaterScore = thread_quadrantScore;
                }
                else
                {
                    thread_quadrantScore = resolveQuadrant(quadrants[quadrant_index], &antennas[current_antenna], &thread_coordinate);
                    if( (thread_quadrantScore > greaterScore) )
                    {
                        thread_greaterScore = thread_quadrantScore;
                        thread_selectedAntennaCoordinate = thread_coordinate;
                    }
                }
            }

            #pragma omp critical
            {
                if (thread_greaterScore > greaterScore) {
                    greaterScore = thread_greaterScore;
                    quadrantScore = thread_quadrantScore;
                    selectedAntennaCoordinate = thread_selectedAntennaCoordinate;
                    coordinate = thread_coordinate;
                }
            }
        }

        //printf("Antena %d# posicionada na coordenada: (%d, %d)/ score: %d\n", antennas[current_antenna].id, antennas[current_antenna].coordinate.x,
        //    antennas[current_antenna].coordinate.y, graterScore);
        
        if( greaterScore > 0) {
            antennas[current_antenna].coordinate = selectedAntennaCoordinate;
            aproximateReacheBuild += 1;
            total += greaterScore;
        }

        if( aproximateReacheBuild == N )
            allBuildAreConnectedd = true;

        //colocar antena
        there_is_antenna[antennas[current_antenna].coordinate.x][antennas[current_antenna].coordinate.y] = true;

        current_antenna += 1;
        contConectedAntennas += 1;
        
    }

    printf("%lld\n", contConectedAntennas);
    for(long long antennaIndex =0; antennaIndex < contConectedAntennas; antennaIndex++)
    {
        printf("%lld %lld %lld\n", antennaIndex, antennas[antennaIndex].coordinate.x, antennas[antennaIndex].coordinate.y);
    }
    return 0;
}

void readInput(){
    //read W and H
    cin >> W >> H;
    
    /*read: N number of buildings, 
      M  number of available antennas and 
      R  the reward assigned if all the 
            buildings are connected 
            to the network
    */
    cin >> N >> M >> R;
    
    //read the builds
    for(int i = 0; i<N; i++){
        cin >> builds[i].coordinate.x >> builds[i].coordinate.y;
        cin >> builds[i].l >> builds[i].c;
        
        //cout<<builds[i].coordinate.x <<" "<< builds[i].coordinate.y<<" " << builds[i].l<<" " << builds[i].c<<endl;
    }
    
    //read the antennas
    for(int i = 0; i<M; i++){
        cin >> antennas[i].r >> antennas[i].c;
        antennas[i].id = i;
        //cout<<antennas[i].r <<" "<< antennas[i].c<<endl;
    }
}
void divideGridSetQuadrants(){
        //center of grid
    center_x = floor(W/2.0f);
    center_y = floor(H/2.0f);

    //seti irst quearant coordinates
    quadrants[0].leftExtrem.x = center_x;
    quadrants[0].leftExtrem.y = center_y - floor(H/2.0f);
    quadrants[0].rightExtrem.x = center_x + floor(W/2.0f);
    quadrants[0].rightExtrem.y = W;

    //set iscond quearant coordinates
    quadrants[1].leftExtrem.x = center_x;
    quadrants[1].leftExtrem.y = center_y;
    quadrants[1].rightExtrem.x = center_x + floor(W/2.0f);
    quadrants[1].rightExtrem.y = W;
    
   //seti hird quearant coordinates
    quadrants[2].leftExtrem.x = center_x - floor(W/2.0f);
    quadrants[2].leftExtrem.y = center_y;
    quadrants[2].rightExtrem.x = center_x;
    quadrants[2].rightExtrem.y = center_y + floor(H/2.0f);

    //set ifurth quearant coordinates
    quadrants[3].leftExtrem.x = center_x - floor(W/2.0f);
    quadrants[3].leftExtrem.y = center_y -  floor(H/2.0);
    quadrants[3].rightExtrem.x = center_x;
    quadrants[3].rightExtrem.y = center_y;
}

bool checkIfAPoitIsInsideOfQuadrant(Coordinate point, Quadrant quadrant){
    return  (point.x>=quadrant.leftExtrem.x) && 
            (point.x <= quadrant.rightExtrem.x) &&
            (point.y>=quadrant.leftExtrem.y) && 
            (point.y <= quadrant.rightExtrem.y);

}

long long resolveQuadrant(Quadrant quadrant, Antenna *antenna, Coordinate* coordinate)
{
    antenna->coordinate.x = (quadrant.leftExtrem.x + quadrant.rightExtrem.x) / 2;
    antenna->coordinate.y = (quadrant.leftExtrem.y + quadrant.rightExtrem.y) / 2;

    long long totalScore = 0;
    int buildIndex = 0;
    int nearestBuildIndex = -1;
    Build nearestBuild;
    int lowestDist = INT_MAX;


    for(const Build &build : builds) 
    {

        if( !checkIfAPoitIsInsideOfQuadrant(build.coordinate, quadrant) )
            continue;
            
        int dist = distAB(build.coordinate, antenna->coordinate);

        if(dist <= antenna->r) {
            totalScore += build.c * antenna->c - dist * build.l;
        }

        if (dist < lowestDist) 
        {
            lowestDist = dist;
            nearestBuild = build;
            nearestBuildIndex = buildIndex;
        }

        buildIndex++;
    }

    if(totalScore == 0) 
    {
        ajustAntenna(nearestBuildIndex, antenna);

        while(thereIsAntennaIncoordinate(antenna->coordinate)){
            nearestBuildIndex +=1;
            ajustAntenna(nearestBuildIndex, antenna);
        }

        int dist = distAB(builds[nearestBuildIndex].coordinate, antenna->coordinate);
        totalScore += builds[nearestBuildIndex].c * antenna->c - dist * builds[nearestBuildIndex].l;
        /*
        int dist = distAB(nearestBuild.coordinate, antenna->coordinate);
        totalScore += nearestBuild.c * antenna->c - dist * nearestBuild.l;
        */
    }
    else if( totalScore != 0 && thereIsAntennaIncoordinate(antenna->coordinate) ) {
        ajustAntennaRandom(nearestBuildIndex, antenna);
        while(thereIsAntennaIncoordinate(antenna->coordinate)){
            nearestBuildIndex +=1;
            ajustAntennaRandom(nearestBuildIndex, antenna);
        }

        int dist = distAB(builds[nearestBuildIndex].coordinate, antenna->coordinate);
        totalScore += builds[nearestBuildIndex].c * antenna->c - dist * builds[nearestBuildIndex].l;
    }

    *coordinate = antenna->coordinate;
    return totalScore;
}

long long resolveQuadrantAntenaScoreZero(Quadrant quadrant, Antenna *antenna, Coordinate* coordinate)
{
    long long score = 0;
    for(const Build &build : builds) {

        if( !checkIfAPoitIsInsideOfQuadrant(build.coordinate, quadrant) )
            continue;

        if( thereIsAntennaIncoordinate(build.coordinate) )
            continue;

        *coordinate = build.coordinate;
        score += build.c * antenna->c;

        return score;
    }

    //printf("Score zero %d\n", score);
    return 0;
}

bool compareAntennasRange(Antenna a, Antenna b) {
    return a.r > b.r; // Sort in descending order
}
bool compareBuildsCoordenates(Build buildA, Build buildB)
{
    bool result = buildA.coordinate.x == buildB.coordinate.x;
    if( !result )
        return buildA.coordinate.x > buildB.coordinate.x;
    
    result = buildA.coordinate.y == buildB.coordinate.y;
    if( !result )
        return  buildA.coordinate.y > buildB.coordinate.y;
    
    return result;
}
void ajustInput()
{
    //Sort the antenna array
    sort(antennas, antennas + M, compareAntennasRange);

    //Sort the Build by coordenate
    sort(builds, builds + N, compareBuildsCoordenates);
}

long long distAB(Coordinate coordinateA, Coordinate coordinateB)
{
    return std::abs( coordinateB.x - coordinateA.x) +  std::abs(coordinateB.y-coordinateA.y);
}

bool thereIsAntennaIncoordinate(Coordinate coordinate)
{
    /*/
    //printf("Currente Antenna %d\n", current_antenna);
    for(int antenna_index = 0; antenna_index < current_antenna; antenna_index++)
    {
        if( (antennas[antenna_index].coordinate.x == coordinate.x) && 
            (antennas[antenna_index].coordinate.y == coordinate.y) )
        {
            //printf("coordenada (%d, %d) já tem antena!\n", coordinate.x, coordinate.y);
            return true;
        }
    }*/

    return there_is_antenna[coordinate.x][coordinate.y];
}
void ajustAntenna(int buildIndex, Antenna *antenna)
{
    Build build = builds[buildIndex];
    int diffInX = build.coordinate.x-antenna->coordinate.x;
    int diffInY = build.coordinate.y-antenna->coordinate.y;
    long long dist = distAB( build.coordinate, antenna->coordinate);

    //cout << "build: (" << build.coordinate.x <<", " <<  build.coordinate.y << ")" << endl;
    //cout << "antenna: (" << antenna->coordinate.x <<", " <<  antenna->coordinate.y << "), "<< antenna->r << endl;

    if( fabs(diffInY) < fabs(diffInX) && diffInY > 0)
        antenna->coordinate.y  = antenna->coordinate.y + (dist - antenna->r);
    else if( fabs(diffInY) < fabs(diffInX) && diffInY < 0)
        antenna->coordinate.y  = antenna->coordinate.y - (dist - antenna->r);
    else if( fabs(diffInY) > fabs(diffInX) && diffInX > 0)
        antenna->coordinate.x  = antenna->coordinate.x + (dist - antenna->r);
    else if( fabs(diffInY) > fabs(diffInX) && diffInX < 0)
        antenna->coordinate.x  = antenna->coordinate.x - (dist - antenna->r);

    //cout i< "new aenenna coordinate: (" << antenna->coordinate.x <<", " <<  antenna->coordinate.y << "), "<< antenna->r << endl;
    //cout << "new distance: "<< distAB( build.coordinate, antenna->coordinate) << endl;       
}

void ajustAntennaRandom(int buildIndex, Antenna *antenna)
{
    std::random_device rd; 
    Build build = builds[buildIndex];

    std::mt19937 gen(rd());
    // Define the range for random numbers (e.g., 1 to 100)
    std::uniform_int_distribution<int> distrix(build.coordinate.x - antenna->r, build.coordinate.x + antenna->r);
    std::uniform_int_distribution<int> distriy(build.coordinate.y - antenna->r, build.coordinate.y + antenna->r);

    antenna->coordinate.x = distrix(gen);
    antenna->coordinate.y = distriy(gen);
}