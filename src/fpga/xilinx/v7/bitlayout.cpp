/*
 *  TBD: currently unused (direct parsing from logic location file)
 *
 * We could use this to support arbitrary 7-series FPGAs (for which we have a logic location file mapping the RAMs
 * of interest).
 */
#ifdef BITLAYOUT_PARSER

#include <cstdint>
#include <cassert>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <functional>
#include <limits>

namespace fpga
{
    namespace xilinx
    {
	struct BramType
	{
	public:
	    static const BramType RAMB36;

	public:
	    /**
	     * @brief Name of this BRAM type
	     */
	    const std::string Name;

	    /**
	     * @brief Total number of data bits.
	     */
	    const std::size_t DataBits;

	    /**
	     * @brief Total number of parity bits.
	     */
	    const std::size_t ParityBits;

	private:
	    BramType(const char* const name, const std::size_t data_bits, const std::size_t parity_bits);
	    ~BramType();

	    BramType(const BramType& other) = delete;
	    BramType& operator=(const BramType& other) = delete;
	};

	BramType::BramType(const char* const name, const std::size_t data_bits, const std::size_t parity_bits)
	    : Name(name), DataBits(data_bits), ParityBits(parity_bits)
	{
	}

	BramType::~BramType()
	{
	}

	// RAMB36 primitive
	const BramType BramType::RAMB36("RAMB36", 32768u, 4096u);
    }
}

namespace fpga
{
    namespace xilinx
    {
	struct LogicLocationFile
	{
	public:
	    /**
	    * @brief Parses a logic location file and extracts BRAM placement data
	    *
	    * Logic location files (.ll) are created by the Xilinx bitgen tool when the "-logic_location_file" option
	    * is specified. A logic location file contains a textual representation of the memory elements and LUT
	    * flip-flops of a design. This information can be used to interpret FPGA readback data, or, in our case,
	    * to manipulate the BRAM elements in a bitstream.
	    */

	    static void ParseV3(std::ifstream& f)
	    {
		LogicLocationFile().DoParseV3(f);
	    }

	private:
	    /**
	     * Classifies a character (as per our definition)
	     */
	    static bool IsWhitespace(std::string::traits_type::char_type c)
	    {
		return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n');
	    }

	    /**
	     * @brief Skip whitespace in a sequence.
	     */
	    static std::string::const_iterator SkipWhitepace(const std::string::const_iterator start, const std::string::const_iterator end)
	    {
		std::string::const_iterator pos = start;
		while ((pos != end) && IsWhitespace(*pos))
		{
		    ++pos;
		}

		return pos;
	    }


	    /**
	     * @brief Skip the remainder of a token.
	     */
	    static void SkipToken(std::string::const_iterator& pos, const std::string::const_iterator end)
	    {
		// Skip leading whitespace
		pos = SkipWhitepace(pos, end);

		// Mark the token start, then scan over it
		std::string::const_iterator tok_start = pos;
		while ((pos != end) && !IsWhitespace(*pos))
		{
		    ++pos;
		}
	    }

	    /**
	     * @brief Scans the next token in a sequence. to the end of a non-whitespace token in a sequence.
	     */
	    static std::string::const_iterator ScanToken(std::string::const_iterator& pos, const std::string::const_iterator end)
	    {
		// Skip leading whitespace
		pos = SkipWhitepace(pos, end);

		// Mark the token start, then scan over it
		std::string::const_iterator tok_start = pos;
		while ((pos != end) && !IsWhitespace(*pos))
		{
		    ++pos;
		}

		// We have either a non-empty token or the end of the line
		assert((tok_start == end) || (tok_start != pos));

		return tok_start;
	    }



	    /**
	     * @brief Helper to match token.
	     */
	    template<std::size_t N>
	    static bool IsTokenEqual(const std::string::const_iterator tok_start, const std::string::const_iterator tok_end, const char(&expected)[N])
	    {
		return std::equal(tok_start, tok_end, expected, expected + N - 1u);
	    }

	    /**
	    * @brief Helper to match the prefix of a token.
	    */
	    template<std::size_t N>
	    static bool MatchTokenPrefix(std::string::const_iterator& tok_start, const std::string::const_iterator tok_end, const char(&expected)[N])
	    {
		std::string::const_iterator pos = tok_start;

		// Skip leading whitespace
		pos = SkipWhitepace(pos, tok_end);

		for (std::size_t i = 0u; i < (N - 1u); ++i, ++pos)
		{
		    if ((pos == tok_end) || (*pos != expected[i]))
		    {
			return false;
		    }
		}

		// Prefix matched, adjust token start
		tok_start = pos;
		return true;
	    }

	    /**
	  * @brief Skip the current token until a given char is found.
	  */
	    static void SkipTokenToChar(std::string::const_iterator& pos, const std::string::const_iterator end, char c)
	    {
		std::string::const_iterator tok_start = pos;
		while ((pos != end) && (*pos != c))
		{
		    ++pos;
		}
	    }

