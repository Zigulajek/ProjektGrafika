#include <iostream>
#include <math.h>
using namespace std;
const int MAX_N = 100; // Maksymalna liczba węzłów

// Struktura przechowująca węzły aproksymacji
struct Node {
    double x;
    double y;
    void wypisz()
    {
        cout << "(" << x << ", " << y << ")";
    }
};

void proste(double** macierz, int rozmiar) {
    for (int i = 1; i < rozmiar; i++) {
        for (int j = 0; j < i; j++) {
            double m = macierz[i][j] / macierz[j][j];
            for (int k = j; k < rozmiar + 1; k++) {
                macierz[i][k] -= m * macierz[j][k];
            }
        }
    }
}

void odwrotne(double** macierz, double* wynik, int rozmiar) {
    for (int i = rozmiar - 1; i >= 0; i--) {
        double suma = 0.0;
        for (int j = i + 1; j < rozmiar; j++) {
            suma += macierz[i][j] * wynik[j];
        }
        wynik[i] = (macierz[i][rozmiar] - suma) / macierz[i][i];
    }
}

// Funkcja obliczająca wielomian aproksymujący dla danego zestawu węzłów
void calculateApproximation(Node nodes[], int n, int m) {
    double sum_g = 0.0;
    double sum_F = 0.0;
    double** G = new double*[n];
    for (int i = 0; i < n; i++)
    {
        G[i] = new double[n+1];
    }
    double* F = new double[n];
    double* a = new double[n];
    // Obliczanie sum potrzebnych do wyznaczenia współczynników wielomianu
    cout << "Nasze g: \n";
    for (int k = 0; k < n; k++)
    {
        sum_F = 0.0;
        for (int j = 0; j < n; j++) {
            sum_g = 0.0;
            for (int i = 0; i < m; i++)
            {
                double x = nodes[i].x;
                double y = nodes[i].y;
                sum_g += pow(x, k) * pow(x, j);
            }
            G[k][j] = sum_g;
            cout << "g(" << k << "," << j << ") = " << G[k][j] << endl;

        }
        for (int i = 0; i < m; i++)
        {
            double x = nodes[i].x;
            double y = nodes[i].y;
            sum_F += pow(x, k) * y;
        }
        F[k] = sum_F;

    }
    cout << "Nasze F:\n";
    for (int k = 0; k < n; k++)
    {   
        G[k][2] = F[k];
        cout << "F(" << k << ") = " << F[k] << endl;
    }
    proste(G, n);
    odwrotne(G, a, n);
    cout << "Nasze a:\n";
    for (int i = 0; i < n; i++)
    {
        cout << "a" << i << " = " << a[i] << endl;
    }

    cout << "\nPorownanie:\n";
    cout << "Podane\tObliczone\n";
    for (int i = 0; i < m; i++)
    {
        nodes[i].wypisz();
        cout << "\t(" << i + 1 << ", " << a[1] * nodes[i].x + a[0]<< ")\n";
    }

}

int main() {
    // Przykładowe węzły aproksymacji
    Node nodes[MAX_N] = { {1.0, 2.0}, {2.0, 4.0}, {3.0, 3.0}, {4.0, 5.0}, {5.0, 6.0}, {6.0, 9.0}, {7.0, 11.0}, {8.0, 11.0} };
    // Liczba węzłów aproksymacji
    int n = 2;
    int m = 8;
    cout << "Nasze wezly: \n";
    for (int i = 0; i < m; i++)
    {

        nodes[i].wypisz();
        cout << endl;
    }
    // Obliczanie aproksymacji
    calculateApproximation(nodes, n, m);

    return 0;
}