use crate::utils::{CupData, GitUnique};

pub fn uninstall(name: &str) -> anyhow::Result<()> {
    if name.starts_with("@") {
        uninstall_git(&name[1..])?;
    } else {
        todo!()
    }
    Ok(())
}

fn uninstall_git(name: &str) -> anyhow::Result<()> {
    let GitUnique { owner, repo, tag } = GitUnique::from(name)?;
    let git = CupData::git(owner, repo, tag);
    if git.exists() {
        Ok(fs_err::remove_dir_all(CupData::git(owner, repo, tag))?)
    } else {
        Err(anyhow::anyhow!("{name} not installed."))
    }
}