	    /**
	     * @brief Scans a token on prefix match
	     */
	    template<std::size_t N>
	    static bool ScanTokenOnPrefixMatch(std::string::const_iterator& tok_start, std::string::const_iterator& line_pos, const std::string::const_iterator line_end, const char(&expected)[N], const std::size_t rewind = 0u)
	    {
		assert(rewind <= N);

		if (MatchTokenPrefix(line_pos, line_end, expected))
		{
		    // Prefix match
		    tok_start = line_pos - rewind;

		    // Scan the remaining token (can be empty)
		    SkipToken(line_pos, line_end);
		    return true;
		}
		else
		{
		    // No match
		    tok_start = line_end;
		    return false;
		}
	    }

	    /**
	     * @brief Parses an integer token
	     */
	    template<typename T>
	    static bool ParseIntToken(T& value, std::string::const_iterator& pos, const std::string::const_iterator end, const unsigned radix)
	    {
		assert((0u < radix) && (radix <= 16u));

		// Start with zero value and zero accumulator
		value = static_cast<T>(0);

		// Ignore leading whitespace
		pos = SkipWhitepace(pos, end);
		if (pos == end)
		{
		    // Nothing to parse
		    return false;
		}

		T acc = static_cast<T>(0);

		while ((pos != end) && !IsWhitespace(*pos))
		{
		    // Scale by one digit
		    if (acc >= (std::numeric_limits<T>::max() / static_cast<T>(radix)))
		    {
			// Integer overflow (too many digits)
			return false;
		    }

		    acc *= static_cast<T>(radix);

		    // Now process the current digit
		    std::string::traits_type::char_type c = *pos;
		    T digit;

		    if (('0' <= c) && (c <= '9'))
		    {
			// Decimal digit
			digit = static_cast<T>(c - '0');
		    }
		    else if (('a' <= c) && (c <= 'z'))
		    {
			// Lower-case letter
			digit = static_cast<T>(c - 'a' + 10);
		    }
		    else if (('a' <= c) && (c <= 'z'))
		    {
			// Upper-case letter
			digit = static_cast<T>(c - 'A' + 10);
		    }
		    else
		    {
			return false;
		    }

		    // Radix-check
		    if (radix < static_cast<T>(digit))
		    {
			// Bad digit
			return false;
		    }

		    // Add current digit, then advance

		    if ((std::numeric_limits<T>::max() - digit) < digit)
		    {
			// Integer overflow
			return false;
		    }

		    acc += digit;
		    ++pos;
		}

		// Parsing succeeded
		value = acc;
		return true;
	    }

	    /**
	     * @brief Parses an integer token
	     */
	    template<typename T>
	    static bool ParseIntToken(T& value, std::string::const_iterator& pos, const std::string::const_iterator end)
	    {
		unsigned radix;

		// Skip leading whitespace
		pos = SkipWhitepace(pos, end);

		// Match the hex prefix (0x or 0X)
		if (MatchTokenPrefix(pos, end, "0x") || MatchTokenPrefix(pos, end, "0X"))
		{
		    // Hex prefix (0x or 0X)
		    radix = 16u;
		}
		else if (MatchTokenPrefix(pos, end, "0"))
		{
		    // Octal prefix or decimal zero (we adjust pos to accomodate for both cases)
		    radix = 8u;
		    pos -= 1u;
		}
		else
		{
		    radix = 10u;
		}

		return ParseIntToken(value, pos, end, radix);
	    }

	    /**
	     * @brief Process a BRAM bit location
	     */
	    void ProcessBramLocation(const uint32_t bit_offset, const uint32_t frame_addr, const uint32_t frame_offset,
		const BramType& ram_type, const uint32_t loc_x, const uint32_t loc_y,
		const uint32_t ram_bit, bool is_parity)
	    {
		//std::cout << "(" << bit_offset << /*"," << frame_addr << "," << frame_offset <<*/ "," << loc_x << "," << loc_y << "," << ram_bit << "," << is_parity << ")" << std::endl;
	    }

