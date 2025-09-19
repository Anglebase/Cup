use std::path::{Path, PathBuf};

use crate::config::Shelling;

#[derive(Debug, Clone, Default, serde::Serialize, serde::Deserialize)]
pub struct ProgramConfig {
    pub name: String,
    pub src: PathBuf,
}

impl<P: AsRef<Path>> Shelling<Program, P> for ProgramConfig {
    fn shelling(self, base: P) -> Program {
        Program {
            name: self.name,
            src: if self.src.is_relative() {
                base.as_ref().join(self.src)
            } else {
                self.src
            },
        }
    }
}

#[derive(Debug, Clone)]
pub struct Program {
    pub name: String,
    pub src: PathBuf,
}
