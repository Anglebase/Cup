#include "cup_plugin/interface.h"

class ModulePlugin : public IPlugin
{
    void _get_all_source_files(const fs::path &root, std::vector<fs::path> &src_files);
    std::vector<fs::path> get_all_source_files(const fs::path &root);
    std::vector<fs::path> get_test_main_files(const fs::path &root);
public:
    Result<std::string, std::string> getName() const override;
    Result<int, std::string> run_new(const NewData &data) override;
    Result<std::string, std::string> gen_cmake(const CMakeContext &ctx, bool is_dependency) override;
    Result<fs::path, std::string> run_project(const RunProjectData &data) override;
    Result<std::optional<std::string>, std::string> get_target(const RunProjectData &data) const override;
    Result<int, std::string> show_help(const cmd::Args &command) const override;
};