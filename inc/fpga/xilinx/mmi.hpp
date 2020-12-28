/**
 * @file
 * @brief Xilinx BRAM Memory Map Info (MMI) File manipulation
 * 
 * 
 */
#ifndef FPGA_XILINX_MMI_HPP_
#define FPGA_XILINX_MMI_HPP_ 1

#include "common.hpp"
#include "mapper.hpp"

namespace fpga
{
	namespace xilinx
	{
		/**
		 * @brief Common infrastructure for Xilinx Series-7 FPGAs
		 */
		namespace v7
		{
			//--------------------------------------------------------------------------------------------------------------------		
			/**
			 * @brief FPGA type-matching callback for @ref mmi_load.
			 */
			typedef std::function<const zynq7& (const std::string&)> mmi_fpga_type_lookup_t;

			/**
			 * @brief Instance filter callback for @ref mmi_load.
			 */
			typedef std::function<bool(const std::string&)> mmi_instance_filter_t;

			/**
			 * @brief Loads the bit mapping of an XPM memory from an MMI file.
			 *
			 * @remark
			 *  See [Xilinx UG898; "BRAM Memory Map Info (MMI) File"; current version 2020.2 at time of writing] for (some) details
			 *  on the file format. Note that UG898 only describes the syntax of MMI files for processor based design in details.
			 *  The user guide specifically mentions that XPM memory macros can be initialized, and that the tools create appropriate
			 *  information for the UpdateMEM tool flow. The details of the MMI file structures are not (directly) discussed in UG898;
			 *  experimentation with Xilinx tools shows that structure are somewhat similar for generic XPM based flows.			 
			 *
			 * @param[in] filename specifies the name of the input file to be parsed.
			 * 
			 * @param[in] fpga specifies a lookup function to resolve the FPGA device. The function is called with the FPGA part number
			 *   as provided in the option tag of the input file.
			 * 
			 * @param[in] instance_filter specifies an (optional) filter function used to select the memory blocks that shall be mapped.
			 *   Filtering is disabled (i.e. all memory blocks are extracted) when no filter is given.
			 */
			void mmi_load(const std::string& filename, mmi_fpga_type_lookup_t fpga_lookup, mmi_instance_filter_t instance_filter);
		}
	}
}

#endif // #ifndef FPGA_XILINX_MMI_HPP_
