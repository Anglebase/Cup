use std::io::Write;

use crate::{cli::CreateProject, debug, write_template};

#[derive(Debug, Clone, Default)]
pub struct Executable {}

impl Executable {
    pub fn new() -> Self {
        Self {}
    }
}

impl CreateProject for Executable {
    fn create_project(&self, name: &str, path: &std::path::PathBuf) -> anyhow::Result<()> {
        debug!("Creating project {name:?} at {path:?}");

        // Cup.toml
        {
            let path = path.join("Cup.toml");
            let mut file = fs_err::File::create(&path)?;
            write_template!(file, "templates/Cup.toml" => [
                NAME = name.to_string();
                TYPE = "executable".to_string();
            ])?;
        }

        // .gitignore
        {
            let path = path.join(".gitignore");
            let mut file = fs_err::File::create(&path)?;
            write_template!(file, "templates/gitignore")?;
        }

        fs_err::create_dir(path.join("src"))?;
        // src/main.cpp
        {
            let path = path.join("src/main.cpp");
            let mut file = fs_err::File::create(&path)?;
            write_template!(file, "templates/main.cpp")?;
        }

        Ok(())
    }
}
