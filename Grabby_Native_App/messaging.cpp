#include "stdafx.h"
#include "messaging.h"
#include <string>
#include <mutex>
#include "utils.h"
#include "exceptions.h"
#include "defines.h"

using namespace std;
using namespace ggicci;


std::mutex theMutex;
std::time_t lastSentTime = 0;


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

		if(bytes_read != 4 || message_length <=0)
		{
			throw grb_exception("bad message length");
		}
	}
	catch(exception &e)
	{
		string msg = "Error reading message length: ";
		msg.append(e.what());
		throw fatal_exception(msg.c_str());
	}

	try
	{
		char* message = new char[message_length];
		int bytes_read = fread(message, sizeof(char), message_length, stdin);

		string m(message, 0, message_length);
		delete message;

		if(bytes_read != message_length)
		{
			throw grb_exception("Read bytes doesn't match message length");
		}

		return m;
	}
	catch(exception &e)
	{
		string msg = "Error reading raw message: ";
		msg.append(e.what());
		throw grb_exception(msg.c_str());
	}

}

void messaging::sendMessage(const string &type, const string &content)
{
	Json json = Json::Parse("{}");
	json.AddProperty("type", Json(type));
	json.AddProperty("content", Json(content));
	sendMessage(json);
}

void messaging::sendMessageLimit(const ggicci::Json &msg, int interval)
{
	std::time_t t = std::time(nullptr) - lastSentTime;
	if(t >= interval)
	{
		sendMessage(msg);
	}
}

void messaging::sendMessage(const ggicci::Json &msg)
{
	sendMessageRaw(msg.ToString());
}

void messaging::sendMessageRaw(string content)
{
	std::lock_guard<std::mutex> lock(theMutex);

	lastSentTime = std::time(nullptr);

	utils::strReplaceAll(content, "\r", "\\r");
	utils::strReplaceAll(content, "\n", "\\n");

	PLOG_INFO << "sending: " << content;

	try
	{
		const unsigned int message_length = content.length();
		fwrite(&message_length, sizeof(char), 4, stdout);
		fwrite(content.c_str(), sizeof(char), message_length, stdout);
		fflush(stdout);
	}
	catch(exception &e)
	{
		string msg = "Failed to send message: ";
		msg.append(e.what());
		throw grb_exception(msg.c_str());
	}

}