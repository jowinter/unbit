/**
 * @file
 * @brief Intel-Hex support library
 */
#include "ihex/ihex.hpp"

#include <cctype>
#include <fstream>
#include <stdexcept>

namespace unbit
{
	namespace
	{
		//-----------------------------------------------------------------------------------------
		/**
		 * @brief Trimes leading and trailing spaces from a string.
		 *
		 * @return A tuple of [start,end) iterators marking the start and end of the string.
		 */
		static auto trim(const std::string& line)
		{
			auto start = line.begin();
			auto end   = line.end();

			// Skip leading whitespace
			while (start != end && std::isspace(*start))
			{
				++start;
			}

			// Skip trailing whitespace
			while (start != end && std::isspace(*(end - 1u)))
			{
				--end;
			}

			return std::make_tuple(start, end);
		}

		//-----------------------------------------------------------------------------------------
		/**
		 * @brief Extracts the next character from a line in an Intel-Hex file.
		 */
		static auto next(std::string::const_iterator& pos, std::string::const_iterator end)
		{
			if (pos == end)
			{
				throw std::runtime_error("unexpected end of line in intel hex file");
			}

			return *pos++;
		}

		//-----------------------------------------------------------------------------------------
		/**
		 * @brief Extracts a single hex-digit (4-bit nibble) from a line in an Intel-Hex file.
		 *
		 * @return The decoded nibble value
		 */
		static uint32_t nibble(std::string::const_iterator& pos, std::string::const_iterator end)
		{
			auto xdigit = std::toupper(next(pos, end));

			if (xdigit >= '0' && xdigit <= '9')
			{
				return static_cast<uint32_t>(xdigit - '0');
			}
			else if (xdigit >= 'A' && xdigit <= 'F')
			{
				return static_cast<uint32_t>(xdigit - 'A' + 10);
			}
			else if (xdigit >= 'a' && xdigit <= 'f')
			{
				return static_cast<uint32_t>(xdigit - 'a' + 10);
			}
			else
			{
				throw std::invalid_argument("invalid hex digit in intel hex file");
			}
		}

		//-----------------------------------------------------------------------------------------
		/**
		 * @brief Extracts an 8-bit usigned integer value
		 */
		static uint32_t u8(std::string::const_iterator& pos, std::string::const_iterator end)
		{
			const auto hi = nibble(pos, end);
			const auto lo = nibble(pos, end);
			return (hi << 4u) | lo;
		}

		//-----------------------------------------------------------------------------------------
		/**
		 * @brief Extracts an 16-bit usigned integer value
		 */
		static uint32_t u16(std::string::const_iterator& pos, std::string::const_iterator end)
		{
			const auto hi = u8(pos, end);
			const auto lo = u8(pos, end);
			return (hi << 8u) | lo;
		}

		//-----------------------------------------------------------------------------------------
		/**
		 * @brief Parses a single line from an Intel-Hex file.
		 */
		static bool parse_record(ihex::record& r, const std::string& line)
		{
			std::string::const_iterator pos, end;

			std::tie(pos, end) = trim(line);
			if (pos == end)
			{
				// Empty record (can be skipped)
				return false;
			}

			// Start of record (':')
			if (':' != next(pos, end))
			{
				throw std::invalid_argument("unexpected character at start of record");
			}

			// Payload len, record type, and address
			size_t payload_len = u8(pos, end); // Payload length
			r.address = u16(pos, end);         // Address (16-bit)
			r.type    = u8(pos, end);          // Record type

			r.data.resize(payload_len);

			for (size_t i = 0u; i < payload_len; ++i)
			{
				r.data[i] = u8(pos, end); // Payload data
			}

			// Checksum
			r.checksum = u8(pos, end);    // Checksum

			// Throw on unexpected extra data
			if (pos != end)
			{
				throw std::invalid_argument("unexpected extra data at end of record");
			}

			// Record parsed
			return true;
		}
	}

	//---------------------------------------------------------------------------------------------
	uint32_t ihex::load(const std::string& filename,
						const std::function<void(uint32_t,const std::vector<uint8_t>&)>& callback)
	{
		std::ifstream stm(filename, std::ios_base::in);

		return load(stm, callback);
	}

	//---------------------------------------------------------------------------------------------
	uint32_t ihex::load(std::istream& stm,
						const std::function<void(uint32_t,const std::vector<uint8_t>&)>& load_callback)
	{
		uint32_t entrypoint    = 0u;
		uint32_t segment_base  = 0u;

		parse(stm, [&] (const auto& r)
		{
			if (r.type == 0x00u)
			{
				// Data Record (type 0)
				load_callback(segment_base + r.address, r.data);
				return true;
			}
			else if (r.type == 0x01u)
			{
				// End of file record (type 1)
				return false;
			}
			else if (r.type == 0x02u)
			{
				// Extended Segment Address Record (type 2)
				if (r.data.size() != 2)
				{
					throw std::invalid_argument("unsupported extended segment address "
												"(type 2) record.");
				}

				const uint32_t segment = static_cast<uint32_t>(r.data[1u]) |
					(static_cast<uint32_t>(r.data[0u]) << 8u);

				segment_base = segment * 0x10u;
				return true;
			}
			else if (r.type == 0x03u)
			{
				// Start Segment Address Record (type 3)
				if (r.data.size() != 4)
				{
					throw std::invalid_argument("unsupported start segment address "
												"(type 3) record.");
				}

				const uint32_t segment = static_cast<uint32_t>(r.data[1u]) |
					(static_cast<uint32_t>(r.data[0u]) << 8u);

				const uint32_t offset = static_cast<uint32_t>(r.data[3u]) |
					(static_cast<uint32_t>(r.data[2u]) << 8u);

				entrypoint = segment * 0x10u + offset;
				return true;
			}
			else if (r.type == 0x04u)
			{
				// Extended Linear Address Record (type 4)
				if (r.data.size() != 2)
				{
					throw std::invalid_argument("unsupported extended linear address "
												"(type 4) record.");
				}

				segment_base = (static_cast<uint32_t>(r.data[0u]) << 24u) |
					(static_cast<uint32_t>(r.data[1u]) << 16u);

				return true;
			}
			else if (r.type == 0x05u)
			{
				// Start Linear Address Record (type 5)
				if (r.data.size() != 4)
				{
					throw std::invalid_argument("unsupported start linear address "
												"(type 5) record.");
				}

				entrypoint = static_cast<uint32_t>(r.data[3u]) |
					(static_cast<uint32_t>(r.data[2u]) << 8u)  |
					(static_cast<uint32_t>(r.data[1u]) << 16u) |
					(static_cast<uint32_t>(r.data[0u]) << 24u);

				return true;
			}
			else
			{
				throw std::invalid_argument("unsupported record type in intel hex file");
			}
		});

		return entrypoint;
	}

	//---------------------------------------------------------------------------------------------
	void ihex::parse(const std::string& filename,
					 const std::function<bool(const record&)>& callback)
	{
		std::ifstream stm(filename, std::ios_base::in);

		parse(stm, callback);
	}

	//---------------------------------------------------------------------------------------------
	void ihex::parse(std::istream& stm, const std::function<bool(const record&)>& callback)
	{
		std::string line;
		record r;

		while (std::getline(stm, line))
		{
			if (parse_record(r, line))
			{
				// Non-empty record (invoke parser callback)
				if (!callback(r))
				{
					break;
				}
			}
		}

		if (stm.fail())
		{
			throw std::runtime_error("failed to parse the intel-hex file");
		}
	}
}
