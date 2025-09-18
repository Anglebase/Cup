#[derive(Debug, Clone, Copy, PartialEq, Eq, Default, serde::Serialize, serde::Deserialize)]
pub enum CUDAStandard {
    #[serde(rename = "98")]
    CUDA98,
    #[serde(rename = "03")]
    CUDA03,
    #[serde(rename = "11")]
    CUDA11,
    #[serde(rename = "14")]
    CUDA14,
    #[serde(rename = "17")]
    #[default]
    CUDA17,
    #[serde(rename = "20")]
    CUDA20,
    #[serde(rename = "23")]
    CUDA23,
    #[serde(rename = "26")]
    CUDA26,
}
