/**
 * @file
 * @brief Intel-Hex image support
 */
#ifndef UNBIT_IHEX_HPP_
#define UNBIT_IHEX_HPP_ 1

#include <functional>
#include <istream>
#include <string>
#include <vector>

namespace unbit
{
	/**
	 * @brief An Intel-Hex file reader (utility class)
	 */
	struct ihex
	{
	private:
		// Utility class (no constructor/destructor)
		ihex() =delete;
		~ihex() =delete;

	public:
		/**
		 * @brief Intel-hex record structure
		 */
		struct record
		{
		public:
			/** @brief Payload data */
			std::vector<uint8_t> data;

			/** @brief Address field */
			uint16_t address;

			/** @brief Record type */
			uint8_t type;

			/** @brief Checksum field */
			uint8_t checksum;

		public:
			/**
			 * @brief Constructs a default record
			 */
			inline record()
				: address(0u), type(0u), checksum(0u)
			{
			}
		};

	public:
		/**
		 * @brief Simulates loading of records from an Intel-Hex file
		 *
		 * @param[in] load_callback specifies the callback to be invoked for loading data records.
		 *
		 * @return The entrypoint indicated in the hex file (if any).
		 */
		static uint32_t load(const std::string& filename,
							 const std::function<void(uint32_t,const std::vector<uint8_t>&)>& callback);

		/**
		 * @brief Simulates loading of records from an Intel-Hex file
		 *
		 * @param[in] load_callback specifies the callback to be invoked for loading data records.
		 *
		 * @return The entrypoint indicated in the hex file (if any).
		 */
		static uint32_t load(std::istream& stm, const std::function<void(uint32_t,const std::vector<uint8_t>&)>& load_callback);

		/**
		 * @brief Parses all records from an Intel-Hex file
		 *
		 * @param[in] filename specifies the path to to Intel-Hex file to be read.
		 */
		static void parse(const std::string& filename, const std::function<bool(const record&)>& callback);

		/**
		 * @brief Parses all records from an Intel-Hex file (given as stream)
		 *
		 * @param[in] stm is the input stream with the Intel-Hex file to be read.
		 */
		static void parse(std::istream& stm, const std::function<bool(const record&)>& callback);
	};
}

#endif // UNBIT_IHEX_HPP_
