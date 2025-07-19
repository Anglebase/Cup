#include "plugin/interface.h"

class StaticPlugin : public IPlugin
{
    void _get_source_files(const fs::path &dir, std::vector<fs::path> &src_files) const;
    std::vector<fs::path> get_source_files(const fs::path &root) const;
    std::vector<fs::path> get_test_mains(const fs::path &root) const;
    std::vector<fs::path> get_example_mains(const fs::path &root) const;
public:
    Result<std::string, std::string> getName() const override;
    Result<int, std::string> run_new(const NewData &data) override;
    Result<std::string, std::string> gen_cmake(const CMakeContext &ctx, bool is_dependency) override;
    Result<fs::path, std::string> run_project(const RunProjectData &data) override;
    Result<std::optional<std::string>, std::string> get_target(const RunProjectData &data) const override;
    Result<int, std::string> show_help(const cmd::Args &command) const override;
};