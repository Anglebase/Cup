#[derive(Debug, Clone, Copy, PartialEq, Eq, Default, serde::Serialize, serde::Deserialize)]
pub enum CXXStandard {
    #[serde(rename = "98")]
    CXX98,
    #[serde(rename = "11")]
    CXX11,
    #[serde(rename = "14")]
    CXX14,
    #[serde(rename = "17")]
    #[default]
    CXX17,
    #[serde(rename = "20")]
    CXX20,
    #[serde(rename = "23")]
    CXX23,
    #[serde(rename = "26")]
    CXX26,
}
