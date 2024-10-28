/**
 * @file
 * @brief Emulation of the FPGA configuration engine for Xilinx Series-7 and UltraScale FPGAs.
 */
#ifndef UNBIT_XILINX_CONFIG_ENGINE_HPP_
#define UNBIT_XILINX_CONFIG_ENGINE_HPP_ 1

#include "unbit/fpga/xilinx/bitstream_engine.hpp"
#include "unbit/fpga/xilinx/config_cmd.hpp"

#include <cstdint>
#include <cstddef>

#include <memory>
#include <span>
#include <tuple>
#include <utility>
#include <optional>

namespace unbit
{
	namespace fpga
	{
		namespace xilinx
		{
			class config_context;

			//------------------------------------------------------------------------------------------
			/**
			 * @brief Emulation of the FPGA configuration engine for Xilinx Series-7 and UltraScale FPGAs.
			 *
			 * The @c config_engine class extends the provides basic functionality of the bitstream parser
			 * engine with emulation of FPGA configuration data writes.
			 */
			class config_engine : public bitstream_engine
			{
			public:
				/**
				 * @brief Pointer to the currently active context.
				 */
				std::unique_ptr<config_context> ctx_;

			protected:
				/**
				 * @brief Construct a new FGPA configuration engine object
				 */
				config_engine();

				/**
				 * @brief Destroy the FPGA configuration engine object
				 */
				~config_engine();

				/**
				 * @brief Gets the currently active configuration context.
				 *
				 * @return A reference to the currently active configuration context.
				 */
				config_context& get_context() const;

				/**
				 * @brief Creates a context object for processing a new SLR.
				 *
				 * @param new_slr_index is the index of the SLR to be processed.
				 *
				 * @return A pointer to the newly created context object.
				 */
				virtual std::unique_ptr<config_context> create_context(uint32_t new_slr_index);

				/**
				 * @brief Process bitstream packets loaded in memory.
				 *
				 * @param cfg_data specifies the bitstream data to be parsed.
				 *
				 * @param is_synchronized indicates if the first word in the bitstream data array contains
				 *   a valid bitstream packet data (true), or if the processing engine should first
				 *   search for a sync word (false).
				 *
				 * @return The total number of words that were processed by this method.
				 */
				parser_status_type process_packets(word_span_type cfg_data, bool is_synchronized) override;

				/**
				 * @brief Processes a configuration write packet.
				 *
				 * @param reg is the configuration register to write.
				 *
				 * @param data points to the start of paramater data of this write.
				 *
				 * @return True if the packet was processed compeltely, or false if the
				 *   bitstream processing should stop at this packet.
				 */
				bool on_config_write(config_reg reg, word_span_type data) override;

				/**
				 * @brief Processes configuration of a (nested) SLR.
				 *
				 * @param data specifies the configuration data words for the next SLR.
				 *
				 * @param next_slr_index is the index (in configuration order) of the
				 *   SLR to be configured. The order in which SLRs are seen in a bitstream
				 *   (configuration order) does not necessarily match the designation of
				 *   SLRs (as SLR0, SLR1, ...).
				 */
				virtual void on_config_slr(word_span_type data, uint32_t next_slr_index);

				/**
				 * @brief Handles a write to the command (CMD) register.
				 *
				 * @param data points to the start of paramater data of this write.
				 */
				virtual void on_config_cmd(config_cmd cmd, word_span_type data);

				/**
				 * @brief Handles a write to the IDCODE register.
				 *
				 * @param idcode is the new IDCODE value.
				 */
				virtual void on_config_idcode(uint32_t idcode);

				/**
				 * @brief Handles a write to the frame address (FAR) register.
				 *
				 * @param new_far is the new value of the frame address register.
				 */
				virtual void on_config_far(uint32_t new_far);

				/**
				 * @brief Handles a write to the frame data input (FDRI) register.
				 *
				 * @param data  is the new value of the frame address register.
				 */
				virtual void on_config_fdri(word_span_type data);

				/**
				 * @brief Handles a write to the multi-frame write (MFWR) register.
				 *
				 * @param data  is the new value of the frame address register.
				 */
				virtual void on_config_mfwr(word_span_type data);

				/**
				 * @brief Handles a NUL command.
				 */
				virtual void on_cmd_nul();

				/**
				 * @brief Handles a WCFG command.
				 */
				virtual void on_cmd_wcfg();

				/**
				 * @brief Handles an MFW command.
				 */
				virtual void on_cmd_mfw();
			};
		}
	}
}

#endif // UNBIT_XILINX_CONFIG_ENGINE_HPP_
