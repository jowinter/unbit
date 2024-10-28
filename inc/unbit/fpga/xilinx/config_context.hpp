/**
 * @file
 * @brief Emulation of the FPGA configuration engine for Xilinx Series-7 and UltraScale FPGAs.
 */
#ifndef UNBIT_XILINX_CONFIG_CONTEXT_HPP_
#define UNBIT_XILINX_CONFIG_CONTEXT_HPP_ 1

#include <cstdint>
#include <cstddef>
#include <optional>
#include <unordered_set>

namespace unbit
{
	namespace fpga
	{
		namespace xilinx
		{
			class config_engine;

			//------------------------------------------------------------------------------------------
			/**
			* @brief Context (per-SLR) of the FPGA configuration engine.
			*
			* This class describes the per-SLR state context of the FPGA configuration engine.
			* When switching between SLRs the configuration engine automatically saves and
			* restores the context data.
			*
			* Writes to the frame data input (FDRI) and multi-frame write (MFWR) registers are
			* prefixed by WCFG and MFW commands. The documentation in [UG570] status that the
			* NUL, WCFG and MFW commands control the behavior of the device on following writes
			* to the frame address (FAR) register.
			*
			* Observation of compressed and uncompressed bitstreams show that:
			* - Freestanding writes to the FAR register are always prefixed by the NUL command.
			* - Writes to the FDRI register are always prefixed by the WCFG command.
			* - Writes to the MFWR register are always prefixed by the MFW command.
			*
			* For uncompressed bitstreams the following can be observed:
			* - For reach SLR there is exactly one big write to the FDRI spanning the whole
			*   configuration array.
			* - The sequence consists of a NUL command, followed by an FAR write to 0, followed
			*   by a WCFG command, followed by the FDRI input.
			*
			* For compressed bitstreams the following can be observed:
			* - FDRI register writes follow the same scheme as in the uncompressed case, i.e.
			*   first a NUL command, then an FAR writem then a WCFG command, then an FDRI register
			*   write.
			*
			* - MFWR register writes take their actual frame data input from the last frame that
			*   was written by a preceding FDRI register write (dummy frames at the end of a
			*   configuration row are ignored, only actual writes are considered as data source).
			*
			* - MFWR register writes always follow a regular FDRI write sequence of at least
			*   one frame. Typical sequences observed in bitstream data consist of a NUL command,
			*   followed by an FAR write, followed by a WCFG command, followed by an FDRI register
			*   write, followed by an MFW command, followed by pairs of FAR writes and MFWR writes.
			*
			* - The first FAR/MFWR pair following a MFW command does *not* necessarily address the
			*   same frame as seen in the preceding FAR/FDRI part.
			*
			* - It can be the case that FAR/FDRI pairs coming later in the bitstream overlap
			*   frames that have been touched previously by FAR/MFWR pairs. In this case the
			*   latter overwrite attempt by the FAR/FDRI pair is ignored for the affected frames,
			*   and the data written initially by the FAR/MFWR pair is retained. This one took
			*   a while to figure out ;)
			*
			* To model the configuration process (given the observations discussed above) the
			* following information is tracked by @c config_context objects (for the SLR to which
			* the object applies):
			*
			* 1. The frame address register of the current SLR being configured.
			*
			* 2. A bitmap tracking the status (configured or unconfigured) of each frame in
			*    the SLR. The status is updated when a frame is written as side-effect of an
			*    FDRI register write, or an MFWR register write.
			*
			* 3. The current "write mode" dependent on the last NUL, WCFG, or MFWR command seen
			*    by the context:
			*    - Following a NUL command the context object rejects any configuration
			*      data writes (a bitstream_error is thrown if an unexpected write is seen).
			*
			*    - Following a WCFG command the context object accepts configuration data write
			*      via the FDRI register. The context object checks its write tracking bitmap
			*      before forwarding frame writes to the backend configuration array. Writes
			*      are only forwarded if the target frames are unconfigured (followed by an
			*      update of the frame status). Writes to already configured frames are
			*      silently ignored. Configuration attempts via MFWR register writes are rejected
			*      (a bitstream_error is thrown if an unexpected write is seen)
			*
			*    - Following a MFW command that context objects accpets configuration data writes
			*      via the FDRI and MFWR register. Writes are alway forwarded to the backend
			*      configuration array, and the tracking bitmap is updated to record any (over-)written
			*      frames as configured.
			*/
			class config_context
			{
			public:
				/**
				 * @brief Write modes (depending on the last NUL, WCFG, or MFWR command)
				 */
				enum class wmode : uint32_t
				{
					/**
					 * @brief Read-only access to the configuration array (NUL).
					 *
					 * Write attempts via the FDRI and/or MFWR registers are rejected by
					 * throwing a bitstream_error.
					 */
					read_only = 0u,

