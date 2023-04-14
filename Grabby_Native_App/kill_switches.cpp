#include "stdafx.h"
#include <mutex>
#include <map>
#include "kill_switches.h"

using namespace std;

std::mutex ksMutex;
map<string, bool> switches;

killswitches::killswitches(void)
{
}

killswitches::~killswitches(void)
{
}

void killswitches::add(string dlHash)
{
	std::lock_guard<std::mutex> lock(ksMutex);
	switches.insert(pair<string, bool>(dlHash, false));
}

void killswitches::remove(string dlHash)
{
	std::lock_guard<std::mutex> lock(ksMutex);
	switches.erase(dlHash);
}

void killswitches::activate(string dlHash)
{
	std::lock_guard<std::mutex> lock(ksMutex);
	switches[dlHash] = true;
}

bool killswitches::isActive(string dlHash)
{
	std::lock_guard<std::mutex> lock(ksMutex);
	if(switches.count(dlHash) == 0) return false;
	return switches[dlHash];
}
