#include "stdafx.h"

#define MSG_TYPE_ERROR 1

#include <iostream>
#include <string>
#include "jsonla.h"
#include "utils.h"
#include <fstream>

#include <fcntl.h>
#include <io.h>

using namespace std;
using namespace ggicci;

// Read a message from stdin and decode it.
string get_message() 
{
	unsigned int message_length = 0;
	
	try
	{
		//fread is blocking
		//it blocks until 4 bytes is read
		int bytes_read = fread(&message_length, sizeof(char), 4, stdin);

		if(feof(stdin))
		{
			utils::log("EOF");
		}

		if(ferror(stdin))
		{
			utils::log("ERROR");
		}

		utils::log("message len: ");
		utils::log(message_length);
		utils::log("bytes read: ");
		utils::log(bytes_read);

		if(bytes_read != 4 || message_length <=0 || message_length > 2048)
		{
			utils::log("bad data");
			exit(EXIT_SUCCESS);
		}
	}
	catch(exception &e){
		string msg = "bad length data\n";
		msg.append(e.what());
		throw msg;
	}

	try
	{
		char* message = new char[message_length];
		int bytes_read = fread(message, sizeof(char), message_length, stdin);

		if(bytes_read != message_length)
		{
			utils::log("read bytes: ");
			utils::log(bytes_read);
		}

		string m(message, 0, message_length);
		delete message;

		utils::log(m.c_str());

		return m;
	}
	catch(exception &e)
	{
		string msg = "error reading raw message\n";
		msg.append(e.what());
		throw msg;
	}

}

Json parseJSON(string JSONstr)
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

void sendMessage(const Json &msg)
{
	string JSONstr = msg.ToString();
	const unsigned int message_length = JSONstr.length();
	fwrite(&message_length, sizeof(char), 4, stdout);
	fwrite(JSONstr.c_str(), sizeof(char), message_length, stdout);
	fflush(stdout);
}

void sendMessage(int type, string content)
{
	Json json = Json::Parse("{}");
	json.AddProperty("type", Json(type));
	json.AddProperty("content", Json(content));
	sendMessage(json);
}

//using the reference of the Json object because pass by value caused exception because of copy constructor error
void processMessage(const Json &msg)
{
	try
	{
		int type = msg["type"].AsInt();
	}
	catch(exception &e)
	{
		throw string("unexpected JSON content in message");
	}
}

int main(int argc, char *argv[])
{
	/* Set "stdin" to have binary mode: */
	int result = _setmode( _fileno( stdin ), _O_BINARY );
	if( result == -1 ){
		utils::log("cannot set stdin mode to binary");
		exit(EXIT_FAILURE);
	}

    while(true)
	{
		try
		{
			string raw_message = get_message();
			Json msg = parseJSON(raw_message);
			processMessage(msg);
		}
		catch(string exp_msg)
		{
			sendMessage(MSG_TYPE_ERROR, exp_msg);
			utils::log(exp_msg.c_str());
		}
    }
}

