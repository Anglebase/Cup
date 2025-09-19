use std::path::PathBuf;

use crate::config::{Expand, Shelling};

#[derive(Debug, Clone, Default, serde::Serialize, serde::Deserialize)]
pub struct ConfigTable {
    pub defines: Option<Vec<String>>,
    pub includes: Option<Vec<String>>,
    pub sources: Option<Vec<String>>,
    pub link_paths: Option<Vec<String>>,
    pub link_libraires: Option<Vec<String>>,
    pub link_options: Option<Vec<String>>,
    pub compile_options: Option<Vec<String>>,
    pub compile_features: Option<Vec<String>>,
}

impl Shelling<anyhow::Result<Config>> for ConfigTable {
    fn shelling(self, base: &PathBuf) -> anyhow::Result<Config> {
        Ok(Config {
            defines: self.defines.unwrap_or_default(),
            includes: self.includes.unwrap_or_default().expand(base)?,
            sources: self.sources.unwrap_or_default().expand(base)?,
            link_paths: self.link_paths.unwrap_or_default().expand(base)?,
            link_libraires: self.link_libraires.unwrap_or_default(),
            link_options: self.link_options.unwrap_or_default(),
            compile_options: self.compile_options.unwrap_or_default(),
            compile_features: self.compile_features.unwrap_or_default(),
        })
    }
}

#[derive(Debug, Clone)]
pub struct Config {
    pub defines: Vec<String>,
    pub includes: Vec<PathBuf>,
    pub sources: Vec<PathBuf>,
    pub link_paths: Vec<PathBuf>,
    pub link_libraires: Vec<String>,
    pub link_options: Vec<String>,
    pub compile_options: Vec<String>,
    pub compile_features: Vec<String>,
}
