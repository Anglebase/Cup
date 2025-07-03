#include "plugin/interface.h"

class StaticPlugin : public IPlugin
{
    static const std::unordered_map<std::string, std::string> templates;

public:
    Info getInfo() const override;
    std::string getName() const override;
    int run_new(const NewData &data) override;
    std::string gen_cmake(const CMakeContext& ctx) override;
};