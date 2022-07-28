#pragma once

#include <string>
#include <Shlobj.h>
#include "jsonla.h"

class utils
{

public:
	utils(void);
	~utils(void);
	static ggicci::Json parseJSON(const std::string &JSONstr);
	static std::string getNewTempFileName();
	static bool mkdir(const std::string &dirName);
	static std::string getDLGTempDir();
	static DWORD launchExe(const std::string &exeName, const std::vector<std::string> &args, 
		std::string *output = NULL, void (*onOutput)(std::string output) = NULL );
	static void strReplaceAll(std::string &data, const std::string &toSearch, const std::string &replaceStr);
	static std::vector<std::string> strSplit(const std::string &str, const char delim);
	static std::string saveDialog(const std::string &filename);
	static std::string sanitizeFilename(const char* filename);
	static std::vector<std::string> getEnvarNames();
	static bool strHasEnvars(const std::string &str);
	static std::string strToLower(const std::string &str);

private:
	static std::string getSpecialPath(REFKNOWNFOLDERID rfid);
	static std::string getTempPath();
	static bool dirExists(const std::string &dirName_in);

};

