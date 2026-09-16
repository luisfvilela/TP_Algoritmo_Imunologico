#include "clonalg.hpp"
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

void escreverCabecalho(std::ofstream& out, const std::string& cabecalho) {
    out << cabecalho << '\n';
}

}  // namespace

int main() {
    // ---------------------------------------------------------------
    // Parametros do modelo
    // ---------------------------------------------------------------
    ClonalgParams params;
    params.populacao = 30;
    params.geracoes = 50;
    params.selecionados = 10;   // n1 (~30% de P)
    params.totalClones = 50;    // Cl
    params.metadinamica = 3;    // n2 (~10% de P)
    params.beta = 1.0;          // fator de mutacao
    params.seed = 42;

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

    std::cout << "Base total:      " << completo.size() << " amostras\n";
    std::cout << "Conjunto treino: " << treino.size() << " amostras\n";
    std::cout << "Validacao:       " << validacao.size() << " amostras\n";
    std::cout << "Populacao (P):   " << params.populacao
              << " | Geracoes: " << params.geracoes
              << " | n1: " << params.selecionados
              << " | Cl: " << params.totalClones
              << " | n2: " << params.metadinamica
              << " | beta: " << params.beta << "\n\n";

    // ---------------------------------------------------------------
    // Treino (a afinidade e medida apenas no conjunto de treino)
    // ---------------------------------------------------------------
    ResultadoClonalg resultado = executarClonalg(treino, params);

    // A validacao e usada APENAS na avaliacao final.
    double acuraciaTreino = acuracia(treino, resultado.melhor);
    double acuraciaValidacao = acuracia(validacao, resultado.melhor);

    // ---------------------------------------------------------------
    // Saida: evolucao da acuracia de treino ao longo das geracoes
    // ---------------------------------------------------------------
    {
        std::ofstream out(caminhoSaida("evolucao.csv"));
        escreverCabecalho(out, "geracao,melhor_afinidade,media_afinidade");
        out << std::fixed << std::setprecision(6);
        for (const HistoricoGeracao& h : resultado.historico) {
            out << h.geracao << ','
                << h.melhorAfinidade << ','
                << h.mediaAfinidade << '\n';
        }
    }

    // ---------------------------------------------------------------
    // Saida: evolucao dos genes do melhor anticorpo (modelo) por geracao
    // ---------------------------------------------------------------
    {
        std::ofstream out(caminhoSaida("prototipos.csv"));
        escreverCabecalho(out,
            "geracao,especie,sepal_length,sepal_width,petal_length,petal_width");
        out << std::fixed << std::setprecision(6);
        for (std::size_t i = 0; i < resultado.melhoresPorGeracao.size(); ++i) {
            const Anticorpo& modelo = resultado.melhoresPorGeracao[i];
            for (const Prototipo& p : modelo.prototipos) {
                out << (i + 1) << ','
                    << especieParaString(p.especie) << ','
                    << p.sepalLength << ','
                    << p.sepalWidth << ','
                    << p.petalLength << ','
                    << p.petalWidth << '\n';
            }
        }
    }

    // ---------------------------------------------------------------
    // Impressao final
    // ---------------------------------------------------------------
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "==== Resultado final ====\n";
    std::cout << "Acurácia no conjunto de treino:    "
              << acuraciaTreino * 100.0 << "%\n";
    std::cout << "Acurácia no conjunto de validacao: "
              << acuraciaValidacao * 100.0 << "%\n\n";

    std::cout << "Melhor anticorpo (3 prototipos):\n";
    for (const Prototipo& p : resultado.melhor.prototipos) {
        std::cout << "  " << std::setw(11) << std::left
                  << especieParaString(p.especie) << std::right
                  << " -> [ " << p.sepalLength << ", " << p.sepalWidth
                  << ", " << p.petalLength << ", " << p.petalWidth << " ]\n";
    }

    std::cout << "\nArquivos gerados em resultados/evolucao.csv"
                 " e resultados/prototipos.csv\n";

    return 0;
}