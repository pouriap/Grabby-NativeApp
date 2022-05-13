#pragma once

#include <string>
#include <Shlobj.h>
#include "jsonla.h"

class utils
{
public:
	utils(void);
	~utils(void);
	static void log(const char* msg);
	static void log(unsigned int msg);
	static std::string getSpecialPath(REFKNOWNFOLDERID rfid);
	static ggicci::Json parseJSON(std::string JSONstr);
};

