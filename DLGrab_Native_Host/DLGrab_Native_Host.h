#pragma once

#include "jsonla.h"
#include <string>

using namespace std;
using namespace ggicci;

string setupTmpDir();
void setupStdin();
void processMessage(const Json &msg);
void handle_getavail(const Json &msg);
void handle_download(const Json &msg);
void handle_ytdlinfo(const Json &msg);
void handle_ytdlvid(const Json &msg);
void handle_ytdlaud(const Json &msg);
void flashGot(const string &jobText);
void ytdl_info_th(const string &pageUrl, const string &manifestUrl, const string &dlHash);
void ytdl_video_th(const string &url, const string &name, const string &dlHash);
void ytdl_audio_th(const string &url, const string &name, const string &dlHash);
string ytdl(const string &url, vector<string> args, void (*onOutput)(string output) = NULL);
void handleDlProgress(string output);