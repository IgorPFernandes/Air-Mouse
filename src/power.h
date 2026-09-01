#pragma once

namespace Power {

// Entra em sono profundo. So retorna pelo reset que acompanha o despertar,
// ou seja, setup() roda de novo.
void deepSleepUntilWakeButton();

}  // namespace Power
