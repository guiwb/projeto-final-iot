# Projeto IoT II — Pista de Corrida (Grupo 1)

Sistema embarcado que controla uma pista de corrida conectada via MQTT: registra o piloto, monitora voltas, calcula a velocidade na reta e gerencia bandeira amarela, reagindo a eventos do broker (direção de prova).

## Visão Geral

A pista opera como um cliente MQTT que:

- **Publica** informações da corrida (piloto, tempo de volta, velocidade na reta, bandeira amarela).
- **Assina** eventos globais da direção de prova (número de voltas, largada, fim, início/fim de bandeira amarela).

## Hardware

| Componente | Função | Lib |
|---|---|---|
| ESP32 | Microcontrolador / conectividade Wi-Fi + MQTT | `WiFi.h`, `PubSubClient.h` |
| LCD 16x2 I2C | Exibição de status da corrida | `LiquidCrystal_I2C.h` |
| Teclado matricial | Registro do nome do piloto | `Keypad.h` |
| Sensor ultrassônico de largada | Detecta passagem na linha de largada | `NewPing.h` |
| Sensor ultrassônico de reta | Detecta passagem no fim da reta (8 m após a largada) | `NewPing.h` |
| Botão | Solicitação local de bandeira amarela | — |
| LED | Indicação visual de bandeira amarela ativa | — |

> Sensores ultrassônicos instanciados como `NewPing sonar(TRIG, ECHO, MAX_DIST)`; passagem detectada por limiar de distância.
> O broker MQTT real ainda não está definido — durante o desenvolvimento usa-se um **mock**.

## Tópicos MQTT

### Publicação

| Tópico | Payload | Descrição |
|---|---|---|
| `g1_piloto` | nome | Nome do piloto registrado |
| `g1_ba` | `1` / `0` | Solicitação (`1`) ou fim (`0`) de bandeira amarela |
| `g1_tvolta` | milissegundos | Tempo da volta (`0` se houve bandeira amarela durante a volta) |
| `g1_vreta` | m/s | Velocidade na reta (8 m / tempo) |

> Todos os tempos são publicados em **milissegundos**; a velocidade em **m/s**.

### Assinatura

| Tópico | Descrição |
|---|---|
| `br_nvoltas` | Quantidade total de voltas da corrida |
| `br_largada` | Início da corrida |
| `br_fim` | Fim da corrida |
| `br_iba` | Início de bandeira amarela (global) |
| `br_fba` | Fim de bandeira amarela (global) |

## Máquina de Estados

```
AGUARDANDO_CONFIGURACAO ──(br_nvoltas)──► INSCRICAO ──(br_largada)──► CORRIDA ──(br_fim)──┐
        ▲                                                                                  │
        └──────────────────────────────────────────────────────────────────────────────────┘
```

| Estado | Ações |
|---|---|
| `AGUARDANDO_CONFIGURACAO` | Conecta ao MQTT e aguarda `br_nvoltas` |
| `INSCRICAO` | Lê nome do piloto pelo teclado, exibe no LCD, publica em `g1_piloto` |
| `CORRIDA` | Inicia cronômetro, habilita sensores, publica tempos e velocidades |

## Lógica da Corrida

### Voltas (sensor de largada)

- **1ª passagem:** `tempoVolta = agora - horarioLargada` → publica `g1_tvolta`, `voltaAtual = 1`.
- **Demais passagens:** `tempoVolta = agora - ultimaPassagemLargada` → publica `g1_tvolta`, `voltaAtual++`.
- Em ambas, atualiza `ultimaPassagemLargada`.

### Velocidade na reta (sensor de reta)

```
tempoReta  = agora - ultimaPassagemLargada      (ms)
velocidade = 8000 / tempoReta                    (m/s, reta de 8 metros)
```

Publica em `g1_vreta`.

### Bandeira amarela

- **Botão local:** alterna a solicitação (publica `g1_ba = 1` se inativa ou `g1_ba = 0` se ativa).
- **`br_iba`:** ativa o modo bandeira amarela, acende o LED, exibe aviso no LCD (carros parados) e **pausa a contabilização de voltas** (passagens ignoradas enquanto a bandeira estiver ativa).
- **`br_fba`:** desativa, apaga o LED, atualiza o LCD e **volta a contabilizar** as voltas.

> **Regra especial:** se durante uma volta ocorreu qualquer bandeira amarela, ao concluir essa volta publica-se `g1_tvolta = 0` em vez do tempo real (a volta foi "contaminada" pela paralisação).

### Encerramento

- **Total de voltas atingido:** ao registrar a última volta (`voltaAtual >= totalVoltas`), encerra a contabilização e exibe a conclusão da corrida no LCD.
- **`br_fim`:** desabilita sensores, para cronômetros, exibe "Corrida Encerrada", ignora novas passagens e retorna a `AGUARDANDO_CONFIGURACAO`.

## Telas do LCD

| Momento | Conteúdo |
|---|---|
| Antes da largada | `Piloto: XXXXX` / `Voltas: N` / `Aguardando largada` |
| Durante a corrida | `Piloto: XXXXX` / `Volta: X/N` |
| Bandeira amarela | `BANDEIRA AMARELA` / `Volta: X/N` |
| Total de voltas atingido | `<piloto> fim` / `Voltas: N/N` |
| Corrida encerrada (`br_fim`) | `CORRIDA` / `FINALIZADA` |

## Documentação

- [Plano de desenvolvimento](PLANO_DE_DESENVOLVIMENTO.md)
