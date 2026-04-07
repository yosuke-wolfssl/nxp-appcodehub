/*
 * Non-Secure SystemInit override for MCXN947.
 *
 * The MCUX SDK weak SystemInit() writes secure-only system registers.
 * When this image runs in Non-Secure state, those accesses can fault.
 */

#include <zephyr/autoconf.h>
#include "fsl_device_registers.h"

#if defined(CONFIG_TRUSTED_EXECUTION_NONSECURE)

void SystemInit(void)
{
	/* Enable coprocessor access in Non-Secure state */
#if ((__FPU_PRESENT == 1) && (__FPU_USED == 1))
	SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2));    /* CP10, CP11 for FPU */
#endif
	SCB->CPACR |= ((3UL << 0*2) | (3UL << 1*2));      /* CP0, CP1 for PowerQuad */

	/* Disable RAM ECC to maximize available memory for Ethernet and app buffers */
	SYSCON->ECC_ENABLE_CTRL = 0;
	SYSCON->NVM_CTRL &= ~SYSCON_NVM_CTRL_DIS_MBECC_ERR_DATA_MASK;

	/* Enable flash cache (LPCAC) for Ethernet/networking performance */
	SYSCON->LPCAC_CTRL &= ~SYSCON_LPCAC_CTRL_DIS_LPCAC_MASK;

    /* Disable aGDET interrupt and reset */
    SPC0->ACTIVE_CFG |= SPC_ACTIVE_CFG_GLITCH_DETECT_DISABLE_MASK;
    SPC0->GLITCH_DETECT_SC &= ~SPC_GLITCH_DETECT_SC_LOCK_MASK;
    SPC0->GLITCH_DETECT_SC = 0x3C;
    SPC0->GLITCH_DETECT_SC |= SPC_GLITCH_DETECT_SC_LOCK_MASK;

	/*
	 * Secure boot firmware is responsible for:
	 * - VTOR setup (Zephyr handles via linker script)
	 * - Glitch detection security setup (bootloader configures)
	 * - System clock initialization (bootloader pre-configures before NS handoff)
	 */
	SystemInitHook();
}

#endif /* CONFIG_TRUSTED_EXECUTION_NONSECURE */
