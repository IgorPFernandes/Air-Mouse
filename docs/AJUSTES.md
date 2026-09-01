# Ajuste do comportamento

Todos os parâmetros ficam em [`include/config.h`](../include/config.h). O efeito
de cada um está descrito em [ARQUITETURA.md](ARQUITETURA.md).

Para ver os números ao vivo enquanto ajusta, defina `DEBUG_PLOT_MS 100` e abra o
monitor serial.

---

## Diagnóstico rápido

| Sintoma | Ajuste |
|---|---|
| Cursor anda no sentido contrário | Alterne `INVERT_X` ou `INVERT_Y` entre `0` e `1` |
| Horizontal e vertical trocados | `SWAP_AXES 1` |
| Cursor treme com o aparelho parado | Aumente `GYRO_DEADZONE_DPS` para `2.0` ou `2.5` |
| Cursor deriva sozinho, sempre no mesmo sentido | Reinicie com o aparelho imóvel: a calibração pegou movimento |
| Cursor lento para atravessar a tela | Aumente `ACCEL_GAIN` (`0.0016` → `0.003`) |
| Difícil acertar alvos pequenos | Reduza `SENSITIVITY` e `ACCEL_GAIN` |
| Cursor "molhado", com atraso | Reduza `SMOOTHING` (`0.35` → `0.2`) |
| Movimento picotado em gestos lentos | Reduza `GYRO_DEADZONE_DPS` |
| Scroll rápido demais | Aumente `SCROLL_DIVISOR` |
| Scroll no sentido errado | `SCROLL_INVERT 1` |
| Cursor anda na diagonal com o aparelho inclinado | Confirme `ROLL_COMPENSATION 1` |

---

## Os dois parâmetros principais

```c
#define SENSITIVITY 0.085f  // ganho linear: velocidade geral
#define SMOOTHING   0.35f   // suavização: estabilidade contra latência
```

Ajuste `SENSITIVITY` primeiro, até a velocidade geral ficar confortável. Só então
mexa em `SMOOTHING`, e em passos pequenos: é o parâmetro que troca estabilidade
por latência, e valores acima de `0.6` deixam o cursor perceptivelmente atrasado
em relação à mão.

`ACCEL_GAIN` é o que separa "preciso" de "rápido". Se você quer os dois, aumente
`ACCEL_GAIN` e reduza `SENSITIVITY` — movimento lento fica fino, movimento rápido
continua cobrindo a tela.

---

## Ordem recomendada

1. **Eixos.** `INVERT_X`, `INVERT_Y`, `SWAP_AXES`. Faça com o aparelho ainda na
   bancada, antes de fechar a caixa.
2. **Zona morta.** Aponte para um alvo e solte a mão: o cursor deve ficar imóvel.
   Aumente `GYRO_DEADZONE_DPS` até parar de tremer, sem passar disso — cada
   décimo a mais tira resolução do movimento fino.
3. **Velocidade.** `SENSITIVITY`, depois `ACCEL_GAIN`.
4. **Suavização.** `SMOOTHING`, por último e em passos pequenos.
5. **Scroll.** `SCROLL_DIVISOR` e `SCROLL_INVERT`.

---

## Energia

```c
#define IDLE_SLEEP_MS   (5UL * 60UL * 1000UL)  // tempo até dormir
#define IDLE_MOTION_DPS 6.0f                   // acima disso conta como uso
```

Se o aparelho dormir enquanto está em uso, reduza `IDLE_MOTION_DPS`. Se nunca
dormir, o valor está baixo demais e o ruído do sensor está sendo lido como
atividade.

Lembre que despertar é um reset: o giroscópio é recalibrado, então o aparelho
precisa estar parado no momento em que acorda.
