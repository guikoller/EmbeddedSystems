# 📚 Índice de Documentação - Simulador de Labirinto

Bem-vindo ao projeto! Este arquivo ajuda você a navegar pela documentação.

---

## 🚀 Para Começar (Essencial)

### 1️⃣ **[README.md](README.md)** - Leia Primeiro!
- Visão geral do projeto
- ✅ **REQUISITOS OBRIGATÓRIOS ATENDIDOS**
- Hardware necessário
- Objetivos do jogo
- Como compilar e fazer upload

### 2️⃣ **[QUICKSTART.md](QUICKSTART.md)** - Setup Rápido ⚡
- Passo a passo para começar imediatamente
- Comandos essenciais
- Conexões de hardware
- Troubleshooting básico

**👉 COMECE POR AQUI SE QUER RODAR O PROJETO AGORA!**

---

## 📋 Requisitos do Projeto

### ✅ **[REQUISITOS_ATENDIDOS.md](REQUISITOS_ATENDIDOS.md)** - Checklist Completo
- ✅ Mínimo 3 threads/tarefas (5 implementadas)
- ✅ Máquina de estados (7 estados)
- ✅ Objetos de sincronização (Queue, Mutex, Semáforo, Timer)
- ✅ Temporização determinística (vTaskDelayUntil)
- ✅ Relógio hh:mm:ss no display

**Para**: Verificar atendimento aos requisitos obrigatórios

---

## 📖 Documentação Técnica

### 📄 **[DOCUMENTATION.md](DOCUMENTATION.md)** - Documentação Completa
- Arquitetura detalhada do software
- FreeRTOS tasks explicadas
- Modelo de física da bolinha
- Estrutura do labirinto
- Parâmetros ajustáveis
- Referências técnicas

**Para**: Entender como o sistema funciona internamente

---

### 📊 **[DIAGRAMS.md](DIAGRAMS.md)** - Diagramas Visuais
- Arquitetura do sistema
- Fluxogramas do jogo
- Diagrama de estados
- Comunicação entre tasks
- Pinout detalhado
- Timing diagrams

**Para**: Visualizar a arquitetura e fluxos do sistema

---

### 📝 **[PROJECT_SUMMARY.md](PROJECT_SUMMARY.md)** - Resumo Executivo
- Resumo de uma página
- Especificações técnicas rápidas
- Tabelas de referência
- Links para outros documentos

**Para**: Visão geral rápida do projeto completo

---

## 🎮 Gameplay e Customização

### 🗺️ **[MAZE_EXAMPLES.md](MAZE_EXAMPLES.md)** - Mapas Alternativos
- 6 mapas prontos para usar
- Diferentes níveis de dificuldade
- Como criar seus próprios mapas
- Dicas de design de labirinto

**Para**: Mudar ou criar novos labirintos

---

## 🔧 Ajuda e Solução de Problemas

### 🐛 **[TROUBLESHOOTING.md](TROUBLESHOOTING.md)** - Guia de Problemas
- Problemas de compilação
- Problemas de hardware
- Problemas de comportamento
- Checklist de diagnóstico
- Soluções passo a passo

**Para**: Resolver erros e problemas comuns

---

### 🔌 **[lib/FREERTOS_SETUP.md](lib/FREERTOS_SETUP.md)** - Setup FreeRTOS
- Como instalar FreeRTOS-Kernel
- Opções de instalação
- Troubleshooting específico do FreeRTOS

**Para**: Configurar o FreeRTOS pela primeira vez

---

## 📁 Estrutura de Código

### 🎯 Arquivos Principais

#### **[src/main.c](src/main.c)** ⭐ - Código Principal
```c
/* Estrutura do arquivo: */

1. Includes e Defines
2. Estruturas de dados (Ball, Maze, GameState)
3. Variáveis globais
4. Definição do labirinto (maze_map)
5. Funções de física da bolinha
6. Funções de renderização
7. Tasks do FreeRTOS:
   - IMU_Task: Lê MPU6050
   - GameLogic_Task: Física e lógica do jogo
   - Display_Task: Renderização
   - Button_Task: Entrada do usuário
8. main(): Inicialização
```

**Para**: Modificar o comportamento do jogo

---

#### Headers importantes em **[include/](include/)**
- `board.h` - Definições da Black Pill
- `mpu6050.h` - API do sensor IMU
- `st7789.h` - API do display LCD
- `FreeRTOSConfig.h` - Configuração do RTOS
- `delay_rtos.h` - Funções de delay
- `serial_stdio.h` - Debug UART

---

## 🎯 Fluxo de Leitura Recomendado

### Para Usuários Iniciantes:
```
1. README.md (visão geral)
2. QUICKSTART.md (começar rapidamente)
3. TROUBLESHOOTING.md (se tiver problemas)
4. MAZE_EXAMPLES.md (customizar jogo)
```

### Para Desenvolvedores:
```
1. README.md
2. DOCUMENTATION.md (arquitetura completa)
3. DIAGRAMS.md (visualizar fluxos)
4. src/main.c (estudar código)
5. PROJECT_SUMMARY.md (referência rápida)
```

### Para Professores/Avaliadores:
```
1. PROJECT_SUMMARY.md (resumo executivo)
2. DOCUMENTATION.md (detalhes técnicos)
3. DIAGRAMS.md (arquitetura visual)
4. src/main.c (implementação)
```

---

## 📊 Tabela de Arquivos