	    /**
	     * @brief Parses location information about a bit
	     */
	    void ParseBramLocation(std::string::const_iterator& line_pos, const std::string::const_iterator line_end,
		const uint32_t bit_offset, const uint32_t frame_addr, const uint32_t frame_offset, const BramType& ram_type)
	    {
		// Parser is at the LOC part of the RAM name. We are parsing a string of the form X<x>Y<y> followed by a space.
		//

		// Match and parse the X coordinate
		uint32_t loc_x;
		{
		    if (!MatchTokenPrefix(line_pos, line_end, "X"))
		    {
			throw std::runtime_error("unhandled LOC (missing X coordinate)");
		    }

		    std::string::const_iterator x_start = line_pos;

		    // End of X = start of Y
		    SkipTokenToChar(line_pos, line_end, 'Y');

		    if (!ParseIntToken(loc_x, x_start, line_pos))
		    {
			throw std::runtime_error("unparseable LOC (bad X coordinate)");
		    }
		}

		// Match and parse the Y coordinate
		uint32_t loc_y;
		{
		    if (!MatchTokenPrefix(line_pos, line_end, "Y"))
		    {
			throw std::runtime_error("unhandled LOC (missing Y coordinate)");
		    }

		    std::string::const_iterator y_start = line_pos;

		    // Skip over the remaining token
		    SkipToken(line_pos, line_end);

		    if (!ParseIntToken(loc_y, y_start, line_pos))
		    {
			throw std::runtime_error("unparseable LOC (bad Y coordinate)");
		    }
		}

		// Now parse the bit type
		std::string::const_iterator tok;
		if (ScanTokenOnPrefixMatch(tok, line_pos, line_end, "Ram=B:BIT", 0u))
		{
		    // We have a data bit
		    uint32_t bit_index;
		    if (!ParseIntToken(bit_index, tok, line_pos))
		    {
			throw new std::runtime_error("invalid data bit position");
		    }

		    ProcessBramLocation(bit_offset, frame_addr, frame_offset, ram_type, loc_x, loc_y, bit_index, false);
		}
		else if (ScanTokenOnPrefixMatch(tok, line_pos, line_end, "Ram=B:PARBIT", 0u))
		{
		    // We have a parity bit
		    uint32_t bit_index;
		    if (!ParseIntToken(bit_index, tok, line_pos))
		    {
			throw new std::runtime_error("invalid parity bit position");
		    }

		    ProcessBramLocation(bit_offset, frame_addr, frame_offset, ram_type, loc_x, loc_y, bit_index, true);
		}
		else
		{
		    // TODO: Error handling. No idea what this is.
		    throw std::runtime_error("unhandled ram bit type");
		}
	    }

	    /**
	     * @brief Parses location information about a bit.
	     */
	    void ParseBitLocation(std::string::const_iterator& line_pos, const std::string::const_iterator line_end)
	    {
		// Parse the bit offset
		uint32_t bit_offset;
		if (!ParseIntToken(bit_offset, line_pos, line_end))
		{
		    // TODO: Error handling
		    throw std::runtime_error("unparseable bit offset");
		}

		// Parse the frame address
		uint32_t frame_addr;
		if (!ParseIntToken(frame_addr, line_pos, line_end))
		{
		    // TODO: Error handling
		    throw std::runtime_error("unparseable frame address");
		}

		// Parse the frame offset
		uint32_t frame_offset;
		if (!ParseIntToken(frame_offset, line_pos, line_end))
		{
		    // TODO: Error handling
		    throw std::runtime_error("unparseable frame offset");
		}

		std::string::const_iterator tok;

		if (ScanTokenOnPrefixMatch(tok, line_pos, line_end, "Block=RAMB36_", 0u))
		{
		    // We have a RAM bit for a RAMB36 type RAM
		    ParseBramLocation(tok, line_end, bit_offset, frame_addr, frame_offset, BramType::RAMB36);
		}
		else
		{
		    // TODO: All other lines (e.g. LUTs) are currently ignored
		}
	    }

	    /**
	     * @brief Parses a logic location file and extracts BRAM placement data
	     *
	     * Logic location files (.ll) are created by the Xilinx bitgen tool when the "-logic_location_file" option
	     * is specified. A logic location file contains a textual representation of the memory elements and LUT
	     * flip-flops of a design. This information can be used to interpret FPGA readback data, or, in our case,
	     * to manipulate the BRAM elements in a bitstream.
	     */
	    void DoParseV3(std::istream& lli)
	    {
		uint32_t line_no = 0u;
		std::string line;

		while (std::getline(lli, line))
		{
		    line_no += 1u;

		    // Skip leading whitespace
		    const std::string::const_iterator line_end = line.cend();
		    std::string::const_iterator line_pos = SkipWhitepace(line.cbegin(), line_end);
		    if ((line_pos == line_end) || (*line_pos == ';'))
		    {
			// Empty line (or comment)
			continue;
		    }

		    // Scan the line type token
		    const std::string::const_iterator tok_type = ScanToken(line_pos, line_end);
		    if (tok_type == line_end)
		    {
			// TODO: Raise an error (we currently skip it)
			continue;
		    }

		    if (IsTokenEqual(tok_type, line_pos, "Bit"))
		    {
			// Memory element or bit cell
			ParseBitLocation(line_pos, line_end);
		    }
		    else if (IsTokenEqual(tok_type, line_pos, "Info"))
		    {
			// Info statement (currently ignored)
		    }
		    else if (IsTokenEqual(tok_type, line_pos, "Revision"))
		    {
			// Revision statement (currently ignored)
		    }
		    else
		    {
			// Other statement (currently ignored)
		    }
		}
	    }
	};
    }
}

int main()
{
    // Parse the logic loc info
    {
	std::cout << "[+] parsing logic location information" << std::endl;
	std::ifstream f("C:\\data\\projects\\fpga\\unbit\\hw\\xc7z020\\xc7z020.runs\\impl_1\\all_ramb36e1_top.ll");
	fpga::xilinx::LogicLocationFile::ParseV3(f);
    }

    std::cout << "[+] done" << std::endl;
}
#endif
