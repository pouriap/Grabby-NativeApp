#pragma once

#include <string>

class killswitches
{

public:
	killswitches(void);
	~killswitches(void);
	static void add(std::string dlHash);
	static void remove(std::string dlHash);
	static void activate(std::string dlHash);
	static bool isActive(std::string dlHash);
};
