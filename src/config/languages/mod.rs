mod c;
mod cuda;
mod cxx;
mod hip;
mod objc;
mod objcxx;

use std::path::PathBuf;

pub use c::CStandard;
pub use cuda::CUDAStandard;
pub use cxx::CXXStandard;
pub use hip::HIPStandard;
pub use objc::OBJCStandard;
pub use objcxx::OBJCXXStandard;

use crate::config::Shelling;

#[derive(Debug, Clone, Default, serde::Serialize, serde::Deserialize)]
pub struct LanguageConfig {
    pub c: Option<CStandard>,
    pub cxx: Option<CXXStandard>,
    pub csharp: Option<String>,
    pub cuda: Option<CUDAStandard>,
    pub objc: Option<OBJCStandard>,
    pub objcxx: Option<OBJCXXStandard>,
    pub fortran: Option<String>,
    pub hip: Option<HIPStandard>,
    pub ispc: Option<String>,
    pub swift: Option<String>,
    pub asm: Option<String>,
    pub asm_nasm: Option<String>,
    pub asm_marmasm: Option<String>,
    pub asm_masm: Option<String>,
    pub asm_att: Option<String>,
}

impl Shelling<Language> for LanguageConfig {
    fn shelling(self, _base: &PathBuf) -> Language {
        Language {
            c: self.c.unwrap_or_default(),
            cxx: self.cxx.unwrap_or_default(),
            csharp: self.csharp.is_some(),
            cuda: self.cuda,
            objc: self.objc,
            objcxx: self.objcxx,
            fortran: self.fortran.is_some(),
            hip: self.hip,
            ispc: self.ispc.is_some(),
            swift: self.swift.is_some(),
            asm: self.asm.is_some(),
            asm_nasm: self.asm_nasm.is_some(),
            asm_marmasm: self.asm_marmasm.is_some(),
            asm_masm: self.asm_masm.is_some(),
            asm_att: self.asm_att.is_some(),
        }
    }
}

#[derive(Debug, Clone)]
pub struct Language {
    pub c: CStandard,
    pub cxx: CXXStandard,
    pub cuda: Option<CUDAStandard>,
    pub objc: Option<OBJCStandard>,
    pub objcxx: Option<OBJCXXStandard>,
    pub hip: Option<HIPStandard>,
    pub csharp: bool,
    pub fortran: bool,
    pub ispc: bool,
    pub swift: bool,
    pub asm: bool,
    pub asm_nasm: bool,
    pub asm_marmasm: bool,
    pub asm_masm: bool,
    pub asm_att: bool,
}
