# Versão chaveiro (XIAO ESP32-C6)

> **Em projeto.** O firmware deste repositório ainda é o da versão ESP32-C3 com
> duas 18650. Este documento descreve a versão compacta, que já tem a caixa
> pronta para imprimir ([hardware/caixa](../hardware/caixa/README.md)). O
> firmware para o C6 é o próximo passo, e nada aqui foi montado ou medido.

Um air mouse do tamanho de um chaveiro, pensado para quem dá aula: aponta e clica
no computador, tem laser, e liga o projetor da sala com um código infravermelho
gravado do controle original — para nunca mais procurar o controle.

| | Versão C3 (atual) | Versão chaveiro |
|---|---|---|
| Placa | ESP32-C3 SuperMini | Seeed XIAO ESP32-C6 |
| Bateria | 2 × 18650 (6800 mAh) | LiPo 400 mAh com TP4056 colado |
| Caixa | ≥ 65 × 40 mm de seção, ~160 g | 40,4 × 37 × 24,5 mm |
| Extras | — | Laser, controle IR com gravação |

---

## Lista de compras

### Já tenho

| Peça | Observação |
|---|---|
| Seeed XIAO ESP32-C6 | Carregador de LiPo embutido, sono profundo de ~15 µA |
| MPU6050 (módulo GY-521) | Tirar os pinos e o LED de alimentação do módulo |
| LiPo ~400 mAh com TP4056 colado | Módulo USB-C **com proteção** (6 pads: IN, B+/B−, OUT+/OUT−) |

### Comprar

| Peça | O que pesquisar | Qtd | Observação |
|---|---|---|---|
| Módulo laser | `módulo laser 650nm 6mm 3V` | 1 | Corpo de 6 mm de diâmetro. Prefira **1 mW (classe 2)**; os de 5 mW são fortes demais para sala de aula. Versão de **3 V**. |
| LED infravermelho | `LED IR 940nm 3mm` (ex.: IR204) | 1 | **3 mm**, transparente ou azulado. É o emissor do controle remoto. |
| Receptor infravermelho | `receptor IR 38kHz VS1838B` ou `TSOP38238` | 1 | Para gravar o código do controle original. |
| Botão táctil | `chave táctil 6x6x7mm` | 4 | 4 terminais, altura total de **7 mm** (a haste sai 1,5 mm acima da tampa). |
| LED RGB | `LED RGB 3mm ânodo comum difuso` | 1 | Difuso fica mais bonito. Se só achar cátodo comum, o firmware tem a opção. |
| Transistor NPN | `transistor S8050` (TO-92) | 2 | Um para o laser, outro para o LED IR. Montar deitado. |
| Resistor 22 Ω | `resistor 22R 1/4W` | 1 | Limita a corrente do LED IR. |
| Resistor 1 kΩ | `resistor 1K 1/4W` | 2 | Base dos transistores. |
| Resistor 100 kΩ | `resistor 100K 1/4W` | 2 | Puxa a base para o GND: sem ele o laser pode acender sozinho no sono profundo. |
| Resistor 220 Ω | `resistor 220R 1/4W` | 3 | Um por cor do LED RGB. |
| Resistor 200 kΩ | `resistor 200K 1/4W` | 2 | Divisor para medir a bateria. |
| Capacitor 100 nF | `capacitor cerâmico 100nF` | 1 | Filtro do divisor da bateria. |
| Capacitor 100 µF | `capacitor eletrolítico 100uF 6.3V 5x5` | 1 | Segura os pulsos do LED IR. Procure o de 5 mm de altura. |
| Placa perfurada | `placa perfurada ilhada` | 1 | Cortar um pedaço de ~11 × 15 mm para os transistores. |
| Fio fino | `fio wire wrap 30AWG` ou `fio cabinho 28AWG silicone` | 1 rolo | Fio fino deixa a montagem caber. |
| Fita kapton | `fita kapton` | 1 | Isola o TP4056 da bateria e as placas entre si. |
| Argola de chaveiro | `argola chaveiro 25mm` | 1 | Até 25 mm com arame de 2 mm, ou até 30 mm com arame de 1,5 mm. |
| Filamento | PETG ou PLA | — | PETG aguenta melhor queda e calor (carro, bolsa ao sol). |

---

## Ligações

O C6 só acorda do sono profundo pelos GPIO 0 a 7. A XIAO expõe três deles nas
laterais (D0–D2) e os outros quatro em **pads embaixo da placa** (MTMS, MTDI,
MTCK, MTDO). Os quatro botões e o INT do MPU precisam acordar a placa, então os
pads de baixo são obrigatórios — solde os fios neles antes de colocar a XIAO na
bandeja.

