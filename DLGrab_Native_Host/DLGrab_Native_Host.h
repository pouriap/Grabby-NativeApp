#pragma once

#include "jsonla.h"
#include <string>

using namespace std;
using namespace ggicci;

void handleType1(const Json &msg);
void handleType2(const Json &msg);
void handleType3(const Json &msg);
void flashGot(const string &jobText);
void processMessage(const Json &msg);
void setupStdin();
string setupTmpDir();