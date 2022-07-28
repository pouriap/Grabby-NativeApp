#pragma once

#include <string>
#include "jsonla.h"

using namespace std;

class messaging
{
public:
	messaging(void);
	~messaging(void);
	static string get_message();
	static void sendMessage(const string &type, const string &content);
	static void sendMessage(const ggicci::Json &msg);
	static void sendMessageRaw(string content);
};

