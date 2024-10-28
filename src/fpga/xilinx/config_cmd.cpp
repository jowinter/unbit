/**
 * @file
 * @brief Well-known FPGA configuration command codes of Xilinx Series-7 and UltraScale FPGAs
 */

#include "unbit/fpga/xilinx/config_cmd.hpp"

#include <iostream>

namespace unbit
{
	namespace fpga
	{
		namespace xilinx
		{
			//------------------------------------------------------------------------------------------
			/**
			 * @brief Gets the display name of a configuration command code.
			 * @internal
			 *
			 * @param c,d specifies the configuration command code.
			 * @return A pointer to a string literal with the name of the configuration command. In case of
			 *   an unknown/invalid register a pointer to the string literal @c "???" is returned.
			 */
			static const char* to_string(config_cmd cmd)
			{
				switch (cmd)
				{
					case config_cmd::NUL      : return "NULL";      // Null command.
					case config_cmd::WCFG     : return "WCFG";      // Write configuration data (used prior to FDRI write)
					case config_cmd::MFW      : return "MFW";       // Multi frame write (used prior to MFWR writes in compressed bitstream)
					case config_cmd::DGHIGH   : return "DGHIGH";    // Last frame / Deassert GHIGH_B signal
					case config_cmd::RCFG     : return "RCFG";      // Read configuration data (used prior to FDRO read)
					case config_cmd::START    : return "START";     // Begin start-up sequence (activates after next CRC check and DESYNC)
					case config_cmd::URAM     : return "URAM";      // Trigger clearing of the URAM
					case config_cmd::RCRC     : return "RCRC";      // Reset CRC register
					case config_cmd::AGHIGH   : return "AGHIGH";    // Assert GHIGH_B signal (places interconnect in High-Z state)
					case config_cmd::SWITCH   : return "SWITCH";    // Switch CCLK frequency
					case config_cmd::GRESTORE : return "GRESTORE";  // Pulse GRESTORE signal (set/reset CLB flip-flops)
					case config_cmd::SHUTDOWN : return "SHUTDOWN";  // Begin shutdown sequences (acivates after next CRC or RCRC)
					case config_cmd::DESYNC   : return "DESYNC";    // Desychronize the devie (at end of configuration)
					case config_cmd::RSVD14   : return "RSVD14";
					case config_cmd::IPROG    : return "IPROG";     // Trigger warm boot (internal PROG)
					case config_cmd::CRCC     : return "CRCC";      // Trigger calculation of first readback CRC after reconfiguration
					case config_cmd::LTIMER   : return "LTIMER";    // Reload watchdog timer
					case config_cmd::BSPI_READ: return "BSPI_READ"; // BPI/SPI re-initialize bitstream read.
					case config_cmd::FALL_EDGE: return "FALL_EDGE"; // Switch to negative edge clocking (config data capture on falling edge)
					case config_cmd::RSVD20   : return "RSVD20";
					case config_cmd::RSVD21   : return "RSVD21";
					case config_cmd::RSVD22   : return "RSVD22";
					case config_cmd::RSVD23   : return "RSVD23";
					case config_cmd::RSVD24   : return "RSVD24";
					case config_cmd::RSVD25   : return "RSVD25";
					case config_cmd::RSVD26   : return "RSVD26";
					case config_cmd::RSVD27   : return "RSVD27";
					case config_cmd::RSVD28   : return "RSVD28";
					case config_cmd::RSVD29   : return "RSVD29";
					case config_cmd::RSVD30   : return "RSVD30";
					case config_cmd::RSVD31   : return "RSVD31";
					default:                    return "???";
				}
			}

			//------------------------------------------------------------------------------------------
			std::ostream& operator<<(std::ostream& os, config_cmd reg)
			{
				return os << to_string(reg);
			}
		}
	}
}
