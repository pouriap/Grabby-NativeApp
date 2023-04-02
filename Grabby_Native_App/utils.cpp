#include "stdafx.h"
#include <fstream>
#include "utils.h"
#include "utf8.h"
#include "fileapi.h"
#include <aclapi.h>
#include <ctime>
#include <strsafe.h>
#include <mutex>
#include "exceptions.h"
#include "defines.h"

using namespace std;
using namespace ggicci;


std::mutex guiMutex;


utils::utils(void)
{
}

utils::~utils(void)
{
}

string utils::getSpecialPath(REFKNOWNFOLDERID rfid)
{
	PWSTR path;
	if(SHGetKnownFolderPath(rfid, 0, NULL, &path) != S_OK)
	{
		throw grb_exception("failed to get special path");
	}

	string stPath = utf8::narrow(path);
	//SHGetKnownFolderPath does not include a trailing backslash
	stPath.append("\\");
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
		throw grb_exception(msg.c_str());
	}
}

string utils::getTempPath()
{
	char str[MAX_PATH];
	if(GetTempPathA(MAX_PATH, str) > 0)
	{
		return string(str);
	}
	//get appdata\local\temp if above failed
	else{
		string appdataLocal = utils::getSpecialPath(FOLDERID_LocalAppData);
		appdataLocal.append("temp\\");
		return appdataLocal;
	}	
}

string utils::getNewTempFileName()
{
	std::time_t t = std::time(nullptr);
	string time = std::to_string(t);
	string filename = utils::getGRBTempDir();
	filename.append("job_").append(time).append(".fgt");
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

	throw grb_exception("failed to create temp directory");
}

bool utils::dirExists(const string& dirName_in)
{
	DWORD ftyp = GetFileAttributesA(dirName_in.c_str());

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
	{
		return true;
	}

	return false;
}

string utils::getGRBTempDir()
{
	static string GRBTempDir = "";
	if(GRBTempDir.length() == 0)
	{
		string tempPath = utils::getTempPath();
		GRBTempDir.append(tempPath).append(GRB_ADDON_ID);
	}
	return GRBTempDir;
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
process_result utils::launchExe(const string &exeName, const vector<string> &args, const string &input, 
					   const bool &kill, output_callback *callback)
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
		throw grb_exception("Failed to create child process handles");
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

	DWORD processFlags = CREATE_UNICODE_ENVIRONMENT | CREATE_NEW_PROCESS_GROUP;

	if(exeName != "yt-dlp.exe")
	{
		processFlags |= CREATE_BREAKAWAY_FROM_JOB;
	}

	//first part of cmd line has to also be the application name
	string cmd = "";
	cmd.append("\"").append(exeName).append("\"");

	//create cmd line of arguments from the args vector
	for(int i=0; i<args.size(); i++)
	{
		//if it's an empty string don't add anything to command line
		if(args[i].length() == 0){
			continue;
		}
		cmd.append(" ").append("\"").append(args[i]).append("\"");
	}

	//some checks
	if(exeName.length() > MAX_PATH)
	{
		throw grb_exception("Executable file name is too big");
	}

	if(cmd.length() > CMD_MAX_LEN)
	{
		throw grb_exception("Command line too big");
	}

	PLOG_INFO << "exe name: " << exeName << " - cmd: " << cmd;

	WCHAR cmdWchar[CMD_MAX_LEN] = { '\0' };
	StringCchCopyW(cmdWchar, CMD_MAX_LEN, utf8::widen(cmd).c_str());

	// Create the child process. 
	bSuccess = CreateProcessW(
		utf8::widen(exeName).c_str(),		// application name
		cmdWchar,      // command line
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		processFlags,  // creation flags 
		NULL,          // use parent environment
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&piProcInfo);  // receives PROCESS_INFORMATION 
   
	// If an error occurs, exit the application. 
	if (!bSuccess) 
	{
		string msg = "Create process failed with error code ";
		msg.append( std::to_string(GetLastError()) );
		throw grb_exception(msg.c_str());
	}
      
	// Close handles to the stdin and stdout pipes no longer needed by the child process.
	// If they are not explicitly closed, there is no way to recognize that the child process has ended.
	CloseHandle(h_child_stdout_w);
	CloseHandle(h_child_stdin_r);

	//write the input to the STDIN of the launched process
	if(input.length() > 0)
	{
		DWORD dwWritten;
		DWORD dataLen = input.length();
		bSuccess = WriteFile(h_child_stdin_w, input.c_str(), dataLen, &dwWritten, NULL);
		if(!bSuccess || dwWritten!=dataLen){
			PLOG_ERROR << "stdin write failed: " << GetLastError();
		}
	}
	//close the handle so that the reading end of the pipe will get out of the fread() loop
	CloseHandle(h_child_stdin_w);

	//we read the output of the process
	const int BUFSIZE = 1024;
	char buf[BUFSIZE];
	unsigned long bytesRead = 0;
	unsigned long totalRead = 0;
	bSuccess = FALSE;
	vector<char> out;

	//keep reading process output until it exits or we receive a kill command
	while(true)
	{
		//ReadFile returns when it reaches a carriage return or eof
		bSuccess = ReadFile(h_child_stdout_r, buf, BUFSIZE, &bytesRead, NULL);

		if(!bSuccess || bytesRead<=0)
		{
			break;
		}

		out.reserve(BUFSIZE);
		out.insert(out.end(), buf, buf+bytesRead);
		totalRead += bytesRead;

		//pass output to callback function
		if(callback != NULL)
		{
			string outStr(buf, buf+bytesRead);
			callback->call(outStr);
		}

		if(kill)
		{
			GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, piProcInfo.dwProcessId);
			break;
		}
	}

	// Process has exited - check its exit code
	WaitForSingleObject(piProcInfo.hProcess, 1000);
	DWORD exitCode;
	GetExitCodeProcess(piProcInfo.hProcess, &exitCode);
	PLOG_INFO << "process exit code is " << exitCode;

	// Close handles to the child process and its primary thread.
	// Some applications might keep these handles to monitor the status
	// of the child process, for example. 
	CloseHandle(piProcInfo.hProcess);
	CloseHandle(piProcInfo.hThread);

	process_result res;
	res.exitCode = exitCode;

	if(totalRead<=0)
	{
		res.output = "";
	}
	else
	{
		res.output = string(out.begin(), out.end());
	}

	PLOG_INFO << "exe output be: " << res.output;

	return res;

}

