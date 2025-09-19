mod build;
mod config;
mod dependency;
pub mod languages;
mod package;

use std::{
    env::{current_dir, set_current_dir},
    path::PathBuf,
};

pub use build::{Build, BuildConfig};
pub use config::{Config, ConfigTable};
pub use dependency::{Dependency, DependencyConfig, DependencySource};
pub use languages::{Language, LanguageConfig};
pub use package::{Package, PackageConfig};

/// 此 trait 用于提供原始配置文件数据(XXConfig)转换到最小配置数据结构(XX)的能力
pub trait Shelling<T> {
    /// 剥壳函数，此函数实现校验逻辑
    /// `base` 为配置文件所在的绝对路径
    fn shelling(self, base: &PathBuf) -> T;
}

pub trait Expand<T> {
    fn expand(self, base: &PathBuf) -> T;
}

impl Expand<anyhow::Result<Vec<PathBuf>>> for Vec<String> {
    fn expand(self, base: &PathBuf) -> anyhow::Result<Vec<PathBuf>> {
        let cpath = current_dir()?;
        set_current_dir(base)?;

        let mut result = vec![];
        for pattern in self.iter() {
            for entry in glob::glob(pattern)? {
                let path = entry?;
                let path = if path.is_relative() {
                    base.join(path)
                } else {
                    path
                };
                result.push(path);
            }
        }

        set_current_dir(cpath)?;
        Ok(result)
    }
}
