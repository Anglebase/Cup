mod create_project;
mod enums;

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

impl Cli {
    /// 构造一个 Cli 对象，并重整化路径
    ///
    /// # Note
    /// 此函数内部已进行命令行参数解析，返回值必定是有效数据
    pub fn new() -> Self {
        let mut cli = Cli::parse();

        // 重整化路径，使其成为绝对路径
        let base_path = std::env::current_dir().unwrap();
        if let Self::New { ref mut path, .. } = cli {
            if path.is_relative() {
                *path = base_path.join(&path).clean();
            }
        }

        cli
    }

    /// 分派不同子命令的任务
    pub fn dispatch(self) -> anyhow::Result<()> {
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
                        "Directory or file {:?} already exists!",
                        project_path.clean()
                    ));
                }
                fs_err::create_dir_all(&project_path)?;
                plugin.create_project(&name, &project_path)?;
            }
            _ => todo!(),
        };
        Ok(())
    }
}
