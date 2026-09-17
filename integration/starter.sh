#!/bin/bash
set -e

if [ -d "/usr/local/opt/libomp" ]; then
    OMP_INC="-I/usr/local/opt/libomp/include"
    OMP_LIB="-L/usr/local/opt/libomp/lib"
elif [ -d "/opt/homebrew/opt/libomp" ]; then
    OMP_INC="-I/opt/homebrew/opt/libomp/include"
    OMP_LIB="-L/opt/homebrew/opt/libomp/lib"
else
    OMP_INC=""
    OMP_LIB=""
fi

echo "1. Компиляция main.cpp"
clang++ -O3 -std=c++17 -Xpreprocessor -fopenmp \
  $OMP_INC \
  $OMP_LIB \
  -lomp \
  main.cpp -o simpson_omp

echo "2. Компиляция plot.cpp"
clang++ -O2 -std=c++17 plot.cpp -o plot

echo ""
echo "3. Запуск вычислений (OpenMP) "
./simpson_omp

echo ""
echo "4. Построение графика масштабируемости "
./plot results.csv scalability.svg

echo ""
echo "График сохранен в scalability.svg"