#[derive(Debug, Clone, Copy, PartialEq, Eq, Default, serde::Serialize, serde::Deserialize)]
pub enum OBJCXXStandard {
    #[serde(rename = "98")]
    OBJCXX98,
    #[serde(rename = "11")]
    OBJCXX11,
    #[serde(rename = "14")]
    OBJCXX14,
    #[serde(rename = "17")]
    #[default]
    OBJCXX17,
    #[serde(rename = "20")]
    OBJCXX20,
    #[serde(rename = "23")]
    OBJCXX23,
    #[serde(rename = "26")]
    OBJCXX26,
}
