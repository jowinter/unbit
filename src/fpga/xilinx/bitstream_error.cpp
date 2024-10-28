/**
 * @file
 * @brief bitstream processing errors
 */
#include "unbit/fpga/xilinx/bitstream_error.hpp"

#include <algorithm>
#include <cassert>

#include <algorithm>

namespace unbit
{
	namespace fpga
	{
		namespace xilinx
		{
			//------------------------------------------------------------------------------------------
			bitstream_error::bitstream_error(const std::string& what)
				: runtime_error(what)
			{
			}

			//------------------------------------------------------------------------------------------
			bitstream_error::bitstream_error(const char *what)
				: runtime_error(what)
			{
			}
		}
	}
}