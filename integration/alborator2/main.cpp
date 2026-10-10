#include <omp.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <print>
#include <vector>

#include "Random.h"

constexpr int N = 16000;
constexpr int M = 16000;

constexpr int MAXI = 9;
constexpr int MINI = 1;

constexpr int NUM_PROGONOV = 10;

constexpr std::array<int, 5> Threads{2, 4, 8, 16, 32};

using Matrix = std::vector<std::vector<int>>;


void simpleMatrixMultiplication(const Matrix& m, const Matrix& r,
                                Matrix& output) {
  for (size_t i = 0; i < N; i++) {
    int sum{0};
    for (size_t j = 0; j < M; j++) {
      sum += m[i][j] * r[j][0];
    }
    output[i][0] = sum;
  }
}


void rowMatrixMultiplication(const Matrix& m, const Matrix& r, Matrix& output) {
#pragma omp parallel for schedule(static)
  for (size_t i = 0; i < N; i++) {
    int sum{0};
    for (size_t j = 0; j < M; j++) {
      sum += m[i][j] * r[j][0];
    }
    output[i][0] = sum;
  }
}


void colMatrixMultiplication(const Matrix& m, const Matrix& r, Matrix& out) {
  const int P = std::min(omp_get_max_threads(), M);

  std::vector<int> flat(N, 0);
  int* pFlat = flat.data();

#pragma omp parallel for reduction(+ : pFlat[ : N])
  for (int p = 0; p < P; ++p) {
    const int j0 = M * p / P;
    const int j1 = M * (p + 1) / P;

    for (size_t i = 0; i < N; i++) {
      int sum{0};
      for (size_t j = j0; j < j1; j++) {
        sum += m[i][j] * r[j][0];
      }
      pFlat[i] = sum;
    }
  }

  for (size_t i = 0; i < N; i++) out[i][0] = flat[i];
}


void blocksMatrixMultiplication(const Matrix& m, const Matrix& r, Matrix& out,
                                int BR = 256, int BC = 256) {
  const int nbr = (N + BR - 1) / BR;
  const int nbc = (M + BC - 1) / BC;

  std::vector<int> flat(N, 0);
  int* pFlat = flat.data();

#pragma omp parallel for collapse(2) schedule(static) reduction(+ : pFlat[ : N])
  for (int bi = 0; bi < nbr; ++bi) {
    for (int bj = 0; bj < nbc; ++bj) {
      const int i0 = bi * BR;
      const int i1 = std::min(i0 + BR, N);
      const int j0 = bj * BC;
      const int j1 = std::min(j0 + BC, M);

      for (int i = i0; i < i1; ++i) {
        int sum = 0;
        for (int j = j0; j < j1; ++j) {
          sum += m[i][j] * r[j][0];
        }
        pFlat[i] += sum;
      }
    }
  }

  for (int i = 0; i < N; ++i) out[i][0] = flat[i];
}

double geomSr(std::vector<double>& res) {
  if (res.empty()) std::println("Vector is empty!");
  double logSum{};
  for (auto r : res) {
    logSum += std::log(r);
  }

  return std::exp(logSum / static_cast<double>(res.size()));
}

int main() {
  Matrix m(N, std::vector<int>(M));

  Matrix row(M, std::vector<int>(1));
  Matrix res(N, std::vector<int>(1));

  for (size_t i = 0; i < N; i++) {
    for (size_t j = 0; j < M; j++) {
      m[i][j] = Random::get(MINI, MAXI);
    }
  }
  for (size_t i = 0; i < M; i++) {
    row[i][0] = Random::get(MINI, MAXI);
  }

  double t0 = omp_get_wtime();
  simpleMatrixMultiplication(m, row, res);
  std::println("SIMPLE Seq matrix mult: {}", omp_get_wtime() - t0);

  std::vector<double> rowRes;
  std::vector<double> colRes;
  std::vector<double> blockRes;

  std::println("");
  for (auto t : Threads) {
    omp_set_num_threads(t);
    std::println("Set threads = {}", t);

    for (size_t i = 0; i < NUM_PROGONOV; i++) {
      t0 = omp_get_wtime();
      rowMatrixMultiplication(m, row, res);
      rowRes.push_back(omp_get_wtime() - t0);
    }

    for (size_t i = 0; i < NUM_PROGONOV; i++) {
      t0 = omp_get_wtime();
      colMatrixMultiplication(m, row, res);
      colRes.push_back(omp_get_wtime() - t0);
    }

    for (size_t i = 0; i < NUM_PROGONOV; i++) {
      t0 = omp_get_wtime();
      blocksMatrixMultiplication(m, row, res);
      blockRes.push_back(omp_get_wtime() - t0);
    }

    std::println("Geom sSr Row matrix mult: {:.4f}", geomSr(rowRes));
    std::println("Geom Sr Col matrix mult: {:.4f}", geomSr(colRes));
    std::println("Geom Sr Blocks matrix mult: {:.4f}", geomSr(blockRes));

std::vector<double> res;
for(auto r : rowRes) {
  res.push_back(r);
}
for(auto r : colRes) {
  res.push_back(r);
}
for(auto r : blockRes) {
  res.push_back(r);
}

  std::println("AVG {:.4f}", geomSr(res));

    std::println("#############################################");
  }
  return 0;
}