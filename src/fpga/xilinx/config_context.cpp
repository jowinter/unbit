/**
 * @file
 * @brief Well-known FPGA configuration registers of Xilinx Series-7 and UltraScale FPGAs
 */

#include "unbit/fpga/xilinx/config_context.hpp"

#include <cassert>

namespace unbit
{
	namespace fpga
	{
		namespace xilinx
		{
			//------------------------------------------------------------------------------------------
			config_context::config_context(config_engine& engine, uint32_t slr_index)
				: engine_(engine), slr_index_(slr_index), far_(0), idcode_(std::nullopt), 
				  write_mode_(wmode::read_only)
			{
			}

			//------------------------------------------------------------------------------------------
			config_context::~config_context()
			{
			}

			//------------------------------------------------------------------------------------------
			void config_context::set_far(uint32_t new_far)
			{
				// TODO: Sanity checks? Any other reaction?
				far_ = new_far;
			}

			//------------------------------------------------------------------------------------------
			void config_context::set_idcode(uint32_t new_idcode)
			{
				// TODO: Sanity checks? Any other reaction?
				// (Could throw if IDCODE does not match any known SLR)
				idcode_ = new_idcode;
			}

			//------------------------------------------------------------------------------------------
			void config_context::set_write_mode(wmode new_mode)
			{
				// TODO: Sanity checks?
				write_mode_ = new_mode;
			}

			//------------------------------------------------------------------------------------------
			bool config_context::can_write_frame(uint32_t frame_addr) const
			{
				if (write_mode_ == wmode::overwrite)
				{
					// No constraints, all frames are fair game.
					return true;
				}
				else if (write_mode_ == wmode::write_once)
				{
					// Write-once mode, must consult the write bitmap.
					return write_bitmap_.contains(frame_addr);
				}
				else
				{
					// Should be read-only mode, all frames are to be left untouched.
					assert(write_mode_ == wmode::read_only);
					return false;
				}
			}

			//------------------------------------------------------------------------------------------
			void config_context::mark_frame_write(uint32_t frame_addr)
			{
				write_bitmap_.insert(frame_addr);
			}
		}
	}
}
