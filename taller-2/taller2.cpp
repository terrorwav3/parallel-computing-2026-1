// Cabeceras necesarias
#include <iostream>
#include <vector>
#include <cstdlib>    // rand(), srand()
#include <ctime>      // time()
#include <algorithm>  // shuffle
#include <random>     // mt19937 (mejor generador aleatorio que rand())

using namespace std;

// Parámetros del taller
const int grid_rows = 500;    // Número de filas en la cuadrícula
const int grid_cols = 500;    // Número de columnas en la cuadrícula
const int initial_fish = 50000;    // Número de peces iniciales
const int initial_sharks = 5000;   // Número de tiburones iniciales
const int fish_breed = 3;      // Tiempo (en pasos) que tarda un pez en reproducirse
const int shark_breed = 10;   // Tiempo (en pasos) que tarda un tiburón en reproducirse
const int shark_starve_t = 4;  // Tiempo (en pasos) que tarda un tiburón en morir de hambre
const int time_steps = 1000;  // Número de pasos de tiempo a simular

// Tipos de celdas en la cuadrícula
const int EMPTY = 0;
const int FISH = 1; 
const int SHARK = 2;

// Estructura para representar cada celda de la cuadrícula
struct Cell {
    int type;           // Tipo de celda: empty, fish o shark
    int age;    // Contador de generaciones vivídas para reproducción
    int energy;   // Usado solo para tiburones: energía restante antes de morir de hambre (0 = death)

};

Cell grid[grid_rows][grid_cols];  // Cuadrícula principal

// Función para inicializar la cuadrícula con peces y tiburones
void initialize_grid() {
   // Llenamos toda la grilla de vacío
    for (int r = 0; r < grid_rows; r++) {
        for (int c = 0; c < grid_cols; c++) {
            grid[r][c] = {EMPTY, 0, 0};
        }
    }

    // Colocamos exactamente initial_fish peces en posiciones aleatorias
    int placed = 0;
    while (placed < initial_fish) {
        int r = rand() % grid_rows;
        int c = rand() % grid_cols;
        if (grid[r][c].type == EMPTY) {
            grid[r][c].type   = FISH;
            grid[r][c].age    = rand() % fish_breed;      // edad inicial aleatoria
            grid[r][c].energy = 0;
            placed++;
        }
    }

    // Colocamos exactamente initial_sharks tiburones en posiciones aleatorias
    placed = 0;
    while (placed < initial_sharks) {
        int r = rand() % grid_rows;
        int c = rand() % grid_cols;
        if (grid[r][c].type == EMPTY) {
            grid[r][c].type   = SHARK;
            grid[r][c].age    = rand() % shark_breed;         // edad inicial aleatoria
            grid[r][c].energy = 1 + rand() % shark_starve_t;  // energía inicial aleatoria
            placed++;
        }
    }
}