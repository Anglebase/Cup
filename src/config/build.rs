use std::path::PathBuf;

use crate::config::{Language, LanguageConfig, Shelling};

#[derive(Debug, Clone, Default, serde::Serialize, serde::Deserialize)]
pub struct BuildConfig {
    pub jobs: Option<usize>,
    pub languages: Option<LanguageConfig>,
}

impl Shelling<Build> for BuildConfig {
    fn shelling(self, base: &PathBuf) -> Build {
        Build {
            jobs: self
                .jobs
                .map(|th| {
                    if th == 0 {
                        let cpus = std::thread::available_parallelism();
                        if let Ok(cpus) = cpus { cpus.get() } else { 1 }
                    } else {
                        th
                    }
                })
                .unwrap_or(1),
            languages: self.languages.unwrap_or_default().shelling(base),
        }
    }
}

#[derive(Debug, Clone)]
pub struct Build {
    pub jobs: usize,
    pub languages: Language,
}
