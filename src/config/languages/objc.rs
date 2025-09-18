#[derive(Debug, Clone, Copy, PartialEq, Eq, Default, serde::Serialize, serde::Deserialize)]
pub enum OBJCStandard {
    #[serde(rename = "90")]
    OBJC90,
    #[serde(rename = "99")]
    OBJC99,
    #[serde(rename = "11")]
    OBJC11,
    #[serde(rename = "17")]
    #[default]
    OBJC17,
    #[serde(rename = "23")]
    OBJC23,
}
