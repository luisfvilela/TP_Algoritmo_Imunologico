"""Graficos do impacto do tamanho da populacao no CLONALG.

Le resultados/populacao.csv e resultados/populacao_evolucao.csv
(gerados pelo binario populacao) e produz:
  - resultados/populacao_acuracia.png
  - resultados/populacao_evolucao.png
"""

import csv
import os

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

DIR_RESULTADOS = "../resultados"


def ler_csv(nome):
    caminho = os.path.join(DIR_RESULTADOS, nome)
    if not os.path.exists(caminho):
        raise FileNotFoundError(
            f"{caminho} nao encontrado. Rode 'make run' antes de gerar os graficos."
        )
    with open(caminho, newline="") as arquivo:
        return list(csv.DictReader(arquivo))


def grafico_acuracia_por_populacao():
    resumo = ler_csv(os.path.join(DIR_RESULTADOS, "populacao.csv"))

    populacoes = np.array([int(l["populacao"]) for l in resumo])
    ac_treino = np.array([float(l["acuracia_treino"]) for l in resumo]) * 100.0
    ac_val = np.array([float(l["acuracia_validacao"]) for l in resumo]) * 100.0
    afinidade = np.array([float(l["afinidade_treino"]) for l in resumo]) * 100.0

    fig, eixo = plt.subplots(figsize=(10, 6))
    eixo.plot(populacoes, ac_treino, "o-", label="Acuracia treino", color="#1f77b4")
    eixo.plot(populacoes, ac_val, "s-", label="Acuracia validacao", color="#d62728")
    eixo.plot(populacoes, afinidade, "^--", label="Afinidade (treino)", color="#7f7f7f",
              alpha=0.7)

    eixo.set_title("Impacto do tamanho da populacao na acuracia (50 geracoes)")
    eixo.set_xlabel("Tamanho da populacao (P)")
    eixo.set_ylabel("Acuracia (%)")
    eixo.set_xticks(populacoes)
    eixo.set_ylim(0, 105)
    eixo.grid(alpha=0.3)
    eixo.legend(loc="lower right")
    fig.tight_layout()

    saida = os.path.join(DIR_RESULTADOS, "graficos/populacao_acuracia.png")
    fig.savefig(saida, dpi=150)
    plt.close(fig)
    print(f"Gerado: {saida}")


def grafico_evolucao_por_populacao():
    linhas = ler_csv("populacao_evolucao.csv")

    por_populacao = {}
    for l in linhas:
        p = int(l["populacao"])
        por_populacao.setdefault(p, {"geracao": [], "melhor": []})
        por_populacao[p]["geracao"].append(int(l["geracao"]))
        por_populacao[p]["melhor"].append(float(l["melhor_afinidade"]) * 100.0)

    fig, eixo = plt.subplots(figsize=(11, 7))
    cmap = plt.get_cmap("viridis")
    populacoes = sorted(por_populacao)

    for i, p in enumerate(populacoes):
        cor = cmap(i / max(1, len(populacoes) - 1))
        eixo.plot(por_populacao[p]["geracao"], por_populacao[p]["melhor"],
                  label=f"P={p}", color=cor, lw=1.5)

    eixo.set_title("Evolucao da acuracia de treino por tamanho de populacao")
    eixo.set_xlabel("Geracao")
    eixo.set_ylabel("Melhor acuracia no treino (%)")
    eixo.set_ylim(0, 105)
    eixo.grid(alpha=0.3)
    eixo.legend(ncol=2, fontsize=8, loc="lower right")
    fig.tight_layout()

    saida = os.path.join(DIR_RESULTADOS, "graficos/populacao_evolucao.png")
    fig.savefig(saida, dpi=150)
    plt.close(fig)
    print(f"Gerado: {saida}")


if __name__ == "__main__":
    grafico_acuracia_por_populacao()
    grafico_evolucao_por_populacao()