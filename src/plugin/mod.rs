mod dll;
mod exe;
mod hdr;
mod lib;
mod plugin;

pub use dll::Shared;
pub use exe::Executable;
pub use hdr::HeaderOnly;
pub use lib::Static;
pub use plugin::Module;

use crate::cli::{CreateProject, TemplateType};

/// 创建一个指定类型的插件实例
pub fn create_instance(ty: TemplateType) -> Box<dyn CreateProject> {
    match ty {
        TemplateType::Executable => Box::new(Executable::default()),
        TemplateType::Static => Box::new(Static::default()),
        TemplateType::Shared => Box::new(Shared::default()),
        TemplateType::Module => Box::new(Module::default()),
        TemplateType::HeaderOnly => Box::new(HeaderOnly::default()),
    }
}
