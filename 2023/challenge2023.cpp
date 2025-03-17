#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <climits>
#include <cmath>
#include <random>

struct Snake {
    int start_col, start_row;
    int length;
};

#define WORMHOLE INT_MIN

Snake snake_array[5000];

int input_matrix[5000][5000] = {0};
bool mark_matrix[5000][5000] = {false};

int cols, rows, snakes;

void readInput();
void walkPath(int c, int r, int length);

void print_mats() {
    // for (int i = 0; i < rows; i++) {
    //     for (int j = 0; j < cols; j++) {
    //         printf("%6d ", input_matrix[i][j]);
    //     }
    //     std::cout << '\n';
    // }
    std::cout<<'\n';
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%6d ", mark_matrix[i][j]);
        }
        std::cout << '\n';
    }
}

int main() 
{
    std::random_device rd;
    std::mt19937 gen(rd());
    int c, r;

    readInput();

    for(int index = 0; index < snakes; index++)
    {
        std::uniform_int_distribution<int> distc(0, cols-1);
        std::uniform_int_distribution<int> distr(0, rows-1);
        c = distc(gen);
        r = distr(gen);
        
        while( mark_matrix[r][c] || input_matrix[r][c] == WORMHOLE )
        {
            c = distc(gen);
            r = distr(gen);
        }

        walkPath(c, r, snake_array[index].length);
        printf("\n");
    }
    //walkPath(0, 0, 10);
    //print_mats();

    return 0;
}

void readInput() {
    std::cin >> cols >> rows >> snakes;

    for (int i = 0; i < snakes; i++) {
        std::cin >> snake_array[i].length;
    }

    for (int i = 0; i < rows; i++) {
        std::string input;
        for (int j = 0; j < cols; j++) {
            std::cin >> input;
            if (input == "*") {
                input_matrix[i][j] = WORMHOLE;
            }
            else {
                input_matrix[i][j] = (short)atoi(input.c_str());
            }
        }
    }
}

void walkPath(int start_col, int start_row, int length ) {
    mark_matrix[start_row][start_col] = true;
    printf("%d %d ", start_col, start_row);
    for (int s = 0; s < length - 1; s++) 
    {
        int d = (start_row + 1) % rows;
        int u = (start_row - 1) % rows;
        int l = (start_col - 1) % cols;
        int r = (start_col + 1) % cols;

        if (u < 0) {
            u += rows;
        }

        if (l < 0) {
            l += cols;
        }

        int neighbors[4][2] = {
                                {u, start_col},
                                {d, start_col},
                                {start_row, l},
                                {start_row, r},
                            };

        int maxval =   -10001, maxi = 0;
        for (int k = 0; k < 4; k++) 
        {
            int row = neighbors[k][0];
            int column = neighbors[k][1];

            int val = input_matrix[row][column];
            if (!mark_matrix[row][column] && val > maxval) {
                maxval = val;
                maxi = k;
            }
        }

        int row = neighbors[maxi][0];
        int column = neighbors[maxi][1];
        char paths[4] = {
            'U',
            'D',
            'L',
            'R'
        };
        (s == length-2) ? printf("%c", paths[maxi]) : printf("%c ", paths[maxi]);
        
        mark_matrix[row][column] = true;
        start_col = column;
        start_row = row;
    }
}