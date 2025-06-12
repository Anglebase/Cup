#include "configinfo.h"
#include "config.h"

ConfigInfo::ConfigInfo(const Config &config)
{
    *this = Deserde<ConfigInfo>::from(config.table_);
}