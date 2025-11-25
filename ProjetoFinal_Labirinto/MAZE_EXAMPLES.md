# Mapas de Labirinto Alternativos

Este arquivo contém mapas alternativos que podem ser usados no jogo.
Para usar, substitua o array `maze_map` em `src/main.c`.

## Legenda
- `0` = Caminho livre (branco)
- `1` = Parede (cinza)
- `2` = Buraco (preto) - perde vida
- `3` = Objetivo (verde) - vitória

## Mapa 1: Fácil - Corredor Simples

Bom para testar física e controles.

```c
static const uint8_t maze_map[MAZE_HEIGHT][MAZE_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
    {1,0,1,2,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1},
    {1,0,1,1,1,1,1,1,1,1,0,1,0,1,0,1},
    {1,0,1,0,0,0,0,2,0,1,0,1,0,1,0,1},
    {1,0,1,0,1,1,1,1,0,1,0,1,0,1,0,1},
    {1,0,1,0,0,0,0,0,0,1,0,1,0,1,0,1},
    {1,0,1,1,1,1,1,1,1,1,0,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,1,0,1,1,1,1,1,1,1,1,1,1,3,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};
// Início: (1, 1) | Objetivo: (14, 14)
```

## Mapa 2: Médio - Campo de Buracos

Muitos buracos para desviar.

```c
static const uint8_t maze_map[MAZE_HEIGHT][MAZE_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,1,0,0,2,0,0,1,0,0,0,0,1},
    {1,0,1,0,1,0,1,1,1,0,1,0,1,1,0,1},
    {1,0,1,0,0,0,0,2,0,0,0,0,0,1,0,1},
    {1,0,1,1,1,1,1,1,1,0,1,1,0,1,0,1},
    {1,0,0,2,0,0,0,0,1,0,0,1,0,0,0,1},
    {1,1,1,1,1,1,1,0,1,1,0,1,1,1,0,1},
    {1,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1},
    {1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,1},
    {1,0,1,2,0,0,0,0,0,2,0,0,0,2,0,1},
    {1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
    {1,0,2,0,0,0,0,0,0,2,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,1,1,3,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};
// Início: (1, 1) | Objetivo: (14, 14)
```

## Mapa 3: Difícil - Espiral

Caminho em espiral até o centro.

```c
static const uint8_t maze_map[MAZE_HEIGHT][MAZE_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,1,0,1,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,1,0,1,2,0,0,0,2,0,1,0,1,0,1},
    {1,0,1,0,1,0,1,1,1,1,0,1,0,1,0,1},
    {1,0,1,0,1,0,1,3,2,1,0,1,0,1,0,1},
    {1,0,1,0,1,0,1,1,1,1,0,1,0,1,0,1},
    {1,0,1,0,1,2,0,0,0,2,0,1,0,1,0,1},
    {1,0,1,0,1,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};
// Início: (1, 1) | Objetivo: (7, 7) - Centro!
// Nota: Altere também:
// maze.goal_x = 7;
// maze.goal_y = 7;
```

## Mapa 4: Muito Difícil - Labirinto Complexo

Caminho tortuoso com muitos buracos.

```c
static const uint8_t maze_map[MAZE_HEIGHT][MAZE_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,1,1,1,0,1},
    {1,0,1,0,0,0,1,0,0,0,0,2,0,1,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,0,0,0,2,0,0,0,0,0,0,0,1,0,1},
    {1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1},
    {1,0,0,2,0,0,0,0,0,2,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
    {1,0,1,0,0,0,0,2,0,0,0,0,0,1,0,1},
    {1,0,1,0,1,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,1,0,0,0,0,0,0,2,0,0,0,1,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
    {1,0,0,0,1,2,0,0,0,2,0,0,0,0,0,1},
    {1,0,1,0,1,1,1,1,1,1,1,1,1,1,3,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};
// Início: (1, 1) | Objetivo: (14, 14)
```

## Mapa 5: Expert - Campo Aberto com Buracos

Requer controle preciso.

```c
static const uint8_t maze_map[MAZE_HEIGHT][MAZE_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,2,0,2,0,2,0,0,2,0,2,0,2,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,2,0,2,0,2,0,0,2,0,2,0,2,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,2,0,2,0,2,0,0,2,0,2,0,2,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,2,0,2,0,2,0,0,2,0,2,0,2,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,2,0,2,0,2,0,0,2,0,2,0,2,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,2,0,2,0,2,0,0,2,0,2,0,2,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,2,0,2,0,2,0,0,2,0,2,0,2,3,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};
// Início: (1, 1) | Objetivo: (14, 14)
// Dica: Campo aberto, mas cheio de buracos em grid
```

## Mapa 6: Mini - Teste Rápido

Pequeno e rápido para testes.

```c
static const uint8_t maze_map[MAZE_HEIGHT][MAZE_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,1,1,1,1,1,1,1,1,1,0,0,1},
    {1,0,0,1,0,0,0,2,0,0,0,0,1,0,0,1},
    {1,0,0,1,0,1,1,1,1,1,1,0,1,0,0,1},
    {1,0,0,1,0,1,0,0,0,0,1,0,1,0,0,1},
    {1,0,0,1,0,1,0,1,1,0,1,0,1,0,0,1},
    {1,0,0,1,0,1,0,1,1,0,1,0,1,0,0,1},
    {1,0,0,1,0,1,0,0,0,0,1,0,1,0,0,1},
    {1,0,0,1,0,1,1,1,1,1,1,0,1,0,0,1},
    {1,0,0,1,0,0,0,2,0,0,0,0,1,0,0,1},
    {1,0,0,1,1,1,1,1,1,1,1,1,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,3,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};
// Início: (1, 1) | Objetivo: (14, 13)
```

## Como Usar um Mapa Alternativo

1. Abra `src/main.c`
2. Localize a seção `/* ==== Labirinto ==== */`
3. Encontre o array `maze_map`
4. Substitua pelo mapa desejado deste arquivo
5. Se necessário, ajuste início e objetivo em `maze_init()`:
   ```c
   maze.start_x = 1;  // coluna inicial
   maze.start_y = 1;  // linha inicial
   maze.goal_x = 14;  // coluna objetivo
   maze.goal_y = 14;  // linha objetivo
   ```
6. Recompile e faça upload:
   ```bash
   pio run -t upload
   ```

## Criando Seus Próprios Mapas

### Regras:
1. Sempre 16x16 células
2. Bordas devem ser paredes (1)
3. Início e objetivo em células vazias (0)
4. Pelo menos um caminho válido do início ao objetivo
5. Buracos (2) opcionais para dificuldade

### Editor Visual (opcional):
Você pode usar um editor de texto ou planilha para visualizar:
- Excel/Calc: Usar formatação condicional
- Online: Grid editors ou pixel art tools
- Papel: Desenhar em papel quadriculado

### Dicas de Design:
- Início: Canto superior esquerdo (1,1)
- Objetivo: Canto inferior direito (14,14) ou centro
- Buracos: Espaçados estrategicamente
- Dificuldade: Mais paredes = mais difícil
- Teste: Sempre testar o caminho é válido

Divirta-se criando seus labirintos! 🎮
