/**
 * @file
 * @brief Well-known FPGA configuration registers of Xilinx Series-7 and UltraScale FPGAs
 */

#include "unbit/fpga/xilinx/bitstream_error.hpp"
#include "unbit/fpga/xilinx/config_engine.hpp"
#include "unbit/fpga/xilinx/config_context.hpp"

#include <iostream>

namespace unbit
{
	namespace fpga
	{
		namespace xilinx
		{
			//------------------------------------------------------------------------------------------
			/**
			 * @brief Guards a context switch of a configuration engine.
			 */
			class context_switch_guard
			{
			private:
				/**
				 * @brief Reference to the engine's context pointer being switched temporarily
				 */
				std::unique_ptr<config_context>& context_ref_;

				/**
				 * @brief Pointer to the previous context of the configuartion engine.
				 */
				std::unique_ptr<config_context> old_context_;

			public:
				/**
				 * @brief Construct a new context switch guard and performs a context switch.
				 *
				 * @param context_ref is a reference to the engine's context pointer to be switched.
				 * @param new_context is the new context to be activated.
				 */
				inline context_switch_guard(std::unique_ptr<config_context>& context_ref, std::unique_ptr<config_context> new_context)
					: context_ref_(context_ref), old_context_(std::move(new_context))
				{
					std::swap(context_ref_, old_context_);
				}

				/**
				 * @brief Destroy the context switch guard object and restores the engine's old context pointer.
				 */
				inline ~context_switch_guard()
				{
					std::swap(context_ref_, old_context_);
				}
			};

			//------------------------------------------------------------------------------------------
			config_engine::config_engine()
			{
			}

			//------------------------------------------------------------------------------------------
			config_engine::~config_engine()
			{
			}

			//------------------------------------------------------------------------------------------
			config_context& config_engine::get_context() const
			{
				if (!ctx_)
					throw bitstream_error("no active configuration context found.");

				return *ctx_;
			}

			//------------------------------------------------------------------------------------------
			std::unique_ptr<config_context> config_engine::create_context(uint32_t new_slr_index)
			{
				return std::make_unique<config_context>(*this, new_slr_index);
			}

			//------------------------------------------------------------------------------------------
			config_engine::parser_status_type config_engine::process_packets(word_span_type cfg_data, bool is_synchronized)
			{
				// Start the root index (first SLR in config order)
				context_switch_guard guard(ctx_, create_context(0u));

				// Process the payload data using the base bitstream engine (on the new context)
				return bitstream_engine::process_packets(cfg_data, false);
			}

			//------------------------------------------------------------------------------------------
			bool config_engine::on_config_write(config_reg reg, word_span_type data)
			{
				switch (reg)
				{
					case config_reg::CMD:
						// Command register write
						if (data.size() < 1)
							throw bitstream_error("malformed write to the command (CMD) register (missing command code)");

						on_config_cmd(static_cast<config_cmd>(data[0]), word_span_type(data.begin() + 1, data.end()));
						break;

					case config_reg::IDCODE:
						// Write to the IDCODE register
						if (data.size() < 1)
							throw bitstream_error("malformed write to the IDCODE register (missing command code)");

						on_config_idcode(data[0]);
						break;

					case config_reg::FAR:
						// Write to the frame address register
						if (data.size() < 1)
							throw bitstream_error("malformed write to the frame address (FAR) register (missing command code)");

						on_config_far(data[0]);
						break;

					case config_reg::RSVD30:
						// Configuration of the next slr.
						on_config_slr(data, get_context().slr_index() + 1u);
						break;

					case config_reg::FDRI:
						// Frame data input register
						on_config_fdri(data);
						break;

					case config_reg::MFWR:
						// Multi-frame write register
						on_config_mfwr(data);
						break;

					default:
						// Ignored
						break;
				}

				return true;
			}

			//------------------------------------------------------------------------------------------
			void config_engine::on_config_slr(word_span_type data, uint32_t next_slr_index)
			{
				// Switch to the new context (the current context - if any - is restored when the
				// guard object is destroyed).
				context_switch_guard guard(ctx_, create_context(next_slr_index));

				// Process the payload data using the base bitstream engine (on the new context)
				bitstream_engine::process_packets(data, false);
			}

			//------------------------------------------------------------------------------------------
			void config_engine::on_config_idcode(uint32_t idcode)
			{
				get_context().set_idcode(idcode);
			}

			//------------------------------------------------------------------------------------------
			void config_engine::on_config_far(uint32_t new_far)
			{
				get_context().set_far(new_far);
			}

			//------------------------------------------------------------------------------------------
			void config_engine::on_config_fdri(word_span_type data)
			{
			}

			//------------------------------------------------------------------------------------------
			void config_engine::on_config_mfwr(word_span_type data)
			{
			}

			//------------------------------------------------------------------------------------------
			void config_engine::on_config_cmd(config_cmd cmd, word_span_type data)
			{
				switch (cmd)
				{
					case config_cmd::NUL:
						// NULL command
						on_cmd_nul();
						break;

					case config_cmd::WCFG:
						// Write Configuration command
						on_cmd_wcfg();
						break;

					case config_cmd::MFW:
						// Multi-frame write command
						on_cmd_mfw();
						break;

					default:
						// Ignored
						break;
				}
			}

			//------------------------------------------------------------------------------------------
			void config_engine::on_cmd_nul()
			{
				get_context().set_write_mode(config_context::wmode::read_only);
			}

			//------------------------------------------------------------------------------------------
			void config_engine::on_cmd_wcfg()
			{
				get_context().set_write_mode(config_context::wmode::write_once);
			}

			//------------------------------------------------------------------------------------------
			void config_engine::on_cmd_mfw()
			{
				get_context().set_write_mode(config_context::wmode::overwrite);
			}
		}
	}
}
