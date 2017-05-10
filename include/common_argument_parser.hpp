/*
 *	File: argument_parser.hpp
 *
 *	Created on: 25 Nov, 2016
 *	Created by: Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *  This file implements the class for parsing 
 *  command line arguments
 *  boost has classes for this purpose,
 *  but sometime dependency from boost is not needed
 *
 */
#ifndef __common_argument_parser_hpp__
#define __common_argument_parser_hpp__

#ifdef __cplusplus

#include <string>
#include <map>

namespace common
{
	class argument_parser
	{
	public:
		argument_parser();
		virtual ~argument_parser();

		argument_parser& AddOption(const std::string& optionName,int isArg, const std::string& defValue); // adding new option
		argument_parser& operator<<(const std::string& optionName); // adding new option
        const char* operator[](const char* option_name); // find option value
		template <typename TypeArgc, typename TypeArgv>
		void	ParseCommandLine(TypeArgc argc, TypeArgv argv[]);
        std::string	HelpString()const;

	private:
		struct SInput{ int isArg; std::string defaultValue; };

	private:
        std::map<std::string,std::string>	m_htOptionsFound;
        std::map<std::string, SInput>		m_htOptionsIn2;
	};

}

#include "common_argument_parser.tos"


#else   // #ifdef __cplusplus
#endif  // #ifdef __cplusplus


#endif  // #ifndef __common_argument_parser_hpp__
