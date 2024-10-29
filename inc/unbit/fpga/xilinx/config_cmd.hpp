/**
 * @file
 * @brief Well-known command register codes for Xilinx Series-7 and UltraScale FPGAs
 */
#ifndef UNBIT_XILINX_CONFIG_CMD_HPP_
#define UNBIT_XILINX_CONFIG_CMD_HPP_ 1

#include <cstdint>
#include <cstddef>

#include <iosfwd>

namespace unbit
{
	namespace fpga
	{
		namespace xilinx
		{			
			//------------------------------------------------------------------------------------------
			/**
			 * @brief Well-known command register codes for Xilinx Series-7 and UltraScale FPGAs
			 */
			enum class config_cmd : uint32_t
			{
				NUL       = 0b00000, //!< Null command.
				WCFG      = 0b00001, //!< Write configuration data (used prior to FDRI write)
				MFW       = 0b00010, //!< Multi frame write (used prior to MFWR writes in compressed bitstream)
				DGHIGH    = 0b00011, //!< Last frame / Deassert GHIGH_B signal
				RCFG      = 0b00100, //!< Read configuration data (used prior to FDRO read)
				START     = 0b00101, //!< Begin start-up sequence (activates after next CRC check and DESYNC)
				URAM      = 0b00110, //!< Trigger clearing of the URAM
				RCRC      = 0b00111, //!< Reset CRC register
				AGHIGH    = 0b01000, //!< Assert GHIGH_B signal (places interconnect in High-Z state)
				SWITCH    = 0b01001, //!< Switch CCLK frequency
				GRESTORE  = 0b01010, //!< Pulse GRESTORE signal (set/reset CLB flip-flops)
				SHUTDOWN  = 0b01011, //!< Begin shutdown sequences (acivates after next CRC or RCRC)
				DESYNC    = 0b01101, //!< Desychronize the devie (at end of configuration)
				RSVD14    = 0b01110,
				IPROG     = 0b01111, //!< Trigger warm boot (internal PROG) 
				CRCC      = 0b10000, //!< Trigger calculation of first readback CRC after reconfiguration
				LTIMER    = 0b10001, //!< Reload watchdog timer
				BSPI_READ = 0b10010, //!< BPI/SPI re-initialize bitstream read.
				FALL_EDGE = 0b10011, //!< Switch to negative edge clocking (config data capture on falling edge)					
				RSVD20    = 0b10100,
				RSVD21    = 0b10101,
				RSVD22    = 0b10110,
				RSVD23    = 0b10111,
				RSVD24    = 0b11000,
				RSVD25    = 0b11001,
				RSVD26    = 0b11010,
				RSVD27    = 0b11011,
				RSVD28    = 0b11100,
				RSVD29    = 0b11101,
				RSVD30    = 0b11110,
				RSVD31    = 0b11111,
			};

			/**
			 * @brief Writes the textual representation of a @c config_cmd value to an output stream.
			 *
			 * @param os is the target output stream to write to.
			 * @param cmd is the @c config_cmd value to be written.
			 * @return A reference to the @p os output stream.
			 */
			std::ostream& operator<<(std::ostream& os, config_cmd cmd);
		}
	}
}

#endif // UNBIT_XILINX_CONFIG_CMD_HPP_