DWORD utils::runCmd(const string &cmd, bool showConsole)
{
	BOOL bSuccess = TRUE;

	PROCESS_INFORMATION piProcInfo; 
	STARTUPINFO siStartInfo;

	ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
	ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
	siStartInfo.cb = sizeof(STARTUPINFO); 

	DWORD processFlags = CREATE_UNICODE_ENVIRONMENT | CREATE_BREAKAWAY_FROM_JOB;

	if(showConsole)
	{
		processFlags |= CREATE_NEW_CONSOLE;
	}
	else
	{
		processFlags |= CREATE_NO_WINDOW;
	}

	if(cmd.length() > CMD_MAX_LEN)
	{
		throw grb_exception("Command line too big");
	}

	PLOG_INFO << "custom cmd: " << cmd;

	WCHAR cmdWchar[CMD_MAX_LEN] = { '\0' };
	StringCchCopyW(cmdWchar, CMD_MAX_LEN, utf8::widen(cmd).c_str());

	// Create the child process. 
	bSuccess = CreateProcessW(
		NULL,		   // No module name (use command line)
		cmdWchar,      // command line
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		FALSE,         // handles not inherited 
		processFlags,  // creation flags 
		NULL,          // use parent environment
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&piProcInfo);  // receives PROCESS_INFORMATION 

	// If an error occurs, exit the application. 
	if (!bSuccess) 
	{
		string msg = "Create process failed with error code ";
		msg.append( std::to_string(GetLastError()) );
		throw grb_exception(msg.c_str());
	}

	// Process has exited - check its exit code
	WaitForSingleObject(piProcInfo.hProcess, INFINITE);
	DWORD exitCode;
	GetExitCodeProcess(piProcInfo.hProcess, &exitCode);
	PLOG_INFO << "custom cmd exit code is " << exitCode;

	// Close handles to the child process and its primary thread.
	// Some applications might keep these handles to monitor the status
	// of the child process, for example. 
	CloseHandle(piProcInfo.hProcess);
	CloseHandle(piProcInfo.hThread);

	return exitCode;
}

void utils::strReplaceAll(string &data, const string &toSearch, const string &replaceStr)
{
	size_t pos = data.find(toSearch);
	while(pos != string::npos)
	{
		data.replace(pos, toSearch.size(), replaceStr);
		pos =data.find(toSearch, pos + replaceStr.size());
	}
}

vector<string> utils::strSplit(const string &str, const char delim)
{
	vector<string> parts;
	stringstream stream(str);
	string temp;

	if(str.find(delim) == string::npos){
		return parts;
	}

	while(getline(stream, temp, delim))
	{
		if(temp.size() > 0) parts.push_back(temp);
	}

	return parts;
}

