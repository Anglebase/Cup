use clap::Parser;
use cup_cli::logo;

#[derive(Debug, Clone, clap::Parser)]
#[clap(author, version, about, long_about = None)]
pub struct Cli {}

fn main() {
    let _ = Cli::parse();
    logo();
}
