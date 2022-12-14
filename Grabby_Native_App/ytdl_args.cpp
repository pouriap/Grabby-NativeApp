#include "stdafx.h"
#include "ytdl_args.h"

ytdl_args::ytdl_args(const Json &msg)
{
	//these are things common to all ytdl commands

	string url = msg["url"].AsString();
	args.push_back(url);

	if(msg.Contains("proxy"))
	{
		string proxy = msg["proxy"].AsString();
		args.push_back("--proxy");
		args.push_back(proxy);
	}

	//args.push_back("--restrict-filenames"); //no need we have sanitize
	args.push_back("--no-warnings");
	args.push_back("--progress-template");
	args.push_back("%(progress._percent_str)s|%(progress._speed_str)s|%(info.playlist_index)s");
	args.push_back("--newline");
}

void ytdl_args::addArg(const string &arg)
{
	args.push_back(arg);
}

ytdl_args::~ytdl_args(void)
{
}



ytdl_info::ytdl_info(const Json &msg): ytdl_args(msg)
{
}

vector<string> ytdl_info::getArgs()
{
	args.push_back("-j");
	//if it's a playlist, flatten it (do not extract info for individual videos)
	args.push_back("--flat-playlist");

	return args;
}



ytdl_video::ytdl_video(const Json &msg): ytdl_args(msg)
{
	formatId = msg["formatId"].AsString();
}

vector<string> ytdl_video::getArgs()
{
	//we need +bestaudio for cases where the audio is separate(youtube)
	//if such format is not found then the normal format will be downloaded
	string formatFull = formatId + "+bestaudio/" + formatId;

	args.push_back("-f");
	args.push_back(formatFull);
	args.push_back("--merge-output-format");
	args.push_back("mkv");

	return args;
}



ytdl_audio::ytdl_audio(const Json &msg): ytdl_args(msg)
{
}

vector<string> ytdl_audio::getArgs()
{
	args.push_back("-f");
	args.push_back("bestaudio");
	args.push_back("--extract-audio");
	args.push_back("--audio-format");
	args.push_back("mp3");

	return args;
}



ytdl_playlist_video::ytdl_playlist_video(const Json &msg): ytdl_args(msg)
{
	indexesStr = msg["indexes"].AsString();
	res = msg["res"].AsString();
}

vector<string> ytdl_playlist_video::getArgs()
{
	args.push_back("--playlist-items");
	args.push_back(indexesStr);

	//-S "+res:480" -f "bestvideo[vcodec*=avc][container=mp4_dash]+bestaudio"
	args.push_back("-S");
	args.push_back("+res:" + res);
	args.push_back("-f");
	args.push_back("bestvideo[vcodec*=avc]+bestaudio");
	args.push_back("--merge-output-format");
	args.push_back("mkv");

	return args;
}



ytdl_playlist_audio::ytdl_playlist_audio(const Json &msg): ytdl_args(msg)
{
	indexesStr = msg["indexes"].AsString();
}

vector<string> ytdl_playlist_audio::getArgs()
{
	args.push_back("--playlist-items");
	args.push_back(indexesStr);

	args.push_back("-f");
	args.push_back("bestaudio");
	args.push_back("--extract-audio");
	args.push_back("--audio-format");
	args.push_back("mp3");

	return args;
}
