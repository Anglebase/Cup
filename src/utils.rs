use std::collections::HashMap;

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
    ($f:expr, $p: expr => [$($k: ident = $e: expr);+]) => {
        $f.write_fmt(format_args!("{}", $crate::utils::template_f(include_str!($p), [$((constcat::concat!("{{", stringify!($k), "}}"), $e),)*])))
    }
}
