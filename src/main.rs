use clap::Parser;
use cup_cli::{cli::Cli, logo};

fn main() {
    let cli = Cli::parse();
    match cli {
        Cli::Logo => logo(),
        _ => todo!(),
    };
}