string utils::fileSaveDialog(const string &filename)
{
	//UI in multiple threads bad
	std::lock_guard<std::mutex> lock(guiMutex);

	string path = "";
	bool cancelled = false;

	CoInitialize(nullptr);
	IFileDialog *pfd = NULL;
	HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
	if (SUCCEEDED(hr))
	{
		DWORD dwFlags;
		// get the options first in order not to override existing options.
		if (SUCCEEDED(pfd->GetOptions(&dwFlags)))
		{
			// set the options
			if (SUCCEEDED(pfd->SetOptions(dwFlags | FOS_OVERWRITEPROMPT | FOS_NOCHANGEDIR | FOS_PATHMUSTEXIST | FOS_FORCEFILESYSTEM)))
			{
				// set the default file name in dialog
				if (SUCCEEDED(pfd->SetFileName(utf8::widen(filename).c_str())))
				{
					// show dialog
					HRESULT res = pfd->Show(GetForegroundWindow());
					if(res == HRESULT_FROM_WIN32(ERROR_CANCELLED))
					{
						cancelled = true;
					}
					if (SUCCEEDED(res))
					{
						IShellItem *psiResult;
						if (SUCCEEDED(pfd->GetResult(&psiResult)))
						{
							PWSTR pszFilePath = NULL;
							if (SUCCEEDED(psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath)))
							{
								path = utf8::narrow(pszFilePath);
							}
							psiResult->Release();
						}
					}
				}
			}
		}
		pfd->Release();
	}

	if(path.length() == 0 && !cancelled)
	{
		throw grb_exception("failed to show save file dialog"); 
	}

	return path;
}

string utils::folderOpenDialog()
{
	//UI in multiple threads bad
	std::lock_guard<std::mutex> lock(guiMutex);

	string path = "";
	bool cancelled = false;

	CoInitialize(nullptr);
	IFileDialog *pfd = NULL;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
	if (SUCCEEDED(hr))
	{
		DWORD dwFlags;
		if (SUCCEEDED(pfd->GetOptions(&dwFlags)))
		{
			if (SUCCEEDED(pfd->SetOptions(dwFlags | FOS_OVERWRITEPROMPT | FOS_NOCHANGEDIR | FOS_PATHMUSTEXIST | FOS_FORCEFILESYSTEM | FOS_PICKFOLDERS)))
			{
				HRESULT res = pfd->Show(GetForegroundWindow());
				if(res == HRESULT_FROM_WIN32(ERROR_CANCELLED))
				{
					cancelled = true;
				}
				if (SUCCEEDED(res))
				{
					IShellItem *psiResult;
					if (SUCCEEDED(pfd->GetResult(&psiResult)))
					{
						PWSTR pszFilePath = NULL;
						if (SUCCEEDED(psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath)))
						{
							path = utf8::narrow(pszFilePath) + "\\";
						}
						psiResult->Release();
					}
				}
			}
		}
		pfd->Release();
	}

	if(path.length() == 0 && !cancelled)
	{
		throw grb_exception("failed to show open folder dialog"); 
	}

	return path;
}

string utils::sanitizeFilename(const char* filename)
{
	string newName("");

	const char illegalChars[9] = { '<', '>', ':', '"', '/', '\\', '|', '?', '*' };

	for(int i=0; i < strlen(filename); i++)
	{
		char c = filename[i];
		bool replace = false;

		//check for non-printable characters
		if(c < 32 || c > 126){
			replace = true;
		}
		//check in illegal characters
		else
		{
			for(int j=0; j<sizeof(illegalChars); j++)
			{
				if(c == illegalChars[j]){
					replace = true;
					break;
				}
			}
		}

		if(!replace){
			newName += c;
		}
		else{
			newName += '_';
		}
	}

	return newName;
}

vector<string> utils::getEnvarNames()
{
	vector<string> envars;

	LPWSTR lpszVariable; 
	LPWCH lpvEnv; 

	// Get a pointer to the environment block. 
	lpvEnv = GetEnvironmentStrings();

	// If the returned pointer is NULL, exit.
	if (lpvEnv == NULL)
	{
		return envars;
	}

	// Variable strings are separated by NULL byte, and the block is 
	// terminated by a NULL byte. 
	lpszVariable = (LPTSTR) lpvEnv;

	while (*lpszVariable)
	{
		string varFull = utf8::narrow(lpszVariable);

		if(varFull.find('=') != 0){
			//there are some weird variables that start with '=' and we don't want them 
			//https://devblogs.microsoft.com/oldnewthing/20100506-00/?p=14133
			//this doesn't seem to be documented anywhere so I'm not sure if all these weird variables start with '='
			//only time will tell
			string varName = varFull.substr(0, varFull.find('='));
			envars.push_back(varName);
		}

		lpszVariable += lstrlen(lpszVariable) + 1;
	}

	FreeEnvironmentStrings(lpvEnv);

	return envars;
}

bool utils::strHasEnvars(const string &str)
{
	vector<string> envars = utils::getEnvarNames();
	string strl = utils::strToLower(str);

	for(int i=0; i<envars.size(); i++)
	{
		string var = "%" + utils::strToLower(envars[i]) + "%";
		if(strl.find(var) != string::npos){
			return true;
		}
	}

	return false;
}

string utils::strToLower(const string &str)
{
	string strl("");

	for(int i=0; i<str.length(); i++)
	{
		strl += std::tolower(str[i]);
	}

	return strl;
}

string utils::trim(string str)
{
	const char* ws = " \t\n\r\f\v";
	str.erase(str.find_last_not_of(ws) + 1);
	str.erase(0, str.find_first_not_of(ws));

	return str;
}