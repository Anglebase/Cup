#[derive(Debug, Clone, Copy, PartialEq, Eq, Default, serde::Serialize, serde::Deserialize)]
pub enum CStandard {
    #[serde(rename = "90")]
    C90,
    #[serde(rename = "99")]
    C99,
    #[serde(rename = "11")]
    C11,
    #[serde(rename = "17")]
    #[default]
    C17,
    #[serde(rename = "23")]
    C23,
}
