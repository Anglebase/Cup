#include "plugin/interface.h"

class SharedPlugin : public IPlugin
{
    void _get_source_files(const fs::path &dir, std::vector<fs::path> &src_files) const;
    std::vector<fs::path> get_source_files(const fs::path &root) const;
    std::vector<fs::path> get_test_mains(const fs::path &root) const;
    std::vector<fs::path> get_example_mains(const fs::path &root) const;
public:
    std::string getName(std::optional<std::string> &except) const override;
    int run_new(const NewData &data, std::optional<std::string> &except) override;
    std::string gen_cmake(const CMakeContext &ctx, bool is_dependency, std::optional<std::string> &except) override;
    fs::path run_project(const RunProjectData &data, std::optional<std::string> &except) override;
    std::optional<std::string> get_target(const RunProjectData &data, std::optional<std::string> &except) const override;
    int show_help(const cmd::Args &command, std::optional<std::string> &except) const override;
};