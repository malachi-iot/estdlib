/*
 * @file
 * STM32 identifier
 * Doing this explicit check as I can't find where to find a general
 * STM32 identifier
 */
#if defined(STM32F746xx) || \
    defined(STM32F7) || \
    defined(STM32F4) || \
    defined(STM32F3)
#define ESTD_MCU_STM32
#endif