#pragma once

#include <iostream>
#include "cmd/cmd.h"
using namespace cmd;
int main()
{
    Command cmd("ls");
    cmd.exec();

    Command git("git");
    git.arg("--version");
    git.exec();
    return 0;
}