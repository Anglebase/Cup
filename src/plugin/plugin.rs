use crate::{cli::CreateProject, debug, write_template};
use std::io::Write;

#[derive(Debug, Clone, Default)]
pub struct Module {}

impl CreateProject for Module {
    fn create_project(&self, name: &str, path: &std::path::PathBuf) -> anyhow::Result<()> {
        debug!("Creating module(plugin) project: {name:?} at {path:?}");

        // Cup.toml
        {
            let path = path.join("Cup.toml");
            let mut file = fs_err::File::create(path)?;
            write_template!(file, "templates/Cup.toml" => [
                NAME = name.to_string();
                TYPE = "module".to_string();
            ])?;
        }

        // .gitignore
        {
            let path = path.join(".gitignore");
            let mut file = fs_err::File::create(path)?;
            write_template!(file, "templates/gitignore")?;
        }

        fs_err::create_dir(path.join("src"))?;
        //  src/lib.cpp
        {
            let path = path.join("src/lib.cpp");
            let mut file = fs_err::File::create(path)?;
            write_template!(file, "templates/lib.cpp")?;
        }

        Ok(())
    }
}
