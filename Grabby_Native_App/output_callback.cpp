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
	vector<string> parts = utils::strSplit(lines.back(), '|');

	if(parts.size() == 0)
	{
		return;
	}

	if(parts[0].find('%') != string::npos)
	{
		Json msg = Json::Parse("{}");
		msg.AddProperty("type", Json(MSGTYP_YTDLPROG));
		msg.AddProperty("dlHash", Json(dlHash));
		msg.AddProperty("percent_str", Json(parts[0]));
		msg.AddProperty("speed_str", Json(parts[1]));
		messaging::sendMessageLimit(msg, 1);
	}
}
