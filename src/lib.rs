pub mod cli;
pub mod config;
mod logo;
pub mod plugin;
pub mod utils;

use colored::Colorize;
pub use logo::logo;
use octocrab::Octocrab;

use crate::utils::CupData;

#[macro_export]
macro_rules! debug {
    ($( $arg:tt )*) => {
        if cfg!(debug_assertions) {
            println!($( $arg )*);
        }
    };
}

lazy_static::lazy_static! {
    pub static ref GITHUB: Octocrab = {
        // 尝试获取 Github 访问令牌
        let mut token = None;
        if let Ok(tk) = std::env::var("GITHUB_TOKEN") {
            token = Some(tk);
        }
        if let Ok(tk) = fs_err::read_to_string(CupData::token()) {
            token = Some(tk.trim().to_string());
        }
        // 创建客户端实例
        let result = if let Some(tk) = token {
            Octocrab::builder().personal_token(tk).build()
        }else {
            println!("{}: Github access token not found.","Warning".yellow().bold());
            Octocrab::builder().build()
        };
        // 返回结果
        if let Err(e) = result {
            println!("{}: {}", "Error".red().bold(), e);
            std::process::exit(1);
        }
        result.unwrap()
    };
}
