use std::{
    collections::HashMap,
    env::home_dir,
    fmt::Debug,
    path::{Path, PathBuf},
};

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

    pub fn cache() -> PathBuf {
        CupData::base().join("cache")
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

pub struct GitUnique<'a> {
    pub owner: &'a str,
    pub repo: &'a str,
    pub tag: &'a str,
}

impl<'a> GitUnique<'a> {
    /// Git 唯一限定名解析
    pub fn from(name: &'a str) -> anyhow::Result<Self> {
        let mut parts = name.split('/').into_iter().collect::<Vec<_>>();
        if parts.len() != 3 {
            return Err(anyhow::anyhow!(
                "Invalid git mark format, should be owner/repo/tag."
            ));
        }
        let owner = parts.remove(0);
        let repo = parts.remove(0);
        let tag = parts.remove(0);
        Ok(Self { owner, repo, tag })
    }
}

/// 将 gzip 归档文件解压到指定目录
pub fn extract_skip_top_dir<P: AsRef<Path>, Q: AsRef<Path>>(
    archive_path: P,
    target_dir: Q,
) -> anyhow::Result<()> {
    // 打开并解压 gzip 流
    let file = fs_err::File::open(archive_path.as_ref())?;
    let gz_decoder = flate2::read::GzDecoder::new(file);
    let mut archive = tar::Archive::new(gz_decoder);

    // 获取归档文件中的条目
    let entries = archive.entries()?;

    for entry in entries {
        let mut entry = entry?;
        let path = entry.path()?;

        // 跳过顶层目录
        let components: Vec<_> = path.components().collect();
        if components.len() > 1 {
            // 构建新路径（跳过第一层）
            let new_path = components
                .iter()
                .skip(1)
                .fold(PathBuf::new(), |mut path, comp| {
                    path.push(comp);
                    path
                });

            // 构建完整目标路径
            let full_path = target_dir.as_ref().join(new_path);

            // 确保目标目录存在
            if let Some(parent) = full_path.parent() {
                std::fs::create_dir_all(parent)?;
            }

            // 解压文件
            entry.unpack(&full_path)?;
        }
    }

    Ok(())
}
