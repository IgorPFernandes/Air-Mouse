# Air Mouse

Mouse controlado por movimento da mão, feito com ESP32-C3 e MPU6050.

O aparelho se apresenta ao computador como um **mouse Bluetooth LE padrão (HID)**:
sem driver, sem dongle, sem software do lado do host. Funciona com Windows,
macOS, Linux, Android e iPadOS.

---

## Características

- **HID BLE nativo**, implementado direto sobre o NimBLE — sem biblioteca HID de
  terceiros.
- **Compensação de rotação do pulso** por filtro complementar: os eixos da tela
  continuam corretos mesmo com o aparelho inclinado de lado.
- **Curva de aceleração** com termo linear e quadrático: precisão no movimento
  fino, alcance no movimento rápido.
- **Acumulador de sub-pixel**, para que gestos lentos não sejam truncados a zero.
- **Calibração validada no boot** — a medição é rejeitada se o aparelho estiver
  sendo movido.
- **Modo scroll** com o botão do meio segurado.
- **Nível de bateria** reportado ao sistema operacional.
- **Gestão de energia em três estados** — ativo (~22 mA), ocioso com a conexão
  mantida (~1,9 mA) e sono profundo (~84 µA). Com duas 18650 em paralelo, cerca
  de **6,7 semanas** entre cargas numa jornada de 8 h com o aparelho na mão; ver
  [Autonomia](#autonomia).
- **Despertar por movimento**: pegar o aparelho já o acorda, sem apertar nada.
- **Botão de centralizar**, para reencontrar o cursor quando se perde de vista.
- **Três níveis de velocidade** com confirmação por LED RGB.

---

## Estrutura

```
├── include/
│   └── config.h              parâmetros de hardware e ajuste
├── src/
│   ├── main.cpp              inicialização e laço principal
│   ├── mpu6050.{h,cpp}       driver do sensor e calibração
│   ├── pointer.{h,cpp}       tratamento de sinal do cursor
│   ├── ble_mouse.{h,cpp}     perfil HID sobre BLE
│   ├── button.{h,cpp}        debounce
│   ├── recenter.{h,cpp}      centralizacao do cursor
│   ├── battery.{h,cpp}       leitura do ADC e curva da célula
│   ├── rgb_led.{h,cpp}       LED RGB: status e velocidade
│   └── power.{h,cpp}         clock, light sleep e sono profundo
├── tools/
│   └── i2c_scanner/          diagnóstico do barramento I²C
└── docs/
```

---

## Documentação

| Documento | Conteúdo |
|---|---|
| [HARDWARE.md](docs/HARDWARE.md) | Lista de materiais, pinagem, alimentação e carga |
| [MONTAGEM.md](docs/MONTAGEM.md) | Montagem em etapas, com teste a cada uma |
| [ARQUITETURA.md](docs/ARQUITETURA.md) | Como o firmware funciona e por quê |
| [AJUSTES.md](docs/AJUSTES.md) | Ajuste de sensibilidade e diagnóstico |
| [ENERGIA.md](docs/ENERGIA.md) | Orçamento de consumo e autonomia |

---

## Início rápido

Ligações completas em [HARDWARE.md](docs/HARDWARE.md). O mínimo para funcionar:

| MPU6050 | ESP32-C3 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SDA | GPIO 5 |
| SCL | GPIO 6 |

### PlatformIO

```bash
pio run --target upload
```

### Arduino IDE

1. Instale o core **esp32 by Espressif**, versão 2.0.x.
2. Instale **NimBLE-Arduino** versão **1.4.x** (a 2.x mudou a API).
3. Copie `src/` e `include/config.h` para a pasta do sketch, renomeando
   `main.cpp` para o nome da pasta com extensão `.ino`.
4. Selecione **ESP32C3 Dev Module** com `USB CDC On Boot: Enabled`.

Se o upload falhar na SuperMini: segure **BOOT**, toque em **RESET**, solte o
**BOOT** e repita.

---

## Uso

1. Ligue e **deixe parado por cerca de 2 segundos** — o giroscópio é calibrado no
   boot. Movimento durante a calibração faz o cursor derivar depois.
2. Parear em *Bluetooth → Adicionar dispositivo → Air Mouse*.
3. LED apagado indica conexão estabelecida.

| Ação | Resultado |
|---|---|
| Girar a mão na horizontal | Move o cursor em X |
| Inclinar para cima e para baixo | Move o cursor em Y |
| Botão esquerdo | Clique esquerdo |
| Botão direito | Clique direito |
| Botão do meio segurado + movimento | Rolagem |
| Botão centralizar | Traz o cursor para perto do centro da tela |
| Botão velocidade | Percorre lento → médio → rápido |
| 4 s parado | Estado ocioso, conexão mantida |
| 20 min parado | Sono profundo |
| Mover o aparelho, ou botão esquerdo | Desperta |

O *bonding* está habilitado: depois do primeiro pareamento o host reconecta
sozinho.

| LED RGB | Estado |
|---|---|
| Azul fixo | Iniciando |
| Azul, piscada lenta | Anunciando, aguardando conexão |
| Vermelho, piscada rápida | MPU6050 não respondeu |
| Apagado | Conectado e em uso |
| Verde / amarelo / vermelho por 1,5 s | Velocidade rápida / média / lenta |

O LED não fica aceso mostrando a velocidade o tempo todo: um LED aceso consome
mais que o aparelho inteiro em repouso. A cor aparece na troca e ao conectar.

---

## Autonomia

> Todos os valores vêm de folhas de dados dos componentes. **Nada foi medido.**
> As correntes reais podem divergir; ver [como medir](docs/ENERGIA.md#medindo-de-verdade).

### Passo 1 — Capacidade realmente disponível

Partindo de duas células de 3400 mAh em paralelo:

| Etapa | Cálculo | Resultado |
|---|---:|---:|
| Nominal | 3400 × 2 | 6800 mAh |
| Término do TP4056 | −100 mAh | 6700 mAh |
| Corte por subtensão do circuito | × 0,92 | **6164 mAh** |

**O término do TP4056** custa pouco mesmo. Ele encerra quando a corrente cai a
~1/10 da programada — 100 mA com o ajuste de 1 A. Num pacote de 6800 mAh isso é
C/68, um ponto muito adiante na curva de tensão constante, então a célula termina
quase cheia. A perda fica entre 1% e 2%.

**O corte por subtensão é o fator que costuma passar despercebido.** A célula é
especificada até 2,5 V, e a proteção do módulo só atua por volta de 2,4 V — mas o
aparelho para de funcionar muito antes disso. O regulador de 3,3 V da placa perde
a regulação quando a entrada cai abaixo de ~3,5 V, e o ESP32-C3 começa a sofrer
brownout nos picos de transmissão do rádio.

Na curva de descarga de uma célula de lítio, o trecho de 4,2 V até ~3,4 V
concentra cerca de 92% da capacidade. Os 8% restantes existem na célula, mas
ficam abaixo da tensão em que este circuito ainda funciona. **São ~536 mAh
inacessíveis** — cinco vezes o que você descontou pelo TP4056.

### Passo 2 — Consumo de cada componente

#### Estado ativo — em movimento

| Componente | Corrente | Origem |
|---|---:|---|
| ESP32-C3, CPU a 80 MHz | ~11 mA | Modem-sleep com CPU ativa |
| Rádio BLE, intervalo 7,5 ms, latência 0 | ~7 mA | Média das janelas de RX/TX a 0 dBm |
| MPU6050, giroscópio + acelerômetro a 200 Hz | 3,8 mA | Folha de dados |
| Divisor de bateria (100 k + 100 k) | 0,019 mA | 3,8 V ÷ 200 kΩ |
| Regulador da placa (ME6211) | 0,040 mA | Corrente de repouso |
| LED RGB | 0 mA | Apagado quando conectado |
| **Total** | **~22 mA** | |

#### Estado ocioso — parado, conexão mantida

| Componente | Corrente | Origem |
|---|---:|---|
| ESP32-C3, light sleep + despertar a 50 Hz | ~1,2 mA | Ciclo de trabalho da CPU |
| Rádio BLE, slave latency 40 (~480 ms) | ~0,6 mA | Ciclo de trabalho de ~0,4% |
| MPU6050 em detecção de movimento a 5 Hz | 0,020 mA | Folha de dados, só acelerômetro |
| Divisor de bateria | 0,019 mA | |
| Regulador da placa | 0,040 mA | |
| **Total** | **~1,9 mA** | |

#### Sono profundo

| Componente | Corrente |
|---|---:|
| ESP32-C3 em sono profundo, despertar por GPIO | 0,005 mA |
| MPU6050 em detecção de movimento | 0,020 mA |
| Divisor de bateria | 0,019 mA |
| Regulador da placa (ME6211) | 0,040 mA |
| **Total** | **~0,084 mA** |

Repare que **o ESP32 é o menor consumidor aqui**. O regulador da placa sozinho
gasta oito vezes o que o microcontrolador gasta, e o divisor de bateria quatro
vezes. É por isso que trocar de placa muda mais o sono profundo do que qualquer
alteração de firmware.

### Passo 3 — Consumo por jornada

**O aparelho fica na mão, e isso muda tudo.** O estado ocioso exige 4 segundos
seguidos abaixo de 6 °/s. Uma mão humana segurando um objeto nunca fica assim:
tremor, balanço do corpo, gesticular ao falar, caminhar pela sala. O giroscópio
lê acima do limiar quase o tempo todo.

Na prática, **segurar o aparelho o mantém em estado ativo**. O estado ocioso só
entra em cena quando ele é apoiado em algum lugar.

Perfil de referência — 8 h na mão, 16 h desligado na chave:

| Fase | Duração | Corrente | Consumo |
|---|---:|---:|---:|
| Ativo | 8 h | 22 mA | 176 mAh |
| Desligado na chave | 16 h | 0 mA | 0 mAh |
| **Total por jornada** | | | **176 mAh** |

A chave física corta OUT+ antes do ESP32, então desligado é zero de verdade.
Deixar em sono profundo em vez de desligar custaria 1,3 mAh na noite — diferença
sem importância, mas a chave é o hábito mais simples de manter.

Dois perfis alternativos, para você situar o seu caso entre eles:

| Perfil | Ativo | Ocioso | Por jornada |
|---|---:|---:|---:|
| Na mão o tempo todo | 8 h | 0 h | 176 mAh |
| Na mão com pausas na mesa | 6 h | 2 h | 136 mAh |
| Apoiado, pego só para apontar | 0,8 h | 7,2 h | 33 mAh |

O primeiro é o realista para uso em sala de aula, e é o que orienta os números
adiante. Os outros dois exigem que o aparelho seja pousado com frequência.

### Passo 4 — A autodescarga, que ninguém conta

Uma célula de lítio perde de 2% a 5% da carga por mês, parada, sem estar ligada a
nada. Adotando 3%:

```
6164 mAh × 3% = 185 mAh por mês = 42,6 mAh por semana
```

Numa semana de trabalho, com o aparelho desligado nos fins de semana:

| Item | Por semana |
|---|---:|
| 5 jornadas × 176 mAh | 880 mAh |
| 2 dias de fim de semana, desligado | 0 mAh |
| Autodescarga | 43 mAh |
| **Total** | **923 mAh** |

A autodescarga não depende de uso — é a química da célula. Neste perfil ela
responde por 5% do total, mas num perfil leve chegaria a 20%, e é a razão pela
qual dobrar a capacidade rende menos que o dobro de autonomia.

### Resultado

```
6164 mAh ÷ 923 mAh por semana = 6,7 semanas
```

Com duas 18650 em paralelo:

| Perfil de uso | Autonomia |
|---|---|
| **Na mão o tempo todo (referência)** | **~6,7 semanas** (~1,5 mês) |
| Na mão com pausas na mesa | ~8,5 semanas |
| Apoiado, pego só para apontar | ~29 semanas |
| Movimento contínuo, sem parar | ~280 horas |

Comparação entre baterias, no perfil de referência:

| Bateria | Capacidade utilizável | Autonomia |
|---|---:|---|
| LiPo 1000 mAh | ~900 mAh | **~1 semana** |
| 1 × 18650 (3400 mAh) | ~3082 mAh | ~3,3 semanas |
| 2 × 18650 (6800 mAh) | ~6164 mAh | **~6,7 semanas** |

É aqui que as duas células se justificam. Com uma LiPo, o professor carregaria
toda semana; com uma 18650, a cada três semanas; com duas, a cada mês e meio. A
carga de 12 a 15 h passa a acontecer seis vezes por ano, num fim de semana.

### O que pode derrubar esses números

| Fator | Autonomia resultante |
|---|---|
| **Células falsificadas** | **~3,4 semanas.** Muita 18650 vendida como "3400 mAh" entrega 1500 a 2200 mAh reais |
| Regulador AMS1117 em vez de ME6211 | ~5,5 semanas (−18%) |
| LED de alimentação não removido | ~6,1 semanas (−9%) |
| Temperatura baixa | A 0 °C, espere 20 a 30% a menos de capacidade |
| Células desbalanceadas | 1 a 2% de perda |

**Atenção à mudança de ordem.** Num aparelho que fica ligado o tempo todo, o LED
de alimentação e o regulador dominam o consumo. Aqui não: com a chave desligada
16 h por dia e nos fins de semana, esses drenos só atuam durante as 8 h de uso, e
nelas os 22 mA do estado ativo já mandam em tudo.

Continua valendo desolder o LED e preferir uma placa com ME6211 — mas o fator
realmente perigoso passou a ser a **procedência das células**. Uma 18650 falsa
corta a autonomia pela metade, e esse é o erro mais comum e mais caro da lista.

### O que passou a valer a pena otimizar

O estado ativo virou praticamente todo o consumo, e ele se divide assim:

| Contribuição | Corrente | Fatia |
|---|---:|---:|
| ESP32-C3, CPU a 80 MHz | ~11 mA | 50% |
| Rádio BLE a 125 Hz | ~7 mA | 32% |
| MPU6050 a 200 Hz | 3,8 mA | 17% |

Toda a economia que o firmware faz hoje — light sleep, slave latency, giroscópio
em espera — atua sobre estados em que este aparelho quase não entra neste perfil
de uso. Se a autonomia precisar subir, os caminhos são outros: reduzir
`REPORT_HZ_ACTIVE` de 125 para 60 ou 75 Hz, e baixar a taxa do MPU6050 junto.
São mudanças que valem medir antes, porque mexem na resposta do cursor.

Discussão completa em [ENERGIA.md](docs/ENERGIA.md).

---

## Estado do projeto

O firmware está completo e documentado, mas **nunca foi compilado nem executado
em hardware**. A primeira compilação é o próximo passo.

Esta tabela existe para quem for montar: ela diz onde procurar primeiro quando
algo não funcionar.

| Subsistema | Estado | Risco |
|---|---|---|
| Driver do MPU6050 | Escrito | Baixo — registradores padrão, amplamente documentados |
| Pipeline do cursor | Escrito, matemática simulada | Baixo — a simulação confere as contas |
| HID BLE | Escrito, API conferida nos headers do NimBLE 1.4.3 | Médio — descritor e serviços não exercitados |
| Botões e debounce | Escrito | Baixo |
| Centralização do cursor | Escrito | Médio — depende da aceleração de ponteiro do sistema |
| Níveis de velocidade e RGB | Escrito | Baixo |
| Slave latency e parâmetros de conexão | Escrito | Médio — negociação pode ser recusada pelo host |
| Light sleep automático | Escrito, com detecção de ausência de suporte | Médio — depende do core ter power management |
| **Wake-on-motion do MPU6050** | **Escrito a partir da documentação** | **Alto — sequência de registradores não verificada** |
| Consumo e autonomia | Estimado por folha de dados | **Alto — nenhuma medição feita** |

Se algo falhar na bancada, comece pelas duas últimas linhas.

Histórico completo do que foi construído em [CHANGELOG.md](CHANGELOG.md).

Relatos de erro de compilação ou de comportamento são bem-vindos via issue.

---

## Referências

O firmware foi escrito do zero, sem reaproveitamento de código. Estes projetos
foram consultados durante o desenvolvimento e valem a leitura:

| Projeto | Comentário |
|---|---|
| [k8r00/comet-air-mouse](https://github.com/k8r00/comet-air-mouse) | O mais próximo deste hardware (ESP32-C3 + MPU6050). A ideia do botão segurado como scroll veio dele. |
| [n1rml/esp32_airmouse](https://github.com/n1rml/esp32_airmouse) | Implementação em ESP-IDF, para o ESP32 clássico. |
| [WhoIsMrSentry/Esp32GestureMouse](https://github.com/WhoIsMrSentry/Esp32GestureMouse) | Boa referência de tratamento de botões. |
| [T-vK/ESP32-BLE-Mouse](https://github.com/T-vK/ESP32-BLE-Mouse) | A biblioteca clássica — **não funciona no C3**, depende de Bluetooth Classic ([issue #69](https://github.com/T-vK/ESP32-BLE-Mouse/issues/69)). |

Sobre a escolha de implementar o HID direto sobre o NimBLE em vez de usar uma
dessas bibliotecas, ver [ARQUITETURA.md](docs/ARQUITETURA.md).

---

## Licença

MIT — ver [LICENSE](LICENSE).
