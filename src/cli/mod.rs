mod clean;
mod create_project;
mod enums;
mod install;
mod login;
mod uninstall;

use std::path::PathBuf;

use anyhow::anyhow;
use clap::Parser;
pub use create_project::*;
pub use enums::*;
use path_clean::PathClean;

use crate::{logo, plugin::create_instance};

#[derive(Debug, Clone, clap::Parser)]
#[clap(author, version, about, long_about = None)]
pub enum Cli {
    /// Create a new project
    New {
        /// The name of the project
        name: String,
        /// The type of the project
        #[arg(short, long, default_value = "exe")]
        template: TemplateType,
        /// Location for creating the project
        #[arg(short, long, default_value = ".")]
        path: PathBuf,
    },
    /// Initialize an existing directory with a project template
    Init {
        /// The type of the project
        #[arg(short, long, default_value = "exe")]
        template: TemplateType,
        /// Location for creating the project
        #[arg(short, long, default_value = ".")]
        path: PathBuf,
    },
    /// Build the project
    Build,
    /// Run the project with specified parameters
    Run,
    /// Clean the project's build directory
    Clean {
        /// If set, clean all the project's build directories(including build output)
        #[arg(long, default_value_t = false)]
        all: bool,
        /// Clean the project's build directory
        #[arg(short, long, default_value = ".")]
        path: PathBuf,
    },
    /// Install dependencies
    Install {
        /// The name of the dependency to install
        name: String,
    },
    /// Uninstall dependencies
    Uninstall {
        /// The name of the dependency to uninstall
        name: String,
    },
    /// Package the project
    Pack,
    /// Publish the project to the internet
    Publish,
    /// List information of the specified type
    List,
    /// Show the cup logo.
    Logo,
    /// Use Github to access token login.
    Login,
}

impl Cli {
    /// 构造一个 Cli 对象，并重整化路径
    ///
    /// # Note
    /// 此函数内部已进行命令行参数解析，返回值必定是有效数据
    pub fn new() -> Self {
        let mut cli = Cli::parse();

        // 重整化路径，使其成为绝对路径
        let base_path = std::env::current_dir().unwrap();
        // 路径重整化宏
        macro_rules! to_abs_path {
            ($($t: tt)*) => {
                if let $($t)* { ref mut path, .. } = cli {
                    if path.is_relative() {
                        *path = base_path.join(&path).clean();
                    }
                }
            };
        }
        // 重整化路径
        to_abs_path!(Self::New);
        to_abs_path!(Self::Init);

        cli
    }

    /// 分派不同子命令的任务
    pub async fn dispatch(self) -> anyhow::Result<()> {
        match self {
            // 显示 LOGO
            Cli::Logo => logo(),
            // 创建项目
            Cli::New {
                name,
                template,
                path,
            } => {
                let plugin = create_instance(template);

                let project_path = path.join(&name);
                if project_path.exists() {
                    return Err(anyhow!(
                        "Directory or file {} already exists!",
                        project_path.clean().display()
                    ));
                }

                fs_err::create_dir_all(&project_path)?;
                let result = plugin.create_project(&name, &project_path);
                // 状态回退
                if let Err(e) = result {
                    fs_err::remove_dir_all(&project_path)?;
                    return Err(e);
                }
            }
            // 初始化项目
            Cli::Init { template, path } => {
                let plugin = create_instance(template);

                if !path.exists() {
                    return Err(anyhow!(
                        "Directory {} does not exist!",
                        path.clean().display()
                    ));
                }
                if !path.is_dir() {
                    return Err(anyhow!("{} is not a directory!", path.clean().display()));
                }
                if fs_err::read_dir(&path)?.next().is_some() {
                    return Err(anyhow!(
                        "Directory {} is not empty!",
                        path.clean().display()
                    ));
                }

                let name = path.file_name().unwrap().to_str().unwrap();
                let result = plugin.create_project(name, &path);
                // 状态回退
                if let Err(e) = result {
                    fs_err::remove_dir_all(&path)?;
                    fs_err::create_dir(path)?;
                    return Err(e);
                }
            }
            Cli::Install { name } => {
                install::install(&name).await?;
            }
            Cli::Uninstall { name } => {
                uninstall::uninstall(&name)?;
            }
            Cli::Login => {
                login::login()?;
            }
            Cli::Clean { all, path } => {
                clean::clean(path, all)?;
            }
            _ => todo!(),
        };
        Ok(())
    }
}
