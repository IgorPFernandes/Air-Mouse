#pragma once

namespace Power {

// Fixa a frequencia de CPU e tenta habilitar light sleep automatico.
void begin();

// True se o light sleep automatico foi aceito pelo core.
bool lightSleepActive();

// True se o despertar desta sessao veio do pino de movimento do MPU6050.
bool wokeFromMotion();

// Entra em sono profundo. So retorna pelo reset que acompanha o despertar,
// ou seja, setup() roda de novo. Desperta pelo botao esquerdo ou, com
// WAKE_ON_MOTION_ENABLED, pelo movimento detectado pelo MPU6050.
void deepSleep();

}  // namespace Power
