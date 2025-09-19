mod build;
mod config;
mod dependency;
pub mod languages;
mod package;
mod program;

use std::{
    collections::HashMap,
    env::{current_dir, set_current_dir},
    path::PathBuf,
};

pub use build::{Build, BuildConfig};
pub use config::{ModeTable, ModeTableConfig, Table, TableConfig};
pub use dependency::{Dependency, DependencyConfig, DependencySource};
pub use languages::{Language, LanguageConfig};
pub use package::{Package, PackageConfig};
pub use program::Program;

use crate::{config::program::ProgramConfig, utils::TryAll};

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

#[derive(Debug, Clone, serde::Serialize, serde::Deserialize)]
pub struct RawConfig {
    pub package: PackageConfig,
    pub build: Option<BuildConfig>,
    pub dependencies: Option<HashMap<String, DependencyConfig>>,
    #[serde(rename = "dev-dependencies")]
    pub dev_dependencies: Option<HashMap<String, DependencyConfig>>,
    pub executables: Option<Vec<ProgramConfig>>,
    pub tests: Option<Vec<ProgramConfig>>,
    pub examples: Option<Vec<ProgramConfig>>,
    pub features: Option<HashMap<String, Vec<String>>>,
    pub target: Option<HashMap<String, ModeTableConfig>>,
    pub feature: Option<HashMap<String, ModeTableConfig>>,
    pub generator: Option<HashMap<String, ModeTableConfig>>,
}

impl Shelling<anyhow::Result<Config>> for RawConfig {
    fn shelling(self, base: &PathBuf) -> anyhow::Result<Config> {
        Ok(Config {
            package: self.package.shelling(base),
            build: self.build.unwrap_or_default().shelling(base)?,
            dependencies: self
                .dependencies
                .unwrap_or_default()
                .into_iter()
                .try_all(|(k, v)| -> anyhow::Result<_> { Ok((k, v.shelling(base)?)) })?
                .collect(),
            dev_dependencies: self
                .dev_dependencies
                .unwrap_or_default()
                .into_iter()
                .try_all(|(k, v)| -> anyhow::Result<_> { Ok((k, v.shelling(base)?)) })?
                .collect(),
            executables: self
                .executables
                .unwrap_or_default()
                .into_iter()
                .map(|v| v.shelling(base))
                .collect(),
            tests: self
                .tests
                .unwrap_or_default()
                .into_iter()
                .map(|v| v.shelling(base))
                .collect(),
            examples: self
                .examples
                .unwrap_or_default()
                .into_iter()
                .map(|v| v.shelling(base))
                .collect(),
            features: self.features.unwrap_or_default(),
            target: self
                .target
                .unwrap_or_default()
                .into_iter()
                .try_all(|(k, v)| -> anyhow::Result<_> { Ok((k, v.shelling(base)?)) })?
                .collect(),
            feature: self
                .feature
                .unwrap_or_default()
                .into_iter()
                .try_all(|(k, v)| -> anyhow::Result<_> { Ok((k, v.shelling(base)?)) })?
                .collect(),
            generator: self
                .generator
                .unwrap_or_default()
                .into_iter()
                .try_all(|(k, v)| -> anyhow::Result<_> { Ok((k, v.shelling(base)?)) })?
                .collect(),
        })
    }
}

#[derive(Debug, Clone)]
pub struct Config {
    pub package: Package,
    pub build: Build,
    pub dependencies: HashMap<String, Dependency>,
    pub dev_dependencies: HashMap<String, Dependency>,
    pub executables: Vec<Program>,
    pub tests: Vec<Program>,
    pub examples: Vec<Program>,
    pub features: HashMap<String, Vec<String>>,
    pub target: HashMap<String, ModeTable>,
    pub feature: HashMap<String, ModeTable>,
    pub generator: HashMap<String, ModeTable>,
}
