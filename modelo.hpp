#pragma once

#include "iris.hpp"

#include <array>
#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

// ============================================================================
// Representacao do modelo (anticorpo)
//
// Um anticorpo e o modelo de predicao: 3 prototipos (um por especie), cada um
// com 4 features. Portanto 12 genes (3 x 4). A especie de cada prototipo e
// fixa pelo indice (0=SETOSA, 1=VERSICOLOR, 2=VIRGINICA), apenas os genes
// numericos sao mutados.
// ============================================================================

struct Prototipo {
    double sepalLength;
    double sepalWidth;
    double petalLength;
    double petalWidth;
    Especie especie;
};

struct Anticorpo {
    std::array<Prototipo, 3> prototipos;
    double afinidade = 0.0;
};

// Limites (min/max) por feature, usados para gerar anticorpos aleatorios e
// para manter os genes em uma faixa plausivel apos a mutacao.
struct Limites {
    std::array<double, 4> min;
    std::array<double, 4> max;
};

// Gerador de numeros aleatorios com seed fixa (reprodutibilidade).
struct Rng {
    std::mt19937 gen;

    explicit Rng(unsigned int seed) : gen(seed) {}

    double uniforme(double a, double b) {
        return std::uniform_real_distribution<double>(a, b)(gen);
    }

    double gaussiana(double media, double sigma) {
        if (sigma <= 0.0) return media;
        return std::normal_distribution<double>(media, sigma)(gen);
    }

    int inteiro(int a, int b) {
        return std::uniform_int_distribution<int>(a, b)(gen);
    }
};

// ---------------------------------------------------------------------------
// Conversao entre Especie, indice e string
// ---------------------------------------------------------------------------

inline int especieIndex(Especie especie) {
    switch (especie) {
        case Especie::SETOSA:     return 0;
        case Especie::VERSICOLOR: return 1;
        case Especie::VIRGINICA:  return 2;
    }
    throw std::runtime_error("Especie desconhecida");
}

inline Especie indexEspecie(int indice) {
    switch (indice) {
        case 0: return Especie::SETOSA;
        case 1: return Especie::VERSICOLOR;
        case 2: return Especie::VIRGINICA;
    }
    throw std::runtime_error("Indice de especie invalido: " + std::to_string(indice));
}

inline std::string especieParaString(Especie especie) {
    switch (especie) {
        case Especie::SETOSA:     return "setosa";
        case Especie::VERSICOLOR: return "versicolor";
        case Especie::VIRGINICA:  return "virginica";
    }
    return "desconhecida";
}

// ---------------------------------------------------------------------------
// Distancia euclidiana
// ---------------------------------------------------------------------------

inline double distancia(const Prototipo& p, const Iris& flor) {
    double ds = p.sepalLength - flor.sepalLength;
    double dw = p.sepalWidth  - flor.sepalWidth;
    double dl = p.petalLength - flor.petalLength;
    double dp = p.petalWidth  - flor.petalWidth;
    return std::sqrt(ds * ds + dw * dw + dl * dl + dp * dp);
}

// ---------------------------------------------------------------------------
// Predicao (1-NN entre os 3 prototipos do anticorpo)
// ---------------------------------------------------------------------------

inline Especie classificar1NN(const Iris& amostra, const Anticorpo& anticorpo) {
    double melhorDistancia = std::numeric_limits<double>::infinity();
    Especie melhorEspecie = anticorpo.prototipos.front().especie;

    for (const Prototipo& p : anticorpo.prototipos) {
        double d = distancia(p, amostra);
        if (d < melhorDistancia) {
            melhorDistancia = d;
            melhorEspecie = p.especie;
        }
    }

    return melhorEspecie;
}

// Proporcao de acertos do anticorpo sobre a base informada.
inline double acuracia(const std::vector<Iris>& base, const Anticorpo& anticorpo) {
    if (base.empty()) return 0.0;

    int acertos = 0;
    for (const Iris& amostra : base) {
        if (classificar1NN(amostra, anticorpo) == amostra.especie) {
            ++acertos;
        }
    }

    return static_cast<double>(acertos) / static_cast<double>(base.size());
}

// ---------------------------------------------------------------------------
// Construcao de anticorpos
// ---------------------------------------------------------------------------

inline Limites calcularLimites(const std::vector<Iris>& base) {
    Limites lim;
    lim.min = { std::numeric_limits<double>::infinity(),
                std::numeric_limits<double>::infinity(),
                std::numeric_limits<double>::infinity(),
                std::numeric_limits<double>::infinity() };
    lim.max = { -std::numeric_limits<double>::infinity(),
                -std::numeric_limits<double>::infinity(),
                -std::numeric_limits<double>::infinity(),
                -std::numeric_limits<double>::infinity() };

    for (const Iris& flor : base) {
        const double valores[4] = { flor.sepalLength, flor.sepalWidth,
                                    flor.petalLength, flor.petalWidth };
        for (int j = 0; j < 4; ++j) {
            lim.min[j] = std::min(lim.min[j], valores[j]);
            lim.max[j] = std::max(lim.max[j], valores[j]);
        }
    }

    return lim;
}

