#include "stdafx.h"
#include <fstream>
#include "utils.h"
#include "utf8.h"
#include "fileapi.h"

using namespace std;
using namespace ggicci;

//static fields
const string utils::DLG_ID = "download.grab.pouriap";

//static members
utils::utils(void)
{
}

utils::~utils(void)
{
}

void utils::log(const char* msg)
{
	string filename = utils::getSpecialPath(FOLDERID_Desktop);
	filename.append("\\log.txt");
    ofstream logFile;
    logFile.open(filename, std::ios_base::app);
	logFile << msg << endl;
	logFile.close();
}

void utils::log(unsigned int msg)
{
	string filename = utils::getSpecialPath(FOLDERID_Desktop);
	filename.append("\\log.txt");
    ofstream logFile;
    logFile.open(filename, std::ios_base::app);
	logFile << msg << endl;
	logFile.close();
}

string utils::getSpecialPath(REFKNOWNFOLDERID rfid)
{
	PWSTR path;
	if(SHGetKnownFolderPath(rfid, 0, NULL, &path) != S_OK)
	{
		throw "failed to get special path";
	}

	string stPath = utf8::narrow(path);
	CoTaskMemFree(path);
	return stPath;
}

Json utils::parseJSON(const string &JSONstr)
{
	try
	{
		Json json = Json::Parse(JSONstr.c_str());
		return json;
	}
	catch (exception& e)
	{
		string msg = "error parsing json\n";
		msg.append(e.what());
		throw msg;
	}
}

std::string utils::getTempPath()
{
	char str[MAX_PATH];
	if(GetTempPathA(MAX_PATH, str) > 0)
	{
		return string(str);
	}
	//get appdata\local\temp if above failed
	else{
		string appdataLocal = utils::getSpecialPath(FOLDERID_LocalAppData);
		appdataLocal.append("\\temp");
		return appdataLocal;
	}	
}

bool utils::mkdir(const string &dirName)
{
	if(utils::dirExists(dirName)){
		return true;
	}

	if(CreateDirectoryA(dirName.c_str(), NULL))
	{
		return true;
	}

	throw "failed to create temp directory";
}

bool utils::dirExists(const std::string& dirName_in)
{
	DWORD ftyp = GetFileAttributesA(dirName_in.c_str());

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
	{
		return true;
	}

	return false;
}

std::string utils::getDLGTempDir()
{
	static string DLGTempDir = "";
	if(DLGTempDir.length() == 0)
	{
		string tempPath = utils::getTempPath();
		DLGTempDir.append(tempPath).append("\\").append(utils::DLG_ID);
	}
	return DLGTempDir;

}
