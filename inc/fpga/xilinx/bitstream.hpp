/**
 * @file
 * @brief Common infrastructure for Xilinx FPGAs
 */
#ifndef UNBIT_XILINX_BITSTREAM_HPP_
#define UNBIT_XILINX_BITSTREAM_HPP_ 1

#include "common.hpp"

namespace unbit
{
	namespace xilinx
	{
		//------------------------------------------------------------------------------------------
		/**
		 * @brief Bitstream manipulation for Series-7 FPGAs
		 */
		class bitstream
		{
		public:
			/** Storage vector type. */
			typedef std::vector<uint8_t> data_vector;

			/** Byte data iterator (non-const) */
			typedef data_vector::iterator byte_iterator;

			/** Byte data iterator (const) */
			typedef data_vector::const_iterator const_byte_iterator;

			/** Bitstream command packet info */
			struct packet
			{
				/**
				 * @brief Zero-based index of the (sub-)bitstream to which this packet belongs.
				 *
				 * @note Virtex-7 style FPGA bitstreams seem to allow some leeway with respect to
				 *   (sub-)bitstreams. Observations on bitstreams for larger FPGA show that there is
				 *   no 1:1 correspondence between this index and the SLR it (may) configure.
				 *
				 *   Tools wishing to parse the config frames for SLRs can do so by tracking a
				 *   change in the stream index and the corresponding FDRI write packets.
				 *   (OBSERVATION: On devices with 3 SLRs there seem to be at least 3 substreams,
				 *   with FDRI write packets)
				 */
				std::size_t stream_index;

				/**
				 * @brief Position of this packet within its enclosing file/buffer storage.
				 *
				 * @note This offset tracks the packet's start position with respect to the physical
				 *   storage container (i.e. it counts the number of bytes since including the size
				 *   of all previous SLRs).
				 */
				std::size_t storage_offset;

				/**
				 * @brief Position of this packet in its enclosing (sub-)bitstream.
				 */
				std::size_t offset;

				/** @brief The raw command header word */
				uint32_t hdr;

				/**
				 * @brief Type of decoded packet
				 */
				uint32_t packet_type;

				/**
				 * @brief Opcode extracted from the packet
				 *
				 * @note The bitstream parser tracks the relationship between type1/type2 packets.
				 *   For type2 packets this field is filled from the previous type1 packet (or
				 *   0x00000000 if no previous type1 packet exists).
				 */
				uint32_t op;

				/**
				 * @brief Register operand extracted from the packet
				 *
				 * @note The bitstream parser tracks the relationship between type1/type2 packets.
				 *   For type2 packets this field is filled from the previous type1 packet (or
				 *   0xffffffff if no previous type1 packet exists).
				 */
				uint32_t reg;

				/** @brief Number of payload words of the packet */
				uint32_t word_count;

				/** @brief Start iterator for payload data */
				const_byte_iterator payload_start;

				/** @brief End iterator for payload data */
				const_byte_iterator payload_end;
			};

			/**
			 * @brief Geometry description of an SLR.
			 */
			struct slr_info final
			{
				/**
				 * @brief Constructs a new (invalid) SLR info
				 */
				inline slr_info()
					: sync_offset(0xFFFFFFFFu),
					  frame_data_offset(0), frame_data_size(0),
					  idcode(0xFFFFFFFF)
				{
				}

				/** @brief Byte offset of the first byte following the sync word. */
				size_t sync_offset;

				/** @brief Byte offset of the first byte of the config frames are. */
				size_t frame_data_offset;

				/** @brief Size of the config frame data in bytes. */
				size_t frame_data_size;

				/**
				 * @brief IDCODE extracted from the bitstream.
				 *
				 * @note 0xFFFFFFFF indicates that no IDCODE was found in this SLR)
				 */
				uint32_t idcode;
			};

			/**
			 * @brief SLR info vector type
			 */
			typedef std::vector<slr_info> slr_info_vector;

		private:
			/** @brief SLR slices of this bitstream */
			slr_info_vector slrs_;

