#include "stdafx.h"
#include "messaging.h"
#include <string>
#include "utils.h"

using namespace std;
using namespace ggicci;

messaging::messaging(void)
{
}


messaging::~messaging(void)
{
}

// Read a message from stdin and decode it.
string messaging::get_message() 
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

void messaging::sendMessage(const Json &msg)
{
	string JSONstr = msg.ToString();
	const unsigned int message_length = JSONstr.length();
	fwrite(&message_length, sizeof(char), 4, stdout);
	fwrite(JSONstr.c_str(), sizeof(char), message_length, stdout);
	fflush(stdout);
}

void messaging::sendMessage(int type, string content)
{
	Json json = Json::Parse("{}");
	json.AddProperty("type", Json(type));
	json.AddProperty("content", Json(content));
	sendMessage(json);
}