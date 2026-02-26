// Cabeceras necesarias
#include <cstdio>     
#include <vector>
#include <cstdlib>   // rand(), srand()
#include <ctime>     // time()

using namespace std;

// Parámetros del taller
const int grid_rows = 500;       // Número de filas en la cuadrícula
const int grid_cols = 500;       // Número de columnas en la cuadrícula
const int initial_fish = 150000;  // Número de peces iniciales
const int initial_sharks = 3000; // Número de tiburones iniciales
const int fish_breed = 3;        // Tiempo (en pasos) que tarda un pez en reproducirse
const int shark_breed = 10;      // Tiempo (en pasos) que tarda un tiburón en reproducirse
const int shark_starve_t = 6;    // Tiempo (en pasos) que tarda un tiburón en morir de hambre
const int time_steps = 2000;     // Número de pasos de tiempo a simular

// Tipos de celdas en la cuadrícula
const int EMPTY = 0;
const int FISH = 1;
const int SHARK = 2;

// Estructura para representar cada celda de la cuadrícula
struct Cell
{
    int type;   // Tipo de celda: empty, fish o shark
    int age;    // Contador de generaciones vivídas para reproducción
    int energy; // Usado solo para tiburones: energía restante antes de morir de hambre (0 = death)
    bool moved; // Indica si el animal ya se ha movido en el paso actual (para evitar moverlo dos veces)
};

Cell grid[grid_rows][grid_cols]; // Cuadrícula principal

// Función para inicializar la cuadrícula con peces y tiburones
void initializeGrid()
{
    // Llenamos toda la grilla de vacío
    for (int r = 0; r < grid_rows; r++)
    {
        for (int c = 0; c < grid_cols; c++)
        {
            grid[r][c] = {EMPTY, 0, 0, false};
        }
    }

    // Colocamos exactamente initial_fish peces en posiciones aleatorias
    int placed = 0; // Contador de peces colocados
    while (placed < initial_fish)
    {                               // Mientras no hayamos colocado todos los peces
        int r = rand() % grid_rows; // Elegimos una fila aleatoria
        int c = rand() % grid_cols; // Elegimos una columna aleatoria
        if (grid[r][c].type == EMPTY)
        {
            grid[r][c].type = FISH;
            grid[r][c].age = rand() % fish_breed; // edad inicial aleatoria entre 0 y 2
            grid[r][c].energy = 0;
            placed++; // Incrementamos el contador de peces colocados
        }
    }

    // Colocamos exactamente initial_sharks tiburones en posiciones aleatorias
    placed = 0;
    while (placed < initial_sharks)
    {
        int r = rand() % grid_rows;
        int c = rand() % grid_cols;
        if (grid[r][c].type == EMPTY)
        {
            grid[r][c].type = SHARK;
            grid[r][c].age = rand() % shark_breed;           // edad inicial aleatoria
            grid[r][c].energy = 1 + rand() % shark_starve_t; // energía inicial aleatoria. Ningun tiburón empieza con energía 0
            placed++;
        }
    }
}
// Toroide: si un animal llega al borde, aparece en el lado opuesto
// Ejemplo: si sube desde la fila 0, aparece en la fila 499

int toroidal_row(int r)
{
    if (r < 0)
        return grid_rows - 1; // se salió por arriba -> aparece abajo
    if (r >= grid_rows)
        return 0; // se salió por abajo  -> aparece arriba
    return r;     // está dentro de la grilla, no cambia
}

int toroidal_col(int c)
{
    if (c < 0)
        return grid_cols - 1; // se salió por la izquierda -> aparece a la derecha
    if (c >= grid_cols)
        return 0; // se salió por la derecha   -> aparece a la izquierda
    return c;     // está dentro de la grilla, no cambia
}

// Conseguir las posiciones vecinas (arriba, abajo, izquierda, derecha) de una celda
// según el tipo (empty, fish o shark)
vector<pair<int, int>> get_neighbors(int r, int c, int type)
{
    vector<pair<int, int>> neighbors;

    // Revisamos los 4 vecinos y solo guardamos los que sean del tipo buscado
    if (grid[toroidal_row(r - 1)][c].type == type)
        neighbors.push_back({toroidal_row(r - 1), c}); // Arriba

    if (grid[toroidal_row(r + 1)][c].type == type)
        neighbors.push_back({toroidal_row(r + 1), c}); // Abajo

    if (grid[r][toroidal_col(c - 1)].type == type)
        neighbors.push_back({r, toroidal_col(c - 1)}); // Izquierda

    if (grid[r][toroidal_col(c + 1)].type == type)
        neighbors.push_back({r, toroidal_col(c + 1)}); // Derecha

    return neighbors;
}