			/** @brief In-memory data of the bitstream */
			data_vector data_;

			/**
			 * @brief Indicates if this object holds readback data (vs. a full bitstream)
			 */
			bool is_readback_;

		public:
			/**
			 * @brief Loads an uncompressed (and unencrypted) bitstream from a given file.
			 *
			 * @param[in] filename specifies the name (and path) of the bitstream file to be loaded.
			 *
			 * @param[in] idcode specifies the expected IDCODE value (or 0xFFFFFFFF to indicate that
			 *   the IDCODE value is to be read from the bitstream data).
			 *
			 * @param[in] accept_readback indicates if "readback" bitstreams shall be accepted
			 *   by this method.
			 *
			 *   Regular configuration bitstreams use FDRI command packets to feed the FPGA with
			 *   frame data. "Readback" bitstreams (*.rbb) are generated as (temporary) output files
			 *   by Xilinx bitstream generation tooling, when the user request generation generation
			 *   of readback files (*.rbt, *.msd). FPGA configuration frames in a readback bitstream
			 *   use FDRO command packets to encapsulate the (expected) readback data.
			 *
			 *   The @p accept_readback paramater controls whether the @ref #load_bitstream method
			 *   accepts such "readback" bitstreams or not (default: reject).
			 *
			 * @return The loaded bitstream object.
			 */
			static bitstream load_bitstream(const std::string& filename,
											uint32_t idcode = 0xFFFFFFFFu,
											bool accept_readback = false);

			/**
			 * @brief Loads an uncompressed (and unencrypted) bitstream from a readback data file.
			 *
			 * @param[in] filename specifies the name (and path) of the readback data file to be
			 *  loaded.
			 *
			 * @param[in] reference specifies a loaded reference bitstream (providing IDCODE and
			 *  geometry information)
			 *
			 * @return The loaded bitstream object.
			 */
			static bitstream load_raw(const std::string& filename, const bitstream& reference);

			/**
			 * @brief Stores an uncompressed (and unencrypted) bitstream to a given file.
			 *
			 * @param[in] filename specifies the destiation filename (and path).
			 *
			 * @param[in] bs specifies the bitstream to be dumped.
			 */
			static void save(const std::string& filename, const bitstream& bs);

			/**
			 * @brief Stores an uncompressed as raw readback data file.
			 *
			 * @param[in] filename specifies the destiation filename (and path).
			 */
			static void save_as_readback(const std::string& filename, const bitstream& bs);

			/**
			 * @brief Parses the packets in a bitstream (all substreams are parsed).
			 *
			 * @note Documentation on the bitstream format of Xilix 7-Series bitstreams can be found
			 *   in [Xilinx UG470; "Bitstream Composition"].
			 *
			 * @param[in] filename specifies the filename of the bitstream to be parsed.
			 */
			static void parse(const std::string& filename, std::function<bool(const packet&)> callback);

			/**
			 * @brief Parses the packets in a bitstream (all substreams are parsed).
			 *
			 * @note Documentation on the bitstream format of Xilix 7-Series bitstreams can
			 *   be found in  [Xilinx UG470; "Bitstream Composition"].
			 *
			 * @param[in] stm specifies the input stream containing the bitstream data to parse.
			 */
			static void parse(std::istream& stm, std::function<bool(const packet&)> callback);

			/**
			 * @brief Parses the packets in a single substream of a bitstream (all substreams are
			 *  parsed).
			 *
			 * @note Documentation on the bitstream format of Xilix 7-Series bitstreams can be found
			 *   in [Xilinx UG470; "Bitstream Composition"].
			 *
			 * @param[in] start is an iterator indicates the start of bit-stream data.
			 *
			 * @param[in] end is an iterator indicates the end of bit-stream data.
			 *
			 * @param[in] callback specifies the packet handler callback. The callback function is
			 *   invoked for every packet in the stream. The return value of the callback indicates
			 *   whether parsing should continue after the call.
			 */
			static void parse(const_byte_iterator start, const_byte_iterator end,
							  std::function<bool(const packet&)> callback);

