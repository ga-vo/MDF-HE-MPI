#include <mpi.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <chrono>

using namespace std;

// initial variables
int L, tmax;
long nx, nc, nr;
int nt = 10000;
int size = 0;
bool lleno = false;
double temp;
double dx, dt, alpha, r, r2, tempIzq, tempDer;

double t = 0;

double *u, *u_parallel;

/*
Separa la entrada de un array de posiciones y llena el arreglo
Parameters
----------
s : string
    array de posiciones en forma de un string separado por comas
size: int
    cantidad de posiciones en el array
Returns
----------
arr : double*
    arreglo lleno con las posiciones recibidas
*/
double *split(string s, int size)
{
    double *arr = new double[size];
    int occ = -1;
    int occ2 = 0;
    for (int i = 1; i < size - 1; i++)
    {
        if (i % nc == 0)
        {
            arr[i] = tempIzq;
        }
        else
        {
            if (i % nc - 1 == 0)
            {
                arr[i] = tempDer;
            }
            else
            {
                occ2 = s.find(",", occ + 1);

                if (occ2 != string::npos)
                {
                    arr[i] = stod(s.substr(occ + 1, occ2));
                }
                else
                {
                    arr[i] = stod(s.substr(occ + 1, s.length() - 1));
                }
                occ = occ2;
            }
        }
    }
    lleno = true;

    return arr;
}

/*
Llena el array de posiciones en caso de estar vacío
Parameters
----------
temp : double
    Temperatura constante dentro del array de posiciones
size: int
    cantidad de posiciones en el array
Returns
----------
arr : double*
    arreglo lleno con las posiciones recibidas
*/
double *llenado(double temp, int size)
{
    double *arr = new double[size];

    for (int j = 0; j < nr; j++)
    {
        int ar = (j * nc);
        arr[ar] = tempIzq;
        if (!lleno)
        {
            for (int i = 1; i < nc - 1; i++)
            {

                arr[ar + i] = temp;
            }
        }
        else
        {
            for (int i = 1; i < nc - 1; i++)
            {

                arr[ar + i] = u[ar + i];
            }
        }
        arr[ar + nc - 1] = tempDer;
    }
    lleno = true;
    return arr;
}

/*
Almacena los valores en un archivo
! Las lineas comentadas podrían usar la librería gnuplot para graficar
Parameters
----------
filename : string
    nombre de archivo de salida
title: string
    titulo a mostrar en la gráfica
Returns
----------
*/
void graph(string filename, string title)
{
    // Gnuplot gp;
    ofstream salida(filename);

    // std::vector<double> pts_X;
    // std::vector<double> pts_Y;
    // std::vector<double> pts_Z;nc, nr,

    for (int j = 0; j < nr; j++)
    {
        int ar = (j * nc);
        for (int i = 0; i < nc; i++)
        {
            // pts_X.push_back(i);
            // pts_Y.push_back(j);
            // pts_Z.push_back(u[ar + i]);
            salida << i << " " << j << " " << u[ar + i] << endl;
        }
    }

    salida.close();

    // gp << "set xrang[0:" << max(nr,nc) << "]" << "\n set yrang[0:" << max(nr,nc) << "]" << endl;
    // gp << "plot '-' with image title '" << title << "'" << endl;c
    // gp.send1d(make_tuple(pts_X, pts_Y, pts_Z));
    // gp.clearTmpfiles();
}