// Función para simular el paso de un pez
void stepFish() {
    // Creamos una lista con todas las posiciones de la grilla
    vector<int> positions;
    for (int i = 0; i < grid_rows * grid_cols; i++) {
        positions.push_back(i);
    }

    // Barajamos las posiciones aleatoriamente (Fisher-Yates)
    for (int i = positions.size() - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        swap(positions[i], positions[j]);
    }

    // Recorremos la grilla en orden aleatorio
    for (int idx = 0; idx < (int)positions.size(); idx++) {
        int r = positions[idx] / grid_cols;
        int c = positions[idx] % grid_cols;

        // Solo procesamos peces que no hayan actuado en este turno
        if (grid[r][c].type != FISH || grid[r][c].moved) continue;

        // El pez envejece un paso
        grid[r][c].age++;

        // Buscamos celdas vacías alrededor
        vector<pair<int,int>> free_cells = get_neighbors(r, c, EMPTY);

        if ((int)free_cells.size() > 0) {
            // Elegimos una celda vacía al azar
            int choice = rand() % free_cells.size();
            int new_r = free_cells[choice].first;
            int new_c = free_cells[choice].second;

            // Movemos el pez a la nueva celda
            grid[new_r][new_c].type   = FISH;
            grid[new_r][new_c].age    = grid[r][c].age;
            grid[new_r][new_c].energy = 0;
            grid[new_r][new_c].moved  = true;

            // ¿Se reproduce?
            if (grid[r][c].age >= fish_breed) {
                // Deja una cría en la celda que abandonó
                grid[r][c].type  = FISH;
                grid[r][c].age   = 0;
                grid[r][c].moved = true;
            } else {
                // Sin reproducción: la celda anterior queda vacía
                grid[r][c].type   = EMPTY;
                grid[r][c].age    = 0;
                grid[r][c].moved  = false;
            }
        }
        // Si no hay celdas vacías: el pez no se mueve ni reproduce
    }
}

// Función para simular el paso de un tiburón

void stepShark() {
    // Creamos una lista con todas las posiciones de la grilla
    vector<int> positions;
    for (int i = 0; i < grid_rows * grid_cols; i++) {
        positions.push_back(i);
    }

    // Barajamos las posiciones aleatoriamente (Fisher-Yates)
    for (int i = positions.size() - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        swap(positions[i], positions[j]);
    }

    // Recorremos la grilla en orden aleatorio
    for (int idx = 0; idx < (int)positions.size(); idx++) {
        int r = positions[idx] / grid_cols;
        int c = positions[idx] % grid_cols;

        // Solo procesamos tiburones que no hayan actuado en este turno
        if (grid[r][c].type != SHARK || grid[r][c].moved) continue;

        // El tiburón envejece un paso
        grid[r][c].age++;

        // Buscamos peces vecinos primero (prioridad: comer)
        vector<pair<int,int>> fish_cells = get_neighbors(r, c, FISH);

        // Buscamos celdas vacías también (para moverse si no hay peces)
        vector<pair<int,int>> free_cells = get_neighbors(r, c, EMPTY);

        int new_r, new_c;
        bool can_move = false;

        if ((int)fish_cells.size() > 0) {
            int choice = rand() % fish_cells.size();
            new_r = fish_cells[choice].first;
            new_c = fish_cells[choice].second;
            grid[r][c].energy = shark_starve_t;
            can_move = true;

        } else if ((int)free_cells.size() > 0) {
            int choice = rand() % free_cells.size();
            new_r = free_cells[choice].first;
            new_c = free_cells[choice].second;
            grid[r][c].energy--;

            if (grid[r][c].energy <= 0) {
                grid[r][c].type   = EMPTY;
                grid[r][c].age    = 0;
                grid[r][c].energy = 0;
                continue;
            }
            can_move = true;

        } else {
            grid[r][c].energy--;
            if (grid[r][c].energy <= 0) {
                grid[r][c].type   = EMPTY;
                grid[r][c].age    = 0;
                grid[r][c].energy = 0;
            }
            continue;
        }

        if (can_move) {
            grid[new_r][new_c].type   = SHARK;
            grid[new_r][new_c].age    = grid[r][c].age;
            grid[new_r][new_c].energy = grid[r][c].energy;
            grid[new_r][new_c].moved  = true;

            if (grid[r][c].age >= shark_breed) {
                grid[r][c].type   = SHARK;
                grid[r][c].age    = 0;
                grid[r][c].energy = shark_starve_t;
                grid[r][c].moved  = true;
            } else {
                grid[r][c].type   = EMPTY;
                grid[r][c].age    = 0;
                grid[r][c].energy = 0;
                grid[r][c].moved  = false;
            }
        }
    }
}

// Función principal
int main() {
    // Inicializamos la semilla del random una sola vez
    srand(time(NULL));

    // Inicializamos la grilla con peces y tiburones
    initializeGrid();

    // Abrimos un archivo CSV para guardar las poblaciones en cada generación
    FILE* file = fopen("populations.csv", "w");
    fprintf(file, "generation,fish,sharks\n");  // encabezado del CSV

    // Bucle principal de simulación
    for (int t = 0; t < time_steps; t++) {

        // Reseteamos la bandera moved al inicio de cada turno
        for (int r = 0; r < grid_rows; r++)
            for (int c = 0; c < grid_cols; c++)
                grid[r][c].moved = false;

        // Simulamos un paso
        stepFish();
        stepShark();

        // Contamos las poblaciones AL FINAL del paso (como dice el pseudocódigo)
        int fish_count  = 0;
        int shark_count = 0;
        for (int r = 0; r < grid_rows; r++)
            for (int c = 0; c < grid_cols; c++) {
                if (grid[r][c].type == FISH)  fish_count++;
                if (grid[r][c].type == SHARK) shark_count++;
            }

        // Guardamos en el CSV
        fprintf(file, "%d,%d,%d\n", t, fish_count, shark_count);

        // Imprimimos progreso cada 100 generaciones
        if (t % 100 == 0)
            printf("Generation %d | Fish: %d | Sharks: %d\n", t, fish_count, shark_count);

        // Si ambas especies se extinguen no tiene sentido seguir
        if (fish_count == 0 && shark_count == 0) {
            printf("Total extinction at generation %d\n", t);
            break;
        }
    }

    fclose(file);
    printf("Data saved in populations.csv\n");
    return 0;
}



