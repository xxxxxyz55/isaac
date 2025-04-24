#ifndef __YS_MOUNT_HTML_H__
#define __YS_MOUNT_HTML_H__

#include "ys_cli_log.h"
#include "tool/ys_file.hpp"
#include "tool/ys_scope_guard.hpp"

namespace isaac {

class YS_MountHtml
{
    static std::string toSize(uint64_t sz)
    {
        if(sz < 1024)
        {
            return fmt::format("{} Bytes", sz);
        }
        else if (sz < 1024 * 1024)
        {
            return fmt::format("{} KB", (uint64_t)round(((double)sz) / 1024));
        }
        else if (sz < 1024 * 1024 * 1024)
        {
            return fmt::format("{} MB", (uint64_t)round(((double)sz) / (1024 * 1024)));
        }
        else
        {
            return fmt::format("{} GB", (uint64_t)round(((double)sz) / (1024 * 1024 * 1024)));
        }
    }
    static std::string dirInfo(const std::string &path, std::vector<YS_File::DirInfo> &vtInfo)
    {
        std::string              vtall;
        std::vector<std::string> rows;
        for (auto &&iter : vtInfo)
        {
            if (iter.name == "." || iter.name == "..") continue;
            std::string line = fmt::format("{{name: '{}', isFolder: {}, modified: '{}', size: '{}' ,url:'{}/{}'}}",
                                           iter.name, iter.isDir ? "true" : "false",
                                           iter.modTm,
                                           iter.isDir ? "-" : toSize(iter.size), path, iter.name);
            rows.emplace_back(line);
        }
        vtall = fmt::format("[{}]", YS_String::fmtVector(rows, ','));
        return vtall;
    }

public:
    static std::string format(const std::string &path, std::vector<YS_File::DirInfo> &vtInfo)
    {
        std::string html;
        { // html
            html.append(R"(
<!DOCTYPE html>
<html lang="zh">
)");
        }
        { // head
            html.append(R"(
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
)");
        }

        { // title
            html.append(fmt::format("<title>Index of {}</title>\n", path));
        }
        { // style-styleend
            html.append(R"(
    <style>
        /* 内联 CSS 样式 */
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
        }

        .file-manager {
            width: 100%;
            height: 100vh;
            display: flex;
            flex-direction: column;
        }

        .path-bar {
            background-color: #f4f4f4;
            padding: 10px;
            border-bottom: 1px solid #ccc;
        }

        .path-bar a {
            text-decoration: none;
            color: #007bff;
            margin-right: 5px;
        }

        .path-bar a:hover {
            text-decoration: underline;
        }

        .file-list {
            flex: 1;
            overflow-y: auto;
            padding: 10px;
        }

        .file-item {
            display: grid;
            grid-template-columns: 2fr 1fr 1fr 1fr;
            align-items: center;
            padding: 5px;
            cursor: pointer;
            border-radius: 4px;
        }

        .file-item:hover {
            background-color: #f0f0f0;
        }

        .folder-icon {
            margin-right: 10px;
            color: #ff9500;
        }

        .file-icon {
            margin-right: 10px;
            color: #007bff;
        }

        .parent-folder-icon {
            margin-right: 10px;
            color: #555;
        }

        .file-name {
            display: flex;
            align-items: center;
        }

        .file-type, .file-modified, .file-size {
            text-align: left;
        }
    </style>
)");
        }

        { // head-end
            html.append("</head>\n");
        }

        {
            html.append(R"()");
        }

        { // body
            html.append(R"(
<body>
    <div class="file-manager">
        <!-- 显示当前路径 -->
        <div class="path-bar" id="path-bar">
            <!-- 动态生成的路径 -->
        </div>

        <!-- 文件和目录列表 -->
        <div class="file-list" id="file-list">
            <!-- 动态生成的文件和目录列表 -->
        </div>
    </div>

    <script>
)");
            html.append(fmt::format("let currentPath = '{}';\n", path));
            auto vtstr = dirInfo(path, vtInfo);
            html.append(fmt::format("const itermsData ={};\n", vtstr));
            CLI_DEBUG("request path = {}", path);
            // CLI_DEBUG("dir info = {}", vtstr);
        }

        html.append(R"(
        // 初始化页面
        document.addEventListener('DOMContentLoaded', () => {
            loadDirectory(currentPath);
        });

        // 加载目录内容
        function loadDirectory(path) {
            const fileList = document.getElementById('file-list');
            fileList.innerHTML = ''; // 清空现有内容

            // 更新路径显示
            updatePathBar(path);

            // 如果不是根目录，则添加 ".." 上一级目录
            if (path !== '/') {
                const parentItem = document.createElement('div');
                parentItem.classList.add('file-item');
                parentItem.innerHTML = `
                    <div class="file-name"><span class="parent-folder-icon">⬆️</span>..</div>
                    <div class="file-type">-</div>
                    <div class="file-modified">-</div>
                    <div class="file-size">-</div>
                `;
                parentItem.addEventListener('click', () => {
                    // 返回上一级目录
                    const newPath = path.split('/').slice(0, -1).join('/') || '/';
                    currentPath = newPath;
                    window.location.href = currentPath;
                });
                fileList.appendChild(parentItem);
            }

            // 获取当前路径下的文件和目录
            const items = itermsData;

            items.forEach(item => {
                const iconClass = item.isFolder ? 'folder-icon' : 'file-icon';
                const icon = item.isFolder ? '📁' : '📄';

                const itemElement = document.createElement('div');
                itemElement.classList.add('file-item');
                itemElement.innerHTML = `
                    <div class="file-name"><span class="${iconClass}">${icon}</span>${item.name}</div>
                    <div class="file-type">${item.isFolder ? 'Folder' : 'File'}</div>
                    <div class="file-modified">${item.modified}</div>
                    <div class="file-size">${item.size}</div>
                `;

                // 点击事件
                itemElement.addEventListener('click', () => {
                    currentPath = `${path}/${item.name}`.replace('//', '/');
                    window.location.href = currentPath;
                });

                fileList.appendChild(itemElement);
            });
        }

        // 更新路径栏
        function updatePathBar(path) {
            const pathBar = document.getElementById('path-bar');
            pathBar.innerHTML = ''; // 清空现有路径

            // 将路径分割成各个部分
            const pathParts = path.split('/').filter(part => part.length > 0);

            // 添加根目录链接
            const rootLink = document.createElement('a');
            rootLink.href = '#';
            rootLink.textContent = '/';
            rootLink.addEventListener('click', (e) => {
                e.preventDefault();
                window.location.href = '/';
            });
            pathBar.appendChild(rootLink);

            // 动态生成路径链接
            let currentFullPath = '';
            pathParts.forEach((part, index) => {
                currentFullPath += `/${part}`;
                const separator = document.createTextNode(' / ');
                const link = document.createElement('a');
                link.href = currentFullPath;
                link.textContent = part;
                link.addEventListener('click', (e) => {
                    e.preventDefault();
                    currentPath = currentFullPath;
                    window.location.href = link.href;
                });
                pathBar.appendChild(separator);
                pathBar.appendChild(link);
            });
        }
    </script>
</body>

</html>
)");
        return html;
    }
};

} // namespace isaac

#endif