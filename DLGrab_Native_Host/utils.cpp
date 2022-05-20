#include "stdafx.h"
#include <fstream>
#include "utils.h"
#include "utf8.h"
#include "fileapi.h"
#include <ctime>
#include "exceptions.h"

#define LOG(x) utils::log(x)
#define FLG_JSON_BUF_SIZE 4000

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
		throw dlg_exception("failed to get special path");
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
		string msg = "Error parsing JSON: ";
		msg.append(e.what());
		throw dlg_exception(msg.c_str());
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

string utils::getNewTempFileName()
{
	std::time_t t = std::time(nullptr);
	string time = std::to_string(t);
	string filename = utils::getDLGTempDir();
	filename.append("\\job_").append(time).append(".fgt");
	return filename;
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

	throw dlg_exception("failed to create temp directory");
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

string utils::launchExe(const std::string &exeName, const bool returnOutput)
{
	return launchExe(exeName, "", returnOutput);
}

// a pipe has two ends, a read handle and a write handle
// the read handle reads from it, the write handle writes to it
// we make two pipes and end up with 2 read handles and 2 write handles
// we give one read handle and one write handle to the child
// after we give it the handles we close them becuase the parent doesn't need them anymore
// they will stay open in the child
//our handles:
//h_child_stdout_r
//h_child_stdout_w
//h_child_stdin_r
//h_child_stdin_w
string utils::launchExe(const string &exeName, const string &args, const bool returnOutput)
{
	HANDLE h_child_stdout_r = NULL;
	HANDLE h_child_stdout_w = NULL;
	HANDLE h_child_stdin_r = NULL;
	HANDLE h_child_stdin_w = NULL;

	SECURITY_ATTRIBUTES saAttr; 
	BOOL bSuccess = TRUE; 
 
	// Set the bInheritHandle flag so pipe handles are inherited. 
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = TRUE; 
	saAttr.lpSecurityDescriptor = NULL; 

	// Create a pipe for the child process's STDOUT. 
	bSuccess &= CreatePipe(&h_child_stdout_r, &h_child_stdout_w, &saAttr, 0);

	// Ensure the read handle to the pipe for STDOUT is not inherited.
	bSuccess &= SetHandleInformation(h_child_stdout_r, HANDLE_FLAG_INHERIT, 0);

	// Create a pipe for the child process's STDIN. 
	bSuccess &= CreatePipe(&h_child_stdin_r, &h_child_stdin_w, &saAttr, 0);

	// Ensure the write handle to the pipe for STDIN is not inherited. 
	bSuccess &= SetHandleInformation(h_child_stdin_w, HANDLE_FLAG_INHERIT, 0);

	if(!bSuccess)
	{
		throw dlg_exception("Failed to create child process handles");
	}

	PROCESS_INFORMATION piProcInfo; 
	STARTUPINFO siStartInfo;
  
	ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
	ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
 
	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection.
	siStartInfo.cb = sizeof(STARTUPINFO); 
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
	siStartInfo.hStdError = h_child_stdout_w;
	siStartInfo.hStdOutput = h_child_stdout_w;
	siStartInfo.hStdInput = h_child_stdin_r;

	DWORD processFlags = CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT;

	// Create the child process. 
	bSuccess = CreateProcess(
		const_cast<wchar_t *>(utf8::widen(exeName).c_str()),		// application name
		const_cast<wchar_t *>(utf8::widen(args).c_str()),           // command line
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		processFlags,  // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&piProcInfo);  // receives PROCESS_INFORMATION 
   
	// If an error occurs, exit the application. 
	if (!bSuccess) 
	{
		throw dlg_exception("Create process failed");
	}

	// Close handles to the child process and its primary thread.
	// Some applications might keep these handles to monitor the status
	// of the child process, for example. 
	CloseHandle(piProcInfo.hProcess);
	CloseHandle(piProcInfo.hThread);
      
	// Close handles to the stdin and stdout pipes no longer needed by the child process.
	// If they are not explicitly closed, there is no way to recognize that the child process has ended.
	CloseHandle(h_child_stdout_w);
	CloseHandle(h_child_stdin_r);

	if(!returnOutput)
	{
		return "";
	}

	//TODO: dynamically allocate this
	char buf[FLG_JSON_BUF_SIZE];
	for(int i=0; i<FLG_JSON_BUF_SIZE; i++)
	{
		buf[i] = '\0';
	}
	unsigned long dwRead = 0;
	bSuccess = FALSE;

	bSuccess = ReadFile(h_child_stdout_r, buf, FLG_JSON_BUF_SIZE, &dwRead, NULL);
	if(!bSuccess || dwRead==0)
	{
		throw dlg_exception("Failed to read process output");
	}

	return buf;

}