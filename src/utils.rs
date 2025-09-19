use std::{collections::HashMap, env::home_dir, path::PathBuf};

pub fn template_f<'a, T: Into<HashMap<&'a str, String>>>(template: &str, replace: T) -> String {
    let mut result = template.to_string();
    let replace_map: HashMap<&str, String> = replace.into();
    for (key, value) in replace_map {
        result = result.replace(key, &value);
    }
    result
}

#[macro_export]
macro_rules! write_template {
    ($f:expr, $p:expr) => {
        $f.write_fmt(format_args!("{}", include_str!($p)))
    };
    ($f:expr, $p: expr => [$($k: ident = $e: expr);+ $(;)?]) => {
        $f.write_fmt(format_args!("{}", $crate::utils::template_f(include_str!($p), [$((constcat::concat!("{{", stringify!($k), "}}"), $e),)*])))
    }
}

pub struct CupData {}

impl CupData {
    pub fn base() -> PathBuf {
        home_dir().unwrap().join(".cup")
    }

    pub fn packages() -> PathBuf {
        CupData::base().join("packages")
    }

    pub fn git(owner: &str, repo: &str, tag: &str) -> PathBuf {
        CupData::base()
            .join("gits")
            .join(format!("{}-{}-{}", owner, repo, tag))
    }
}
