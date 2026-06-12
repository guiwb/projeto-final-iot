# Plano de Desenvolvimento — Pista de Corrida (Grupo 1)

## Fase 1 — Infraestrutura base

**Objetivo:** placa conectada e comunicando.

- [ ] Configurar projeto (PlatformIO ou Arduino IDE) e estrutura de pastas.
- [ ] Conexão Wi-Fi com reconexão automática.
- [ ] Cliente MQTT: conexão ao broker, reconexão e callback de mensagens.
- [ ] Assinar `br_nvoltas`, `br_largada`, `br_fim`, `br_iba`, `br_fba`.
- [ ] Teste: publicar/receber mensagens manualmente (ex.: `mosquitto_pub`/`mosquitto_sub`).

**Entrega:** ESP32 conectado ao broker, logando mensagens recebidas no serial.

## Fase 2 — Periféricos

**Objetivo:** cada componente funcionando isoladamente.

- [ ] LCD: inicialização e função utilitária de exibição (2 linhas, limpeza, etc.).
- [ ] Teclado matricial: leitura de caracteres e montagem do nome do piloto.
- [ ] Sensores de largada e reta: leitura com debounce.
- [ ] Botão de bandeira amarela: leitura com debounce.
- [ ] Teste unitário manual de cada periférico via serial.

**Entrega:** sketch de teste exercitando todos os periféricos.

## Fase 3 — Máquina de estados

**Objetivo:** fluxo completo da corrida sem as regras finas.

- [ ] Implementar estados: `AGUARDANDO_CONFIGURACAO` → `INSCRICAO` → `CORRIDA` → (fim) → `AGUARDANDO_CONFIGURACAO`.
- [ ] `AGUARDANDO_CONFIGURACAO`: aguardar `br_nvoltas` e armazenar total de voltas.
- [ ] `INSCRICAO`: capturar nome pelo teclado, exibir no LCD, publicar `g1_piloto`; aguardar `br_largada`.
- [ ] `CORRIDA`: registrar `horarioLargada`, habilitar sensores.
- [ ] Encerramento por `br_fim`: desabilitar sensores, parar cronômetros, tela final, voltar ao estado inicial.
- [ ] Telas do LCD por estado (antes da largada, durante, encerrada).

**Entrega:** corrida completa de ponta a ponta dirigida pelo broker.

## Fase 4 — Cronometragem e telemetria

**Objetivo:** medições corretas publicadas no broker.

- [ ] Sensor de largada: 1ª passagem (`tempoVolta = agora - horarioLargada`, `voltaAtual = 1`) e demais (`agora - ultimaPassagemLargada`, `voltaAtual++`); publicar `g1_tvolta`; atualizar `ultimaPassagemLargada`.
- [ ] Sensor de reta: `velocidade = 8 / (agora - ultimaPassagemLargada)`; publicar `g1_vreta`.
- [ ] Atualizar `Volta: X/N` no LCD a cada passagem.
- [ ] Validar unidades e formato dos payloads com o padrão combinado entre os grupos.

**Entrega:** tempos de volta e velocidades corretos visíveis no broker.

## Fase 5 — Bandeira amarela

**Objetivo:** comportamento completo de bandeira amarela.

- [ ] Botão local: alternar publicação `g1_ba = 1` / `g1_ba = 0` conforme estado atual.
- [ ] `br_iba`: ativar modo bandeira amarela, exibir `BANDEIRA AMARELA / Volta: X/N` no LCD.
- [ ] `br_fba`: desativar e restaurar tela normal.
- [ ] **Regra especial:** flag `voltaComBandeira` — setada quando ocorre bandeira amarela durante a volta; ao concluir a volta, publicar `g1_tvolta = 0` em vez do tempo real e limpar a flag.
- [ ] Garantir que a flag é avaliada por volta (bandeira que termina antes da linha de chegada ainda zera a volta em curso).

**Entrega:** ciclo completo de bandeira amarela validado.

## Fase 6 — Integração e robustez

**Objetivo:** sistema confiável para a apresentação.

- [ ] Teste integrado com o broker da disciplina e demais grupos.
- [ ] Casos de borda: bandeira amarela na 1ª volta, `br_fim` durante bandeira amarela, reinício de corrida (novo `br_nvoltas` após o fim), perda/reconexão de Wi-Fi/MQTT no meio da corrida.
- [ ] Revisão do debounce dos sensores em condições reais da pista.
- [ ] Ensaio geral simulando a corrida completa.

**Entrega:** sistema validado de ponta a ponta.

## Variáveis principais

| Variável | Uso |
|---|---|
| `horarioLargada` | Timestamp do `br_largada` (base da 1ª volta) |
| `ultimaPassagemLargada` | Base de cálculo das voltas seguintes e da reta |
| `passagemInicioReta` | Referência da passagem no início da reta |
| `voltaAtual` | Contador de voltas (`X/N` no LCD) |
| `totalVoltas` | Recebido em `br_nvoltas` |
| `bandeiraAtiva` | Estado atual da bandeira amarela |
| `voltaComBandeira` | Marca volta "contaminada" → publica `g1_tvolta = 0` |

## Decisões pendentes

- Endereço/porta do broker MQTT e credenciais.
- Formato exato dos payloads (`g1_tvolta` em s ou ms; `g1_vreta` em m/s ou km/h).
- Modelo dos sensores (IR, reed switch, etc.) e do LCD (16x2 I2C?).
- Distribuição de tarefas entre os membros do grupo.
