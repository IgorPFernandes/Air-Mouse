# Histórico

Formato baseado em [Keep a Changelog](https://keepachangelog.com/pt-BR/1.1.0/).

---

## Não lançado

Todo o conteúdo abaixo foi escrito, revisado e documentado, mas **nada foi
compilado nem executado em hardware**. Não há versão lançada porque não há nada
validado. Ver [Estado de verificação](README.md#estado-do-projeto).

### Firmware — base

- Driver do MPU6050 por acesso direto aos registradores I²C, sem biblioteca
  externa. Escala de ±500 °/s, DLPF de 44 Hz, taxa interna de 200 Hz.
- Calibração de bias no boot **com validação**: a medição é rejeitada quando a
  dispersão indica que a placa foi movida, caso em que o bias obtido faria o
  cursor derivar sozinho. Até três tentativas.
- Perfil HID de mouse implementado direto sobre o NimBLE, sem biblioteca HID de
  terceiros. As bibliotecas mais conhecidas dependem de Bluetooth Classic, que o
  ESP32-C3 não possui.
- Descritor com X e Y em **16 bits** em vez dos 8 bits habituais: em 8 bits o
  deslocamento satura em 127 por relatório e movimentos rápidos ficam truncados.
- Bonding habilitado — o host reconecta sozinho após o primeiro pareamento.

### Firmware — tratamento de sinal

Pipeline em `pointer.cpp`, nesta ordem (o porquê de cada etapa está em
[ARQUITETURA.md](docs/ARQUITETURA.md)):

- **Compensação de rotação do pulso** por filtro complementar. Sem ela, segurar o
  aparelho inclinado faz o cursor andar na diagonal.
- **Zona morta subtrativa** em vez de zerada, evitando o degrau de velocidade ao
  cruzar a borda do limiar.
- **Suavização exponencial**.
- **Curva de aceleração** com termo linear e quadrático.
- **Acumulador de sub-pixel**: a fração de pixel é preservada entre relatórios.
  Sem isso, todo movimento lento seria truncado a zero.

### Firmware — controles

- Modo scroll com o botão do meio segurado.
- **Botão de centralizar o cursor.** O HID reporta deslocamento relativo, nunca
  posição absoluta; a sequência encosta o cursor no canto superior esquerdo, onde
  o sistema o prende, e de lá caminha meia tela em passos pequenos. A chegada é
  próxima do centro, não exata — a aceleração de ponteiro do sistema distorce a
  segunda metade.
- **Três níveis de velocidade** (0,55× / 1,0× / 1,6×), percorridos por botão. O
  multiplicador incide sobre `SENSITIVITY` e `ACCEL_GAIN` juntos, preservando o
  formato da curva. O nível sobrevive ao sono profundo via RAM do RTC.
- **LED RGB** acumulando estado do aparelho e confirmação de velocidade. Ânodo
  comum, para manter GPIO 8 seguro como pino de strapping.

### Firmware — energia

Três estados: ativo (~22 mA), ocioso com conexão mantida (~2 mA, sai em ~50 ms) e
sono profundo (~65 a 325 µA, sai em 1 a 2 s). Estimativas, não medições.

- **Slave latency** alternando entre 0 (ativo) e 40 (ocioso). O rádio deixa de
  acordar a cada 7,5 ms sem perder a capacidade de responder no evento seguinte.
  É o maior ganho isolado do projeto com a conexão ativa.
- Giroscópio posto em espera no estado ocioso; o acelerômetro assume a detecção
  de movimento a ~20 µA, contra 3,9 mA do modo pleno.
- **Despertar por movimento**: pegar o aparelho o acorda, sem apertar nada.
- CPU fixada em 80 MHz, mínimo com o rádio BLE estável no ESP32-C3.
- Light sleep automático entre quadros, com detecção de ausência de suporte a
  power management no core.
- Potência de transmissão reduzida de +9 para 0 dBm.
- Anúncio passa a intervalo lento após 30 s e dorme após 3 min sem conexão.
- Sono profundo adiado para 20 min, para que uma pausa durante uma aula não custe
  reconexão no momento em que se aponta para a tela.

### Estrutura

- Firmware separado em oito módulos com responsabilidade única; `main.cpp` cuida
  apenas de orquestração.
- Configuração inteira concentrada em [`include/config.h`](include/config.h).
- Versões fixadas no `platformio.ini`: arduino-esp32 2.0.17 via espressif32 6.9.0
  e NimBLE-Arduino 1.4.3.
- Scanner I²C em [`tools/i2c_scanner`](tools/i2c_scanner) para diagnóstico.

### Documentação

- [HARDWARE.md](docs/HARDWARE.md) — materiais, pinagem, alimentação, carga, peso.
- [MONTAGEM.md](docs/MONTAGEM.md) — montagem em oito etapas, com teste a cada uma.
- [ARQUITETURA.md](docs/ARQUITETURA.md) — como o firmware funciona e por quê.
- [AJUSTES.md](docs/AJUSTES.md) — sensibilidade, velocidade e diagnóstico.
- [ENERGIA.md](docs/ENERGIA.md) — orçamento de consumo e autonomia.

---

## Decisões de projeto que valem registro

**HID direto sobre o NimBLE.** O ESP32-C3 só tem BLE. As bibliotecas de mouse
mais citadas dependem de Bluetooth Classic e não funcionam nesta peça; as
alternativas em BLE encapsulam o NimBLE e quebram a cada atualização do core.
Como o NimBLE já traz `NimBLEHIDDevice`, a camada extra só adicionaria
superfície de falha.

**O LED não fica aceso mostrando a velocidade.** Um LED aceso consome de 2 a 5 mA,
mais que o aparelho inteiro em repouso, e anularia boa parte da gestão de energia.
A cor aparece por 1,5 s na troca e ao conectar, que são os momentos em que a
informação serve.

**Divisor de bateria não chaveado.** Chegou a ser implementado, mas o GPIO que o
fazia virou o botão de centralizar quando os controles novos esgotaram os pinos.
Os 20 µA drenados são 0,3% de uma 18650 em um ano.

**Duas 18650 em paralelo.** Escolha consciente de autonomia sobre peso, para a
etapa de protótipo. O conjunto fica em ~160 g contra ~102 g com uma célula, e a
análise de peso está em
[HARDWARE.md](docs/HARDWARE.md#o-peso--vale-conversar-antes-de-comprar).

---

## Limites conhecidos

- **Nada foi compilado.** É o próximo passo.
- A sequência de registradores de wake-on-motion do MPU6050 foi escrita a partir
  da documentação e não foi verificada em hardware. É a parte com maior chance de
  precisar de ajuste.
- Os números de consumo são estimativas de folha de dados. Ver
  [ENERGIA.md](docs/ENERGIA.md#medindo-de-verdade).
- A centralização do cursor chega perto do centro, não no centro exato.
- Não há posicionamento absoluto, e não pode haver com HID relativo. Um formato
  que exija "apontar para onde o cursor deve ir" precisaria de referência externa.
