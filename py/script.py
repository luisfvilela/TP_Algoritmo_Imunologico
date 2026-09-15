from sklearn.datasets import load_iris
import csv

iris = load_iris()

with open("iris.csv", "w", newline="") as arquivo:
    escritor = csv.writer(arquivo)

    # Cabeçalho
    escritor.writerow([
        "sepal_length",
        "sepal_width",
        "petal_length",
        "petal_width",
        "species"
    ])

    # Dados
    for i in range(len(iris.data)):
        escritor.writerow([
            iris.data[i][0],
            iris.data[i][1],
            iris.data[i][2],
            iris.data[i][3],
            iris.target_names[iris.target[i]]
        ])

print("iris.csv criado com sucesso!")
