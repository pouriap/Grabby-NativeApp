#pragma once

#include <string>
#include <Shlobj.h>
#include "jsonla.h"

class utils
{
public:
	static const std::string DLG_ID;

	utils(void);
	~utils(void);
	static void log(const char* msg);
	static void log(unsigned int msg);
	static std::string getSpecialPath(REFKNOWNFOLDERID rfid);
	static ggicci::Json parseJSON(const std::string &JSONstr);
	static std::string getTempPath();
	static bool mkdir(const std::string &dirName);
	static bool dirExists(const std::string &dirName_in);
	static std::string getDLGTempDir();
	static std::string launchExe(const std::string &exeName);
};

