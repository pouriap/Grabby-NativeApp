#include "stdafx.h"
#include "output_callback.h"
#include "messaging.h"
#include "utils.h"
#include "defines.h"
#include "jsonla.h"

using namespace ggicci;

output_callback::output_callback(const string &hash) : dlHash(hash)
{
}

output_callback::~output_callback(void)
{
}

void output_callback::call(const string &output)
{
	//get last line of output, cause output can be multiple lines
	vector<string> lines = utils::strSplit(output, '\n');
	string line = lines.back();

	if(line.find('|') == string::npos || line.find('%') == string::npos)
	{
		return;
	}

	vector<string> parts = utils::strSplit(line, '|');

	if(parts.size() == 0)
	{
		return;
	}

	string percent_str = utils::trim(parts[0]);
	string speed_str = utils::trim(parts[1]);
	string plIndex_str = utils::trim(parts[2]);

	percent_str = percent_str.substr(0, percent_str.find_last_of('%'));

	Json msg = Json::Parse("{}");
	msg.AddProperty("type", Json(MSGTYP_YTDLPROG));
	msg.AddProperty("dlHash", Json(dlHash));
	msg.AddProperty("percent_str", Json(percent_str));
	msg.AddProperty("speed_str", Json(speed_str));
	msg.AddProperty("playlist_index", Json(plIndex_str));

	//always send the 100% message
	if(percent_str == "100")
	{
		messaging::sendMessage(msg);
	}
	else
	{
		messaging::sendMessageLimit(msg, 1);
	}
}
