#include "stdafx.h"
#include "messaging.h"
#include <string>
#include "utils.h"

#define LOG(x) utils::log(x)

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
			LOG("EOF");
		}

		if(ferror(stdin))
		{
			LOG("ERROR");
		}

		LOG("message len: ");
		LOG(message_length);
		LOG("bytes read: ");
		LOG(bytes_read);

		if(bytes_read != 4 || message_length <=0 || message_length > 2048)
		{
			LOG("bad data");
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
			LOG("read bytes: ");
			LOG(bytes_read);
		}

		string m(message, 0, message_length);
		delete message;

		LOG(m.c_str());

		return m;
	}
	catch(exception &e)
	{
		string msg = "error reading raw message\n";
		msg.append(e.what());
		throw msg;
	}

}

void messaging::sendMessage(int type, const string &content)
{
	Json json = Json::Parse("{}");
	json.AddProperty("type", Json(type));
	json.AddProperty("content", Json(content));
	sendMessage(json);
}

void messaging::sendMessage(const ggicci::Json &msg)
{
	sendMessageRaw(msg.ToString());
}

void messaging::sendMessageRaw(const string &content)
{
	const unsigned int message_length = content.length();
	fwrite(&message_length, sizeof(char), 4, stdout);
	fwrite(content.c_str(), sizeof(char), message_length, stdout);
	fflush(stdout);
}