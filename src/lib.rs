pub mod cli;
pub mod config;
mod logo;
pub mod plugin;
pub mod utils;

pub use logo::logo;

#[macro_export]
macro_rules! debug {
    ($( $arg:tt )*) => {
        if cfg!(debug_assertions) {
            println!($( $arg )*);
        }
    };
}
