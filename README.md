# Portaria Concorrente

Simulação de uma portaria de condomínio com controle de acesso concorrente, implementada em C com POSIX Pthreads.

## Descrição

O sistema simula catracas registrando entradas e saídas de moradores de forma concorrente sobre um contador compartilhado (`pessoas_dentro`). Um porteiro pode ativar o modo manutenção, bloqueando temporariamente novas entradas sem interromper as saídas.

A sincronização é feita com 1 mutex e 3 variáveis de condição:

- `tem_vaga` — sinalizada após uma saída; acorda catracas aguardando vaga
- `nao_vazio` — sinalizada após uma entrada; acorda catracas aguardando alguém para sair
- `fim_manut` — broadcast ao fim da manutenção; acorda todas as catracas bloqueadas

## Compilar e executar

```bash
make
./src/main
```

## Parâmetros (em `src/main.c`)

| Constante | Valor padrão | Descrição |
|---|---|---|
| `MAX_PESSOAS` | 10 | Capacidade máxima do condomínio |
| `NUM_CATRACAS` | 5 | Número de threads catraca |
| `OPS_POR_CATRACA` | 6 | Operações por catraca (alternando entrada/saída) |

