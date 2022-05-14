//TODO: make stdout binary in flashgot nativehost
//TODO: make flashgot nativehost message lenght logic like this one

#include "stdafx.h"

#define LOG(x) utils::log(x)

#define MSG_TYPE_ERROR 1

#include <iostream>
#include <string>
#include <fstream>
#include <fcntl.h>
#include <io.h>
#include "utils.h"
#include "messaging.h"
#include <ctime>

using namespace std;
using namespace ggicci;

void handleType1(const Json &msg)
{
	Json json = Json::Parse("{}");
	json.AddProperty("type", Json("native_client_available"));
	messaging::sendMessage(json);
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
	}
	catch(exception &e)
	{
		throw string("unexpected JSON content in message");
	}
}

//sets stdin to binary mode
void setupStdin()
{
	/* Set "stdin" to have binary mode: */
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

string getNewTempFileName()
{
	std::time_t t = std::time(nullptr);
	string time = std::to_string(t);
	string filename = utils::getDLGTempDir();
	filename.append("\\job_").append(time).append(".fgt");
	return filename;
}

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
		messaging::sendMessage(MSG_TYPE_ERROR, exp_msg);
		LOG(exp_msg.c_str());
	}

	//the loop for sending/receiving messages
    while(true)
	{
		try
		{
			string raw_message = messaging::get_message();
			Json msg = utils::parseJSON(raw_message);
			processMessage(msg);
		}
		catch(string exp_msg)
		{
			messaging::sendMessage(MSG_TYPE_ERROR, exp_msg);
			LOG(exp_msg.c_str());
		}
    }
}

