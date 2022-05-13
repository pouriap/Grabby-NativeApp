#include "stdafx.h"

#define MSG_TYPE_ERROR 1

#include <iostream>
#include <string>
#include <fstream>
#include <fcntl.h>
#include <io.h>
#include "utils.h"
#include "messaging.h"

using namespace std;
using namespace ggicci;

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
			string raw_message = messaging::get_message();
			Json msg = utils::parseJSON(raw_message);
			processMessage(msg);
		}
		catch(string exp_msg)
		{
			messaging::sendMessage(MSG_TYPE_ERROR, exp_msg);
			utils::log(exp_msg.c_str());
		}
    }
}

