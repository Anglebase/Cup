use std::path::PathBuf;

use anyhow::anyhow;

use crate::config::Shelling;

#[derive(Debug, Clone, serde::Serialize, serde::Deserialize)]
pub struct DependencyConfig {
    pub path: Option<String>,
    pub git: Option<String>,
    pub features: Option<Vec<String>>,
    #[serde(rename = "default-features")]
    pub defaut_features: Option<bool>,
    pub optional: Option<bool>,
}

impl Shelling<anyhow::Result<Dependency>> for DependencyConfig {
    fn shelling(self) -> anyhow::Result<Dependency> {
        let src = if let Some(path) = self.path {
            DependencySource::Local(PathBuf::from(path))
        } else if let Some(git) = self.git {
            let mut parts = git.split('/').into_iter().collect::<Vec<_>>();
            if parts.len() != 3 {
                return Err(anyhow!(
                    "Invalid git mark format, should be owner/repo/tag."
                ));
            }
            let owner = parts.remove(0);
            let repo = parts.remove(0);
            let tag = parts.remove(0);
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

#[derive(Debug, Clone)]
pub struct Dependency {
    pub src: DependencySource,
    pub features: Vec<String>,
    pub defaut_features: bool,
    pub optional: bool,
}