| Arquivo | Tipo | Para que serve | Quando usar |
|---------|------|----------------|-------------|
| README.md | Geral | Visão geral | Sempre primeiro |
| QUICKSTART.md | Tutorial | Setup rápido | Para começar |
| DOCUMENTATION.md | Técnico | Detalhes completos | Entender sistema |
| DIAGRAMS.md | Visual | Fluxogramas | Visualizar arquitetura |
| PROJECT_SUMMARY.md | Resumo | Referência rápida | Consulta rápida |
| MAZE_EXAMPLES.md | Gameplay | Mapas alternativos | Customizar jogo |
| TROUBLESHOOTING.md | Ajuda | Resolver problemas | Quando der erro |
| lib/FREERTOS_SETUP.md | Setup | Instalar FreeRTOS | Primeira vez |
| INDEX.md | Navegação | Encontrar docs | Este arquivo! |

---

## 🔍 Encontrar Informação Específica

### "Como faço para..."

| Pergunta | Arquivo | Seção |
|----------|---------|-------|
| ...começar o projeto? | QUICKSTART.md | Todo |
| ...conectar o hardware? | QUICKSTART.md | Seção 3 |
| ...compilar o código? | QUICKSTART.md | Seção 2 |
| ...resolver erro XYZ? | TROUBLESHOOTING.md | Busque pelo erro |
| ...mudar o labirinto? | MAZE_EXAMPLES.md | "Como Usar" |
| ...ajustar velocidade da bolinha? | DOCUMENTATION.md | "Parâmetros" |
| ...entender as tasks? | DOCUMENTATION.md | "FreeRTOS Tasks" |
| ...ver o fluxo do jogo? | DIAGRAMS.md | "Fluxo Principal" |
| ...modificar o código? | src/main.c | Comentários |
| ...instalar FreeRTOS? | lib/FREERTOS_SETUP.md | Todo |

---

## 📞 Onde Obter Ajuda

1. **Erro de compilação?** → TROUBLESHOOTING.md → Seção "Compilação"
2. **Hardware não funciona?** → TROUBLESHOOTING.md → Seção "Hardware"
3. **Comportamento estranho?** → TROUBLESHOOTING.md → Seção "Comportamento"
4. **Quer customizar?** → MAZE_EXAMPLES.md ou DOCUMENTATION.md
5. **Dúvida técnica?** → DOCUMENTATION.md
6. **Não sabe por onde começar?** → QUICKSTART.md

---

## 🎓 Materiais para Apresentação

Se você vai apresentar este projeto:

1. **Slides**: Use DIAGRAMS.md para imagens
2. **Demo**: Siga QUICKSTART.md para setup
3. **Explicação técnica**: Use DOCUMENTATION.md
4. **Resumo**: Use PROJECT_SUMMARY.md

---

## ✅ Checklist de Primeiro Uso

- [ ] Li README.md
- [ ] Instalei FreeRTOS (lib/FREERTOS_SETUP.md)
- [ ] Conectei hardware (QUICKSTART.md seção 3)
- [ ] Compilei código (QUICKSTART.md seção 2)
- [ ] Fiz upload para placa
- [ ] Testei funcionamento
- [ ] Li TROUBLESHOOTING.md (caso tenha problemas)
- [ ] Customizei labirinto (MAZE_EXAMPLES.md - opcional)

---

## 📈 Mapa do Projeto

```
ProjetoFinal_Labirinto/
│
├── 📘 Documentação (você está aqui!)
│   ├── INDEX.md ⭐ (Este arquivo)
│   ├── README.md (Comece aqui)
│   ├── QUICKSTART.md (Setup rápido)
│   ├── DOCUMENTATION.md (Técnico completo)
│   ├── DIAGRAMS.md (Visual)
│   ├── PROJECT_SUMMARY.md (Resumo)
│   ├── MAZE_EXAMPLES.md (Mapas)
│   └── TROUBLESHOOTING.md (Ajuda)
│
├── ⚙️ Configuração
│   ├── platformio.ini
│   └── .gitignore
│
├── 💻 Código Fonte
│   ├── src/
│   │   └── main.c ⭐ (Principal)
│   └── include/
│       └── *.h (Headers)
│
└── 📚 Bibliotecas
    └── lib/
        ├── FREERTOS_SETUP.md
        └── FreeRTOS-Kernel/
```

---

## 🎯 Objetivos de Aprendizado

Este projeto demonstra:

- ✅ Programação bare-metal STM32
- ✅ FreeRTOS (tasks, queues, mutex)
- ✅ Drivers de periféricos (SPI, I2C, GPIO)
- ✅ Física 2D em tempo real
- ✅ Renderização gráfica
- ✅ Processamento de sensores inerciais
- ✅ Máquina de estados
- ✅ Documentação técnica profissional

---

## 📜 Informações do Projeto

**Curso**: ELF74 - Sistemas Embarcados  
**Instituição**: UTFPR - Campus Curitiba  
**Professor**: Eduardo Nunes Dos Santos  
**Ano**: 2025

**Equipe**:
- Alfons Carlos Cesar Heiermann de Andrade
- Mateus Filipe de Ornelas Rampim
- Guilherme Corrêa Koller

---

<div align="center">

**🎮 Divirta-se com o Simulador de Labirinto! 🎮**

*Tem dúvidas? Comece pelo [QUICKSTART.md](QUICKSTART.md)*

</div>
