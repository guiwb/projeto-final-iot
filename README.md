# Projeto IoT II — Pista de Corrida (Grupo 1)

Sistema embarcado que controla uma pista de corrida conectada via MQTT: registra o piloto, monitora voltas, calcula a velocidade na reta e gerencia bandeira amarela, reagindo a eventos do broker (direção de prova).

## Visão Geral

A pista opera como um cliente MQTT que:

- **Publica** informações da corrida (piloto, tempo de volta, velocidade na reta, bandeira amarela).
- **Assina** eventos globais da direção de prova (número de voltas, largada, fim, início/fim de bandeira amarela).

## Hardware

| Componente | Função |
|---|---|
| ESP32 | Microcontrolador / conectividade Wi-Fi + MQTT |
| Display LCD | Exibição de status da corrida |
| Teclado matricial | Registro do nome do piloto |
| Sensor de largada | Detecta passagem na linha de largada |
| Sensor de reta | Detecta passagem no fim da reta (8 m após a largada) |
| Botão | Solicitação local de bandeira amarela |

## Tópicos MQTT

### Publicação

| Tópico | Payload | Descrição |
|---|---|---|
| `g1_piloto` | nome | Nome do piloto registrado |
| `g1_ba` | `1` / `0` | Solicitação (`1`) ou fim (`0`) de bandeira amarela |
| `g1_tvolta` | segundos | Tempo da volta (`0` se houve bandeira amarela durante a volta) |
| `g1_vreta` | m/s | Velocidade na reta (8 m / tempo) |

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
tempoReta  = agora - ultimaPassagemLargada
velocidade = 8 / tempoReta   (reta de 8 metros)
```

Publica em `g1_vreta`.

### Bandeira amarela

- **Botão local:** alterna a solicitação — publica `g1_ba = 1` (se inativa) ou `g1_ba = 0` (se ativa).
- **`br_iba`:** ativa o modo bandeira amarela e exibe aviso no LCD (carros parados).
- **`br_fba`:** desativa e atualiza o LCD.

> **Regra especial:** se durante uma volta ocorreu qualquer bandeira amarela, ao concluir essa volta publica-se `g1_tvolta = 0` em vez do tempo real (a volta foi "contaminada" pela paralisação).

### Encerramento (`br_fim`)

Desabilita sensores, para cronômetros, exibe "Corrida Encerrada", ignora novas passagens e retorna a `AGUARDANDO_CONFIGURACAO`.

## Telas do LCD

| Momento | Conteúdo |
|---|---|
| Antes da largada | `Piloto: XXXXX` / `Voltas: N` / `Aguardando largada` |
| Durante a corrida | `Piloto: XXXXX` / `Volta: X/N` |
| Bandeira amarela | `BANDEIRA AMARELA` / `Volta: X/N` |
| Corrida encerrada | `CORRIDA FINALIZADA` / `Obrigado!` |

## Documentação

- [Plano de desenvolvimento](PLANO_DE_DESENVOLVIMENTO.md)
