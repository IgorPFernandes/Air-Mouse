# Arquitetura do firmware

Este documento descreve como o firmware está organizado e, principalmente,
**por que** o tratamento do sinal é feito nessa ordem. A parte difícil de um air
mouse não é ler o sensor — é transformar velocidade angular em um cursor que se
comporte bem.

---

## Visão geral

```
   MPU6050 ──I²C──▶ mpu6050 ──ImuSample──▶ pointer ──PointerOutput──▶ ble_mouse ──BLE──▶ host
                                              ▲
                                              │
                        button ───────────────┘  (modo scroll)

   battery ──▶ ble_mouse (nível de bateria)
   status_led, power  ◀── main
```

`main.cpp` não contém regra de negócio: inicializa os módulos, mantém a cadência
fixa e decide quando enviar relatório ou dormir.

| Módulo | Responsabilidade |
|---|---|
| `mpu6050` | Acesso I²C ao sensor, calibração e remoção de bias |
| `pointer` | Todo o tratamento de sinal: filtros, curva, sub-pixel, scroll |
| `ble_mouse` | Perfil HID sobre BLE e envio dos relatórios |
| `button` | Debounce de um botão em pull-up |
| `battery` | Leitura do ADC e conversão para percentual |
| `status_led` | Estado visual do aparelho |
| `power` | Sono profundo e despertar |
| `config.h` | Único ponto de configuração |

Cada módulo é um namespace com estado interno em anônimo — não há singleton nem
alocação dinâmica fora do NimBLE, que exige.

---

## O pipeline de movimento

Tudo acontece em `pointer.cpp`, na função `update()`. A ordem importa: trocar
duas etapas de lugar degrada o resultado de forma visível.

### 1. Remoção de bias (em `mpu6050`)

Todo giroscópio MEMS tem um desvio de zero — parado, ele reporta algo diferente
de zero. Uns poucos graus por segundo bastam para o cursor atravessar a tela
sozinho em alguns segundos.

O bias é medido no boot, com a média de 600 amostras. A medição **se valida**:
se a variação entre a menor e a maior leitura passar de 25 deg/s, a placa estava
sendo movida, a média não representa o desvio de zero, e `calibrate()` retorna
`false`. `main` tenta até três vezes.

Essa validação é o que evita o modo de falha mais comum e mais confuso do
projeto: calibrar com o aparelho na mão e não entender por que o cursor deriva.

### 2. Compensação de rotação do pulso

O giroscópio mede rotação nos eixos **dele**, não nos da tela. Se o usuário
segura o aparelho inclinado 30°, girar o pulso na horizontal produz componente
nos dois eixos e o cursor anda na diagonal.

A correção usa um filtro complementar:

```
roll = 0.98 * (roll + gx * dt) + 0.02 * atan2(ay, az)
```

O giroscópio dá a variação rápida e precisa do ângulo, mas acumula deriva; o
acelerômetro sabe onde é "para baixo" pela gravidade, mas é ruidoso sob
movimento. O peso 0.98/0.02 confia no giroscópio no curto prazo e deixa a
gravidade corrigir a deriva lentamente.

Com o ângulo em mãos, o vetor de movimento é rotacionado de volta para os eixos
da tela. Desligável em `ROLL_COMPENSATION`.

### 3. Zona morta subtrativa

O tremor natural da mão fica na casa de 1 a 2 deg/s. Sem filtro, o cursor vibra
parado.

O detalhe está em **subtrair** o limiar em vez de zerar abaixo dele:

```c
if (v >  limiar) return v - limiar;
if (v < -limiar) return v + limiar;
return 0;
```

Zerar cria um degrau — ao cruzar a borda, o cursor salta de parado para a
velocidade cheia. Subtraindo, a saída cresce a partir do zero e o movimento
começa suave.

### 4. Suavização exponencial

Média móvel de um polo, com peso em `SMOOTHING`. Tira o ruído que sobrou. Custa
latência: quanto maior o valor, mais suave e mais "molhado" fica o cursor.

### 5. Curva de aceleração

```
saída = |v| * SENSITIVITY + |v|² * ACCEL_GAIN
```

O termo linear domina no movimento lento e dá precisão para acertar um botão. O
termo quadrático domina no movimento rápido e permite atravessar a tela com um
giro curto de pulso. É o mesmo princípio da aceleração de ponteiro do sistema
operacional.

### 6. Acumulador de sub-pixel

O relatório HID carrega inteiros. Um movimento lento que produza 0,4 px por
quadro seria truncado para 0 — e a 125 Hz isso significa cursor **totalmente
parado** em movimentos finos, justamente onde a precisão importa.

A fração é guardada entre quadros:

```c
residual += curva(v);
passo = trunc(residual);
residual -= passo;
```

Assim os 0,4 px viram 1 px a cada dois quadros e meio, em vez de nada.

---

## Cadência e envio

O laço roda a `REPORT_HZ` (125 Hz por padrão) com `dt` constante. Manter o `dt`
estável importa porque tanto o filtro complementar quanto a suavização assumem
intervalo regular; um `dt` que varia com o tempo gasto no rádio faria a resposta
do cursor variar junto.

O relatório só é enviado quando há deslocamento, roda ou mudança de botão.
Relatórios de zero repetidos não informam nada ao host e gastam rádio.

---

## Energia

Após `IDLE_SLEEP_MS` sem movimento acima de `IDLE_MOTION_DPS` e sem botão, o
aparelho entra em sono profundo. O MPU6050 também é posto para dormir antes.

O despertar é por nível baixo no botão esquerdo. **No ESP32-C3 apenas os
GPIO 0 a 5 acordam o chip do sono profundo** — a restrição é verificada em tempo
de compilação por um `static_assert` em `power.cpp`, para que mudar o pino no
`config.h` para um valor inválido falhe na compilação em vez de produzir um
aparelho que nunca acorda.

Despertar do sono profundo é um reset: `setup()` roda de novo e o giroscópio é
recalibrado.

---

## Por que HID direto sobre o NimBLE

O ESP32-C3 tem **apenas BLE**, sem Bluetooth Classic. As bibliotecas de mouse
mais citadas para ESP32 dependem do Bluedroid com Bluetooth Classic e não
funcionam nesta peça.

As alternativas em BLE são bibliotecas de terceiros que encapsulam o NimBLE e
tendem a quebrar a cada atualização do core do Arduino. Como o NimBLE já traz
`NimBLEHIDDevice` com tudo o que é preciso, a camada extra só adicionaria
superfície de falha. `ble_mouse.cpp` inteiro tem menos de 150 linhas.

Um detalhe não óbvio, marcado no código: a sobrecarga `manufacturer(std::string)`
é usada de propósito. Passar um `const char*` para `setValue()` casa com o
template genérico do NimBLE e grava o **ponteiro**, não o texto.

---

## Descritor HID

Mouse padrão de 5 botões, com uma escolha deliberada: X e Y em **16 bits** em vez
dos 8 bits do descritor clássico. Em 8 bits o deslocamento satura em 127 por
relatório, e movimentos rápidos ficam truncados — o cursor parece engasgar
justamente quando se gira a mão depressa.

Campos: botões (5 bits + 3 de preenchimento), X (int16), Y (int16), roda (int8),
AC Pan (int8). Sete bytes, com o Report ID transportado fora pelo NimBLE.
