use std::path::PathBuf;

use crate::config::Config;

pub fn clean(project: PathBuf, all: bool) -> anyhow::Result<()> {
    let config = Config::read(project)?;
    if all {
        if config.package.build.exists() {
            fs_err::remove_dir_all(config.package.build)?;
        }
    } else {
        if config.package.build.join("build").exists() {
            fs_err::remove_dir_all(config.package.build.join("build"))?;
        }
    }
    Ok(())
}