| Função | Pino da XIAO | GPIO | Ligação |
|---|---|---|---|
| Bateria (ADC) | D0 | 0 | BAT+ → 200 kΩ → D0 → 200 kΩ → GND, e 100 nF de D0 ao GND |
| Botão 2 · clique | D1 | 1 | Outro lado do botão no GND |
| Botão 4 · projetor | D2 | 2 | Outro lado no GND |
| Botão 1 · liga / laser | pad MTMS | 4 | Outro lado no GND |
| Botão 3 · centro | pad MTDI | 5 | Outro lado no GND |
| INT do MPU6050 | pad MTCK | 6 | |
| Sinal do receptor IR | pad MTDO | 7 | |
| SDA / SCL do MPU6050 | D4 / D5 | 22 / 23 | VCC do MPU no 3V3 |
| LED IR | D3 | 21 | Via transistor, ver abaixo |
| Laser | D9 | 20 | Via transistor, ver abaixo |
| Alimentação do receptor IR | D6 | 16 | O receptor só é ligado no modo de gravação (gasta ~1 mA) |
| LED RGB — R / G / B | D7 / D10 / D8 | 17 / 18 / 19 | Cada cor com 220 Ω; ânodo comum no 3V3 |

Os botões usam o pull-up interno: não precisam de resistor.

### Alimentação

```
LiPo ── TP4056 (B+/B−) ── OUT+ ──► pad BAT+ da XIAO
                          OUT− ──► pad BAT− da XIAO
```

Carregue pela **USB-C da XIAO**: ela carrega a bateria por dentro e é a mesma
porta usada para gravar o firmware. O TP4056 fica só como proteção da bateria.
Se quiser carregar pelo TP4056, ele vem de fábrica carregando a **1 A**, demais
para 400 mAh: troque o resistor marcado `122` (R3) por um de 3 kΩ (~400 mA) e
abra o furo dele na caixa (`abrir_usb_tp4056 = true`).

### Laser e LED IR

```
3V3 ──► laser (+)                 BAT+ ──► 22 Ω ──► LED IR (ânodo)
        laser (−) ──► coletor              LED IR (cátodo) ──► coletor
                      S8050                                    S8050
D9 ──► 1 kΩ ──► base                D3 ──► 1 kΩ ──► base
       100 kΩ da base ao GND               100 kΩ da base ao GND
        emissor ──► GND                     emissor ──► GND

100 µF entre BAT+ e GND, perto do LED IR
```

- O laser vai no **3V3**: o módulo de 3 V ligado direto na bateria (até 4,2 V)
  passaria da corrente.
- O LED IR vai no **BAT+**: os pulsos de ~120 mA puxados do 3V3 podem derrubar a
  tensão e reiniciar a placa. É essa corrente que dá alcance de controle remoto
  comum (8–10 m).

---

## Botões

| Botão | Toque | Dois toques | Segurar |
|---|---|---|---|
| 1 · liga / laser | Desligado: liga. Ligado: mostra a bateria no LED | Desliga | Laser aceso, cursor congelado |
| 2 · clique | Clique esquerdo (avança o slide no PowerPoint) | Duplo clique normal | Arrastar |
| 3 · centro | Centraliza o cursor | Troca a velocidade (lento → médio → rápido) | Inclinar para rolar a tela |
| 4 · projetor | Envia o código IR gravado | Envia o 2º código | 3 s: grava um código novo |
| 3 + 4 juntos | — | — | 5 s: parear um PC novo |

O laser só acende depois de ~150 ms segurando: assim os toques para ligar e
desligar não fazem o laser piscar.

### Gravar o código do projetor

1. Segure o botão 4 por 3 s — o LED pisca, avisando que está gravando.
2. Aponte o controle do projetor para o **topo** do chaveiro, a uns 5 cm, e
   aperte Power uma vez.
3. LED verde: o código foi salvo na flash (sobrevive ao sono e à bateria
   acabando). LED vermelho: nada chegou em 10 s.

Para usar, toque no botão 4 apontando para o projetor. Muitos projetores pedem
dois toques em Power para desligar.

---

## Energia

| Estado | Como entra | Como sai | Consumo estimado |
|---|---|---|---|
| Desligado | Dois toques no botão 1, ou 3 min sem PC conectado | Toque no botão 1 | ~30 µA |
| Ativo | Ao ligar, movimento ou qualquer botão | Parado por 4 s | ~22 mA |
| Ocioso | Parado por 4 s, conexão mantida | Movimento | ~1,9 mA |
| Dormindo | Parado por 20 min | Movimento ou botão | ~50 µA |

- **Desligado não acorda com movimento**, só com o botão 1. É o estado para
  levar na bolsa.
- **Desliga sozinho** depois de 3 min sem PC conectado. Usar o laser ou o IR
  reinicia a contagem, para dar para ligar o projetor antes do computador.
- Os consumos de ativo e ocioso são as estimativas do C3 em
  [ENERGIA.md](ENERGIA.md). Nada foi medido no C6.

Com 400 mAh: ~2 dias com 8 h por dia na mão, ~5 dias numa rotina de aula
(~3 h de uso ativo por dia), e mais de um ano guardado desligado.

---

## Cuidados

- **Laser**: 1 mW no máximo. Nunca apontar para o rosto de ninguém.
- **MPU6050 firme**: cole o módulo na bandeja. Se ele balançar dentro da caixa,
  o cursor treme.
- **LED do GY-521**: tire o LED de alimentação do módulo — ele gasta o tempo
  todo, inclusive em sono profundo.
- **PC da sala**: muito desktop de escola não tem Bluetooth, ou tem o pareamento
  bloqueado pela TI. Um adaptador USB Bluetooth resolve o primeiro caso.
