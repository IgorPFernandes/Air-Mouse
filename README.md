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
  de **7 meses** entre cargas em uso de apresentação; ver [Autonomia](#autonomia).
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

Perfil de apresentação: 8 h de expediente com ~10% do tempo em movimento.

| Fase | Duração | Corrente | Consumo |
|---|---:|---:|---:|
| Ativo | 0,8 h | 22 mA | 17,6 mAh |
| Ocioso | 7,2 h | 1,9 mA | 13,7 mAh |
| Sono profundo | 16 h | 0,084 mA | 1,3 mAh |
| **Total por jornada** | | | **32,6 mAh** |

Fim de semana, 24 h dormindo: 2,0 mAh por dia.

### Passo 4 — A autodescarga, que ninguém conta

Uma célula de lítio perde de 2% a 5% da carga por mês, parada, sem estar ligada a
nada. Adotando 3%:

```
6164 mAh × 3% = 185 mAh por mês = 42,6 mAh por semana
```

Numa semana de uso, isso é o seguinte:

| Item | Por semana |
|---|---:|
| 5 jornadas × 32,6 mAh | 163 mAh |
| 2 dias de fim de semana × 2,0 mAh | 4 mAh |
| Autodescarga | 43 mAh |
| **Total** | **210 mAh** |

**A autodescarga responde por 20% do consumo total.** Ela não depende de uso — é
a química da célula. É também a razão pela qual dobrar a capacidade rende menos
que o dobro de autonomia: quanto maior o pacote, mais ele perde sozinho.

### Resultado

```
6164 mAh ÷ 210 mAh por semana = 29 semanas
```

| Perfil de uso | Autonomia |
|---|---|
| Apresentação (~10% em movimento) | **~29 semanas** (~6,7 meses) |
| Uso misto (~25% em movimento) | ~18 semanas (~4,2 meses) |
| Uso intenso (~50% em movimento) | ~11 semanas (~2,5 meses) |
| Movimento contínuo, sem pausas | **~277 horas** |

### O que pode derrubar esses números

| Fator | Efeito |
|---|---|
| **LED de alimentação da placa não removido** | 2 mA contínuos = 336 mAh/semana. Autonomia cai de 29 para **11 semanas** |
| **Regulador AMS1117 em vez de ME6211** | 5 mA de repouso = 840 mAh/semana. Autonomia cai para **~4 semanas** |
| **Células falsificadas** | Muita 18650 vendida como "3400 mAh" entrega 1500 a 2200 mAh reais. Divide tudo por dois |
| Temperatura baixa | Capacidade cai; a 0 °C, espere 20 a 30% a menos |
| Células desbalanceadas | 1 a 2% de perda |

Os dois primeiros são de longe os mais importantes, e ambos são resolvidos na
bancada, não no código: **desolde o LED de alimentação** e **confirme o CI
regulador da placa** antes de acreditar em qualquer número acima.

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
