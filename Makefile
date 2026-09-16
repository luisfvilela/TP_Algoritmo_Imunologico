CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -pedantic

BIN_DIR  := bin
BINARIOS := $(BIN_DIR)/imunologico $(BIN_DIR)/centroide $(BIN_DIR)/populacao

VENV     := py/.venv
PYTHON   := $(VENV)/bin/python

.PHONY: all run centroide populacao graficos clean

all: $(BINARIOS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/imunologico: imunologico.cpp clonalg.hpp modelo.hpp iris.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) imunologico.cpp -o $@

$(BIN_DIR)/centroide: centroide.cpp modelo.hpp iris.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) centroide.cpp -o $@

$(BIN_DIR)/populacao: populacao.cpp clonalg.hpp modelo.hpp iris.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) populacao.cpp -o $@

run: all
	./$(BIN_DIR)/imunologico
	./$(BIN_DIR)/centroide
	./$(BIN_DIR)/populacao

venv: $(VENV)/bin/activate

$(VENV)/bin/activate: py/requirements.txt
	python3 -m venv $(VENV)
	$(VENV)/bin/pip install --upgrade pip
	$(VENV)/bin/pip install -r py/requirements.txt
	touch $(VENV)/bin/activate

graficos: venv
	$(PYTHON) py/plot_evolucao.py
	$(PYTHON) py/plot_populacao.py

clean:
	rm -rf $(BIN_DIR) resultados