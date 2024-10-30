/**
 * @file
 * @brief Information about a specific Xilinx FPGA device
 */
#ifndef UNBIT_FPGA_XILINX_DEVICE_HPP_
#define UNBIT_FPGA_XILINX_DEVICE_HPP_ 1

#include <cstdint>
#include <cstddef>

#include <string>
#include <memory>

namespace unbit
{
	namespace fpga
	{
		namespace xilinx
		{
			/**
			* @brief Description of a Xilinx FPGA.
			*
			* The @ref device class is the primary interface to query detais about
			* a particular Xilinx FPGA. This class provides basic information about the
			* device name and device geometry (including frame size and total number
			* of frames).
			*
			* Knowledge about the geometry of an FPGA is required to emulate the
			* state of an FPGA's configuration array, for example when loading a
			* bitstream via the @ref config_engine class. Xilinx FPGA use a geometry
			* dependent frame address register (FAR) to address frames within each
			* Super Logic Region (SLR).
			*
			* Frame addresses are incremented while loading larger blocks of
			* configuration data, for example through writes to the FPGA's
			* Frame Data Input Register (FDRI). The exact way how this increment
			* works depends on the exact geometry (number of rows, columns, minor
			* blocks).
			*
			* The @ref device class provides linear frame addresses as an abstraction
			* to encapsulate details of the FPGA device specific frame address increment
			* sequence from its users. See the @ref linear_frame_addr_t typedef for more
			* details on device-independent linear frame addresses.
			*/
			class device
			{
			public:
				// Allow deletion through smart-pointers
				friend class std::default_delete<device>;

				/**
				 * @brief Device-independent linear frame address.
				 *
				 * The @ref linear_frame_addr_t type represent a device-indepdendent
				 * linear frame address on a particular FPGA device.
				 *
				 * Linear frame addresses ensure several properties to simplfiy
				 * device-indepdent handling of FPGA configuration data:
				 *
				 * - Each physical configuration frame (identified by its SLR,
				 *   and its physcial frame address within the SLR) can be uniquely
				 *   mapped to exactly one linear frame address.
				 *
				 * - Linear frame address 0 maps to the first physical configuration
				 *   frame, in the first SLR (configuration order) of the device.
				 *
				 * - The last valid linear frame address on a device is linear
				 *   frame address N-1 (with N being the total number of frames
				 *   as reported by the @ref frames_per_device method).
				 *
				 * - Linear frame address -1, as provided by the @ref invalid_frame
				 *   consts, is reserved to denote a canonical "not a valid frame address"
				 *   or "no frame" value.
				 *
				 * - Incrementing a valid linear frame address by +1 gives the linear
				 *   frame address of the next physical configuration frame that is present
				 *   in the configuration array. Helper methods in the  @ref device class
				 *   provide the necessary facilities to handle padding frames in a device
				 *   agnostic manner.
				 */
				typedef std::size_t linear_frame_addr_t;

				/**
				 * @brief Canonical invalid linear frame address.
				 */
				static constexpr linear_frame_addr_t invalid_frame = static_cast<linear_frame_addr_t>(-1);

			protected:
				/**
				 * @brief The name of this FPGA device.
				 */
				const std::string name_;

				/**
				 * @brief The number of (32-bit) configuration words per frame.
				 */
				const std::size_t words_per_frame_;

				/**
				 * @brief The number of Super Logic Regions (SLRs) on this device.
				 */
				const std::size_t slrs_per_device_;

				/**
				 * @brief The total number of configuration frames per device.
				 */
				const std::size_t frames_per_device_;

			protected:
				/**
				 * @brief Constructs a new FPGA device.
				 *
				 * @param name specifies the name of the device.
				 * @param words_per_frame specifies the number of 32-bit words per configuration frame.
				 * @param frames_per_device specifies the number of configuration frames per device.
				 * @param slrs_per_device specifies the total number (at least 1) of Super Logic Region (SLRs) of this device.
				 */
				device(const std::string& name, std::size_t words_per_frame, std::size_t frames_per_device,
					std::size_t slrs_per_device);

				/**
				 * @brief Destroys an FPGA device object.
				 */
				virtual ~device();

			public:
				/**
				 * @brief Gets the name of this FPGA device object.
				 *
				 * @return The name of the FPGA that is described by this device object.
				 */
				inline const std::string& name() const
				{
					return name_;
				}

				/**
				 * @brief Gets the size of an FPGA configuration frame (in 32-bit words).
				 *
				 * @return The number of 32-bit configuration words in an FPGA configuration frame.
				 */
				inline std::size_t words_per_frame() const
				{
					return words_per_frame_;
				}

				/**
				 * @brief Gets the total number of FPGA configuration frames on this device.
				 *
				 * @return The total number of configuration frames on this device. The returned
				 *   frame count only includes frames that are actually present in the configuration
				 *   array of the device. Dummy frames, like the two extra padding frames at the end
				 *   of each FPGA row in a Xilinx device, are NOT included in this count.
				 */
				inline std::size_t frames_per_device() const
				{
					return frames_per_device_;
				}

				/**
				 * @brief Gets the number of Super Logic Regions (SLRs) on this device.
				 *
				 * @return The number of Super Logic Regions (SLRs) on this device.
				 */
				inline std::size_t slrs_per_device() const
				{
					return slrs_per_device_;
				}

				/**
				 * @brief Translates a (phyiscal) frame address to a linear frame address.
				 *
				 * @param far is the frame address to be translated.
				 *
				 * @return The linear frame address corresponding to @p far on success,
				 *   or @ref invalid_frame in case that no matching configuration frame
				 *   exists.
				 */
				virtual linear_frame_addr_t phys_to_linear(uint32_t far) const;

				/**
				 * @brief Translates a linear frame address to a physical frame address.
				 *
				 * @param addr is the linear frame address to be translated.
				 *
				 * @return The physcial frame address corresponding to @p linear on success.
				 *   In case of a translation error this method does not return, but throws
				 *   a @c std::invalid_argument exception instaed.
				 */
				virtual uint32_t linear_to_phys(uint32_t addr) const;

			private:
				// No copy-construction / assignment
				device(const device&) = delete;
				device& operator=(const device&) = delete;
			};
		}
	}
}

#endif // UNBIT_OLD_XILINX_FPGA_HPP_
