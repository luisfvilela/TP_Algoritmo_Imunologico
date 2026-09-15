#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

enum class Especie {
    SETOSA,
    VERSICOLOR,
    VIRGINICA
};

struct Iris {
    double sepalLength;
    double sepalWidth;
    double petalLength;
    double petalWidth;
    Especie especie;
};

std::ostream& operator<<(std::ostream& os, const Iris& iris) {
    os << "{ "
       << "sepalLength: " << iris.sepalLength
       << ", sepalWidth: " << iris.sepalWidth
       << ", petalLength: " << iris.petalLength
       << ", petalWidth: " << iris.petalWidth
       << ", especie: ";

    switch (iris.especie) {
        case Especie::SETOSA:
            os << "SETOSA";
            break;

        case Especie::VERSICOLOR:
            os << "VERSICOLOR";
            break;

        case Especie::VIRGINICA:
            os << "VIRGINICA";
            break;
    }

    os << " }";

    return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<Iris>& flores) {
    os << "[\n";

    for (const Iris& iris : flores) {
        os << "    " << iris << '\n';
    }

    os << "]";

    return os;
}

Especie converterEspecie(std::string especie) {
    // Remove \r e \n do final
    while (!especie.empty() &&
           (especie.back() == '\r' || especie.back() == '\n')) {
        especie.pop_back();
    }

    if (especie == "setosa")
        return Especie::SETOSA;

    if (especie == "versicolor")
        return Especie::VERSICOLOR;

    if (especie == "virginica")
        return Especie::VIRGINICA;

    throw std::runtime_error("Especie desconhecida: " + especie);
}

std::vector<Iris> lerIris(const std::string& nomeArquivo) {
    std::vector<Iris> flores;

    std::ifstream arquivo(nomeArquivo);

    if (!arquivo.is_open()) {
        std::cerr << "Erro ao abrir o arquivo: "
                  << nomeArquivo << std::endl;
        return flores;
    }

    std::string linha;

    // Ignora o cabeçalho
    //std::getline(arquivo, linha);

    while (std::getline(arquivo, linha)) {
        std::stringstream ss(linha);

        Iris flor;
        std::string campo;

        // sepal_length
        std::getline(ss, campo, ',');
        flor.sepalLength = std::stod(campo);

        // sepal_width
        std::getline(ss, campo, ',');
        flor.sepalWidth = std::stod(campo);

        // petal_length
        std::getline(ss, campo, ',');
        flor.petalLength = std::stod(campo);

        // petal_width
        std::getline(ss, campo, ',');
        flor.petalWidth = std::stod(campo);

        // species
        std::getline(ss, campo, ',');
        flor.especie = converterEspecie(campo);

        flores.push_back(flor);
    }

    return flores;
}
