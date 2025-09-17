pub mod cli;
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
