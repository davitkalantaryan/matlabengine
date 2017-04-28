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

		argument_parser& AddOption(const char* option_name,int needs_arg); // adding new option
		argument_parser& operator<<(const char* option_name); // adding new option
        const char* operator[](const char* option_name); // find option value
		template <typename TypeArgc, typename TypeArgv>
		void	ParseCommandLine(TypeArgc argc, TypeArgv argv[]);
        std::string	HelpString()const;

	private:
        std::map<std::string,std::string>	m_htOptionsFound;
        std::map<std::string,int>			m_htOptionsIn;
	};

}

#include "common_argument_parser.tos"


#else   // #ifdef __cplusplus
#endif  // #ifdef __cplusplus


#endif  // #ifndef __common_argument_parser_hpp__
