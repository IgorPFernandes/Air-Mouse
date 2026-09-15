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
ligado) seguida de `Anunciando como "Air Mouse"`. O LED RGB pisca em vermelho, rápido.

Se não aparecer nada no monitor: confira se `USB CDC On Boot` está habilitado.

---

## Etapa 2 — MPU6050

Ligue os quatro fios: VCC→3V3, GND→GND, SDA→GPIO 5, SCL→GPIO 6.

Esperado no monitor:

```
Calibrando o giroscopio, mantenha o aparelho parado...
Calibrado. Bias: -1.23 / 0.45 / 2.10 deg/s
```

Deixe a placa **imóvel na mesa** durante essa parte.

**Se der `MPU6050 nao respondeu`:**

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
- GPIO 7 → centralizar o cursor
- GPIO 9 → trocar a velocidade

Teste os cinco antes de soldar definitivo.

O botão de centralizar precisa que `SCREEN_WIDTH` e `SCREEN_HEIGHT` batam com a
sua tela. Clique e veja onde o cursor para: se ficar sistematicamente longe do
centro, aumente `RECENTER_STEPS`.

**GPIO 9 é o pino BOOT.** Se esse botão estiver pressionado durante o reset, a
placa entra em modo de gravação em vez de arrancar. É inofensivo — e útil na hora
de gravar o firmware.

---

## Etapa 5 — LED RGB

Ânodo comum no **3V3**; cada cátodo vai ao GPIO por um resistor de **220 Ω**.

- GPIO 8 → vermelho
- GPIO 20 → verde
- GPIO 21 → azul

Ao ligar, o LED deve ficar azul fixo e depois piscar em azul devagar. Aperte o
botão de velocidade: a cor deve mudar para verde, amarelo ou vermelho por 1,5 s
e depois apagar.

Se as cores saírem trocadas, confira a ordem dos pinos. Se o LED ficar aceso ao
contrário — apagado quando deveria acender — você tem um LED de cátodo comum:
defina `RGB_COMMON_ANODE 0`. Nesse caso reveja o boot, porque GPIO 8 é pino de
strapping e passa a ficar em nível baixo em repouso.

---

## Etapa 6 — Baterias

**Faça esta etapa com o USB desconectado.**

### Antes de unir as células

Duas 18650 em paralelo com tensões diferentes trocam corrente entre si, e uma
diferença de 0,5 V pode gerar dezenas de ampères no instante da conexão.

1. Carregue cada célula **separadamente** até a mesma tensão.
2. Meça as duas com multímetro. A diferença precisa estar **abaixo de 0,05 V**.
3. Só então una os positivos entre si e os negativos entre si.

### Ligação

1. Positivos unidos → **B+** do TP4056. Negativos unidos → **B−**.
2. **OUT+** → um terminal da chave; outro terminal da chave → pino **5V** do ESP32.
3. **OUT−** → **GND** do ESP32.
4. Ligue a chave. A placa deve ligar e o LED começar a piscar em azul.

Confira com multímetro antes de ligar a chave pela primeira vez: entre OUT+ e
OUT− você deve ler algo entre 3,4 V e 4,2 V.

Uma célula só usa exatamente a mesma ligação — é só omitir a segunda.

### Carregando

Plugue o USB **no TP4056**, não no ESP32. LED vermelho = carregando, azul (ou
verde, depende do módulo) = cheio.

Com 6800 mAh, a carga completa leva de 12 a 15 horas — o TP4056 é linear e entra
em limitação térmica antes de sustentar 1 A. Ele **conclui** a carga, porque
encerra por corrente e não por tempo; só demora. Ver
[HARDWARE.md](HARDWARE.md#o-calor-que-é-o-limite-de-verdade).

O módulo fica quente durante horas. Não carregue com ele enfiado na caixa
fechada, e prefira a chave desligada durante a carga.

---

## Etapa 7 — Divisor de bateria (opcional)

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

## Etapa 8 — Caixa

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
- [ ] Os cinco botões respondem
- [ ] Centralizar leva o cursor para perto do meio da tela
- [ ] As três cores de velocidade aparecem e apagam sozinhas
- [ ] Reconecta sozinho ao ligar (bonding)
- [ ] Fica ocioso após 4 s e dorme após 20 min
- [ ] Acorda ao mover o aparelho, sem apertar nada
- [ ] Células equalizadas antes de unir em paralelo
- [ ] Carrega pelo TP4056 e a chave corta a alimentação
- [ ] MPU6050 firme na caixa
- [ ] LED de alimentação da placa removido
