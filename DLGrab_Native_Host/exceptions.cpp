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



dlg_exception::dlg_exception(const char* msg) : message(msg)
{
}
dlg_exception::~dlg_exception(void)
{
}
const char* dlg_exception::what() const throw()
{
	return message.c_str();
}