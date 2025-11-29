# TP2 - Renderização 3D com Iluminação Phong/Blinn-Phong

Aplicação OpenGL que renderiza um modelo 3D de um veado com iluminação realista, suportando os modelos de iluminação Phong e Blinn-Phong.

## Compilação e Execução

### Compilar
```bash
cd build
cmake ..
make
```

### Executar
```bash
./tp2
```

## Controles

### Câmera
- **W/A/S/D** - Mover câmera (frente/esquerda/trás/direita)
- **Rato** - Olhar à volta (mover rato)
- **Scroll** - Zoom in/out

### Modelo
- **Setas (↑↓←→)** - Rodar modelo

### Luz
- **ESPAÇO** - Pausar/retomar rotação da luz
- **+/-** - Aumentar/diminuir velocidade da luz

### Visualização
- **F** - Alternar modo wireframe
- **B** - Alternar Blinn-Phong
- **R** - Reiniciar tudo
- **ESC** - Sair

## Descrição

O projeto renderiza um veado 3D carregado de um ficheiro OBJ com materiais definidos em MTL. A cena inclui:

- **Modelo**: Veado carregado de `deer.obj` com propriedades de material
- **Iluminação**: Luz pontual que orbita à volta do modelo
- **Shaders**: Implementação de Phong e Blinn-Phong
- **Câmera**: Sistema FPS com WASD + rato
- **Modos**: Wireframe, Blinn-Phong, rotação de luz

### Diferença Phong vs Blinn-Phong

- **Phong**: Reflexo agudo e realista em ângulos pequenos
- **Blinn-Phong**: Brilho mais suave e mais realista em geral (recomendado)

Pressione **B** durante a execução para alternar entre os dois modos e veja a diferença!

## Ficheiros Importantes

- `src/main.cpp` - Código principal
- `shaders/phong.vert/frag` - Shaders de iluminação
- `shaders/simple.vert/frag` - Shaders da fonte de luz
- `deer.obj` / `deer.mtl` - Modelo e materiais
- `common/Mesh.hpp` - Classe para renderização de malhas
- `common/objloader.*` - Carregador de ficheiros OBJ/MTL

![blinn phong](image.png)