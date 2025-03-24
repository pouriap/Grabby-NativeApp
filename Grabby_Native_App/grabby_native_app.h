#pragma once

#include "jsonla.h"
#include "output_callback.h"
#include "ytdl_args.h"
#include "types.h"

using namespace std;
using namespace ggicci;

string setupTmpDir();
void setupStdin();
void processMessage(const Json &msg);
void handle_getversion(const Json &msg);
void handle_getavail(const Json &msg);
void handle_download(const Json &msg);
void handle_custom_cmd(const Json &msg);
void handle_ytdlinfo(const Json &msg);
void handle_ytdlget(const Json &msg);
void handle_ytdlkill(const Json &msg);
void flashgot_job(const string &jobJSON);
void custom_cmd_th(const string exeName, vector<string> args, const string filename, bool showConsole, bool showSaveas);
void ytdl_info_th(const string url, const string dlHash, ytdl_args *arger);
void ytdl_get_th(const string url, const string dlHash, ytdl_args *arger, const string filename, const int filetype);
process_result ytdl(const string &url, const string &dlHash, vector<string> &args, output_callback *callback = NULL);