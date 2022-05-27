//TODO: make stdout binary in flashgot nativehost
//TODO: make flashgot nativehost message lenght logic like this one
//TODO: add proper log

#include "stdafx.h"
#include <iostream>
#include <string>
#include <fstream>
#include <fcntl.h>
#include <io.h>
#include <thread>
#include "DLGrab_Native_Host.h"
#include "utils.h"
#include "messaging.h"
#include "exceptions.h"
#include "defines.h"

using namespace std;
using namespace ggicci;


int main(int argc, char *argv[])
{
	//initializations
	try{
		setupStdin();
		setupTmpDir();
	}
	catch(exception &e)
	{
		try{
			messaging::sendMessage("flashgot_output", e.what());
			LOG(e.what());
		}catch(...){}
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
		catch(fatal_exception &e)
		{
			try{
				messaging::sendMessage("flashgot_output", e.what());
				LOG(e.what());
			}catch(...){}
			//we exit after a fatal exception because otherwise the infinite loop will run rapidly
			exit(EXIT_FAILURE);
		}
		catch(exception &e)
		{
			try{
				messaging::sendMessage("flashgot_output", e.what());
				LOG(e.what());
			}catch(...){}
		}
    }
}

//sets stdin to binary mode
void setupStdin()
{
	int res1 = _setmode( _fileno( stdin ), _O_BINARY );
	int res2 = _setmode( _fileno( stdout ), _O_BINARY );
	if( res1 == -1 || res2 == -1){
		throw fatal_exception("cannot set stdin mode to binary");
	}
}

//sets up our temp directory where we put files
string setupTmpDir()
{
	try
	{
		string DLGTempDir = utils::getDLGTempDir();
		utils::mkdir(DLGTempDir);
		return DLGTempDir;
	}
	catch(...)
	{
		throw fatal_exception("cannot setup temp directory");
	}
}

//using the reference of the Json object because pass by value caused exception because of copy constructor error
void processMessage(const Json &msg)
{
	try
	{
		string type = msg["type"].AsString();
		if(type == MSGTYP_HOST_AVAIL)
		{
			handleType1(msg);
		}
		else if(type == MSGTYP_GET_AVAIL_DMS)
		{
			handleType2(msg);
		}
		else if(type == MSGTYP_DOWNLOAD)
		{
			handleType3(msg);
		}
		else if(type == MSGTYP_YTDL)
		{
			handleType4(msg);
		}
	}
	catch(exception &e)
	{
		string msg = "Error in processing message: ";
		msg.append(e.what());
		throw dlg_exception(msg.c_str());
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

void handleType4(const Json &msg)
{
	string url = msg["url"].AsString();
	std::thread th1(ytdl, url);
	th1.detach();
	messaging::sendMessage("flashgot_output", "thread started");
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

		vector<string> args;
		args.push_back(jobFileName);
		utils::launchExe("FlashGot.exe", args, false);
	}
	catch(exception &e)
	{
		string msg = "Error in FlashGot execution: ";
		msg.append(e.what());
		throw dlg_exception(msg.c_str());
	}
}

//TODO: check if launchExe is thread safe
//TODO: make log() thread safe
void ytdl(const string url)
{
	try
	{
		vector<string> args;
		args.push_back("-j");
		args.push_back(url);
		string ytdlJSON = utils::launchExe("ytdl.exe", args);
		messaging::sendMessageRaw(ytdlJSON);
	}
	catch(exception &e){
		try{
			messaging::sendMessage("error", e.what());
			LOG(e.what());
		}catch(...){}
	}
}