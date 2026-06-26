# Plano de Desenvolvimento — Pista de Corrida (Grupo 1)

> **Abordagem:** o código é escrito antes da ESP32 estar disponível e validado manualmente depois.
> Não há fase de testes automatizados; a verificação é feita rodando na placa e observando serial/LCD/broker.
> Este documento descreve **como** queremos implementar; a codificação será feita posteriormente, módulo a módulo.

## Convenções

- **Tempo:** todos os tempos são medidos, armazenados e publicados em **milissegundos** (`millis()`).
- **Velocidade:** publicada em **m/s**, calculada sobre a reta de **8 metros**.
- **MQTT:** enquanto o broker real não é definido, usar um **mock** (cliente/stub local) que simula publish/subscribe via serial. A configuração real (endereço, porta, credenciais) será inserida depois.

## Bibliotecas

| Lib | Uso |
|---|---|
| `WiFi.h` | Conexão Wi-Fi |
| `PubSubClient.h` | Cliente MQTT (substituível pelo mock) |
| `LiquidCrystal_I2C.h` | LCD 16x2 I2C |
| `NewPing.h` | Sensores ultrassônicos: `NewPing sonar(TRIG, ECHO, MAX_DIST)` |
| `Keypad.h` | Teclado matricial |

## Fase 1 — Infraestrutura base

**Objetivo:** placa conectada e comunicando.

- [ ] Configurar projeto (PlatformIO ou Arduino IDE) e estrutura de pastas.
- [ ] Conexão Wi-Fi com reconexão automática.
- [ ] Camada MQTT abstraída (interface única) com implementação **mock** por enquanto.
- [ ] Cliente MQTT: conexão ao broker, reconexão e callback de mensagens.
- [ ] Assinar `br_nvoltas`, `br_largada`, `br_fim`, `br_iba`, `br_fba`.

**Entrega:** ESP32 conectado, logando no serial mensagens recebidas via mock.

## Fase 2 — Periféricos

**Objetivo:** cada componente funcionando isoladamente.

- [ ] LCD 16x2 I2C (`LiquidCrystal_I2C`): inicialização e função utilitária de exibição (2 linhas, limpeza, etc.).
- [ ] Teclado matricial (`Keypad`): leitura de caracteres e montagem do nome do piloto.
- [ ] Sensores ultrassônicos de largada e reta (`NewPing`): leitura de distância e detecção de passagem por limiar, com debounce.
- [ ] Botão de bandeira amarela: leitura com debounce.

**Entrega:** módulos de periféricos prontos e isolados.

## Fase 3 — Máquina de estados

**Objetivo:** fluxo completo da corrida sem as regras finas.

- [ ] Implementar estados: `AGUARDANDO_CONFIGURACAO` → `INSCRICAO` → `CORRIDA` → (fim) → `AGUARDANDO_CONFIGURACAO`.
- [ ] `AGUARDANDO_CONFIGURACAO`: aguardar `br_nvoltas` e armazenar total de voltas.
- [ ] `INSCRICAO`: capturar nome pelo teclado, exibir no LCD, publicar `g1_piloto` (marca `pilotoInscrito`); aguardar `br_largada`.
- [ ] `br_largada` sem inscrição confirmada: exibir aviso de inscrição perdida no LCD e voltar a `AGUARDANDO_CONFIGURACAO` (piloto aguarda nova corrida).
- [ ] `CORRIDA`: registrar `horarioLargada` (ms), habilitar sensores.
- [ ] Encerramento por `br_fim`: desabilitar sensores, parar cronômetros, tela final, voltar ao estado inicial.
- [ ] Telas do LCD por estado (antes da largada, durante, encerrada).

**Entrega:** corrida completa de ponta a ponta dirigida pelo broker.

## Fase 4 — Cronometragem e telemetria

**Objetivo:** medições corretas publicadas no broker.

- [ ] Sensor de largada: 1ª passagem (`tempoVolta = agora - horarioLargada`, `voltaAtual = 1`) e demais (`agora - ultimaPassagemLargada`, `voltaAtual++`); publicar `g1_tvolta` (ms); atualizar `ultimaPassagemLargada`.
- [ ] Sensor de reta: `velocidade = 8000 / (agora - ultimaPassagemLargada)` (ms → m/s, reta de 8 m); publicar `g1_vreta` (m/s).
- [ ] Atualizar `Volta: X/N` no LCD a cada passagem.

**Entrega:** tempos de volta (ms) e velocidades (m/s) corretos no broker.

## Fase 5 — Bandeira amarela

**Objetivo:** comportamento completo de bandeira amarela.

- [ ] Botão local: alternar publicação `g1_ba = 1` / `g1_ba = 0` conforme estado atual.
- [ ] `br_iba`: ativar modo bandeira amarela, acender o LED (`LED_BANDEIRA`), exibir `BANDEIRA AMARELA / Volta: X/N` no LCD e **pausar a contabilização de voltas** (sensor ignorado enquanto a bandeira estiver ativa).
- [ ] `br_fba`: desativar, apagar o LED, restaurar tela normal e **voltar a contabilizar** as voltas.
- [ ] **Regra especial:** flag `voltaComBandeira` — setada quando ocorre bandeira amarela durante a volta; ao concluir a volta, publicar `g1_tvolta = 0` em vez do tempo real e limpar a flag.
- [ ] Garantir que a flag é avaliada por volta (bandeira que termina antes da linha de chegada ainda zera a volta em curso).
- [ ] Ao atingir `totalVoltas`, encerrar a contabilização (`finalizarCorrida`) e exibir a conclusão no LCD.

**Entrega:** ciclo completo de bandeira amarela implementado.

## Fase 6 — Integração e robustez

**Objetivo:** sistema confiável para a apresentação.

- [ ] Substituir o mock pelo broker real (endereço, porta, credenciais).
- [ ] Casos de borda: bandeira amarela na 1ª volta, `br_fim` durante bandeira amarela, reinício de corrida (novo `br_nvoltas` após o fim), perda/reconexão de Wi-Fi/MQTT no meio da corrida.
- [ ] Ajuste do limiar e debounce dos sensores ultrassônicos em condições reais da pista.

**Entrega:** sistema validado de ponta a ponta na placa.

## Variáveis principais

| Variável | Uso |
|---|---|
| `horarioLargada` | Timestamp (ms) do `br_largada` (base da 1ª volta) |
| `ultimaPassagemLargada` | Base (ms) de cálculo das voltas seguintes e da reta |
| `passagemInicioReta` | Referência (ms) da passagem no início da reta |
| `voltaAtual` | Contador de voltas (`X/N` no LCD) |
| `totalVoltas` | Recebido em `br_nvoltas` |
| `bandeiraAtiva` | Estado atual da bandeira amarela (pausa a contagem e acende o LED) |
| `voltaComBandeira` | Marca volta "contaminada" → publica `g1_tvolta = 0` |
| `corridaEmAndamento` | Indica corrida ativa; habilita a leitura do sensor de largada |
| `pilotoInscrito` | Nome confirmado (`g1_piloto` publicado); habilita a largada |
