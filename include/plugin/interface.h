#pragma once

#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

class PluginInterface
{
public:
    virtual void execute(int argc, char **argv) = 0;
};

#endif // PLUGIN_INTERFACE_H

