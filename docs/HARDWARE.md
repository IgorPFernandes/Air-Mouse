# Hardware

## Lista de materiais

| Peça | Observação |
|---|---|
| ESP32-C3 | SuperMini ou DevKitM-1 |
| MPU6050 | Módulo GY-521 |
| 2 × 18650 3,7 V | Em **paralelo** — ver [Alimentação](#alimentação) |
| Suporte para 2 × 18650 | Ou solda direta em células com tab |
| Módulo de carga TP4056 | **Com proteção** (6 pinos, dois CIs) |
| 5 × botão táctil | Esquerdo, direito, meio, centralizar, velocidade |
| LED RGB de anodo comum | 5 mm ou SMD |
| 3 × resistor 220 Ω | Um por cor do RGB |
| Chave liga/desliga | Recomendada |
| 2 × resistor 100 kΩ | Divisor de bateria, opcional |
| 1 × capacitor 100 nF | Filtro do divisor, opcional |

---

## Pinagem

A SuperMini expõe treze GPIO. Doze estão em uso — o projeto está no limite.

| Sinal | Pino | Observação |
|---|---|---|
| MPU6050 SDA | GPIO 5 | I²C a 400 kHz |
| MPU6050 SCL | GPIO 6 | |
| MPU6050 INT | GPIO 0 | Desperta por movimento |
| MPU6050 VCC / GND | 3V3 / GND | O GY-521 aceita 3,3 V direto |
| Botão esquerdo | GPIO 3 | Também desperta do sono profundo |
| Botão direito | GPIO 4 | |
| Botão do meio | GPIO 10 | Segurado = modo scroll |
| **Botão centralizar** | **GPIO 7** | Leva o cursor ao centro da tela |
| **Botão velocidade** | **GPIO 9** | Percorre lento → médio → rápido |
| **RGB vermelho** | **GPIO 8** | Via resistor de 220 Ω |
| **RGB verde** | **GPIO 20** | Via resistor de 220 Ω |
| **RGB azul** | **GPIO 21** | Via resistor de 220 Ω |
| Divisor de bateria | GPIO 1 | ADC1 |
| *(livre)* | GPIO 2 | Deixado livre de propósito |

```
              ESP32-C3
            ┌───────────┐
   MPU6050  │           │
   VCC ─────┤ 3V3       │
   GND ─────┤ GND       │
   SDA ─────┤ GPIO 5    │
   SCL ─────┤ GPIO 6    │
   INT ─────┤ GPIO 0    │
            │           │
  Botão E ──┤ GPIO 3    │──┐
  Botão D ──┤ GPIO 4    │  │
  Botão M ──┤ GPIO 10   │  │ o outro lado de
  Centrar ──┤ GPIO 7    │  │ todos vai no GND
  Veloc.  ──┤ GPIO 9    │──┘
            │           │
  RGB R ─[220Ω]─ GPIO 8 │   anodo comum
  RGB G ─[220Ω]─ GPIO 20│   vai no 3V3
  RGB B ─[220Ω]─ GPIO 21│
            │           │
  Divisor ──┤ GPIO 1    │
   TP4056 ──┤ 5V        │
    OUT+    │           │
   TP4056 ──┤ GND       │
    OUT-    └───────────┘
```

Os botões usam o pull-up interno: um lado no GPIO, o outro no GND, sem resistor
externo.

### Pinos de strapping — leia antes de ligar

GPIO 2, 8 e 9 participam da escolha do modo de boot. Consequências práticas:

- **GPIO 9 é o pino BOOT.** Se o botão de velocidade estiver pressionado durante
  o reset, a placa entra em modo de gravação em vez de arrancar. É inofensivo, e
  na verdade conveniente na hora de gravar o firmware — só não estranhe.
- **GPIO 8 é seguro aqui porque o RGB é de ânodo comum.** Os pinos ficam em nível
  alto em repouso. Um RGB de *cátodo* comum inverteria isso e poderia atrapalhar
  o boot: não troque o tipo de LED sem trocar `RGB_COMMON_ANODE` e reavaliar.
- **GPIO 2 ficou livre** justamente por ser o mais sensível: em nível baixo no
  reset, a placa não arranca. Se precisar dele, garanta que nada o puxe ao GND.

GPIO 20 e 21 são a UART0, livres porque a serial de depuração usa o USB nativo.

---

## LED RGB

Uma peça, duas funções:

| Situação | Cor |
|---|---|
| Iniciando | Azul fixo |
| Anunciando, aguardando conexão | Azul, piscada lenta |
| MPU6050 não respondeu | Vermelho, piscada rápida |
| Conectado, em uso | **Apagado** |
| Velocidade rápida (ao trocar ou conectar) | Verde por 1,5 s |
| Velocidade média | Amarelo por 1,5 s |
| Velocidade lenta | Vermelho por 1,5 s |

O LED **não fica aceso** mostrando a velocidade o tempo todo, e isso é
deliberado: um LED aceso consome de 2 a 5 mA, mais que o aparelho inteiro em
repouso. Deixá-lo ligado anularia boa parte do trabalho de autonomia. A cor
aparece quando você troca a velocidade e quando o aparelho conecta — que são os
dois momentos em que você precisa da informação.

Para mudar a duração, ajuste `RGB_CONFIRM_MS`. Para deixá-lo permanentemente
aceso, o lugar é `rgb_led.cpp`, no caso `State::kConnected`.

---

## Botão de centralizar

Um mouse HID reporta **deslocamento relativo**, nunca posição absoluta: não
existe "vá para 960, 540". O firmware contorna isso em duas etapas: encosta o
cursor no canto superior esquerdo, onde o sistema operacional o prende, e de lá
caminha meia tela.

A primeira etapa é exata — por mais que se empurre, o cursor fica preso no
canto. A segunda não: a aceleração de ponteiro do sistema multiplica o
deslocamento conforme a velocidade, então a chegada é **próxima** do centro, não
exata. Para reencontrar um cursor perdido, é o suficiente.

Ajuste `SCREEN_WIDTH` e `SCREEN_HEIGHT` para a sua tela. Se a chegada ficar
sistematicamente longe do centro, aumente `RECENTER_STEPS`: passos menores e mais
espalhados sofrem menos com a aceleração.

No Windows, desmarcar "aumentar a precisão do ponteiro" torna a chegada quase
exata.

---

## Alimentação

### Duas 18650 em paralelo

```
   18650 #1  ──┬──(+)──┬── B+ ──┐
   18650 #2  ──┘       │        │  TP4056
                       │        │  (com proteção)
   18650 #1  ──┬──(−)──┴── B- ──┤
   18650 #2  ──┘                │
                     OUT+ ───[chave]─── 5V do ESP32-C3
                     OUT- ─────────────  GND do ESP32-C3
```

Em **paralelo**, não em série. A tensão continua 3,7 V nominais e a capacidade
soma: duas células de 3400 mAh dão 6800 mAh.

**O TP4056 carrega este pacote sem nenhum problema.** Ele é um carregador de um
estágio de tensão (1S) — o que significa um *nível de tensão*, não uma única
célula física. Para o TP4056, duas 18650 em paralelo são indistinguíveis de uma
18650 grande: mesmos 4,2 V de término, mesmo critério de encerramento.

O que ele **não** faz é carregar duas células em **série**. 2S resultaria em
8,4 V, exigiria um carregador e um BMS próprios, e essa tensão passaria do limite
do regulador da SuperMini.

### O que o paralelo compra, e o que não compra

O ganho é **capacidade**, não corrente. Uma única 18650 entrega de 2 a 10 A de
descarga contínua, e este aparelho consome 22 mA no pico — a capacidade de
corrente de uma célula já sobra por um fator de cem. O paralelo não deixa nada
mais rápido nem mais forte; ele dobra o tempo entre cargas, e só.

Vale ter isso claro antes de decidir, porque o peso extra está comprando dias de
autonomia, não desempenho.

### O pacote carrega completamente?

Sim, e por dois motivos que vale conhecer.

**Não há temporizador de segurança.** O TP4056 encerra por corrente, não por
tempo: termina quando a corrente cai a cerca de 1/10 da programada, durante a
fase de tensão constante. Uma carga de treze horas não é interrompida por nada.
Carregadores com temporizador de 6 a 8 h cortariam um pacote deste tamanho pela
metade; este não.

**E o pacote grande enche mais que um pequeno.** O corte é em corrente absoluta —
100 mA com o ajuste de 1 A. Num pacote de 400 mAh, 100 mA é C/4 e o corte
acontece cedo, com a célula em torno de 90%. Em 6800 mAh, 100 mA é **C/68**, e o
corte cai muito mais adiante na curva. Proporcionalmente, o pacote grande termina
mais cheio.

A corrente também se divide entre as células: 1 A em duas 18650 dá 0,5 A por
célula, cerca de 0,15C. Carga mansa, boa para a vida útil.

### O calor, que é o limite de verdade

O TP4056 é um regulador **linear**: ele dissipa a diferença de tensão em calor.
A 1 A, com a célula em 3,4 V, são `(5 − 3,4) × 1 = 1,6 W` num encapsulamento
SOP-8. A regulação térmica reduz a corrente ao aproximar-se de 145 °C no die, e o
sintoma é a carga demorar bem mais do que a conta sugere.

| Ajuste | Dissipação | Tempo real de carga |
|---|---|---|
| 1 A (padrão do módulo) | ~1,6 W, entra em limitação térmica | ~12 a 15 h |
| 500 mA (Rprog 2,4 kΩ) | ~0,8 W, sem limitação | ~14 h |

Baixar para 500 mA **quase não custa tempo**, porque a limitação térmica já
segurava a corrente de qualquer modo. Troca-se um módulo escaldando por um morno
pelo mesmo tempo total — e é a escolha certa se a carga acontecer com o aparelho
fechado na caixa.

Se catorze horas incomodarem, o caminho é o **TP5100**: ele é chaveado em vez de
linear, então não queima a diferença em calor e entrega 2 A sem esquentar como o
TP4056 esquenta a 1 A.

### Três cuidados com o paralelo

1. **Equalize antes da primeira ligação.** Células em paralelo com tensões
   diferentes trocam corrente entre si — a diferença de 0,5 V entre duas 18650
   pode gerar dezenas de ampères no instante da conexão. Carregue cada uma
   separadamente até a mesma tensão (meça com multímetro, diferença abaixo de
   0,05 V) antes de uni-las pela primeira vez.
2. **Use células iguais e do mesmo lote.** Mesma marca, mesmo modelo, idade
   parecida. Misturar células de capacidades ou resistências internas diferentes
   faz uma trabalhar mais que a outra.
3. **Prefira células sem proteção individual**, deixando a proteção por conta do
   TP4056. Dois circuitos de proteção em paralelo podem disparar de forma
   assimétrica e deixar uma célula carregando sozinha.

E o de sempre com 18650: elas entregam corrente de curto muito maior que uma
LiPo pequena. Um curto no circuito não é um fio que esquenta, é um fio que
derrete. Capriche na bitola e evite emendas frouxas dentro da caixa.

---

## O peso — vale conversar antes de comprar

Você supôs que as duas células não fariam "taaaaanta diferença". Os números
dizem o contrário:

| Peça | Peso |
|---|---|
| 18650 × 2 | ~92 g |
| Suporte das células | ~12 g |
| Caixa impressa dimensionada para elas | ~40 g |
| ESP32-C3 + MPU6050 + TP4056 | ~7 g |
| Botões, RGB, resistores, fios | ~8 g |
| **Total** | **~160 g** |

As células e o volume que elas obrigam são cerca de **90% do peso final**.

### Contra o que comparar

A tentação é comparar com um mouse de mesa, que pesa 100 a 140 g. **É a
comparação errada**: o mouse de mesa está apoiado na mesa, e a carga sustentada
pela mão é essencialmente zero — vence-se apenas o atrito. Este aparelho fica no
ar, e 100 g no ar se comparam a 0 g na mesa, não a 120 g na mesa.

A classe certa é a dos objetos que fazem o mesmo trabalho na mesma postura:

| Referência | Peso | Observação |
|---|---|---|
| Apresentador de slides comercial | 50 a 75 g | O objeto que este substitui |
| Controle de TV | 100 a 150 g | Usado em rajadas curtas, braço apoiado |
| Wii Remote com pilhas | 145 g | Fadiga documentada em sessões longas |

### Onde a massa está importa mais que quanta ela é

O gesto principal deste aparelho é girar o pulso, e o esforço para girar escala
com massa × distância ao quadrado. Duas consequências práticas:

- Com duas células, monte-as **lado a lado, nunca em fila**. Enfileiradas dão
  130 mm de comprimento e mais que dobram o esforço de rotação em relação às
  mesmas 160 g dispostas lado a lado em 65 mm.
- Em qualquer configuração, concentre a massa o mais perto possível do punho.
  Massa na ponta do aparelho é a pior posição possível para o gesto que este
  projeto pede o tempo todo.

E o retorno é menor do que parece, porque a autonomia já estava resolvida:

| Bateria | Jornadas de 8 h por carga | Peso do conjunto |
|---|---|---|
| LiPo 1000 mAh | ~31 | **~66 g** |
| 1 × 18650 (3400 mAh) | ~106 | ~102 g |
| 2 × 18650 (6800 mAh) | ~212 | ~160 g |

(Jornadas em perfil de apresentação; ver [ENERGIA.md](ENERGIA.md#perfil-de-jornada-de-8-horas).)

A LiPo de 1000 mAh entra na faixa de peso do apresentador comercial e ainda dá
seis semanas entre cargas. Para uso de sala de aula, é a opção que mais se parece
com o objeto que as pessoas já sabem segurar.

A segunda célula troca **+45 g no punho por dois meses a mais entre cargas**,
partindo de um aparelho que já ficava dois meses. Uma célula só usa exatamente a
mesma ligação — é só omitir a segunda.

Dito isso, o projeto está documentado para duas, e a decisão é sua: se a ideia é
um controle que fica meses largado na gaveta e raramente é usado em sessões
longas, duas células fazem sentido. Se você vai segurá-lo por uma hora seguida,
vale pegar uma 18650 na mão antes de decidir.

### Medição de bateria (opcional)

```
  OUT+ ──[100k]──┬──[100k]── GND
                 │
                 ├── GPIO 1
                 │
              [100nF]
                 │
                GND
```

O divisor voltou a ligar direto no GND: os 20 µA que ele drena são 0,3% de
6800 mAh em um ano, e o GPIO que fazia o chaveamento passou a ser o botão de
centralizar. Para retomar o chaveamento, defina `BATTERY_GATED 1` e escolha um
pino livre para `PIN_BAT_GND`.

Se o percentual sair pela metade ou dobrado, ajuste `BAT_DIVIDER_RATIO` — o valor
é `(R1 + R2) / R2`, ou seja, `2.0` com dois resistores iguais.

---

## Consumo e autonomia

Orçamento completo em [ENERGIA.md](ENERGIA.md): ~22 mA em uso, ~2 mA parado com
a conexão mantida, e da ordem de 65 a 325 µA em sono profundo.

### Duas modificações na placa que valem mais que o firmware

**Remova o LED de alimentação.** O LED vermelho que acende sozinho consome de 1 a
3 mA continuamente, inclusive com o ESP32 dormindo. A 2 mA são 48 mAh por dia —
mais que todo o resto do aparelho somado. Desolde o LED ou corte a trilha do
resistor em série.

**Confira o regulador.** Se a placa usar AMS1117, o repouso dele sozinho é de
~5 mA e o sono profundo deixa de significar qualquer coisa. ME6211 (~40 µA) e
XC6206 (~1 µA) servem.

Com 6800 mAh, um LED de 2 mA esquecido custa 48 mAh por dia e derruba os 133 dias
para cerca de 70. Continua muito, mas é metade da autonomia jogada fora por um
componente que não faz nada.

---

## Montagem mecânica

O MPU6050 precisa estar **rigidamente preso** à caixa. Se o módulo se mover
dentro dela, o sensor lê essa vibração como movimento real e nenhum ajuste de
filtro resolve. Cola quente nas bordas do módulo basta.

Com duas 18650 lado a lado, a caixa fica com pelo menos 65 × 40 mm de seção.
Posicione as células o mais próximo possível do punho: massa concentrada longe da
mão aumenta muito o esforço para girar o aparelho, que é exatamente o gesto que
este projeto pede o tempo todo.

Deixe o conector USB do TP4056 acessível por uma abertura.
