#include "plugin/interface.h"

class ModulePlugin : public IPlugin
{
    static const std::unordered_map<std::string, std::string> templates;

public:
    std::string getName() const override;
    int run_new(const NewData &data) override;
    std::string gen_cmake(const CMakeContext& ctx, bool is_dependency) override;
    fs::path run_project(const RunProjectData &data) override;
    std::optional<std::string> get_target(const RunProjectData &data) const override;
};