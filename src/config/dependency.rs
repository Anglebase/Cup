use std::path::{Path, PathBuf};

use anyhow::anyhow;
use path_clean::PathClean;

use crate::{
    config::Shelling,
    utils::{CupData, GitUnique},
};

#[derive(Debug, Clone, serde::Serialize, serde::Deserialize)]
pub struct DependencyConfig {
    pub path: Option<String>,
    pub git: Option<String>,
    pub features: Option<Vec<String>>,
    #[serde(rename = "default-features")]
    pub defaut_features: Option<bool>,
    pub optional: Option<bool>,
}

impl<P: AsRef<Path>> Shelling<anyhow::Result<Dependency>, P> for DependencyConfig {
    fn shelling(self, base: P) -> anyhow::Result<Dependency> {
        let src = if let Some(path) = self.path {
            let path = PathBuf::from(path);
            let path = if path.is_absolute() {
                path
            } else {
                base.as_ref().join(path)
            };
            DependencySource::Local(path.clean())
        } else if let Some(git) = self.git {
            let GitUnique { owner, repo, tag } = GitUnique::from(&git)?;
            DependencySource::Git {
                owner: owner.to_string(),
                repo: repo.to_string(),
                tag: tag.to_string(),
            }
        } else {
            return Err(anyhow!(
                "Invalid dependency config, must specify either git or path."
            ));
        };
        Ok(Dependency {
            src,
            features: self.features.unwrap_or_default(),
            defaut_features: self.defaut_features.unwrap_or(true),
            optional: self.optional.unwrap_or(false),
        })
    }
}

#[derive(Debug, Clone)]
pub enum DependencySource {
    Local(PathBuf),
    Git {
        owner: String,
        repo: String,
        tag: String,
    },
}

impl DependencySource {
    /// 获取依赖项所对应的本地路径
    pub fn local_path(&self) -> PathBuf {
        match self {
            Self::Local(path) => path.clone(),
            Self::Git { owner, repo, tag } => CupData::git(owner, repo, tag),
        }
    }
}

#[derive(Debug, Clone)]
pub struct Dependency {
    pub src: DependencySource,
    pub features: Vec<String>,
    pub defaut_features: bool,
    pub optional: bool,
}
