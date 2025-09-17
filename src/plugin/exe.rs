use crate::cli::CreateProject;

#[derive(Debug, Clone, Default)]
pub struct Executable {}

impl Executable {
    pub fn new() -> Self {
        Self {}
    }
}

impl CreateProject for Executable {
    fn create_project(&self, name: &str, path: &std::path::PathBuf) -> anyhow::Result<()> {
        println!("Creating project {name:?} at {path:?}");
        Ok(())
    }
}