					/**
					 * @brief Write-once access to the configuration array (WCFG).
					 *
					 * Write attempts via the FDRI register writes are accepted. Frame writes
					 * are only forwarded to the backing configuration array if the target
					 * frames are unconfigured. The state of any written target frames is
					 * updated to configured. Write attempts via the MFWR register are
					 * rejected by throwing a bitstream_error.
					 */
					write_once = 1u,

					/**
					 * @brief Overwrite access to the configuration array (MFWR).
					 *
					 * Write attempts via the FDRI and/or MFWR registers are accepted.
					 * Frames writes are always forwarded to the backing configuration array
					 * and the state of any written target frames is updated to configured.
					 */
					overwrite = 2u
				};

			private:
				/**
				 * @brief The parent engine of this configuration context.
				 */
				config_engine& engine_;

				/**
				 * @brief Active SLR (in configuratin order) being configured with this context.
				 */
				uint32_t slr_index_;

				/**
				 * @brief Frame address register
				 */
				uint32_t far_;

				/**
				 * @brief The last IDCODE value seen on this context.
				 */
				std::optional<uint32_t> idcode_;

				/**
				 * @brief Active write mode of this context.
				 */
				wmode write_mode_;

				/**
				 * @brief Write bit-map
				 *
				 * @todo Prototype implementation uses a @c std::unordered_set instead of a @c std::vector<bool>
				 *   as mapping from the FAR to linear frame addresses, and FPGA layout handling is not yet
				 *   implemented. Using @c std::vector<bool> would give reduction in runtime memory. Check if this
				 *   is needed for performance.
				 */
				std::unordered_set<uint32_t> write_bitmap_;

			public:
				/**
				 * @brief Construct a new context object.
				 *
				 * @param slr_index is the SLR index (configuration order) of the new object.
				 */
				config_context(config_engine& engine, uint32_t slr_index);

				/**
				 * @brief Destroys the config context object.
				 */
				~config_context();

				/**
				 * @brief ets the parent configuration engine.
				 *
				 * @return The parent configuration engine.
				 */
				inline config_engine& get_engine() const
				{
					return engine_;
				}

				/**
				 * @brief Gets the SLR index (configuration order) of this context.
				 *
				 * @return The SLR index (configuration order) of this context.
				 */
				inline uint32_t slr_index() const
				{
					return slr_index_;
				}

				/**
				 * @brief Gets current value of the frame address register.
				 *
				 * @return The value of the frame address register.
				 */
				inline uint32_t far() const
				{
					return far_;
				}

				/**
				 * @brief Set the frame address register to a new value.
				 *
				 * @param new_far is the new frame address register value.
				 */
				void set_far(uint32_t new_far);

				/**
				 * @brief Gets the last IDCODE (if any) seen by this context.
				 *
				 * @return The last IDCODE value that was seen by this context.
				 */
				inline const std::optional<uint32_t>& idcode() const
				{
					return idcode_;
				}

				/**
				 * @brief Set the IDCODE of this context.
				 *
				 * @param new_idcode is the new IDCODE value.
				 */
				void set_idcode(uint32_t new_idcode);

				/**
				 * @brief Gets the current write mode of this context.
				 *
				 * @return The current write mode of this context.
				 */
				inline wmode write_mode() const
				{
					return write_mode_;
				}

				/**
				 * @brief Sets the frame write mode of this context.
				 *
				 * @param wmode is the new write mode.
				 */
				void set_write_mode(wmode new_mode);

				/**
				 * @brief Tests if a frame is writeable given the write bitmap and current write mode.
				 *
				 * @param frame_addr is the frame address of the frame being written.
				 *
				 * @return True if the frame can be written in the current write mode, false otherwise.
				 */
				bool can_write_frame(uint32_t frame_addr) const;

				/**
				 * @brief Updates the write bitmap to mark a frame as written.
				 *
				 * This method unconditionally marks a frame as written (configured) in the
				 * write bitmap. The current write mode is not checked. It is the caller's
				 * responsibility to perform a previous check of frame writeability and
				 * write mode (if needed), before writing to the frame and updating its status.
				 *
				 * @param frame_addr is the frame address of the frame to be marked as written.
				 */
				void mark_frame_write(uint32_t frame_addr);

			private:
				config_context(const config_context&) =delete;
				config_context& operator=(const config_context&) =delete;

			};
		}
	}
}

#endif // UNBIT_XILINX_CONFIG_CONTEXT_HPP_
