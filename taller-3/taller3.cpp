#include <iostream>
#include <cmath>
#include <vector>
#include <chrono>

// Si se compila con -fopenmp, _OPENMP queda definido automáticamente
// y se incluye la librería. Si no, se ignora todo lo relacionado con OpenMP
#ifdef _OPENMP
#include <omp.h>
#endif

struct Vector3
{
    double e[3] = { 0 };

    Vector3() {}
    Vector3(double e0, double e1, double e2)
    {
        e[0] = e0; e[1] = e1; e[2] = e2;
    }
};

struct OrbitalEntity
{
    // e[0],e[1],e[2] = posición (x,y,z)
    // e[3],e[4],e[5] = velocidad (vx,vy,vz)
    // e[6]           = masa
    double e[7] = { 0 };

    OrbitalEntity() {}
    OrbitalEntity(double e0, double e1, double e2,
                  double e3, double e4, double e5, double e6)
    {
        e[0]=e0; e[1]=e1; e[2]=e2;
        e[3]=e3; e[4]=e4; e[5]=e5;
        e[6]=e6;
    }
};

int main()
{
    int N_ASTEROIDS = 2000;
    int N = 9 + N_ASTEROIDS;  // Total de cuerpos en la simulación

    OrbitalEntity* orbital_entities = new OrbitalEntity[N];

    // Condiciones iniciales: posición (m), velocidad (m/s), masa (kg)
    orbital_entities[0] = { 0.0,        0.0, 0.0,  0.0,      0.0,  0.0,  1.989e30   }; // Sol
    orbital_entities[1] = { 57.909e9,   0.0, 0.0,  0.0, 47.36e3,  0.0,  0.33011e24 }; // Mercurio
    orbital_entities[2] = { 108.209e9,  0.0, 0.0,  0.0, 35.02e3,  0.0,  4.8675e24  }; // Venus
    orbital_entities[3] = { 149.596e9,  0.0, 0.0,  0.0, 29.78e3,  0.0,  5.9724e24  }; // Tierra
    orbital_entities[4] = { 227.923e9,  0.0, 0.0,  0.0, 24.07e3,  0.0,  0.64171e24 }; // Marte
    orbital_entities[5] = { 778.570e9,  0.0, 0.0,  0.0,    13e3,  0.0,  1898.19e24 }; // Jupiter
    orbital_entities[6] = { 1433.529e9, 0.0, 0.0,  0.0,  9.68e3,  0.0,  568.34e24  }; // Saturno
    orbital_entities[7] = { 2872.463e9, 0.0, 0.0,  0.0,  6.80e3,  0.0,  86.813e24  }; // Urano
    orbital_entities[8] = { 4495.060e9, 0.0, 0.0,  0.0,  5.43e3,  0.0,  102.413e24 }; // Neptuno

    // Asteroides generados aleatoriamente para aumentar la carga de trabajo
    // y poder medir el speedup real de OpenMP
    for (int i = 9; i < N; i++)
    {
        orbital_entities[i] = {
            double(rand()) / RAND_MAX * 1e12,  // posición x aleatoria
            double(rand()) / RAND_MAX * 1e12,  // posición y aleatoria
            double(rand()) / RAND_MAX * 1e12,  // posición z aleatoria
            0.0, 0.0, 0.0,                     // velocidad inicial cero
            double(rand()) / RAND_MAX * 1e20   // masa aleatoria
        };
    }

    // Arreglo auxiliar donde se guarda la aceleración de cada cuerpo
    // calculada en FASE 1, para usarla en FASE 2 sin condiciones de carrera.
    // Cada posición accelerations[i] corresponde al cuerpo orbital_entities[i]
    Vector3* accelerations = new Vector3[N];

    // Guardamos la trayectoria de la Tierra (índice 3) a lo largo del tiempo
    std::vector<Vector3> trajectories;
    trajectories.push_back({ orbital_entities[3].e[0],
                              orbital_entities[3].e[1],
                              orbital_entities[3].e[2] });

    double dt    = 86400;               // Paso de tiempo: 1 día en segundos
    double t     = 0;
    double t_end = 86400.0 * 30.0;     // Tiempo final: 30 días (reducido para pruebas)
    double BIG_G = 6.67e-11;           // Constante gravitacional universal

    // Si se compiló con -fopenmp muestra los hilos reales del CPU,
    // si no, informa que corre en modo serial con 1 hilo
#ifdef _OPENMP
    std::cout << "Modo: PARALELO | Hilos disponibles: " << omp_get_max_threads() << std::endl;
#else
    std::cout << "Modo: SERIAL   | Hilos disponibles: 1 (sin OpenMP)" << std::endl;
#endif
    std::cout << "Cuerpos en simulacion: " << N << std::endl;

    auto t_start = std::chrono::high_resolution_clock::now();

    // Bucle principal de integración temporal (Euler explícito)
    // NO se puede paralelizar este while porque cada paso t depende del anterior
    while (t < t_end)
    {
        // =============================================================
        // FASE 1: Cálculo de aceleraciones
        // Cada cuerpo m1 recibe la fuerza gravitacional de todos los demás.
        // Esta fase SOLO LEE posiciones de orbital_entities,
        // por eso es seguro ejecutarla en paralelo.
        // =============================================================

        // #pragma omp parallel for le dice al compilador que reparta las
        // iteraciones del for entre los hilos disponibles.
        // schedule(static) divide el rango [0, N) en bloques iguales
        // y asigna cada bloque a un hilo fijo desde el inicio.
        // Es la mejor opción cuando todas las iteraciones tienen
        // el mismo costo computacional, como ocurre aquí.
        #pragma omp parallel for schedule(static)
        for (int m1_idx = 0; m1_idx < N; m1_idx++)
        {
            // a_g es LOCAL a cada hilo (declarada dentro del for)
            // Cada hilo tiene su propia copia, no hay conflicto entre hilos
            Vector3 a_g = { 0, 0, 0 };

            // Loop interno: suma la contribución gravitacional de cada m2 sobre m1
            // Este loop NO se paraleliza porque pertenece al hilo que ya tomó m1_idx
            for (int m2_idx = 0; m2_idx < N; m2_idx++)
            {
                if (m1_idx == m2_idx) continue; // un cuerpo no se atrae a sí mismo

                // Vector diferencia de posición entre m1 y m2
                double rx = orbital_entities[m1_idx].e[0] - orbital_entities[m2_idx].e[0];
                double ry = orbital_entities[m1_idx].e[1] - orbital_entities[m2_idx].e[1];
                double rz = orbital_entities[m1_idx].e[2] - orbital_entities[m2_idx].e[2];

                // Magnitud del vector r (distancia escalar entre m1 y m2)
                double r_mag = sqrt(rx*rx + ry*ry + rz*rz);

                // Magnitud de la aceleración: a = -G * m2 / r^2
                // (negativa porque la fuerza es atractiva, apunta hacia m2)
                double acc = -BIG_G * orbital_entities[m2_idx].e[6] / (r_mag * r_mag);

                // Se descompone la aceleración en sus componentes x,y,z
                // usando el vector unitario rx/r_mag, ry/r_mag, rz/r_mag
                a_g.e[0] += acc * rx / r_mag;
                a_g.e[1] += acc * ry / r_mag;
                a_g.e[2] += acc * rz / r_mag;
            }

            // Cada hilo escribe en accelerations[m1_idx], que es su posición
            // exclusiva. Dos hilos distintos nunca escriben en el mismo índice
            // porque m1_idx es diferente para cada uno → sin condición de carrera
            accelerations[m1_idx] = a_g;
        }
        // Barrera implícita al final del parallel for:
        // ningún hilo continúa hasta que TODOS terminaron la FASE 1

        // =============================================================
        // FASE 2: Actualización de velocidades y posiciones
        // Ahora sí podemos escribir sobre orbital_entities porque
        // la FASE 1 ya terminó completamente para todos los cuerpos.
        // =============================================================

        // Otro parallel for independiente del anterior.
        // Cada hilo actualiza un subconjunto de cuerpos sin interferir
        // con los demás, porque cada índice i es exclusivo de un hilo.
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < N; i++)
        {
            // Integración de Euler: v_nueva = v_vieja + a * dt
            orbital_entities[i].e[3] += accelerations[i].e[0] * dt;
            orbital_entities[i].e[4] += accelerations[i].e[1] * dt;
            orbital_entities[i].e[5] += accelerations[i].e[2] * dt;

            // Integración de Euler: x_nueva = x_vieja + v_nueva * dt
            orbital_entities[i].e[0] += orbital_entities[i].e[3] * dt;
            orbital_entities[i].e[1] += orbital_entities[i].e[4] * dt;
            orbital_entities[i].e[2] += orbital_entities[i].e[5] * dt;
        }
        // Barrera implícita: todos los cuerpos actualizados antes de avanzar t

        // Guardamos la posición actual de la Tierra para trazar su trayectoria
        trajectories.push_back({ orbital_entities[3].e[0],
                                  orbital_entities[3].e[1],
                                  orbital_entities[3].e[2] });
        t += dt;
    }

    auto t_end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = t_end_time - t_start;

    std::cout << "Simulacion terminada en " << elapsed.count() << " segundos." << std::endl;
    std::cout << "Puntos de trayectoria recolectados: " << trajectories.size() << std::endl;

    delete[] orbital_entities;
    delete[] accelerations;
    return 0;
}
