#[derive(Debug, Clone, clap::Parser)]
#[clap(author, version, about, long_about = None)]
pub enum Cli {
    /// Create a new project
    New,
    /// Initialize an existing directory with a project template
    Init,
    /// Build the project
    Build,
    /// Run the project with specified parameters
    Run,
    /// Clean the project's build directory
    Clean,
    /// Install dependencies
    Install,
    /// Uninstall dependencies
    Uninstall,
    /// Package the project
    Pack,
    /// Publish the project to the internet
    Publish,
    /// List information of the specified type
    List,
    /// Show the cup logo.
    Logo,
}
