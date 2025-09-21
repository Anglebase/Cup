mod build;
mod config;
mod dependency;
pub mod languages;
mod package;
mod program;
pub mod utils;

use std::{
    collections::HashMap,
    env::{current_dir, set_current_dir},
    path::{Path, PathBuf},
};

pub use build::{Build, BuildConfig};
pub use config::{ModeTable, ModeTableConfig, Table, TableConfig};
pub use dependency::{Dependency, DependencyConfig, DependencySource};
pub use languages::{Language, LanguageConfig};
pub use package::{Package, PackageConfig};
pub use program::Program;

use crate::{config::program::ProgramConfig, utils::TryAll};

/// 此 trait 用于提供原始配置文件数据(XXConfig)转换到最小配置数据结构(XX)的能力
pub trait Shelling<T, P: AsRef<Path>> {
    /// 剥壳函数，此函数实现校验逻辑
    /// `base` 为配置文件所在的绝对路径
    fn shelling(self, base: P) -> T;
}

/// 此 trait 用于提取缩减表示的原始展开，例如路径通配符的展开
pub trait Expand<T, P: AsRef<Path>> {
    fn expand(self, base: P) -> T;
}

impl<P: AsRef<Path>> Expand<anyhow::Result<Vec<PathBuf>>, P> for Vec<String> {
    fn expand(self, base: P) -> anyhow::Result<Vec<PathBuf>> {
        let cpath = current_dir()?;
        set_current_dir(&base)?;

        // 展开路径通配符
        let mut result = vec![];
        for pattern in self.iter() {
            for entry in glob::glob(pattern)? {
                let path = entry?;
                let path = if path.is_relative() {
                    base.as_ref().join(path)
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

impl<P: AsRef<Path>> Shelling<anyhow::Result<Config>, P> for RawConfig {
    fn shelling(self, base: P) -> anyhow::Result<Config> {
        Ok(Config {
            package: self.package.shelling(&base),
            build: self.build.unwrap_or_default().shelling(&base)?,
            dependencies: self
                .dependencies
                .unwrap_or_default()
                .into_iter()
                .try_all(|(k, v)| -> anyhow::Result<_> { Ok((k, v.shelling(&base)?)) })?
                .collect(),
            dev_dependencies: self
                .dev_dependencies
                .unwrap_or_default()
                .into_iter()
                .try_all(|(k, v)| -> anyhow::Result<_> { Ok((k, v.shelling(&base)?)) })?
                .collect(),
            executables: self
                .executables
                .unwrap_or_default()
                .into_iter()
                .map(|v| v.shelling(&base))
                .collect(),
            tests: self
                .tests
                .unwrap_or_default()
                .into_iter()
                .map(|v| v.shelling(&base))
                .collect(),
            examples: self
                .examples
                .unwrap_or_default()
                .into_iter()
                .map(|v| v.shelling(&base))
                .collect(),
            features: self.features.unwrap_or_default(),
            target: self
                .target
                .unwrap_or_default()
                .into_iter()
                .try_all(|(k, v)| -> anyhow::Result<_> { Ok((k, v.shelling(&base)?)) })?
                .collect(),
            feature: self
                .feature
                .unwrap_or_default()
                .into_iter()
                .try_all(|(k, v)| -> anyhow::Result<_> { Ok((k, v.shelling(&base)?)) })?
                .collect(),
            generator: self
                .generator
                .unwrap_or_default()
                .into_iter()
                .try_all(|(k, v)| -> anyhow::Result<_> { Ok((k, v.shelling(&base)?)) })?
                .collect(),
        })
    }
}

/// 原始项目配置
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

impl Config {
    /// 读取项目的配置信息
    pub fn read<P: AsRef<Path>>(project: P) -> anyhow::Result<Config> {
        let content = fs_err::read_to_string(project.as_ref().join("Cup.toml"))?;
        let raw_config: RawConfig = toml::from_str(&content)?;
        Ok(raw_config.shelling(project)?)
    }

    /// 此函数缺少路径展开分析，仅作为远程依赖项解析使用
    pub unsafe fn from_str(content: &str) -> anyhow::Result<Config> {
        let raw_config: RawConfig = toml::from_str(content)?;
        Ok(raw_config.shelling(PathBuf::from("."))?)
    }

    pub fn get_all_remote_dependencies(&self) -> Vec<&Dependency> {
        let mut result = vec![];
        for dep in self.dependencies.values() {
            if let DependencySource::Git { .. } = dep.src {
                result.push(dep);
            }
        }
        for dep in self.dev_dependencies.values() {
            if let DependencySource::Git { .. } = dep.src {
                result.push(dep);
            }
        }
        result
    }
}
