use std::path::{Path, PathBuf};

use crate::{cli::TemplateType, config::Shelling};

#[derive(Debug, Clone, serde::Serialize, serde::Deserialize)]
pub struct PackageConfig {
    pub name: String,
    pub version: String,
    #[serde(rename = "type")]
    pub ty: TemplateType,
    pub workspace: Option<PathBuf>,
    pub publish: Option<bool>,
}

impl<P: AsRef<Path>> Shelling<Package, P> for PackageConfig {
    fn shelling(self, base: P) -> Package {
        Package {
            name: self.name,
            version: self.version,
            ty: self.ty,
            workspace: self.workspace.unwrap_or(PathBuf::from(base.as_ref())),
            publish: self.publish.unwrap_or(true),
        }
    }
}

#[derive(Debug, Clone)]
pub struct Package {
    pub name: String,
    pub version: String,
    pub ty: TemplateType,
    pub workspace: PathBuf,
    pub publish: bool,
}
