#pragma once

#include "modelo.hpp"

#include <algorithm>
#include <numeric>
#include <vector>

// ============================================================================
// CLONALG - Algoritmo de Selecao Clonal
//
// Passos (conforme roteiro):
//   1. Inicializacao: populacao inicial de P anticorpos.
//   2. Avaliacao de aptidao: afinidade de cada anticorpo (acertos no treino).
//   3. Selecao e expansao clonal: seleciona os n1 melhores e gera clones
//      proporcionalmente a afinidade: QC_k = (af_k / sum af) * Cl.
//   4. Maturacao por afinidade: hipermutacao com taxa inversamente
//      proporcional a afinidade: hiper_k = (1 - af_k/af_max) * beta.
//   5. Metadinamica: substitui n2 piores individuos por novos aleatorios.
//   6. Ciclo: repete 2-5 por G geracoes.
// ============================================================================

struct ClonalgParams {
    int populacao = 30;      // P
    int geracoes = 50;       // G
    int selecionados = 10;   // n1
    int totalClones = 50;    // Cl
    int metadinamica = 3;    // n2
    double beta = 1.0;       // fator de mutacao
    double fatorInicial = 0.5;  // dispersao em torno dos centroides iniciais
    unsigned int seed = 42;
};

struct HistoricoGeracao {
    int geracao;
    double melhorAfinidade;
    double mediaAfinidade;
};

struct ResultadoClonalg {
    Anticorpo melhor;
    std::vector<HistoricoGeracao> historico;
    std::vector<Anticorpo> melhoresPorGeracao;
};

namespace clonalg_detalhe {

inline int quantidadeClones(double afinidade, double somaAfinidades,
                            int selecionados, int totalClones) {
    if (selecionados <= 0) return 0;

    double fracao;
    if (somaAfinidades > 0.0) {
        fracao = afinidade / somaAfinidades;
    } else {
        fracao = 1.0 / static_cast<double>(selecionados);
    }

    int qc = static_cast<int>(std::lround(fracao * static_cast<double>(totalClones)));
    return std::max(1, qc);
}

inline Anticorpo hipermutar(const Anticorpo& base, double afinidade,
                            double afinidadeMaxima, double beta,
                            const std::array<double, 4>& escala,
                            const Limites& lim, Rng& rng) {
    double hiper = 1.0;
    if (afinidadeMaxima > 0.0) {
        hiper = (1.0 - afinidade / afinidadeMaxima) * beta;
    } else {
        hiper = beta;
    }
    hiper = std::max(0.0, hiper);

    Anticorpo clone = base;

    for (Prototipo& p : clone.prototipos) {
        double* genes[4] = { &p.sepalLength, &p.sepalWidth,
                             &p.petalLength, &p.petalWidth };

        for (int j = 0; j < 4; ++j) {
            double sigma = hiper * escala[j];
            double valor = rng.gaussiana(*genes[j], sigma);
            valor = std::clamp(valor, lim.min[j], lim.max[j]);
            *genes[j] = valor;
        }
    }

    return clone;
}

}  // namespace clonalg_detalhe

inline ResultadoClonalg executarClonalg(const std::vector<Iris>& treino,
                                        const ClonalgParams& params) {
    const Limites lim = calcularLimites(treino);
    const std::array<double, 4> escala = calcularDesvios(treino);

    const int P = std::max(2, params.populacao);
    const int n1 = std::clamp(params.selecionados, 1, P);
    const int n2 = std::clamp(params.metadinamica, 0, P);

    Rng rng(params.seed);

    // Passo 1: populacao inicial.
    // Parte da populacao e ancorada nos centroides das classes (com ruido),
    // o restante e gerado aleatoriamente, garantindo diversidade.
    const Anticorpo semente = calcularCentroides(treino);

    std::vector<Anticorpo> populacao;
    populacao.reserve(P);
    for (int i = 0; i < P; ++i) {
        if (i % 2 == 0) {
            populacao.push_back(anticorpoPerturbado(
                semente, escala, lim, params.fatorInicial, rng));
        } else {
            populacao.push_back(anticorpoAleatorio(rng, lim));
        }
    }

    auto avaliar = [&treino](Anticorpo& a) {
        a.afinidade = acuracia(treino, a);
    };

    for (Anticorpo& a : populacao) avaliar(a);

    // Ordena indices por afinidade decrescente.
    auto indicesOrdenados = [&populacao]() {
        std::vector<int> idx(populacao.size());
        std::iota(idx.begin(), idx.end(), 0);
        std::stable_sort(idx.begin(), idx.end(), [&populacao](int a, int b) {
            return populacao[a].afinidade > populacao[b].afinidade;
        });
        return idx;
    };

    ResultadoClonalg resultado;

    for (int geracao = 1; geracao <= params.geracoes; ++geracao) {
        // Passo 2: avaliacao de aptidao.
        for (Anticorpo& a : populacao) avaliar(a);

        std::vector<int> ordem = indicesOrdenados();

        // Passo 3: selecao dos n1 melhores e expansao clonal.
        double somaAfinidades = 0.0;
        for (int i = 0; i < n1; ++i) {
            somaAfinidades += populacao[ordem[i]].afinidade;
        }

        double afinidadeMaxima = populacao[ordem.front()].afinidade;

        std::vector<Anticorpo> clones;
        for (int i = 0; i < n1; ++i) {
            const Anticorpo& selecionado = populacao[ordem[i]];
            int qc = clonalg_detalhe::quantidadeClones(
                selecionado.afinidade, somaAfinidades, n1, params.totalClones);

            for (int c = 0; c < qc; ++c) {
                // Passo 4: maturacao por afinidade (hipermutacao).
                Anticorpo clone = clonalg_detalhe::hipermutar(
                    selecionado, selecionado.afinidade, afinidadeMaxima,
                    params.beta, escala, lim, rng);
                avaliar(clone);
                clones.push_back(std::move(clone));
            }
        }

        // Passo 5: metadinamica - novos individuos aleatorios.
        std::vector<Anticorpo> novos;
        novos.reserve(n2);
        for (int i = 0; i < n2; ++i) {
            Anticorpo novo = anticorpoAleatorio(rng, lim);
            avaliar(novo);
            novos.push_back(std::move(novo));
        }

        // Nova populacao: P melhores entre antigos + clones.
        std::vector<Anticorpo> combinada;
        combinada.reserve(populacao.size() + clones.size());
        combinada.insert(combinada.end(), populacao.begin(), populacao.end());
        combinada.insert(combinada.end(), clones.begin(), clones.end());

        std::stable_sort(combinada.begin(), combinada.end(),
                         [](const Anticorpo& a, const Anticorpo& b) {
                             return a.afinidade > b.afinidade;
                         });
        combinada.resize(P);

        // Substitui os n2 piores pelos novos aleatorios.
        for (int i = 0; i < n2 && i < static_cast<int>(novos.size()); ++i) {
            combinada[P - 1 - i] = novos[i];
        }

        populacao = std::move(combinada);

        // Passo 6: registra historico da geracao.
        std::vector<int> ordemFinal = indicesOrdenados();
        double media = 0.0;
        double melhor = populacao[ordemFinal.front()].afinidade;
        for (const Anticorpo& a : populacao) media += a.afinidade;
        media /= static_cast<double>(populacao.size());

        resultado.historico.push_back({ geracao, melhor, media });

        int melhorIdx = ordemFinal.front();
        resultado.melhoresPorGeracao.push_back(populacao[melhorIdx]);
    }

    std::vector<int> ordemFinal = indicesOrdenados();
    resultado.melhor = populacao[ordemFinal.front()];

    return resultado;
}