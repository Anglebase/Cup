use std::path::PathBuf;

use crate::config::Shelling;

#[derive(Debug, Clone, Default, serde::Serialize, serde::Deserialize)]
pub struct ProgramConfig {
    pub name: String,
    pub src: PathBuf,
}

impl Shelling<Program> for ProgramConfig {
    fn shelling(self, base: &PathBuf) -> Program {
        Program {
            name: self.name,
            src: if self.src.is_relative() {
                base.join(self.src)
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
