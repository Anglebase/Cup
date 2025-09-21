use cup_cli::cli::Cli;

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    let cli = Cli::new();
    cli.dispatch().await
}
