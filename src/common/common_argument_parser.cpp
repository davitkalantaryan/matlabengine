/*
 *	File: argument_parser.cpp
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

//#include "stdafx.h"
#ifdef __cplusplus
#include "common_argument_parser.hpp"

common::argument_parser::argument_parser()
{}

common::argument_parser::~argument_parser()
{}


common::argument_parser&  common::argument_parser::operator<<(const std::string& a_strOptionName)
{
	return AddOption(a_strOptionName,0,"");
}


common::argument_parser& common::argument_parser::AddOption(
	const std::string& a_strOptionName, int a_nIsArg, const std::string& a_strDefValue)
{
	SInput aInput;

	aInput.isArg = a_nIsArg;
	aInput.defaultValue = a_strDefValue;
    m_htOptionsIn2.insert(std::pair<std::string, SInput>(a_strOptionName, aInput));
	return *this;
}


const char* common::argument_parser::operator[](const char* a_option_name)
{
    if(m_htOptionsFound.count(a_option_name)){return m_htOptionsFound[a_option_name].c_str();}
    return NULL;
}


std::string common::argument_parser::HelpString()const
{
    return "To be done!";
}


#else   // #ifdef __cplusplus
#endif  // #ifdef __cplusplus
