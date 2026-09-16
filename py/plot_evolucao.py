"""Graficos da evolucao do CLONALG para o problema Iris.

Le resultados/evolucao.csv, resultados/prototipos.csv e
resultados/centroide_acuracia.csv (gerados pelos binarios C++) e produz:
  - resultados/evolucao_acuracia.png
  - resultados/evolucao_prototipos.png
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


def grafico_acuracia():
    evolucao = ler_csv("evolucao.csv")
    centroide = ler_csv("centroide_acuracia.csv")

    geracoes = np.array([int(l["geracao"]) for l in evolucao])
    melhor = np.array([float(l["melhor_afinidade"]) for l in evolucao]) * 100.0
    media = np.array([float(l["media_afinidade"]) for l in evolucao]) * 100.0

    acc_centroide = {l["conjunto"]: float(l["acuracia"]) * 100.0 for l in centroide}

    fig, eixo = plt.subplots(figsize=(10, 6))
    eixo.plot(geracoes, melhor, label="Melhor anticorpo (treino)", color="#1f77b4", lw=2)
    eixo.plot(geracoes, media, label="Media da populacao (treino)", color="#ff7f0e",
              lw=1.6, ls="--")

    eixo.axhline(acc_centroide["treino"], color="#2ca02c", ls=":", lw=1.5,
                 label=f"Centroide - treino ({acc_centroide['treino']:.1f}%)")
    eixo.axhline(acc_centroide["validacao"], color="#d62728", ls=":", lw=1.5,
                 label=f"Centroide - validacao ({acc_centroide['validacao']:.1f}%)")

    eixo.set_title("Evolucao da acuracia no treino (CLONALG)")
    eixo.set_xlabel("Geracao")
    eixo.set_ylabel("Acuracia (%)")
    eixo.set_ylim(0, 105)
    eixo.grid(alpha=0.3)
    eixo.legend(loc="lower right", fontsize=9)
    fig.tight_layout()

    saida = os.path.join(DIR_RESULTADOS, "graficos/evolucao_acuracia.png")
    fig.savefig(saida, dpi=150)
    plt.close(fig)
    print(f"Gerado: {saida}")


def grafico_prototipos():
    linhas = ler_csv("prototipos.csv")

    geracoes_ordenadas = sorted({int(l["geracao"]) for l in linhas})
    if not geracoes_ordenadas:
        raise ValueError("prototipos.csv vazio")

    # Seleciona ate 6 geracoes distribuidas ao longo da evolucao.
    n = len(geracoes_ordenadas)
    indices = np.linspace(0, n - 1, min(6, n)).astype(int)
    geracoes = [geracoes_ordenadas[i] for i in indices]

    por_geracao = {}
    for l in linhas:
        g = int(l["geracao"])
        por_geracao.setdefault(g, []).append(l)

    cores = {"setosa": "#1f77b4", "versicolor": "#ff7f0e", "virginica": "#2ca02c"}

    fig, eixos = plt.subplots(2, 3, figsize=(15, 9), sharex=True, sharey=True)
    eixos = eixos.ravel()

    for ax, g in zip(eixos, geracoes):
        for l in por_geracao[g]:
            ax.scatter(float(l["petal_length"]), float(l["petal_width"]),
                       color=cores.get(l["especie"], "gray"), s=90,
                       edgecolor="black", zorder=3,
                       label=l["especie"])
        ax.set_title(f"Geracao {g}")
        ax.set_xlabel("Comprimento da petala")
        ax.set_ylabel("Largura da petala")
        ax.grid(alpha=0.3)

    for ax in eixos[len(geracoes):]:
        ax.axis("off")

    # Legenda unica.
    handles, labels = eixos[0].get_legend_handles_labels()
    unicos = dict(zip(labels, handles))
    fig.legend(unicos.values(), unicos.keys(), loc="upper center", ncol=3,
               bbox_to_anchor=(0.5, 0.99))
    fig.suptitle("Evolucao dos prototipos do melhor anticorpo", y=1.03)
    fig.tight_layout()

    saida = os.path.join(DIR_RESULTADOS, "graficos/evolucao_prototipos.png")
    fig.savefig(saida, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"Gerado: {saida}")


if __name__ == "__main__":
    grafico_acuracia()
    grafico_prototipos()