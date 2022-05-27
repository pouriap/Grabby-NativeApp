#pragma once

#include "jsonla.h"
#include <string>

using namespace std;
using namespace ggicci;

void handleType1(const Json &msg);
void handleType2(const Json &msg);
void handleType3(const Json &msg);
void handleType4(const Json &msg);
void flashGot(const string &jobText);
void ytdl(const string url);
void processMessage(const Json &msg);
void setupStdin();
string setupTmpDir();