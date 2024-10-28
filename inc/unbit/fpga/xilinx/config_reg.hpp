/**
 * @file
 * @brief Well-known FPGA configuration registers of Xilinx Series-7 and UltraScale FPGAs
 */
#ifndef UNBIT_XILINX_CONFIG_REG_HPP_
#define UNBIT_XILINX_CONFIG_REG_HPP_ 1

#include <cstdint>
#include <cstddef>

#include <iosfwd>

namespace unbit
{
	namespace fpga
	{
		namespace xilinx
		{
			/**
			 * @brief Well-known FPGA configuration registers of Xilinx Series-7 and UltraScale FPGAs
			 */
			enum class config_reg : uint32_t
			{
				CRC    = 0b00000, //!< CRC register
				FAR    = 0b00001, //!< Frame address register
				FDRI   = 0b00010, //!< Frame data register input (config data write)
				FDRO   = 0b00011, //!< Frame data register output (config data read)
				CMD    = 0b00100, //!< Command register
				CTL0   = 0b00101, //!< Control register 0
				MASK   = 0b00110, //!< Masking register for CTL0 and CTL1
				STAT   = 0b00111, //!< Status register
				LOUT   = 0b01000, //!< Legacy output register
				COR0   = 0b01001, //!< Configuration option register 0
				MFWR   = 0b01010, //!< Multi frame write register
				CBC    = 0b01011, //!< Initial CBC value register
				IDCODE = 0b01100, //!< Device ID register
				AXSS   = 0b01101, //!< User access register
				COR1   = 0b01110, //!< Configuration option register 1
				RSVD15 = 0b01111,
				WBSTAR = 0b10000, //!< Warm boot start address register.
				TIMER  = 0b10001, //!< Watchdog timer register.
				RSVD18 = 0b10010,
				RSVD19 = 0b10011,
				RSVD20 = 0b10100,
				RSVD21 = 0b10101,
				BOOTSTS= 0b10110, //!< Boot history status register
				RSVD23 = 0b10111,
				CTL1   = 0b11000, //!< Control register 1
				RSVD25 = 0b11001,
				RSVD26 = 0b11010,
				RSVD27 = 0b11011,
				RSVD28 = 0b11100,
				RSVD29 = 0b11101,
				RSVD30 = 0b11110, //!< Reserved 30 (Switch to next SLR)
				BSPI   = 0b11111, //!< BPI/SPI configuration options register
			};

			//------------------------------------------------------------------------------------------
			
			const char* to_string(config_reg reg);

			//------------------------------------------------------------------------------------------
			/**
			 * @brief Writes the name of an FPGA configuration register to an output stream.
			 * 
			 * @param os is the target output stream.
			 * @param reg specifies the configuration register.
			 * @return
			 */
			std::ostream& operator<<(std::ostream& os, config_reg reg);
		}
	}
}

#endif // UNBIT_XILINX_CONFIG_REG_HPP_
