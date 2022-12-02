#pragma once

#include "jsonla.h"

using namespace std;
using namespace ggicci;

class ytdl_args
{
	protected:
		vector<string> args;
	public:
		ytdl_args(const Json &msg);
		~ytdl_args(void);
		void addArg(const string &arg);
		virtual vector<string> getArgs() = 0;
};

class ytdl_info: public ytdl_args
{
	public:
	ytdl_info(const Json &msg);
	vector<string> getArgs();
};

class ytdl_video: public ytdl_args
{
	private:
	string formatId;

	public:
	ytdl_video(const Json &msg);
	vector<string> getArgs();
};

class ytdl_audio: public ytdl_args
{
	public:
	ytdl_audio(const Json &msg);
	vector<string> getArgs();
};

class ytdl_playlist_video: public ytdl_args
{
	private:
	string indexesStr;
	string res;

	public:
	ytdl_playlist_video(const Json &msg);
	vector<string> getArgs();
};

class ytdl_playlist_audio: public ytdl_args
{
	private:
	string indexesStr;

	public:
	ytdl_playlist_audio(const Json &msg);
	vector<string> getArgs();
};