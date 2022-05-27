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
void flashGot(const string &jobText);
void ytdl_info(const string &url);
void ytdl_dl_video(const string &url, const string &formatID);
void ytdl_dl_audio(const string &url);
void ytdl(const string &url, const char* type, vector<string> args);