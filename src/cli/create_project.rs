use std::path::PathBuf;

/// 模板项目创建API接口
pub trait CreateProject {
    /// 以`name`为项目名称，以`path`为项目根路径，创建项目模板
    ///
    /// # Note
    /// 此函数在调用时保证`path`已经存在且为空
    fn create_project(&self, name: &str, path: &PathBuf) -> anyhow::Result<()>;
}
