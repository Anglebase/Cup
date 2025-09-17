use cup_cli::cli::Cli;

fn main() -> anyhow::Result<()> {
    let cli = Cli::new();
    cli.dispatch()
}
