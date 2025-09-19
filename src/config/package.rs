use std::path::Path;

use crate::{cli::TemplateType, config::Shelling};

#[derive(Debug, Clone, serde::Serialize, serde::Deserialize)]
pub struct PackageConfig {
    pub name: String,
    pub version: String,
    #[serde(rename = "type")]
    pub ty: TemplateType,
}

impl<P: AsRef<Path>> Shelling<Package, P> for PackageConfig {
    fn shelling(self, _base: P) -> Package {
        Package {
            name: self.name,
            version: self.version,
            ty: self.ty,
        }
    }
}

#[derive(Debug, Clone)]
pub struct Package {
    pub name: String,
    pub version: String,
    pub ty: TemplateType,
}
