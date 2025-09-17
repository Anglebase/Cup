mod exe;

pub use exe::Executable;

use crate::cli::{CreateProject, TemplateType};

/// 创建一个指定类型的插件实例
pub fn create_instance(ty: TemplateType) -> Box<dyn CreateProject> {
    match ty {
        TemplateType::Executable => Box::new(Executable::default()),
        _ => todo!(),
    }
}
