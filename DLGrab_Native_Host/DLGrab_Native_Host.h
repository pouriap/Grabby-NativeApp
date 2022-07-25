#pragma once

#include "jsonla.h"
#include <string>

using namespace std;
using namespace ggicci;

string setupTmpDir();
void setupStdin();
void processMessage(const Json &msg);
void handleType1(const Json &msg);
void handleType2(const Json &msg);
void handleType3(const Json &msg);
void handleType4(const Json &msg);
void handleType5(const Json &msg);
void handleType6(const Json &msg);
void flashGot(const string &jobText);
void ytdl_info(const string &pageUrl, const string &manifestUrl, const string &dlHash);
void ytdl_video(const string &url, const string &name, const string &location, const string &dlHash);
void ytdl_audio(const string &url, const string &name, const string &location, const string &dlHash);
string ytdl(const string &url, vector<string> args, void (*onOutput)(std::string output) = NULL);
void handleDlProgress(string output);