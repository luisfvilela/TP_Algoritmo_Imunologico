#include "clonalg.hpp"
#include "iris.hpp"
#include "modelo.hpp"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr int TAMANHO_TREINO = 100;
constexpr int GERACOES = 50;

std::string caminhoSaida(const std::string& arquivo) {
    std::filesystem::create_directories("resultados");
    return "resultados/" + arquivo;
}

}  // namespace

int main() {
    // ---------------------------------------------------------------
    // Leitura da base e split
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
    // Varredura do tamanho da populacao entre 10 e 50 (passo 5)
    // ---------------------------------------------------------------
    std::vector<int> populacoes;
    for (int p = 10; p <= 50; p += 5) populacoes.push_back(p);

    std::ofstream resumo(caminhoSaida("populacao.csv"));
    resumo << "populacao,afinidade_treino,acuracia_treino,acuracia_validacao\n";
    resumo << std::fixed << std::setprecision(6);

    std::ofstream evolucao(caminhoSaida("populacao_evolucao.csv"));
    evolucao << "populacao,geracao,melhor_afinidade,media_afinidade\n";
    evolucao << std::fixed << std::setprecision(6);

    unsigned int seed = 42;  // mesma seed para todas as populacoes

    std::cout << "==== Impacto do tamanho da populacao ====\n";
    std::cout << "Geracoes: " << GERACOES << " | n1/Cl/n2 proporcionais a P\n\n";

    std::cout << std::fixed << std::setprecision(4);
    std::cout << std::setw(10) << "Populacao"
              << std::setw(18) << "Ac. treino"
              << std::setw(20) << "Ac. validacao" << '\n';

    for (int P : populacoes) {
        ClonalgParams params;
        params.populacao = P;
        params.geracoes = GERACOES;
        params.selecionados = std::max(1, static_cast<int>(std::lround(0.30 * P)));
        params.totalClones = 50;
        params.metadinamica = std::max(1, static_cast<int>(std::lround(0.10 * P)));
        params.beta = 1.0;
        params.seed = seed;

        ResultadoClonalg r = executarClonalg(treino, params);

        double acTreino = acuracia(treino, r.melhor);
        double acValidacao = acuracia(validacao, r.melhor);

        resumo << P << ','
               << r.melhor.afinidade << ','
               << acTreino << ','
               << acValidacao << '\n';

        for (const HistoricoGeracao& h : r.historico) {
            evolucao << P << ','
                     << h.geracao << ','
                     << h.melhorAfinidade << ','
                     << h.mediaAfinidade << '\n';
        }

        std::cout << std::setw(10) << P
                  << std::setw(18) << acTreino * 100.0
                  << std::setw(20) << acValidacao * 100.0 << '\n';
    }

    std::cout << "\nArquivos gerados em resultados/populacao.csv"
                 " e resultados/populacao_evolucao.csv\n";

    return 0;
}