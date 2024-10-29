/**
 * @file
 * @brief Common infrastructure for Xilinx FPGAs
 */
#ifndef UNBIT_XILINX_BITSTREAM_ERROR_HPP_
#define UNBIT_XILINX_BITSTREAM_ERROR_HPP_ 1

#include <stdexcept>
#include <string>
#include <optional>

namespace unbit
{
	namespace fpga
	{
		namespace xilinx
		{
			/**
			 * @brief Bitstream processing error.
			 */
			class bitstream_error : public std::runtime_error
			{
			public:
				/**
				 * @brief Constructs a new bitstream error with the given error message.
				 *
				 * @param[in] what is the error message for the bitstream error object.
				 */
				explicit bitstream_error(const std::string& what);

				/**
				 * @brief Constructs a new bitstream error with the given error message.
				 *
				 * @param[in] what is the error message for the bitstream error object.
				 */
				explicit bitstream_error(const char *what);
			};
		}
	}
}

#endif // UNBIT_XILINX_BITSTREAM_ERROR_HPP_
