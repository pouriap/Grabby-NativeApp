#include "stdafx.h"
#include "exceptions.h"


fatal_exception::fatal_exception(const char* msg) : message("")
{
	message.append("Fatal exception: ");
	message.append(msg);
}
fatal_exception::~fatal_exception(void)
{
}
const char* fatal_exception::what() const throw()
{
	return message.c_str();
}



grb_exception::grb_exception(const char* msg) : message(msg)
{
}
grb_exception::~grb_exception(void)
{
}
const char* grb_exception::what() const throw()
{
	return message.c_str();
}



grb_exception_gui::grb_exception_gui(const char* msg) : grb_exception(msg)
{
}
grb_exception_gui::~grb_exception_gui(void)
{
}
