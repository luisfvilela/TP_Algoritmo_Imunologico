#include "iris.hpp"

#include <iostream>

int main (int argc, char *argv[]) {
  //Parâmetros do modelo
  int sizeTrainSet = 100;

  //Lê o csv e carrega para o vetor 
  std::vector<Iris> fullSet = lerIris("./shuf.csv");

  //divide em set de treino e validação
  std::vector<Iris> trainSet(fullSet.begin(), fullSet.begin() + sizeTrainSet );
  std::vector<Iris> valSet(fullSet.begin() + sizeTrainSet, fullSet.end());
  
  return 0;
}


