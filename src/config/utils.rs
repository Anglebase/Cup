use std::collections::{HashMap, HashSet};

use petgraph::{Graph, algo, visit::Dfs};

/// 展开指定特性列表对应的特性集
pub fn expand_features<'a>(
    features: &Vec<String>,
    table: &'a HashMap<String, Vec<String>>,
) -> anyhow::Result<HashSet<&'a String>> {
    let mut graph = Graph::new();
    let mut indeies = HashMap::new();

    // 构造特性关系图
    for (feature, deps) in table {
        let index = if indeies.contains_key(feature) {
            indeies[feature]
        } else {
            let i = graph.add_node(feature);
            indeies.insert(feature.to_string(), i);
            i
        };
        for dep in deps {
            let dep_index = if indeies.contains_key(dep) {
                indeies[dep]
            } else {
                let i = graph.add_node(dep);
                indeies.insert(dep.to_string(), i);
                i
            };
            graph.add_edge(index, dep_index, ());
        }
    }

    // 检查是否存在循环依赖
    if algo::is_cyclic_directed(&graph) {
        return Err(anyhow::anyhow!(
            "There is a cyclic dependency between features."
        ));
    }

    // 生成展开的特性集
    let mut result = HashSet::new();
    for feature in features {
        if !indeies.contains_key(feature) {
            return Err(anyhow::anyhow!("Feature {} is not defined.", feature));
        }
        let index = indeies[feature];
        result.insert(graph[index]);
        let mut visited = Dfs::new(&graph, index);
        while let Some(node) = visited.next(&graph) {
            result.insert(graph[node]);
        }
    }

    Ok(result)
}
