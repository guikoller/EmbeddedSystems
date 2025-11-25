# IMPORTANTE: FreeRTOS-Kernel

O FreeRTOS-Kernel precisa estar presente em `lib/FreeRTOS-Kernel/`.

## Opções:

### 1. Copiar do Lab2_RTOS (recomendado se já existe):
```bash
xcopy "..\Lab2_RTOS\lib\FreeRTOS-Kernel" "lib\FreeRTOS-Kernel\" /E /I /Y
```

### 2. Baixar do repositório oficial:
```bash
cd lib
git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git
cd FreeRTOS-Kernel
git checkout V10.5.1
```

### 3. Usar link simbólico (requer admin):
```bash
New-Item -ItemType SymbolicLink -Path "lib\FreeRTOS-Kernel" -Target "..\Lab2_RTOS\lib\FreeRTOS-Kernel"
```
