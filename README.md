# TP2 - Renderização de Modelo 3D com Iluminação Dinâmica

Projeto de Computação Gráfica que renderiza um modelo 3D de veado (.obj) com iluminação Phong e rotação dinâmica da fonte de luz.

## 📋 Descrição

Este projeto implementa um visualizador 3D interativo usando OpenGL que:
- Carrega e renderiza modelos 3D em formato .obj
- Aplica iluminação Phong em tempo real
- Permite rotação e translação do modelo
- Possui uma fonte de luz animada que roda ao redor do objeto
- Visualiza a posição da luz como uma esfera amarela brilhante
- Oferece controles para ajustar a velocidade e pausar a rotação da luz

## 🛠️ Tecnologias Utilizadas

- **C++** - Linguagem de programação
- **OpenGL 4.1** - API de renderização gráfica
- **GLFW** - Gestão de janelas e input
- **GLEW** - Carregamento de extensões OpenGL
- **GLM** - Biblioteca de matemática para gráficos
- **CMake** - Sistema de build

## 📦 Compilação

```bash
make
```

O projeto utiliza CMake como sistema de build. Certifica-te que tens todas as dependências instaladas.

## 🚀 Execução

```bash
./tp2
```

## 🎮 Controles

### Controles do Modelo (Veado)

#### Rotação (Mouse)
- **Clique esquerdo + arrastar**: Roda o modelo 3D

#### Translação (Teclado)
- **W** ou **↑**: Move para cima
- **S** ou **↓**: Move para baixo
- **A** ou **←**: Move para a esquerda
- **D** ou **→**: Move para a direita
- **Q** ou **Shift Direito**: Move para frente (eixo Z+)
- **E** ou **Ctrl Direito**: Move para trás (eixo Z-)

### Controles da Luz

#### Velocidade de Rotação
- **+** (ou **=**): Aumenta a velocidade de rotação da luz (+0.5)
- **-**: Diminui a velocidade de rotação da luz (-0.5, mínimo 0.0)
- **R**: Reseta a velocidade para o padrão (1.0)

#### Pausa/Retoma
- **SPACE** (Barra de Espaço): Pausa ou retoma a rotação da luz

Todas as alterações de velocidade e estado da luz são mostradas no console.

## 🔆 Sistema de Iluminação

### Modelo de Iluminação
O projeto utiliza o **modelo de iluminação Phong**, que inclui:
- **Componente Ambiente (Ka)**: Luz difusa constante
- **Componente Difusa (Kd)**: Reflexão baseada no ângulo de incidência
- **Componente Especular (Ks)**: Reflexos brilhantes (shininess = 32)

### Comportamento da Luz
- A luz roda em círculo ao redor do veado
- Raio da órbita: 2.0 unidades
- Altura fixa: 1.0 (nível do eixo Y)
- A posição é calculada usando coordenadas polares (cos θ, sin θ)
- Visualizada como uma esfera amarela brilhante de raio 0.05

## 📁 Estrutura do Projeto

```
ExercicioPrtico2/
├── src/
│   ├── main.cpp          # Código principal da aplicação
│   ├── shader.cpp        # Carregamento e compilação de shaders
│   └── objloader.cpp     # Carregamento de ficheiros .obj
├── common/
│   ├── shader.hpp        # Headers para shaders
│   └── objloader.hpp     # Headers para loader .obj
├── shaders/
│   ├── phong.vert        # Vertex shader Phong
│   ├── phong.frag        # Fragment shader Phong
│   ├── simple.vert       # Vertex shader simples (luz)
│   └── simple.frag       # Fragment shader simples (luz)
├── deer.obj              # Modelo 3D do veado
├── CMakeLists.txt        # Configuração CMake
└── README.md             # Este ficheiro
```

## 🎨 Detalhes Técnicos

### Renderização
- **Buffer interleaved**: Vértices e normais armazenados sequencialmente
- **Normalização automática**: O modelo é escalado e centrado automaticamente
- **Depth test**: Ativado para renderização 3D correta
- **Viewport dinâmico**: Ajusta-se ao redimensionamento da janela

### Shaders
- **phong.vert/frag**: Implementa iluminação Phong completa no vertex shader
- **simple.vert/frag**: Renderiza a esfera da luz com cor sólida

### Animação
- **Frame-independent**: Rotação baseada em incrementos por frame
- **Velocidade configurável**: De 0.0 (parada) até valores arbitrários
- **Estado pausável**: Mantém o ângulo atual quando pausado

## 🐛 Resolução de Problemas

### A janela não abre
- Verifica se tens drivers OpenGL 4.1+ instalados
- Confirma que GLFW está corretamente instalado

### O modelo não aparece
- Certifica-te que `deer.obj` está no diretório correto
- Verifica o console para mensagens de erro

### Shaders não compilam
- Confirma que a pasta `shaders/` foi copiada para o diretório de execução
- Verifica a compatibilidade com OpenGL 4.1

## 📝 Notas

- A aplicação corre em modo de core profile OpenGL 4.1
- Compatível com sistemas que suportam OpenGL Forward Compatible
- Os printf's de debug aparecem no console/terminal

## 👨‍💻 Autor

Projeto desenvolvido no âmbito da disciplina de Computação Gráfica.

---

**Diverte-te a explorar o modelo 3D! 🦌✨**
