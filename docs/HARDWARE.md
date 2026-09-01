# Hardware

## Lista de materiais

| Peça | Observação |
|---|---|
| ESP32-C3 | SuperMini ou DevKitM-1 |
| MPU6050 | Módulo GY-521 |
| Bateria LiPo/Li-ion 3.7 V | 400 a 1000 mAh |
| Módulo de carga TP4056 | **Com proteção** — ver aviso abaixo |
| 3 × botão táctil | Esquerdo, direito e meio |
| Chave liga/desliga | Recomendada |
| 2 × resistor 100 kΩ | Divisor de bateria, opcional |
| 1 × capacitor 100 nF | Filtro do divisor, opcional |

---

## Pinagem

| Sinal | Pino | Observação |
|---|---|---|
| MPU6050 SDA | GPIO 5 | I²C a 400 kHz |
| MPU6050 SCL | GPIO 6 | |
| MPU6050 VCC | 3V3 | O GY-521 aceita 3,3 V direto |
| MPU6050 GND | GND | |
| Botão esquerdo | GPIO 3 | Também é o botão de despertar |
| Botão direito | GPIO 4 | |
| Botão do meio | GPIO 10 | Segurado = modo scroll |
| Divisor de bateria | GPIO 1 | ADC1 |
| LED de status | GPIO 8 | LED embutido da SuperMini, aceso em nível baixo |

```
              ESP32-C3
            ┌───────────┐
   MPU6050  │           │
   VCC ─────┤ 3V3       │
   GND ─────┤ GND       │
   SDA ─────┤ GPIO 5    │
   SCL ─────┤ GPIO 6    │
            │           │
  Botão E ──┤ GPIO 3    │──┐
  Botão D ──┤ GPIO 4    │  │ o outro lado de
  Botão M ──┤ GPIO 10   │──┘ todos vai no GND
            │           │
  Divisor ──┤ GPIO 1    │
            │           │
   TP4056 ──┤ 5V        │
    OUT+    │           │
   TP4056 ──┤ GND       │
    OUT-    └───────────┘
```

Os botões usam o pull-up interno: um lado no GPIO, o outro no GND, sem resistor
externo.

### Pinos evitados

GPIO 2, 8 e 9 são pinos de *strapping* e interferem no boot; GPIO 18 e 19 são o
USB nativo. A exceção é o GPIO 8, usado apenas como saída para o LED, o que é
seguro depois do boot.

O botão esquerdo **precisa** ficar entre GPIO 0 e 5: é a única faixa que acorda o
ESP32-C3 do sono profundo. `power.cpp` verifica isso em tempo de compilação.

---

## Alimentação

```
  Bateria 3.7V ──┬── B+ ──┐
                 │        │  TP4056
                 └── B- ──┤  (com proteção)
                          │
              OUT+ ───[chave]─── 5V do ESP32-C3
              OUT- ─────────────  GND do ESP32-C3
```

O pino marcado "5V" da SuperMini entra no regulador de 3,3 V da placa. Com 3,7 V
na entrada ele ainda regula, e continua funcionando até a bateria chegar perto de
3,4 V — que é onde se quer parar de descarregar de qualquer forma.

### Três pontos que decidem a segurança do projeto

1. **Use TP4056 com proteção.** O módulo sem os dois CIs auxiliares não protege
   contra descarga profunda. Uma célula LiPo levada abaixo de ~2,5 V degrada de
   forma permanente e pode inchar.
2. **Bateria em B+/B−, ESP32 em OUT+/OUT−.** Nunca os dois no mesmo par: é essa
   separação que faz a proteção atuar.
3. **A chave vai entre OUT+ e o ESP32**, não entre a bateria e o TP4056. Assim é
   possível carregar com o aparelho desligado.

Ao gravar o firmware pelo USB do ESP32, mantenha a chave desligada para a placa
não devolver tensão ao circuito de carga.

---

## Medição de bateria (opcional)

```
  OUT+ ──[100k]──┬──[100k]── GND
                 │
                 ├── GPIO 1
                 │
              [100nF]
                 │
                GND
```

Ligue no lado **OUT+** para a leitura refletir o que a proteção está entregando.

Sem essa montagem, ajuste `BATTERY_SENSE_ENABLED` para `0` em
[`include/config.h`](../include/config.h); o firmware passa a reportar 100%.

Se o percentual sair pela metade ou dobrado, ajuste `BAT_DIVIDER_RATIO` — o valor
é `(R1 + R2) / R2`, ou seja, `2.0` com dois resistores iguais.

---

## Consumo e autonomia

Cerca de 45 mA conectado e em movimento. Em sono profundo o chip fica na casa de
poucos µA, mas o consumo real depende do regulador da placa: na SuperMini costuma
ficar entre 40 e 300 µA.

| Bateria | Uso contínuo |
|---|---|
| 400 mAh | ~9 h |
| 1000 mAh | ~22 h |

---

## Montagem mecânica

O MPU6050 precisa estar **rigidamente preso** à caixa. Se o módulo se mover
dentro dela, o sensor lê essa vibração como movimento real e nenhum ajuste de
filtro resolve. Cola quente nas bordas do módulo basta.

Deixe o conector USB do TP4056 acessível por uma abertura.
