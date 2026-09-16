#include "iris.hpp"
#include "modelo.hpp"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

namespace {

constexpr int TAMANHO_TREINO = 100;

std::string caminhoSaida(const std::string& arquivo) {
    std::filesystem::create_directories("resultados");
    return "resultados/" + arquivo;
}

}  // namespace

int main() {
    // ---------------------------------------------------------------
    // Leitura da base e split treino / validacao
    // ---------------------------------------------------------------
    std::vector<Iris> completo = lerIris("./shuf.csv");
    if (completo.empty()) {
        std::cerr << "Base vazia ou arquivo nao encontrado.\n";
        return 1;
    }

    std::vector<Iris> treino;
    std::vector<Iris> validacao;
    splitTreinoValidacao(completo, TAMANHO_TREINO, treino, validacao);

    // ---------------------------------------------------------------
    // Modelo baseline: centroide de cada especie
    // ---------------------------------------------------------------
    Anticorpo centroides = calcularCentroides(treino);

    double acuraciaTreino = acuracia(treino, centroides);
    double acuraciaValidacao = acuracia(validacao, centroides);

    // ---------------------------------------------------------------
    // Saida: centroides e acuracias
    // ---------------------------------------------------------------
    {
        std::ofstream out(caminhoSaida("centroide.csv"));
        out << "especie,sepal_length,sepal_width,petal_length,petal_width\n";
        out << std::fixed << std::setprecision(6);
        for (const Prototipo& p : centroides.prototipos) {
            out << especieParaString(p.especie) << ','
                << p.sepalLength << ','
                << p.sepalWidth << ','
                << p.petalLength << ','
                << p.petalWidth << '\n';
        }
    }

    {
        std::ofstream out(caminhoSaida("centroide_acuracia.csv"));
        out << "conjunto,acuracia\n";
        out << std::fixed << std::setprecision(6);
        out << "treino," << acuraciaTreino << '\n';
        out << "validacao," << acuraciaValidacao << '\n';
    }

    std::cout << "==== Modelo por centroides ====\n";
    std::cout << "Conjunto treino:    " << treino.size() << " amostras\n";
    std::cout << "Validacao:          " << validacao.size() << " amostras\n\n";
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Acurácia no conjunto de treino:    "
              << acuraciaTreino * 100.0 << "%\n";
    std::cout << "Acurácia no conjunto de validacao: "
              << acuraciaValidacao * 100.0 << "%\n\n";

    std::cout << "Centroides:\n";
    for (const Prototipo& p : centroides.prototipos) {
        std::cout << "  " << std::setw(11) << std::left
                  << especieParaString(p.especie) << std::right
                  << " -> [ " << p.sepalLength << ", " << p.sepalWidth
                  << ", " << p.petalLength << ", " << p.petalWidth << " ]\n";
    }

    std::cout << "\nArquivos gerados em resultados/centroide.csv"
                 " e resultados/centroide_acuracia.csv\n";

    return 0;
}