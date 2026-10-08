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

Modelos escolhidos para caber na caixa. Os três primeiros são os que mais
variam entre anúncios: confira os itens da coluna *Confira no anúncio*.

| Peça | Modelo | Pesquise por | Confira no anúncio | Qtd |
|---|---|---|---|---|
| Laser | Módulo laser de ponto 650 nm, **1 mW, 3 V, 6 × 10 mm**, com driver embutido | `650nm 1mW 3V 6x10mm laser dot module` (AliExpress, Amazon) | 1 mW (não 5 mW), 3 V (não 5 V), 6 mm de diâmetro, até 10,5 mm de comprimento, "dot" (ponto, não linha nem cruz) | 1 |
| LED infravermelho | **Vishay TSAL4400** — 3 mm, 940 nm, ±25°, 100 mA contínuo | `TSAL4400` (Mouser, DigiKey, LCSC, AliExpress) | 940 nm (não 850 nm), corpo de 3 mm | 1 |
| Receptor infravermelho | **Vishay TSOP38238** — 38 kHz, 2,5–5,5 V, 5 × 4,8 × 6,95 mm | `TSOP38238` | Componente solto de 3 pernas, não o módulo KY-022 na plaquinha | 1 |
| Botão táctil | Chave táctil 6 × 6 mm, **altura total 7 mm**, 4 terminais (ex.: BTT-A06-7.0, Metaltex série A06 de 7,0 mm) | `chave táctil 6x6x7mm` | Furo passante (não SMD), 7 mm com o atuador, corpo de 3,5 mm | 4 |
| LED RGB | LED RGB de 3 mm, **ânodo comum**, lente difusa, 4 terminais | `LED RGB 3mm ânodo comum difuso` | "Ânodo comum" (common anode) e 3 mm | 1 |
| Transistor | **SS8050** (onsemi ou JSMSEMI), NPN, TO-92, 1,5 A | `SS8050 TO-92` | SS8050, não S8050 — o S8050 aguenta só 0,5 A. Pinagem E-B-C | 2 |
| Capacitor | Eletrolítico 100 µF, 6,3 V ou 10 V, **5 × 5 mm** | `capacitor eletrolítico 100uF 6.3V 5x5` | Altura de 5 mm (os comuns têm 11 mm e não cabem) | 1 |
| Capacitor | Cerâmico 100 nF | `capacitor cerâmico 100nF` | — | 1 |
| Resistores | 1/8 W (menores que os de 1/4 W): 22 Ω, 2 × 68 Ω, 220 Ω, 2 × 1 kΩ, 2 × 100 kΩ, 2 × 200 kΩ | `resistor 1/8W` | — | 10 |
| Placa perfurada | Placa ilhada, cortada em ~11 × 15 mm para os transistores | `placa perfurada ilhada` | — | 1 |
| Fio | Wire wrap 30 AWG (isolamento Kynar) | `fio wire wrap 30AWG` | — | 1 rolo |
| Fita kapton | — | `fita kapton` | — | 1 |
| Argola de chaveiro | 25 mm, arame de até 2 mm | `argola chaveiro 25mm` | Ou até 30 mm se o arame for de 1,5 mm | 1 |
| Filamento | PETG (ou PLA) | — | PETG aguenta melhor queda e calor | — |

**Se não achar no Brasil:**

- **Laser:** quase todo anúncio daqui é de 5 V e 5 mW (o KY-008). Para sala de
  aula, vale importar o de 1 mW. O KY-008 com plaquinha nem cabe na caixa.
- **LED IR:** um `LED IR 940nm 3mm` genérico funciona, desde que seja de
  940 nm. O TSAL4400 tem a ficha conhecida (aguenta os pulsos de 120 mA do
  circuito).
- **Receptor:** o **VS1838B** é vendido em todo lugar e também serve. O soquete
  da tampa foi alargado para caber os dois.

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
| LED RGB — R / G / B | D7 / D10 / D8 | 17 / 18 / 19 | R com 220 Ω, G e B com 68 Ω; ânodo comum no 3V3 |

Os botões usam o pull-up interno: não precisam de resistor. No LED RGB, verde e
azul levam resistor menor porque precisam de quase 3 V para acender e ficariam
apagados com 220 Ω no 3,3 V.

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