// Desvio-padrao por feature da base (escala da mutacao).
inline std::array<double, 4> calcularDesvios(const std::vector<Iris>& base) {
    std::array<double, 4> media = { 0.0, 0.0, 0.0, 0.0 };
    std::array<double, 4> desvio = { 0.0, 0.0, 0.0, 0.0 };

    if (base.empty()) return desvio;

    for (const Iris& flor : base) {
        const double valores[4] = { flor.sepalLength, flor.sepalWidth,
                                    flor.petalLength, flor.petalWidth };
        for (int j = 0; j < 4; ++j) media[j] += valores[j];
    }
    for (int j = 0; j < 4; ++j) media[j] /= static_cast<double>(base.size());

    for (const Iris& flor : base) {
        const double valores[4] = { flor.sepalLength, flor.sepalWidth,
                                    flor.petalLength, flor.petalWidth };
        for (int j = 0; j < 4; ++j) {
            double d = valores[j] - media[j];
            desvio[j] += d * d;
        }
    }
    for (int j = 0; j < 4; ++j) {
        desvio[j] = std::sqrt(desvio[j] / static_cast<double>(base.size()));
    }

    return desvio;
}

inline Anticorpo anticorpoAleatorio(Rng& rng, const Limites& lim) {
    Anticorpo anticorpo;

    for (int k = 0; k < 3; ++k) {
        anticorpo.prototipos[k].especie = indexEspecie(k);
        anticorpo.prototipos[k].sepalLength = rng.uniforme(lim.min[0], lim.max[0]);
        anticorpo.prototipos[k].sepalWidth  = rng.uniforme(lim.min[1], lim.max[1]);
        anticorpo.prototipos[k].petalLength = rng.uniforme(lim.min[2], lim.max[2]);
        anticorpo.prototipos[k].petalWidth  = rng.uniforme(lim.min[3], lim.max[3]);
    }

    return anticorpo;
}

// Variacao de um anticorpo de referencia aplicando ruido gaussiano por gene.
inline Anticorpo anticorpoPerturbado(const Anticorpo& referencia,
                                     const std::array<double, 4>& escala,
                                     const Limites& lim, double fator, Rng& rng) {
    Anticorpo anticorpo = referencia;

    for (Prototipo& p : anticorpo.prototipos) {
        double* genes[4] = { &p.sepalLength, &p.sepalWidth,
                             &p.petalLength, &p.petalWidth };
        for (int j = 0; j < 4; ++j) {
            double valor = rng.gaussiana(*genes[j], fator * escala[j]);
            *genes[j] = std::clamp(valor, lim.min[j], lim.max[j]);
        }
    }

    return anticorpo;
}

// Centroide de cada especie: modelo baseline (sem algoritmo imunologico).
inline Anticorpo calcularCentroides(const std::vector<Iris>& base) {
    Anticorpo modelo;

    for (int k = 0; k < 3; ++k) {
        modelo.prototipos[k].especie = indexEspecie(k);
        modelo.prototipos[k].sepalLength = 0.0;
        modelo.prototipos[k].sepalWidth  = 0.0;
        modelo.prototipos[k].petalLength = 0.0;
        modelo.prototipos[k].petalWidth  = 0.0;
    }

    std::array<int, 3> contagem = { 0, 0, 0 };

    for (const Iris& flor : base) {
        int k = especieIndex(flor.especie);
        modelo.prototipos[k].sepalLength += flor.sepalLength;
        modelo.prototipos[k].sepalWidth  += flor.sepalWidth;
        modelo.prototipos[k].petalLength += flor.petalLength;
        modelo.prototipos[k].petalWidth  += flor.petalWidth;
        ++contagem[k];
    }

    for (int k = 0; k < 3; ++k) {
        if (contagem[k] > 0) {
            double n = static_cast<double>(contagem[k]);
            modelo.prototipos[k].sepalLength /= n;
            modelo.prototipos[k].sepalWidth  /= n;
            modelo.prototipos[k].petalLength /= n;
            modelo.prototipos[k].petalWidth  /= n;
        }
    }

    return modelo;
}

// ---------------------------------------------------------------------------
// Split determinista treino / validacao
// ---------------------------------------------------------------------------

inline void splitTreinoValidacao(const std::vector<Iris>& completo,
                                 int tamanhoTreino,
                                 std::vector<Iris>& treino,
                                 std::vector<Iris>& validacao) {
    if (tamanhoTreino < 0 || tamanhoTreino > static_cast<int>(completo.size())) {
        throw std::runtime_error("Tamanho de treino invalido");
    }

    treino.assign(completo.begin(), completo.begin() + tamanhoTreino);
    validacao.assign(completo.begin() + tamanhoTreino, completo.end());
}