			/**
			 * @brief Parses the packets in a single substream of a bitstream.
			 *
			 * @note Documentation on the bitstream format of Xilix 7-Series bitstreams can be found
			 *   in [Xilinx UG470; "Bitstream Composition"].
			 *
			 * @param[in] start is an iterator indicates the start of bit-stream data.
			 *
			 * @param[in] end is an iterator indicates the end of bit-stream data.
			 *
			 * @param[in] base_file_offset indicates the absolute byte offset of the byte referenced
			 *   by @p start with respect to its enclosing file/array. This parameter is used to
			 *   calculate the absolute byte offset of data packets with respect to the enclosing
			 *   file/array.
			 *
			 * @param[in] stream_index indicates the index of the (sub-)bitstream for this parse
			 *   operation.  This parameter is forwarded verbatimly to the callback.
			 *
			 * @param[in] callback specifies the packet handler callback. The callback function is
			 *   invoked for every packet in the stream. The return value of the callback indicates
			 *   whether parsing should continue after the call.
			 *
			 * @return An iterator indicating the end of bit-stream data that has been parsed by
			 *   this call. The returned iterator is equal to @p end if the complete input data has
			 *   been exhausted.
			 */
			static const_byte_iterator parse(const_byte_iterator start, const_byte_iterator end,
											 size_t base_file_offset, size_t slr,
											 std::function<bool(const packet&)> callback);

		public:
			/**
			 * @brief Constructs a bitstream from a given input stream.
			 *
			 * @param[in] stm is the input stream to read the bitstream data from. The input stream
			 *   should be opened in binary mode.
			 *
			 * @param[in] idcode specifies the expected IDCODE value (or 0xFFFFFFFF to indicate that
			 *   the IDCODE value is to be read from the bitstream data).
			 *
			 * @param[in] accept_readback indicates if readback bitstream shall be accepted as
			 *   input format (in addition to normal config bitstreams).
			 */
			bitstream(std::istream& stm, uint32_t idcode = 0xFFFFFFFFu, bool readback = false);

			/**
			 * @brief Constructs a bitstream from a given input stream (containing raw readback
			 *  data).
			 *
			 * @param[in] stm is the input stream to read the readback data from. The input stream
			 *   should be opened in binary mode.
			 *
			 * @param[in] reference specifies a compatible reference bitstream (that provides
			 *   geometry and readback information). The reference bitstream can be a full
			 *   bitstream with configuration data, or a compatible readback data stream.
			 */
			bitstream(std::istream& stm, const bitstream& reference);

			/**
			 * @brief Move constructor for bitstream objects.
			 */
			bitstream(bitstream&& other) noexcept;

			/**
			 * @brief Disposes a bitstream object and its resources.
			 */
			~bitstream() noexcept;

			/**
			 * @brief Tests if this object holds readback data (vs. a full bitstream)
			 */
			inline bool is_readback() const
			{
				return is_readback_;
			}

			/**
			 * @brief Gets an SLR information object.
			 *
			 * @param[in] index specifies the zero-based index of the SLR info object.
			 */
			inline const slr_info& slr(unsigned slr_index) const
			{
				return slrs_.at(slr_index);
			}

			/**
			 * @brief Gets a read-only reference to the vector of SLR information objects of this
			 * bitstream.
			 */
			inline const slr_info_vector& slrs() const
			{
				return slrs_;
			}

			/**
			 * @brief Gets the byte offset from the start of the bitstream data to the first byte of
			 *   the FPGA configuration frames.
			 */
			inline size_t frame_data_offset(unsigned slr_index) const
			{
				return slr(slr_index).frame_data_offset;
			}

			/**
			 * @brief Gets the size of the FPGA configuration frame data in bytes.
			 */
			inline size_t frame_data_size(unsigned slr_index) const
			{
				return slr(slr_index).frame_data_size;
			}

			/**
			 * @brief Gets a byte iterator to the begin of the config packets area. (const)
			 */
			const_byte_iterator config_packets_begin(unsigned slr_index) const;

