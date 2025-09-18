use std::{io::Write, path::PathBuf};

use crate::{cli::CreateProject, debug, write_template};

#[derive(Debug, Clone, Default)]
pub struct Shared {}

impl CreateProject for Shared {
    fn create_project(&self, name: &str, path: &PathBuf) -> anyhow::Result<()> {
        debug!("Creating shared project {name:?} at {path:?}");

        // Cup.toml
        {
            let path = path.join("Cup.toml");
            let mut file = fs_err::File::create(path)?;
            write_template!(file, "templates/Cup.toml" => [
                NAME = name.to_string();
                TYPE = "shared".to_string();
            ])?;
        }

        // .gitignore
        {
            let path = path.join(".gitignore");
            let mut file = fs_err::File::create(path)?;
            write_template!(file, "templates/gitignore")?;
        }

        fs_err::create_dir(path.join("src"))?;
        // src/{{name}}.cpp
        {
            let path = path.join(format!("src/{}.cpp", name));
            let mut file = fs_err::File::create(path)?;
            write_template!(file, "templates/shared.cpp" => [
                NAME = name.to_string();
            ])?;
        }

        fs_err::create_dir(path.join("include"))?;
        // include/{{name}}.h
        {
            let path = path.join(format!("include/{}.h", name));
            let mut file = fs_err::File::create(path)?;
            write_template!(file, "templates/shared.h")?;
        }

        Ok(())
    }
}
