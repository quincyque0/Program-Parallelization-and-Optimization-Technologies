#include <iostream>
#include <cmath>
#include <iomanip>
#include <fstream>
#include <vector>
#include <omp.h>

inline double g(double x) {
    double denom = 0.5 * x * x + x + 6.0;
    return (x * x * x * (x * x + 3.0 * x + 24.0)) / (denom * denom);
}

inline double f(double x) {
    return (x * x * x * x) / (0.5 * x * x + x + 6.0);
}

double simpson_omp(double a, double b, long long n) {
    double h = (b - a) / n;
    double sum_even = 0.0;
    double sum_odd = 0.0;

    #pragma omp parallel for reduction(+:sum_odd) schedule(static)
    for (long long i = 1; i < n; i += 2) {
        sum_odd += g(a + i * h);
    }

    #pragma omp parallel for reduction(+:sum_even) schedule(static)
    for (long long i = 2; i < n; i += 2) {
        sum_even += g(a + i * h);
    }

    return (h / 3.0) * (g(a) + g(b) + 4.0 * sum_odd + 2.0 * sum_even);
}

struct RunResult {
    int threads;
    double elapsed;
    double speedup;
    double efficiency;
    long long n;
    double error;
};

int main() {
    const double a = 0.4;
    const double b = 1.5;
    const double eps = 1e-6;
    const int REPEATS = 50; 

    const double exact = f(b) - f(a);
    std::cout << std::fixed << std::setprecision(10);
    std::cout << "Точное значение (по формуле Ньютона-Лейбница): " << exact << "\n\n";

    std::vector<int> thread_counts = {1, 2, 4, 8 , 16, 32, 64};
    std::vector<RunResult> results;
    double t1 = 0.0;

    for (int threads : thread_counts) {
        omp_set_num_threads(threads);

        long long n = 100000;
        double error = 1.0;
        double i_n = 0.0;
        double i_2n = 0.0;

        double start_time = omp_get_wtime();

        for (int rep = 0; rep < REPEATS; ++rep) {
            n = 100000;
            i_n = simpson_omp(a, b, n);
            error = 1.0;

            while (error > eps) {
                n *= 2;
                i_2n = simpson_omp(a, b, n);
                error = std::abs(i_2n - i_n) / 15.0;
                i_n = i_2n;
            }
        }

        double end_time = omp_get_wtime();
        double elapsed = (end_time - start_time) / REPEATS;

        if (threads == 1) t1 = elapsed;
        double speedup = t1 / elapsed;
        double efficiency = speedup / threads;

        results.push_back({threads, elapsed, speedup, efficiency, n, error});

        std::cout << "Потоков: " << std::setw(2) << threads
                  << " | Время: " << std::setw(10) << elapsed << " c"
                  << " | Ускорение: " << std::setw(6) << std::setprecision(2) << speedup
                  << " | Эффективность: " << std::setw(6) << efficiency
                  << " | Шагов n: " << std::setw(7) << n
                  << " | Погрешность: " << std::scientific << error << "\n";
        std::cout.unsetf(std::ios::scientific);
    }

    std::ofstream csv("results.csv");
    csv << "threads,time,speedup,efficiency,n,error\n";
    for (const auto& r : results) {
        csv << r.threads << "," << r.elapsed << "," << r.speedup << ","
            << r.efficiency << "," << r.n << "," << r.error << "\n";
    }
    csv.close();

    std::cout << "\nФайл results.csv успешно записан\n";
    return 0;
}