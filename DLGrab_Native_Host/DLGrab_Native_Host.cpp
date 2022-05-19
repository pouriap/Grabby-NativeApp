//TODO: make stdout binary in flashgot nativehost
//TODO: make flashgot nativehost message lenght logic like this one
//TODO: add proper log

#include "stdafx.h"
#include <iostream>
#include <string>
#include <fstream>
#include <fcntl.h>
#include <io.h>
#include "DLGrab_Native_Host.h"
#include "utils.h"
#include "messaging.h"

#define LOG(x) utils::log(x)

using namespace std;
using namespace ggicci;

int main(int argc, char *argv[])
{
	string DLGTmpDir;

	//initializations
	try{
		setupStdin();
		setupTmpDir();
	}
	catch(string exp_msg)
	{
		messaging::sendMessage("flashgot_output", exp_msg);
		LOG(exp_msg.c_str());
		exit(EXIT_FAILURE);
	}
	catch(...)
	{
		messaging::sendMessage("flashgot_output", "an unknown exception occured");
		LOG("an unknown exception occured");
		exit(EXIT_FAILURE);
	}

	//the loop for sending/receiving messages
    while(true)
	{
		try
		{
			string raw_message = messaging::get_message();
			Json &msg = utils::parseJSON(raw_message);
			processMessage(msg);
		}
		catch(string &exp_msg)
		{
			messaging::sendMessage("flashgot_output", exp_msg);
			LOG(exp_msg.c_str());
		}
		catch(std::length_error &e)
		{
			messaging::sendMessage("flashgot_output", e.what());
			LOG(e.what());
			//we exit after a length error because otherwise we'll end up in a loop
			exit(EXIT_FAILURE);
		}
		catch(...)
		{
			messaging::sendMessage("flashgot_output", "an unknown exception occured");
			LOG("an unknown exception occured");
		}
    }
}

//sets stdin to binary mode
void setupStdin()
{
	int result = _setmode( _fileno( stdin ), _O_BINARY );
	if( result == -1 ){
		LOG("cannot set stdin mode to binary");
		exit(EXIT_FAILURE);
	}
}

//sets up our temp directory where we put files
string setupTmpDir()
{
	string DLGTempDir = utils::getDLGTempDir();
	utils::mkdir(DLGTempDir);
	return DLGTempDir;
}

//using the reference of the Json object because pass by value caused exception because of copy constructor error
void processMessage(const Json &msg)
{
	try
	{
		string type = msg["type"].AsString();
		if(type == "native_client_available")
		{
			handleType1(msg);
		}
		else if(type == "get_available_dms")
		{
			handleType2(msg);
		}
		else if(type == "download")
		{
			handleType3(msg);
		}
	}
	//if it's one of my own thrown exceptions throw it higher
	catch(string &s)
	{
		throw s;
	}
	catch(exception &e)
	{
		string msg = "error parsing JSON: ";
		msg.append(e.what());
		throw string(msg);
	}
	//otherwise throw this
	catch(...)
	{
		throw string("unexpected error occured");
	}
}

//handles "native_client_available" request
void handleType1(const Json &msg)
{
	Json json = Json::Parse("{}");
	json.AddProperty("type", Json("native_client_available"));
	messaging::sendMessage(json);
}

//handles "get_available_dms" request
void handleType2(const Json &msg)
{
	//TODO: output this directly from flashgot.exe
	string fgOutput = utils::launchExe("flashgot.exe");
	Json &dmsJSON = utils::parseJSON(fgOutput.c_str());
	Json res = Json::Parse("{}");
	res.AddProperty("type", Json("available_dms"));
	Json dms = Json::Parse("[]");
	for(int i=0; i<dmsJSON.Size(); i++)
	{
		if(dmsJSON[i]["available"].AsBool())
		{
			string name = dmsJSON[i]["name"].AsString();
			dms.Push(Json(name));
		}
	}
	res.AddProperty("availableDMs", dms);
	messaging::sendMessage(res);
}

//TODO: make FlashGot directly accept the JSON as job file instead of this 
//handles "download" request
void handleType3(const Json &msg)
{
	const Json &job = msg["job"];
	const Json &downloadsInfo = job["downloadsInfo"];
	string dmName = job["dmName"].AsString();
	string header("");
	//length;dmName;0;;
	string msgstr = msg.ToString();
	string jobstr = job.ToString();
	string infostr = downloadsInfo.ToString();
	header.append(std::to_string(downloadsInfo.Size()));
	header.append(";");
	header.append(dmName);
	header.append(";0;;");

	string jobText = header;
	jobText.append("\n");
	jobText.append(job["referer"].AsString());
	jobText.append("\n");

	for(int i=0; i<downloadsInfo.Size(); i++)
	{
		jobText.append(downloadsInfo[i]["url"].AsString());
		jobText.append("\n");
		jobText.append(downloadsInfo[i]["desc"].AsString());
		jobText.append("\n");
		jobText.append(downloadsInfo[i]["cookies"].AsString());
		jobText.append("\n");
		jobText.append(downloadsInfo[i]["postData"].AsString());
		jobText.append("\n");
		jobText.append(downloadsInfo[i]["filename"].AsString());
		jobText.append("\n");
		jobText.append(downloadsInfo[i]["extension"].AsString());
		jobText.append("\n");
	}

	jobText.append(job["originPageReferer"].AsString());
	jobText.append("\n");
	jobText.append(job["originPageCookies"].AsString());
	jobText.append("\n");
	jobText.append("\n");
	jobText.append("\n");
	jobText.append(job["useragent"].AsString());
	jobText.append("\n");

	flashGot(jobText);
}

//launches FlashGot to perform a download with a DM
void flashGot(const string &jobText)
{
	try
	{
		string jobFileName = utils::getNewTempFileName();
		std::ofstream jobFile(jobFileName);
		jobFile << jobText;
		jobFile.close();
		string commandLine = "FlashGot.exe ";
		commandLine.append("\"");
		commandLine.append(jobFileName);
		commandLine.append("\"");
		string flashGotResponse = utils::launchExe("FlashGot.exe", commandLine);
		messaging::sendMessage("flashgot_output", flashGotResponse);
	}
	catch(string &e)
	{
		string msg = "creating job file failed: ";
		msg.append(e);
		throw msg;
	}
	catch(...)
	{
		throw "unexpected error when creating job file";
	}
}