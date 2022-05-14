#pragma once

#include <string>
#include "jsonla.h"

class messaging
{
public:
	messaging(void);
	~messaging(void);
	static std::string get_message();
	static void sendMessage(const ggicci::Json &msg);
	static void sendMessage(int type, const std::string &content);
};

