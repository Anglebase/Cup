use std::{collections::HashSet, io::Write};

use anyhow::anyhow;
use colored::Colorize;
use futures::future::join_all;

use crate::{
    GITHUB,
    config::{Config, DependencySource},
    utils::{CupData, GitUnique, TryAll, extract_skip_top_dir},
};

/// 安装包API
pub async fn install(name: &str) -> anyhow::Result<()> {
    if name.starts_with("@") {
        // Git 唯一限定名以 @ 开头
        install_git(&name[1..]).await?;
    } else {
        todo!()
    }
    Ok(())
}

/// Git 依赖元信息
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct GitDependency {
    pub owner: String,
    pub repo: String,
    pub tag: String,
}

/// 递归元：依赖项获取
async fn _get_git_dependencies(
    base: &GitDependency,
    deps: &mut Vec<GitDependency>,
) -> anyhow::Result<()> {
    // 获取远程仓库的配置文件
    let remote_config = get_git_remote_config(&base.owner, &base.repo, &base.tag).await?;
    let config = unsafe { Config::from_str(&remote_config) }?;
    // 获取其远程依赖
    let itdeps = config.get_all_remote_dependencies();
    // 递归获取依赖
    for dep in itdeps {
        let dep = if let DependencySource::Git { owner, repo, tag } = &dep.src {
            GitDependency {
                owner: owner.clone(),
                repo: repo.clone(),
                tag: tag.clone(),
            }
        } else {
            unreachable!()
        };
        Box::pin(_get_git_dependencies(&dep, deps)).await?;
    }
    deps.push(base.clone());
    Ok(())
}

/// 获取 Git 依赖项的递归包装
async fn get_git_dependencies(
    owner: &str,
    repo: &str,
    tag: &str,
) -> anyhow::Result<Vec<GitDependency>> {
    let base = GitDependency {
        owner: owner.to_string(),
        repo: repo.to_string(),
        tag: tag.to_string(),
    };
    let mut deps = vec![];
    _get_git_dependencies(&base, &mut deps).await?;
    Ok(deps)
}

/// 获取仓库远程配置文件内容
async fn get_git_remote_config(owner: &str, repo: &str, tag: &str) -> anyhow::Result<String> {
    // API
    let github = &GITHUB;

    // 访问仓库的配置文件
    let repos = github.repos(owner, repo);
    let config = repos
        .get_content()
        .path("Cup.toml")
        .r#ref(tag)
        .send()
        .await?
        .take_items();
    if config.len() != 1 {
        return Err(anyhow!(
            "https://github.com/{owner}/{repo} has no Cup.toml."
        ));
    }

    // 解析配置文件内容
    let config_content = config[0].decoded_content().unwrap();
    Ok(config_content)
}

async fn download_task(dep: GitDependency) -> anyhow::Result<()> {
    // 解析包的 Url
    let GitDependency { owner, repo, tag } = dep;
    println!("{} {owner}/{repo} @{tag}", "Downloading".green().bold());
    let url = format!("https://github.com/{owner}/{repo}/archive/refs/tags/{tag}.tar.gz");

    // 下载包
    let response = reqwest::get(&url).await?;
    if response.status() != 200 {
        return Err(anyhow!("Download failed: {}", response.status()));
    }
    let file_cache = CupData::cache().join(format!("{owner}-{repo}-{tag}.tar.gz"));
    let mut file = std::fs::File::create(&file_cache)?;
    file.write(&response.bytes().await?)?;

    // 安装包
    println!("{} {owner}/{repo} @{tag}", "Installing".green().bold());
    extract_skip_top_dir(&file_cache, CupData::git(&owner, &repo, &tag))?;

    // 删除缓存文件
    fs_err::remove_file(&file_cache)?;
    Ok(())
}

async fn install_git(name: &str) -> anyhow::Result<()> {
    let GitUnique { owner, repo, tag } = GitUnique::from(name)?;
    println!("{}", "Collecting...".cyan().bold());
    let deps = get_git_dependencies(&owner, &repo, &tag).await?;

    // 依赖项去重并移除已安装的依赖
    let deps = deps
        .into_iter()
        .filter(|GitDependency { owner, repo, tag }| {
            println!("{owner}/{repo} @{tag} already installed.");
            !CupData::git(owner, repo, tag).exists()
        })
        .collect::<HashSet<_>>();

    // 安装所有依赖项
    let tasks = deps.into_iter().map(download_task);
    let _ = join_all(tasks)
        .await
        .into_iter()
        .try_all(|_| Ok::<(), anyhow::Error>(()))?;
    Ok(())
}
