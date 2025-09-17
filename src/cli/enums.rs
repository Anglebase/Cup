#[derive(Debug, Clone, Copy, PartialEq, Eq, clap::ValueEnum)]
pub enum TemplateType {
    #[clap(alias = "exe")]
    Executable,
    #[clap(alias = "lib")]
    Static,
    #[clap(alias = "dll")]
    Shared,
    #[clap(alias = "mod")]
    Module,
    #[clap(alias = "header")]
    HeaderOnly,
}
