/**
 * @file
 * @brief Well-known FPGA configuration registers of Xilinx Series-7 and UltraScale FPGAs
 */

#include "unbit/fpga/xilinx/config_reg.hpp"

#include <iostream>

namespace unbit
{
	namespace fpga
	{
		namespace xilinx
		{
			//------------------------------------------------------------------------------------------
			/**
			 * @brief Gets the display name of a configuration register.
			 * @internal
			 *
			 * @param reg specifies the configuration register.
			 * @return A pointer to a string literal with the namd of the configuration register. In case of
			 *   an unknown/invalid register a pointer to the string literal @c "???" is returned.
			 */
			static const char* to_string(config_reg reg)
			{
				switch (reg)
				{
					case config_reg::CRC    : return "CRC";    // CRC register
					case config_reg::FAR    : return "FAR";    // Frame address register
					case config_reg::FDRI   : return "FDRI";   // Frame data register input (config data write)
					case config_reg::FDRO   : return "FDRO";   // Frame data register output (config data read)
					case config_reg::CMD    : return "CMD";    // Command register
					case config_reg::CTL0   : return "CTL0";   // Control register 0
					case config_reg::MASK   : return "MASK";   // Masking register for CTL0 and CTL1
					case config_reg::STAT   : return "STAT";   // Status register
					case config_reg::LOUT   : return "LOUT";   // Legacy output register
					case config_reg::COR0   : return "COR0";   // Configuration option register 0
					case config_reg::MFWR   : return "MFWR";   // Multi frame write register
					case config_reg::CBC    : return "CBC";    // Initial CBC value register
					case config_reg::IDCODE : return "IDCODE"; // Device ID register
					case config_reg::AXSS   : return "AXSS";   // User access register
					case config_reg::COR1   : return "COR1";   // Configuration option register 1
					case config_reg::RSVD15 : return "RSVD15";
					case config_reg::WBSTAR : return "WBSTAR"; // Warm boot start address register.
					case config_reg::TIMER  : return "TIMER";  // Watchdog timer register.
					case config_reg::RSVD18 : return "RSVD18";
					case config_reg::RSVD19 : return "RSVD19";
					case config_reg::RSVD20 : return "RSVD20";
					case config_reg::RSVD21 : return "RSVD21";
					case config_reg::BOOTSTS: return "BOOTSTS"; // Boot history status register
					case config_reg::RSVD23 : return "RSVD23";
					case config_reg::CTL1   : return "CTL1";    // Control register 1
					case config_reg::RSVD25 : return "RSVD25";
					case config_reg::RSVD26 : return "RSVD26";
					case config_reg::RSVD27 : return "RSVD27";
					case config_reg::RSVD28 : return "RSVD28";
					case config_reg::RSVD29 : return "RSVD29";
					case config_reg::RSVD30 : return "RSVD30";  // Reserved 30 (Switch to next SLR)
					case config_reg::BSPI   : return "BSPI";    // BPI/SPI configuration options register
					default:                  return "???";
				}
			}

			//------------------------------------------------------------------------------------------
			std::ostream& operator<<(std::ostream& os, config_reg reg)
			{
				return os << to_string(reg);
			}
		}
	}
}
