use crate::{cli::CreateProject, debug, write_template};
use std::io::Write;

#[derive(Debug, Clone, Default)]
pub struct HeaderOnly {}

impl CreateProject for HeaderOnly {
    fn create_project(&self, name: &str, path: &std::path::PathBuf) -> anyhow::Result<()> {
        debug!("Creating header-only project {name:?} at {path:?}");

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

        fs_err::create_dir(path.join("include"))?;
        //  include/header.h
        {
            let path = path.join("include").join(format!("{}.h", name));
            let mut file = fs_err::File::create(path)?;
            write_template!(file, "templates/header.h")?;
        }

        Ok(())
    }
}
