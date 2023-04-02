#pragma once

#include <string>
#include <Shlobj.h>
#include "jsonla.h"
#include "output_callback.h"
#include "types.h"

using namespace std;

class utils
{

public:
	utils(void);
	~utils(void);
	static ggicci::Json parseJSON(const string &JSONstr);
	static string getNewTempFileName();
	static bool mkdir(const string &dirName);
	static string getGRBTempDir();
	static process_result launchExe(const string &exeName, const vector<string> &args,
		const string &input = "", const bool &kill = false, output_callback *callback = NULL );
	static DWORD runCmd(const string &cmd);
	static void strReplaceAll(string &data, const string &toSearch, const string &replaceStr);
	static vector<string> strSplit(const string &str, const char delim);
	static string fileSaveDialog(const string &filename);
	static string folderOpenDialog();
	static string sanitizeFilename(const char* filename);
	static vector<string> getEnvarNames();
	static bool strHasEnvars(const string &str);
	static string strToLower(const string &str);
	static string trim(string str);

private:
	static string getSpecialPath(REFKNOWNFOLDERID rfid);
	static string getTempPath();
	static bool dirExists(const string &dirName_in);

};

