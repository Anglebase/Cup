use std::io::{self, Write};

use crate::utils::CupData;

pub fn login() -> anyhow::Result<()> {
    print!("Github accesss token: ");
    io::stdout().flush()?;
    let mut token = String::new();
    std::io::stdin().read_line(&mut token)?;
    let mut file = fs_err::File::create(CupData::token())?;
    file.write_fmt(format_args!("{}", token.trim()))?;
    println!("Token saved to {}", CupData::token().display());
    Ok(())
}
