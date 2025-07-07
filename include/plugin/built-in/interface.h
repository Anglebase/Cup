#include "plugin/interface.h"

class InterfacePlugin : public IPlugin
{
    std::vector<fs::path> get_all_tests_main_files(const fs::path &root);
    std::vector<fs::path> get_examples_main_files(const fs::path &root);
public:
    std::string getName(std::optional<std::string> &except) const override;
    int run_new(const NewData &data, std::optional<std::string> &except) override;
    std::string gen_cmake(const CMakeContext &ctx, bool is_dependency, std::optional<std::string> &except) override;
    fs::path run_project(const RunProjectData &data, std::optional<std::string> &except) override;
    std::optional<std::string> get_target(const RunProjectData &data, std::optional<std::string> &except) const override;
    int show_help(const cmd::Args &command, std::optional<std::string> &except) const override;
};