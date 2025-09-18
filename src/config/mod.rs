mod build;
pub mod languages;
mod package;

pub use build::{Build, BuildConfig};
pub use languages::{Language, LanguageConfig};
pub use package::{Package, PackageConfig};

pub trait Shelling<T> {
    fn shelling(self) -> T;
}
