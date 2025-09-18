mod exe;
mod lib;

pub use exe::Executable;
pub use lib::Static;

use crate::cli::{CreateProject, TemplateType};

/// 创建一个指定类型的插件实例
pub fn create_instance(ty: TemplateType) -> Box<dyn CreateProject> {
    match ty {
        TemplateType::Executable => Box::new(Executable::default()),
        TemplateType::Static => Box::new(Static::default()),
        _ => todo!(),
    }
}
