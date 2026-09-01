# Ordem de montagem e testes

Monte por etapas e teste cada uma antes de seguir. É muito mais fácil achar um
fio errado quando só existem três fios.

---

## Etapa 1 — Só a placa

Ligue o ESP32-C3 no USB, grave o firmware e abra o monitor serial.

```bash
pio run --target upload && pio device monitor
```

Esperado: a mensagem `MPU6050 nao respondeu` (normal, o sensor ainda não está
ligado) seguida de `Anunciando como "Air Mouse"`. O LED da placa pisca rápido.

Se não aparecer nada no monitor: confira se `USB CDC On Boot` está habilitado.

---

## Etapa 2 — MPU6050

Ligue os quatro fios: VCC→3V3, GND→GND, SDA→GPIO 5, SCL→GPIO 6.

Esperado no monitor:

```
Calibrando giroscopio - deixe o mouse PARADO por ~2 s...
Calibrado. bias = -1.23 / 0.45 / 2.10 dps
```

Deixe a placa **imóvel na mesa** durante essa parte.

**Se der `ERRO: MPU6050 nao respondeu`:**

- SDA e SCL invertidos é o erro mais comum — tente trocar.
- Alguns módulos GY-521 vêm com o pino AD0 solto; ligue AD0 no GND para forçar o
  endereço 0x68.
- Confira se o módulo está em 3V3, não em 5V.
- Use o scanner I²C em [../tools/i2c_scanner](../tools/i2c_scanner) para ver se
  o sensor aparece em algum endereço.

**Se o bias sair muito alto (acima de ~10 dps em algum eixo):** você mexeu
durante a calibração. Reinicie.

---

## Etapa 3 — Bluetooth

Ainda pelo USB, pareie com o computador: *Bluetooth → Adicionar dispositivo →
Air Mouse*.

Assim que conectar, o LED apaga e o cursor já deve responder ao movimento da
placa.

É aqui que você ajusta `INVERT_X`, `INVERT_Y`, `SWAP_AXES` e `SENSITIVITY` em
[`include/config.h`](../include/config.h) — ver [AJUSTES.md](AJUSTES.md). Faça
isso agora, com tudo ainda na bancada: depois de soldar dentro da caixa fica bem
menos divertido.

---

## Etapa 4 — Botões

Cada botão: um lado no GPIO, o outro no GND. Não precisa de resistor.

- GPIO 3 → clique esquerdo
- GPIO 4 → clique direito
- GPIO 10 → segure para rolar a página

Teste os três antes de soldar definitivo.

---

## Etapa 5 — Bateria

**Faça esta etapa com o USB desconectado.**

1. Bateria: fio vermelho em **B+**, fio preto em **B−** do TP4056.
2. **OUT+** → um terminal da chave; outro terminal da chave → pino **5V** do ESP32.
3. **OUT−** → **GND** do ESP32.
4. Ligue a chave. A placa deve ligar e o LED começar a piscar.

Confira com multímetro antes de ligar a chave pela primeira vez: entre OUT+ e
OUT− você deve ler algo entre 3,4 V e 4,2 V.

### Carregando

Plugue o USB **no TP4056**, não no ESP32. LED vermelho = carregando, azul (ou
verde, depende do módulo) = cheio.

Pode deixar a chave ligada e usar enquanto carrega, mas o mais seguro é
desligar.

---

## Etapa 6 — Divisor de bateria (opcional)

```
  BAT+ (ou OUT+) ──[100k]──┬──[100k]── GND
                           │
                           ├── GPIO 1
                           │
                        [100nF] ── GND
```

Ligue no lado **OUT+** para que a leitura acompanhe o que a proteção está
entregando.

Depois de montar, coloque `#define DEBUG_PLOT_MS 500` e confira no monitor se a
porcentagem faz sentido. Bateria cheia deve mostrar perto de 100%.

Se o valor sair pela metade ou dobrado, ajuste `BAT_DIVIDER_RATIO` — ele é
`(R1+R2)/R2`. Com dois resistores iguais, é 2.0.

---

## Etapa 7 — Caixa

Ponto que faz diferença: **o MPU6050 precisa estar bem preso**. Se ele balançar
dentro da caixa, o cursor treme e nenhuma quantidade de filtro resolve. Cola
quente nas bordas do módulo já basta.

Oriente o módulo com o texto para cima e o lado dos pinos apontando para o lado
que você vai segurar. Se ficar em outra orientação, é só ajustar `SWAP_AXES` e
os `INVERT_*`.

Deixe o conector USB do TP4056 acessível por uma abertura na caixa.

---

## Checklist final

- [ ] Calibração no boot com o mouse parado
- [ ] Cursor anda no sentido certo nos dois eixos
- [ ] Os três botões respondem
- [ ] Reconecta sozinho ao ligar (bonding)
- [ ] Dorme depois de 5 min e acorda no botão esquerdo
- [ ] Carrega pelo TP4056 e a chave corta a alimentação
- [ ] MPU6050 firme na caixa
