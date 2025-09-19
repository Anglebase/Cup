use std::{collections::HashMap, env::home_dir, fmt::Debug, path::PathBuf};

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

/// 此 trait 用于处理返回 Result 类型的迭代器，只有当所有元素都成功时才返回 Ok，否则返回 Err。
pub trait TryAll<T, U, E: Debug> {
    fn try_all(self, f: impl Fn(T) -> Result<U, E>) -> Result<impl Iterator<Item = U>, E>;
}

impl<T, U, E: Debug, I: Iterator<Item = T>> TryAll<T, U, E> for I {
    fn try_all(self, f: impl Fn(T) -> Result<U, E>) -> Result<impl Iterator<Item = U>, E> {
        let mut iter = self.map(f);
        iter.try_for_each(|i| i.map(|_| ()))?;
        Ok(iter.map(Result::unwrap))
    }
}
