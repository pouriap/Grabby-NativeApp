#pragma once

#include <string>
#include <Shlobj.h>
#include "jsonla.h"
#include "output_callback.h"

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
	static DWORD launchExe(const string &exeName, const vector<string> &args, string *output, 
		const string &input = "", output_callback *callback = NULL );
	static void strReplaceAll(string &data, const string &toSearch, const string &replaceStr);
	static vector<string> strSplit(const string &str, const char delim);
	static string fileSaveDialog(const string &filename);
	static string folderOpenDialog();
	static string sanitizeFilename(const char* filename);
	static vector<string> getEnvarNames();
	static bool strHasEnvars(const string &str);
	static string strToLower(const string &str);

private:
	static string getSpecialPath(REFKNOWNFOLDERID rfid);
	static string getTempPath();
	static bool dirExists(const string &dirName_in);

};

