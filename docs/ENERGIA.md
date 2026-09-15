# Energia e autonomia

> Os números abaixo são **estimativas de projeto**, calculadas a partir das
> folhas de dados e do comportamento esperado do rádio. Nada foi medido com
> multímetro ainda. Ver [Medindo de verdade](#medindo-de-verdade).

---

## Os três estados

| Estado | Quando | Consumo estimado | Custo para sair |
|---|---|---|---|
| **Ativo** | Em movimento ou com botão pressionado | ~22 mA | — |
| **Ocioso** | 4 s parado, conexão mantida | ~2 mA | ~50 ms |
| **Sono profundo** | 20 min parado | ~65 a 325 µA | 1 a 2 s (reconexão) |

O estado **ocioso** é o que faz a diferença no uso real. Um mouse passa a maior
parte do tempo parado entre um gesto e outro, e sair dele custa 50 ms — na
prática, imperceptível. Sem esse estado intermediário haveria só a escolha ruim
entre ficar em 22 mA o tempo todo ou pagar reconexão a cada pausa.

---

## O que mudou em relação à primeira versão

| Medida | Antes | Depois | Economia |
|---|---|---|---|
| Frequência de CPU | 160 MHz | 80 MHz | ~8 mA no estado ativo |
| Potência de transmissão | +9 dBm | 0 dBm | Corrente de pico nas rajadas |
| Espera entre quadros | Laço girando | `delay()` com light sleep | Grande, depende do suporte a PM |
| Slave latency em repouso | 0 | 40 | Rádio acorda a cada ~480 ms em vez de 7,5 ms |
| Giroscópio em repouso | Sempre ligado, 3,9 mA | Em espera, ~20 µA | 3,9 mA |
| Divisor de bateria | 20 µA contínuos | 20 µA — ver nota | — |
| Anúncio sem conexão | Rápido para sempre | Lento após 30 s, dorme após 3 min | Evita drenar a bateria esquecido |
| Despertar | Só botão | Botão **ou** movimento | Usabilidade, não consumo |

### O que é *slave latency*, e por que importa tanto

Num BLE HID, o intervalo de conexão precisa ser curto (7,5 ms) para o cursor
responder sem atraso. Mas manter 7,5 ms significa o rádio acordar 133 vezes por
segundo, mesmo com o aparelho parado na mesa.

*Slave latency* resolve isso: com latência 40, o periférico tem permissão de
ignorar até 40 eventos consecutivos quando **não tem nada a enviar**. O host
continua acreditando num intervalo de 7,5 ms e, no instante em que há
movimento, o relatório sai no evento seguinte — sem renegociar nada.

É exatamente o mecanismo que mouses e teclados BLE comerciais usam. O firmware
alterna entre latência 0 (ativo) e 40 (ocioso) nas transições de estado.

### Nota sobre o divisor de bateria

O chaveamento do divisor chegou a existir, mas foi revertido: o GPIO que o fazia
passou a ser o botão de centralizar, quando os controles novos esgotaram os pinos.
Os 20 µA que o divisor drena são 0,3% de uma 18650 em um ano — irrelevantes nesta
escala de bateria, ainda que fossem significativos com uma LiPo de 400 mAh.

Para retomar o chaveamento, defina `BATTERY_GATED 1` e escolha um pino livre para
`PIN_BAT_GND`. Só o GPIO 2 está livre, e ele é pino de strapping.

---

## Autonomia estimada

Uso contínuo, sem pausas:

| Bateria | Antes | Depois |
|---|---|---|
| 400 mAh | ~8 h | ~18 h |
| 1000 mAh | ~20 h | ~45 h |

Uso realista — 2 h de movimento, 2 h de pausas curtas e 20 h guardado por dia:

| Bateria | Antes | Depois |
|---|---|---|
| 400 mAh | ~2 dias | ~8 dias |
| 1000 mAh | ~5 dias | ~20 dias |
| 1 × 18650 (3400 mAh) | ~17 dias | ~66 dias |
| 2 × 18650 (6800 mAh) | ~34 dias | ~133 dias |

Cerca de 51 mAh por dia no perfil realista, contra cerca de 200 mAh antes.

A partir de ~66 dias, o fator que decide a autonomia deixa de ser o firmware e
passa a ser a **autodescarga** da célula: uma 18650 de lítio perde entre 2% e 5%
ao mês parada. Em 133 dias isso já é da ordem de 10% a 20% da capacidade. Vale
saber antes de pagar peso por capacidade que a química vai consumir sozinha —
ver a discussão de peso em [HARDWARE.md](HARDWARE.md#o-peso--vale-conversar-antes-de-comprar).

---

## Perfil de jornada de 8 horas

O cenário que orientou os padrões atuais: um professor usando o aparelho durante
um expediente e guardando-o no resto do dia.

Oito horas de expediente **não são** oito horas de movimento. O firmware cai para
o estado ocioso 4 s depois que o movimento para, e durante uma aula o aparelho
passa a maior parte do tempo parado na mão de quem está falando.

| Perfil da jornada | Tempo em movimento | Consumo no dia |
|---|---|---|
| Apresentação — trocar slide, apontar | ~10% | ~32 mAh |
| Uso misto — mouse principal parte do dia | ~25% | ~56 mAh |
| Uso intenso | ~50% | ~96 mAh |
| Movimento ininterrupto (limite teórico) | 100% | ~176 mAh |

Jornadas de 8 h por carga:

| Bateria | Apresentação | Misto | Intenso | Ininterrupto |
|---|---|---|---|---|
| LiPo 1000 mAh | ~31 | ~18 | ~10 | ~6 |
| 1 × 18650 (3400 mAh) | ~106 | ~61 | ~35 | ~19 |
| 2 × 18650 (6800 mAh) | ~212 | ~121 | ~71 | ~38 |

Em movimento contínuo, sem pausa nenhuma, uma 18650 dá **~154 horas** e duas dão
**~309 horas**.

### O que isso implica

**Não se carrega este aparelho todo dia.** Uma jornada consome de 1% a 3% de uma
18650. Com uma célula, a carga acontece a cada dois meses — o que torna irrelevante
o tempo de carga longo discutido em [HARDWARE.md](HARDWARE.md#o-calor-que-é-o-limite-de-verdade):
uma carga bimestral cabe num fim de semana.

**Uma célula basta, e é a escolha melhor aqui.** A segunda leva a autonomia de
dois para quatro meses, diferença sem efeito prático, ao custo de 45 g a mais no
punho de quem segura o aparelho erguido durante aulas de 50 minutos.

### Por que o sono profundo demora 20 minutos

`IDLE_SLEEP_MS` é 20 min, não 2. Sair do sono profundo custa uma reconexão BLE de
1 a 2 s, que no meio de uma aula aparece como um aparelho que não responde quando
se aponta para a tela.

O estado ocioso custa 2 mA. Uma jornada inteira de 8 h sem dormir nenhuma vez
gasta 16 mAh — meio por cento de uma 18650. Trocar isso por latência diante de
uma turma não se justifica.

Para um aparelho de uso doméstico esporádico, valores entre 2 e 5 min fazem mais
sentido.

---

## Os dois limites que o firmware não alcança

Aqui está a parte desconfortável: **o hardware da placa pode inviabilizar tudo
acima**, e nenhuma linha de código conserta isso.

### 1. O LED de alimentação da placa

Quase toda placa de desenvolvimento tem um LED vermelho que acende assim que há
tensão e nunca apaga. Ele consome de 1 a 3 mA **o tempo todo**, inclusive com o
ESP32 em sono profundo.

A 2 mA contínuos, são 48 mAh por dia — praticamente o dobro de tudo o que o
firmware gasta no perfil realista. Um aparelho que dorme a 65 µA com um LED de
2 mA ao lado está, na prática, consumindo 2 mA.

**A solução é remover o LED**: desolde-o, ou corte a trilha do resistor em série
com um estilete. É a modificação de maior retorno do projeto inteiro.

### 2. O regulador de tensão

O consumo em repouso do regulador de 3,3 V da placa aparece direto no total:

| Regulador | Corrente de repouso | Efeito no sono profundo |
|---|---|---|
| ME6211 | ~40 µA | Aceitável |
| XC6206 | ~1 µA | Excelente |
| **AMS1117** | **~5 mA** | **Destrói qualquer economia** |

O AMS1117 é comum em placas baratas e consome sozinho mais do que o resto do
aparelho somado. Se a sua SuperMini tiver um, o sono profundo não serve para
nada: 5 mA contínuos são 120 mAh por dia.

Identifique o CI regulador da sua placa antes de contar com os números deste
documento. Trocar de placa é mais simples do que trocar o regulador.

---

## Medindo de verdade

As estimativas acima valem pouco sem confirmação. Para medir:

1. Desligue a serial: `#define DEBUG_SERIAL 0`. O USB CDC mantém circuitos
   ativos e mascara completamente o consumo em repouso.
2. Alimente pela bateria, não pelo USB.
3. Meça em série com o positivo da bateria.

Um multímetro comum **não serve** para o sono profundo: a faixa de µA costuma
ter resistência interna alta o bastante para derrubar a tensão, e a comutação
entre faixas perde as rajadas do rádio. Para os estados ativo e ocioso ele
serve; para o sono profundo é preciso um medidor dedicado.

Ordem sugerida: meça primeiro o sono profundo. Se der muito acima de 300 µA,
o problema está no LED ou no regulador, e otimizar firmware não vai adiantar.

---

## Ajustando o compromisso

Todos os parâmetros estão em [`include/config.h`](../include/config.h).

| Quero | Ajuste |
|---|---|
| Mais autonomia, aceitando pausa perceptível | Reduza `IDLE_ENTER_MS` para `2000` |
| Menos espera ao voltar a usar | Aumente `IDLE_ENTER_MS`, ou `IDLE_SLEEP_MS` para adiar a reconexão |
| Ainda mais economia em repouso | Aumente `CONN_LATENCY_IDLE` para `60` — respeite `CONN_TIMEOUT` |
| Mais alcance | Aumente `BLE_TX_POWER_DBM` para `3` ou `6` |
| Cursor mais responsivo, custe o que custar | `REPORT_HZ_ACTIVE 200`, `CONN_LATENCY_IDLE 0` |
| Não montei o pino INT | `WAKE_ON_MOTION_ENABLED 0` — volta a despertar só por botão |

`CONN_TIMEOUT` precisa ser maior que `intervalo × (latency + 1)`. Com intervalo
de 15 ms e latência 40, são 615 ms; o padrão de 6 s dá folga larga. Latências
muito altas com timeout curto derrubam a conexão de forma intermitente e difícil
de diagnosticar.
