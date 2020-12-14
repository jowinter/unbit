/**
 * @file
 * @brief Common infrastructure for Xilinx Series-7 FPGAs
 */
#ifndef FPGA_XILINX_COMMON_HPP_
#define FPGA_XILINX_COMMON_HPP_ 1

#include <cstdint>
#include <cstddef>

#include <functional>
#include <optional>
#include <memory>
#include <stdexcept>
#include <string>
#include <iosfwd>
#include <vector>

namespace fpga
{
	namespace xilinx
	{
		/**
		 * @brief Common infrastructure for Xilinx Series-7 FPGAs
		 */
		namespace v7
		{
			// Pull-in standard types
			using std::size_t;

			using std::uint8_t;
			using std::uint16_t;
			using std::uint32_t;

			using std::int8_t;
			using std::int16_t;
			using std::int32_t;

			// Forward declarations
			class bitstream;
			class bram;
			class zynq7;

			/**
			 * @brief Optional reference to an object of type T
			 */
			template<typename T>
			using optional_ref = std::optional<std::reference_wrapper<T>>;

			/**
			 * @brief Optional reference to an object of type T
			 */
			template<typename T>
			using optional_const_ref = std::optional<std::reference_wrapper<const T>>;
		}
	}
}

#endif // #ifndef FPGA_XILINX_COMMON_HPP_