/*
Ejecuta cálculos de la ecuación de calor utilizando openMPI
Parameters
----------
filename : string
    nombre de archivo de salida
title: string
    titulo a mostrar en la gráfica
Returns
----------
*/
void MDF_MPI()
{
    int rank;
    int proc = 6;
    long chunksize = nr / (proc);
    int size_comm;

    MPI_Status status;

    // Obtener id del node actual
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Definición del tamaño del arreglo en el que se encuentra
    int size = nx - (rank * chunksize * nc);

    // Reserva memoria para un arreglo de tamaño size
    double *u_proc = new double[size];

    // Si se encuentra en el nodo 0
    if (rank == 0)
    {
        // Copia del arreglo originalmente llenado
        memcpy(u_proc, u, size * sizeof(double));

        // Envío del segmento del arreglo a cada nodo
        /*
            La lógica utilizada para dividir el trabajo entre los nodos, consiste
            en que el tamaño del chunk está definido por un número de filas, por tanto
            el array tendrá un tamaño del número de filas por el número de columnas.

            Con esto se envía entonces segmentos de tamaño chunksize a cada nodo
        */
        for (int i = 1; i < proc; i++)
        {
            MPI_Send(&u[i * chunksize * nc], nx - (i * chunksize * nc), MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
        }
    }
    else
    {
        // Los nodos diferentes al 0 reciben el segmento del arreglo
        MPI_Recv(&u_proc[0], size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
    }

    // Reservar y realizar la copia para los valores iniciales
    double uold[size];
    memcpy(uold, u_proc, size * sizeof(double));

    // Ciclo for para cada instante de tiempo
    for (int m = 0; m < nt; m++)
    {

        // Se recorre cada fila
        for (int j = 0; j < chunksize; j++)
        {
            // Debido a la naturaleza 1D del array, se calcula la posición del arreglo para esa fila
            int ar = (j * nc);

            // Se recorre cada elemento de la fila
            for (int i = 1; i < nc - 1; i++)
            {
                // Verificación que no sobrepase el tamaño del arreglo, buscando errores de lógica
                if (ar + 1 < nx)
                {
                    u_proc[ar + i] = r * uold[ar + i - 1] + r2 * uold[ar + i] + r * uold[ar + i + 1];
                }
                else
                {
                    cout << "i: " << i << "  j: " << j << endl;
                }
            }
        }

        // Actualización del arreglo de posiciones iniciales
        memcpy(uold, u_proc, size * sizeof(double));
    }

    // Una vez ha acabado el cálculo los nodos diferentes del 0 envían el arreglo procesado
    if (rank != 0)
    {
        MPI_Send(u_proc, nx - (rank * chunksize * nc), MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }

    if (rank == 0)
    {
        // Se almacena el arreglo procesado en el original
        memcpy(u, u_proc, size * sizeof(double));
        free(u_proc);

        // Se reciben los arreglos procesados de cada nodo
        for (int i = 1; i < proc; i++)
        {
            // El arreglo (porción del original) se asigna a la posición de la que fue enviado
            MPI_Recv(&u[i * chunksize * nc], nx - (i * chunksize * nc), MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);
        }

        // Llamado a la función graph para el almacenamiento del archivo de salida
        graph("output", "Estado Final");
        cout << "Finished :D" << endl;
    }
    MPI_Finalize();
}

/*
Función principal
Parameters
----------
argc : int
    Número de parámetros
argv: char**
    Arreglo de parámetros de línea de comandos
Returns
0 : int
    Si se ejecutó correctamente
----------
*/
int main(int argc, char *argv[])
{
    // Inicializa el ambiente de ejecución de MPI
    MPI_Init(&argc, &argv);
    int rank;
    int size_comm;

    // Verificar el tamaño de la interfaz de comunicación
    // En caso de no ser igual a 6 se detiene, porque está diseñado para esa cantidad de nodos
    MPI_Comm_size(MPI_COMM_WORLD, &size_comm);
    if (size_comm != 6)
    {
        printf("This application is meant to be run with 6 processes.\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    // Obtener el id del nodo
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /*
        Separación y asignación de parámetros recibidos en el archivo de entrada
    */

    // Verificiación de que exista un parámetro (un archivo de entrada)
    if (argc > 1)
    {
        ifstream inputFile(argv[1]);
        string line;
        int nline = 1;
        double *arr;

        // Verificación de que exista el archivo
        if (!inputFile)
        {
            cout << "Please use $ ./MDA <file> or verify filename" << endl;
            cout << "File format:\n     nrows\n     ncolumns\n     dt\n     dx\n     u\n     alpha(or k)" << endl;
        }
        else
        {
            // Recorrido del archivo y asignación de cada parámetro contenido
            while (getline(inputFile, line))
            {
                switch (nline)
                {
                case 1:
                    try
                    {
                        nr = stoi(line);
                    }
                    catch (invalid_argument e)
                    {
                        cout << "Please provide a valid file format: error in number of rows, provide a int" << endl;
                        exit(1);
                    }

                    break;
                case 2:
                    try
                    {
                        nc = stoi(line);
                    }
                    catch (invalid_argument e)
                    {
                        cout << "Please provide a valid file format: error in number of columns, provide a int" << endl;
                        exit(1);
                    }

                    break;
                case 3:
                    try
                    {
                        dt = stod(line);
                    }
                    catch (invalid_argument e)
                    {
                        cout << "Please provide a valid file format: error in number of columns, provide a int" << endl;
                        exit(1);
                    }

                    break;
                case 4:
                    try
                    {
                        dx = stod(line);
                    }
                    catch (invalid_argument e)
                    {
                        cout << "Please provide a valid file format: error in number of columns, provide a int" << endl;
                        exit(1);
                    }

                    break;
                case 5:
                    if (argc > 2)
                    {
                        if (!strcmp(argv[2], "--array"))
                        {
                            if (rank == 0)
                            {
                                size = std::count(line.begin(), line.end(), ',') + 3;
                                if (size > 0)
                                {
                                    arr = split(line, size);
                                    u = arr;
                                    nx = size;
                                }
                                else
                                {
                                    cout << "Please provide a valid file format: error in U vector" << endl;
                                }
                            }
                        }
                    }
                    else
                    {
                        try
                        {
                            size = nr * nc;
                            nx = size;
                            temp = stod(line);
                        }
                        catch (invalid_argument e)
                        {
                            cout << "Please provide a valid file format: error in initial temperature, provide a float" << endl;
                            exit(1);
                        }
                    }
                    break;
                case 6:
                    try
                    {
                        tempIzq = stod(line);
                    }
                    catch (invalid_argument e)
                    {
                        cout << "Please provide a valid file format: error in number of columns, provide a int" << endl;
                        exit(1);
                    }

                    break;
                case 7:
                    try
                    {
                        tempDer = stod(line);
                    }
                    catch (invalid_argument e)
                    {
                        cout << "Please provide a valid file format: error in number of columns, provide a int" << endl;
                        exit(1);
                    }

                    break;
                case 8:
                    try
                    {
                        alpha = stod(line);
                    }
                    catch (invalid_argument e)
                    {
                        cout << "Please provide a valid file format: error in number of columns, provide a int" << endl;
                        exit(1);
                    }
                    break;
                }
                nline++;
            }

            // Cálculo de parámetros del proceso
            r = alpha * dt / (dx * dx);
            r2 = 1 - 2 * r;

            // Realizar el llenado del arreglo sólo en el nodo 0
            if (rank == 0)
            {
                u = llenado(temp, size);
                u[0] = tempIzq;
                u[nx - 1] = tempDer;
                u_parallel = new double[nx];
                memcpy(u_parallel, u, nx * sizeof(double));
                cout << "--- Ecuación de calor: Método de diferencias finitas MPI ---" << endl;
                cout << "nc: " << nc << "  nr: " << nr << "  nx: " << nx << "  dt: " << dt << "  dx: " << dx << "  alpha: " << alpha << endl;
                cout << "r: " << r << " | r2: " << r2 << endl;
            }
            inputFile.close();
        }
    }
    else
    {

        cout << "Please use $ ./MDA <file> " << endl;
        cout << "File format:\n     nrows\n     ncolumns\n     dt\n     dx\n     u\n     alpha(or k)" << endl;
    }

    // Lanzamiento de proceso de cálculo con MPI
    MDF_MPI();
    
    return 0;
}