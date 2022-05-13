#include "stdafx.h"
#include <fstream>
#include "utils.h"
#include "utf8.h"

using namespace std;

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
	SHGetKnownFolderPath(rfid, 0, NULL, &path);
	string stPath = utf8::narrow(path);
	CoTaskMemFree(path);
	return stPath;
}