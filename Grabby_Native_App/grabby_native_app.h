#pragma once

#include "jsonla.h"
#include "output_callback.h"
#include <string>

using namespace std;
using namespace ggicci;

string setupTmpDir();
void setupStdin();
void processMessage(const Json &msg);
void handle_getavail(const Json &msg);
void handle_download(const Json &msg);
void handle_ytdlinfo(const Json &msg);
void handle_ytdlget(const Json &msg);
void flashGot(const string &jobText);
void ytdl_info_th(const string &url, const string &dlHash);
void ytdl_get_th(const string &url, const string &fileName, const string &dlHash, const Json &msg);
string ytdl(const string &url, vector<string> args, output_callback *callback = NULL);