			/**
			 * @brief Gets a byte iterator to the end of the config packets area. (const)
			 */
			const_byte_iterator config_packets_end(unsigned slr_index) const;

			/**
			 * @brief Gets a byte iterator to the begin of the frame data area. (non-const)
			 */
			inline byte_iterator frame_data_begin(unsigned slr_index)
			{
				return data_.begin() + frame_data_offset(slr_index);
			}

			/**
			 * @brief Gets a byte iterator to the end of the frame data area. (non-const)
			 */
			inline byte_iterator frame_data_end(unsigned slr_index)
			{
				return frame_data_begin(slr_index) + frame_data_size(slr_index);
			}

			/**
			 * @brief Gets a byte iterator to the begin of the frame data area. (const)
			 */
			inline const_byte_iterator frame_data_begin(unsigned slr_index) const
			{
				return data_.cbegin() + frame_data_offset(slr_index);
			}

			/**
			 * @brief Gets a byte iterator to the end of the frame data area. (const)
			 */
			inline const_byte_iterator frame_data_end(unsigned slr_index) const
			{
				return frame_data_begin(slr_index) + frame_data_size(slr_index);
			}

			/**
			 * @brief In-place rewrite of the bitstream.
			 *
			 * @param[in] callback specifies a function for in-place editing of the bitstream.  The
			 *   callback function receives the packet of interest (read-only) and a writeable byte
			 *   iterator pair spanning the packet boundary.
			 *
			 * @note This method internally uses the @ref parser method to scan over the packets in
			 *   the bitstream and in its sub-streams.
			 */
			void edit(std::function<void(const packet&,byte_iterator,byte_iterator)> callback);

			/**
			 * @brief Strips all CRC check commands from the bitstream.
			 */
			void strip_crc_checks();

			/**
			 * @brief Reads a bit from the frame data area.
			 *
			 * @param[in] bit_offset specifies the offset (in bits) relative to the start of the
			 *   frame data area.  (the method internally handles 32-bit word swaps as needed)
			 *
			 * @return The read-back value of the bit.
			 */
			bool read_frame_data_bit(size_t bit_offset, unsigned slr_index) const;

			/**
			 * @brief Writes a bit in the frame data area.
			 *
			 * @param[bit] bit_offset specifies the offset (in bits) relative to the start of the
			 *   frame data area.  (the method internally handles 32-bit word swaps as needed)
			 *
			 * @param[in] value the value to write at the given location.
			 */
			void write_frame_data_bit(size_t bit_offset, bool value, unsigned slr_index);

			/**
			 * @brief Gets the device IDCODE that was parsed from the bitstream's configuration
			 *  packets.
			 */
			inline uint32_t idcode() const
			{
				return slr(0).idcode;
			}

			/**
			 * @brief Saves this bitstream to disk.
			 *
			 * @param[in,out] stm specifies the output stream to save this bitstream to.
			 */
			void save(std::ostream& stm) const;


			/**
			 * @brief Saves this bitstream as raw readback data file.
			 *
			 * @param[in] filename specifies the name (and path) of the readback data file to be
			 *  created
			 */
			void save_as_readback(std::ostream& stm) const;

		private:
			/**
			 * @brief Helper to load a binary data array from an input stream.
			 *
			 * @return An byte data vector with the loaded binary data.
			 */
			static data_vector load_binary_data(std::istream& stm);

			/**
			 * @brief Performs a range check for a slice of the frame data range.
			 */
			void check_frame_data_range(size_t offset, size_t length, unsigned slr_index) const;

			/**
			 * @brief Remaps a byte offset into the frame data area (adjusting for 32-bit word swaps
			 *   as needed).
			 *
			 * @param[in] offset is the (byte) offset into the frame data area to be adjusted.
			 * @return The mapped offset (adjusted for 32-bit word swap if needed).
			 */
			size_t map_frame_data_offset(size_t offset) const;

		private:
			// Non-copyable
			bitstream(const bitstream& other) = delete;
			bitstream& operator=(const bitstream& other) = delete;
		};
	}
}

#endif // UNBIT_XILINX_BITSTREAM_HPP_
