#[derive(Debug, Clone, Copy, PartialEq, Eq, Default, serde::Serialize, serde::Deserialize)]
pub enum HIPStandard {
    #[serde(rename = "98")]
    HIP98,
    #[serde(rename = "11")]
    HIP11,
    #[serde(rename = "14")]
    HIP14,
    #[serde(rename = "17")]
    #[default]
    HIP17,
    #[serde(rename = "20")]
    HIP20,
    #[serde(rename = "23")]
    HIP23,
    #[serde(rename = "26")]
    HIP26,
}
