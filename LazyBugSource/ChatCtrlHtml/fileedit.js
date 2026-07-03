// ====== FileEdit 功能模块 ======

/**
 * 创建 FileEdit 窗口
 * @param {string} fileEditId - FileEdit ID
 * @param {string} messageId - 所属消息 ID
 * @param {string} title - 标题
 * @param {string} content - 内容
 * @param {Array} buttons - 按钮配置数组
 * @param {boolean} isCollapsed - 是否折叠
 * @param {string} diffContent - diff 内容
 * @param {string} fullPath - 完整文件路径
 */
function createFileEditWindow(fileEditId, messageId, title, content, buttons, isCollapsed, diffContent, fullPath) {
    const chatContainer = document.getElementById('chat-container');
    const messageElem = document.getElementById(messageId);
    if (!messageElem) {
        console.error('Message element not found for FileEdit:', messageId);
        return;
    }

    const contentElem = messageElem.querySelector('.message-content');
    if (!contentElem) {
        console.error('Message content element not found for FileEdit:', messageId);
        return;
    }

    // Find and remove any existing thinking containers
    const thinkingContainers = contentElem.querySelectorAll('.ai-thinking-container');
    thinkingContainers.forEach(container => container.remove());

    const fileEditDiv = document.createElement('div');
    fileEditDiv.id = fileEditId;
    fileEditDiv.className = 'file-edit-window' + (isCollapsed ? ' file-edit-collapsed' : '');
    
    // 存储两种内容和当前显示状态
    fileEditDiv.setAttribute('data-content', content || '');
    fileEditDiv.setAttribute('data-diff-content', diffContent || '');
    // 如果有 diff 内容，默认显示 diff；否则显示原始内容
    fileEditDiv.setAttribute('data-show-diff', (diffContent && diffContent.trim()) ? 'true' : 'false');

    const titlebar = document.createElement('div');
    titlebar.className = 'file-edit-titlebar';
    
    // 设置标题栏的 tooltip 显示完整路径
    if (fullPath && fullPath.trim()) {
        titlebar.title = fullPath;
    }

    // Container for left-aligned controls (collapse button and title)
    const leftControlsArea = document.createElement('div');
    leftControlsArea.className = 'file-edit-titlebar-left-controls'; // New class for styling

    // Create and add the collapse button first
    const collapseBtn = document.createElement('button');
    collapseBtn.className = 'file-edit-button file-edit-titlebar-collapse-btn'; // Added a specific class
    collapseBtn.textContent = isCollapsed ? '+' : '-';
    collapseBtn.title = 'Toggle Collapse';
    collapseBtn.onclick = (e) => {
        e.stopPropagation();
        const isCurrentlyCollapsed = fileEditDiv.classList.contains('file-edit-collapsed');
        if (isCurrentlyCollapsed) {
            fileEditDiv.classList.remove('file-edit-collapsed');
            collapseBtn.textContent = '-';
        } else {
            fileEditDiv.classList.add('file-edit-collapsed');
            collapseBtn.textContent = '+';
        }
        window.chrome.webview.postMessage({
            action: 'fileEditToggled',
            fileEditId: fileEditId,
            isCollapsed: !isCurrentlyCollapsed
        });
    };
    leftControlsArea.appendChild(collapseBtn);


    const titleSpan = document.createElement('span');
    titleSpan.className = 'file-edit-title';
    titleSpan.textContent = title || 'Untitled';
    
    // 创建旋转圆环修改状态指示器
    const modificationIcon = document.createElement('div');
    modificationIcon.className = 'file-edit-modification-icon';
    modificationIcon.id = fileEditId + '-modification-icon'; // 添加唯一 ID 便于控制
    
    // 将标题文字和图标包装在一个容器中
    titleSpan.innerHTML = ''; // 清空原有内容
    const titleText = document.createElement('span');
    titleText.textContent = title || 'Untitled';
    titleSpan.appendChild(titleText);
    titleSpan.appendChild(modificationIcon);
    
    // Title click handler - applies to the title text itself
    titleSpan.onclick = (e) => {
        e.stopPropagation(); // 阻止事件冒泡，避免触发 leftControlsArea 的点击事件
        console.log('FileEdit title text clicked:', fileEditId);
        window.chrome.webview.postMessage({
            action: 'fileEditTitleClicked',
            fileEditId: fileEditId
        });
    };
    leftControlsArea.appendChild(titleSpan);
    
    // Titlebar click (for the whole bar, excluding buttons)
    // This will now be on leftControlsArea to avoid conflict with right buttons
    leftControlsArea.style.cursor = 'pointer'; // Make it clear this area is clickable
    leftControlsArea.onclick = (e) => {
        // Ensure click is not on the collapse button or title span itself
        if (e.target !== collapseBtn && !collapseBtn.contains(e.target) &&
            e.target !== titleSpan && !titleSpan.contains(e.target)) {
            console.log('FileEdit title area (left controls) clicked:', fileEditId);
            window.chrome.webview.postMessage({
                action: 'fileEditTitleClicked', // Or a more general titlebar click if needed
                fileEditId: fileEditId
            });
        }
    };


    const buttonsArea = document.createElement('div');
    buttonsArea.className = 'file-edit-buttons'; // This will now only hold custom buttons

    // 添加修改统计显示区域
    const statsArea = document.createElement('div');
    statsArea.className = 'file-edit-stats';
    statsArea.id = fileEditId + '-stats';
    
    // 错误前缀常量
    const ERROR_PREFIX = '~Error~ :';
    
    // 统计添加和删除的行数
    function updateStats() {
        const diffCnt = fileEditDiv.getAttribute('data-diff-content') || '';
        const content = fileEditDiv.getAttribute('data-content') || '';
        
        // 检查是否是失败状态（内容以错误前缀开头）
        if (content.startsWith(ERROR_PREFIX) || diffCnt.startsWith(ERROR_PREFIX)) {
            statsArea.innerHTML = '';
            const failureSpan = document.createElement('span');
            failureSpan.className = 'file-edit-stats-failure';
            failureSpan.textContent = 'failure';
            statsArea.appendChild(failureSpan);
            statsArea.style.display = '';
            return;
        }
        
        if (!diffCnt.trim()) {
            statsArea.style.display = 'none';
            return;
        }
        
        let addedCount = 0;
        let removedCount = 0;
        const lines = diffCnt.split('\n');
        for (const line of lines) {
            if (line.startsWith('[+]')) {
                addedCount++;
            } else if (line.startsWith('[-]')) {
                removedCount++;
            }
        }
        
        if (addedCount > 0 || removedCount > 0) {
            statsArea.innerHTML = '';
            if (addedCount > 0) {
                const addedSpan = document.createElement('span');
                addedSpan.className = 'file-edit-stats-added';
                addedSpan.textContent = '+' + addedCount;
                statsArea.appendChild(addedSpan);
            }
            if (removedCount > 0) {
                const removedSpan = document.createElement('span');
                removedSpan.className = 'file-edit-stats-removed';
                removedSpan.textContent = '-' + removedCount;
                statsArea.appendChild(removedSpan);
            }
            statsArea.style.display = '';
        } else {
            statsArea.style.display = 'none';
        }
    }
    
    // 初始化统计
    updateStats();
    
    // 存储更新函数供后续调用
    statsArea.updateStats = updateStats;

    // Add custom buttons
    if (buttons && Array.isArray(buttons)) {
        buttons.forEach(btn => {
            const button = document.createElement('button');
            button.className = 'file-edit-button';
            button.textContent = btn.text;
            button.onclick = (e) => {
                e.stopPropagation();
                window.chrome.webview.postMessage({
                    action: 'fileEditButtonClicked',
                    fileEditId: fileEditId,
                    buttonId: btn.id,
                    buttonAction: btn.action
                });
            };
            buttonsArea.appendChild(button);
        });
    }

    // "..." 按钮已隐藏
    // const optionsBtn = document.createElement('button');
    // optionsBtn.className = 'file-edit-button file-edit-options-btn';
    // optionsBtn.textContent = '...';
    // optionsBtn.title = 'Options';
    
    // // Option 按钮总是启用，不再根据 diffContent 状态禁用

    // const optionsMenu = document.createElement('div');
    // optionsMenu.className = 'file-edit-options-menu';
    // optionsMenu.id = fileEditId + '-options-menu'; // Unique ID for the menu

    // // Helper function to update menu items
    // function updateMenuItems() {
    //     optionsMenu.innerHTML = ''; // Clear existing items
        
    //     const showDiff = fileEditDiv.getAttribute('data-show-diff') === 'true';
    //     const diffContent = fileEditDiv.getAttribute('data-diff-content') || '';
    //     const originalContent = fileEditDiv.getAttribute('data-content') || '';
        
    //     // Add Show code/Show diff option
    //     const toggleItem = document.createElement('div');
    //     toggleItem.className = 'file-edit-options-menu-item';
        
    //     // 检查是否有足够的内容来切换显示
    //     const hasValidDiffContent = diffContent.trim() && originalContent.trim();
        
    //     if (hasValidDiffContent) {
    //         // 有 diff 内容，菜单项可用
    //         toggleItem.textContent = showDiff ? 'Show code' : 'Show diff';
    //         toggleItem.onclick = (e) => {
    //             e.stopPropagation();
    //             toggleFileEditContentDisplay(fileEditDiv);
    //             optionsMenu.classList.remove('show');
    //         };
    //     } else {
    //         // 没有 diff 内容，菜单项禁用
    //         toggleItem.textContent = 'Show diff';
    //         toggleItem.classList.add('disabled');
    //         toggleItem.style.opacity = '0.5';
    //         toggleItem.style.cursor = 'not-allowed';
    //         toggleItem.onclick = (e) => {
    //             e.stopPropagation();
    //             // 禁用状态下不执行任何操作
    //         };
    //     }
    //     optionsMenu.appendChild(toggleItem);
    // }

    // // Initialize menu items
    // updateMenuItems();
    
    // // Store the update function for later use
    // optionsMenu.updateMenuItems = updateMenuItems;

    // buttonsArea.appendChild(optionsBtn);
    // buttonsArea.appendChild(optionsMenu); // Menu is a child of buttonsArea, positioned by CSS

    // optionsBtn.onclick = (e) => {
    //     e.stopPropagation();
    //     // 如果按钮被禁用，不处理点击
    //     if (optionsBtn.disabled) {
    //         return;
    //     }
    //     const shouldShow = !optionsMenu.classList.contains('show');
    //     hideAllFileEditOptionMenus(); // Hide all other similar menus first
    //     if (shouldShow) {
    //         optionsMenu.updateMenuItems(); // Update menu items before showing
    //         optionsMenu.classList.add('show');
    //     }
    //     // If it was already visible, hideAllFileEditOptionMenus would have hidden it.
    // };

    titlebar.appendChild(leftControlsArea); // Add left controls (collapse btn + title)
    titlebar.appendChild(buttonsArea);    // Add right controls (custom buttons)
    titlebar.appendChild(statsArea);      // Add stats display (rightmost)

    const contentDiv = document.createElement('div');
    contentDiv.className = 'file-edit-content';
    
    // 应用 C++ 语法高亮和行高亮
    const preElement = document.createElement('pre');
    const codeElement = document.createElement('code');
    codeElement.className = 'language-cpp';
    
    // 根据显示状态选择要显示的内容
    const showDiff = fileEditDiv.getAttribute('data-show-diff') === 'true';
    const contentToShow = (showDiff && diffContent) ? diffContent : content;
    
    // 处理语法高亮和行高亮标记
    const processedContent = processFileEditContentWithHighlights(contentToShow || '');
    
    // 使用 innerHTML 设置处理后的内容（包含语法高亮和行高亮）
    codeElement.innerHTML = processedContent;
    
    preElement.appendChild(codeElement);
    contentDiv.appendChild(preElement);

    fileEditDiv.appendChild(titlebar);
    fileEditDiv.appendChild(contentDiv);

    // 在添加到 DOM 后计算自适应高度
    contentElem.appendChild(fileEditDiv);
    
    // 设置自适应高度
    updateFileEditAutoHeight(fileEditDiv, contentToShow);
    
    scrollToBottom();
}

/**
 * 更新 FileEdit 窗口
 * @param {string} fileEditId - FileEdit ID
 * @param {Object} updates - 更新对象
 */
function updateFileEditWindow(fileEditId, updates) {
    const fileEditElem = document.getElementById(fileEditId);
    if (!fileEditElem) {
        console.error('FileEdit element not found:', fileEditId);
        return;
    }

    if (updates.title !== undefined) {
        const titleElem = fileEditElem.querySelector('.file-edit-title');
        if (titleElem) {
            // 查找标题文字元素（第一个 span 子元素）
            const titleTextElem = titleElem.querySelector('span');
            if (titleTextElem) {
                titleTextElem.textContent = updates.title;
            } else {
                // 如果没有找到 span 子元素，说明是老的结构，需要重建
                const modificationIcon = titleElem.querySelector('.file-edit-modification-icon');
                titleElem.innerHTML = '';
                const titleText = document.createElement('span');
                titleText.textContent = updates.title;
                titleElem.appendChild(titleText);
                if (modificationIcon) {
                    titleElem.appendChild(modificationIcon);
                } else {
                    // 创建新的修改状态图标
                    const newModificationIcon = document.createElement('div');
                    newModificationIcon.className = 'file-edit-modification-icon';
                    newModificationIcon.id = fileEditId + '-modification-icon';
                    titleElem.appendChild(newModificationIcon);
                }
            }
        }
    }

    if (updates.content !== undefined || updates.diffContent !== undefined) {
        const contentElem = fileEditElem.querySelector('.file-edit-content');
        if (contentElem) {
            // 检查更新前是否应该滚动到底部
            const shouldScrollToBottom = isNearBottom();
            const isExpanded = !fileEditElem.classList.contains('collapsed');
            
            // 更新存储的内容
            if (updates.content !== undefined) {
                fileEditElem.setAttribute('data-content', updates.content);
            }
            if (updates.diffContent !== undefined) {
                fileEditElem.setAttribute('data-diff-content', updates.diffContent);
            }
            
            // 重新设置默认显示状态：优先显示 diff，没有 diff 时显示原始内容
            const diffContent = fileEditElem.getAttribute('data-diff-content') || '';
            const originalContent = fileEditElem.getAttribute('data-content') || '';
            
            // 如果有 diff 内容，默认显示 diff；否则显示原始内容
            if (diffContent.trim()) {
                fileEditElem.setAttribute('data-show-diff', 'true');
            } else {
                fileEditElem.setAttribute('data-show-diff', 'false');
            }
            
            // 根据当前显示状态决定显示哪个内容
            const showDiff = fileEditElem.getAttribute('data-show-diff') === 'true';
            
            let contentToDisplay;
            if (showDiff && diffContent.trim()) {
                contentToDisplay = diffContent;
            } else {
                contentToDisplay = originalContent;
            }
            
            // 查找现有的 code 元素
            const codeElement = contentElem.querySelector('code.language-cpp');
            if (codeElement) {
                // 处理语法高亮和行高亮标记
                const processedContent = processFileEditContentWithHighlights(contentToDisplay);
                
                // 使用 innerHTML 设置处理后的内容（包含语法高亮和行高亮）
                codeElement.innerHTML = processedContent;
            } else {
                // 如果没有找到 code 元素，创建新的结构
                contentElem.innerHTML = '';
                const preElement = document.createElement('pre');
                const newCodeElement = document.createElement('code');
                newCodeElement.className = 'language-cpp';
                
                // 处理语法高亮和行高亮标记
                const processedContent = processFileEditContentWithHighlights(contentToDisplay);
                
                // 使用 innerHTML 设置处理后的内容（包含语法高亮和行高亮）
                newCodeElement.innerHTML = processedContent;
                
                preElement.appendChild(newCodeElement);
                contentElem.appendChild(preElement);
            }
            
            // 更新自适应高度
            updateFileEditAutoHeight(fileEditElem, contentToDisplay);
            
            // 更新修改统计显示
            const statsArea = fileEditElem.querySelector('.file-edit-stats');
            if (statsArea && statsArea.updateStats) {
                statsArea.updateStats();
            }
            
            // 更新菜单项
            const optionsMenu = fileEditElem.querySelector('.file-edit-options-menu');
            if (optionsMenu && optionsMenu.updateMenuItems) {
                optionsMenu.updateMenuItems();
            }
            
            // 根据 FileEdit 状态决定滚动行为
            if (fileEditElem.classList.contains('collapsed')) {
                // 折叠状态：滚动 FileEdit 内容到末尾
                setTimeout(() => {
                    contentElem.scrollTop = contentElem.scrollHeight;
                }, 0);
            } else if (isExpanded && shouldScrollToBottom) {
                // 展开状态且聊天窗口在底部附近：滚动整个聊天窗口到底部
                setTimeout(() => {
                    scrollToBottom();
                }, 0);
            }
        }
    }

    if (updates.height !== undefined) {
        // height 已废弃
    }

    if (updates.buttons !== undefined) {
        const buttonsArea = fileEditElem.querySelector('.file-edit-buttons');
        if (buttonsArea) {
            const optionsBtn = buttonsArea.querySelector('.file-edit-options-btn');
            const optionsMenu = buttonsArea.querySelector('.file-edit-options-menu');

            // Remove all children that are not the optionsBtn or optionsMenu
            let child = buttonsArea.firstChild;
            while(child) {
                let nextChild = child.nextSibling;
                if (child !== optionsBtn && child !== optionsMenu) {
                    buttonsArea.removeChild(child);
                }
                child = nextChild;
            }

            // Add new custom buttons before the optionsBtn if it exists
            if (updates.buttons && Array.isArray(updates.buttons)) {
                updates.buttons.forEach(btnDef => {
                    const button = document.createElement('button');
                    button.className = 'file-edit-button';
                    button.textContent = btnDef.text;
                    button.onclick = (e) => {
                        e.stopPropagation();
                        window.chrome.webview.postMessage({
                            action: 'fileEditButtonClicked',
                            fileEditId: fileEditId,
                            buttonId: btnDef.id,
                            buttonAction: btnDef.action
                        });
                    };
                    // Insert new custom buttons before the optionsBtn
                    if (optionsBtn) {
                        buttonsArea.insertBefore(button, optionsBtn);
                    } else {
                        // Fallback if optionsBtn is not found (shouldn't happen in normal flow)
                        buttonsArea.appendChild(button);
                    }
                });
            }
        }
    }

    if (updates.isCollapsed !== undefined) {
        // Find the collapse button using its new specific class or location
        const collapseBtn = fileEditElem.querySelector('.file-edit-titlebar-collapse-btn');
        if (collapseBtn) {
            if (updates.isCollapsed) {
                fileEditElem.classList.add('file-edit-collapsed');
                collapseBtn.textContent = '+';
            } else {
                fileEditElem.classList.remove('file-edit-collapsed');
                collapseBtn.textContent = '-';
            }
        }
    }
}

/**
 * 切换 FileEdit 内容显示（code/diff）
 * @param {HTMLElement} fileEditDiv - FileEdit 元素
 */
function toggleFileEditContentDisplay(fileEditDiv) {
    const showDiff = fileEditDiv.getAttribute('data-show-diff') === 'true';
    const diffContent = fileEditDiv.getAttribute('data-diff-content') || '';
    const originalContent = fileEditDiv.getAttribute('data-content') || '';
    
    // 切换显示状态
    const newShowDiff = !showDiff;
    fileEditDiv.setAttribute('data-show-diff', newShowDiff.toString());
    
    // 确定要显示的内容
    const contentToShow = newShowDiff ? diffContent : originalContent;
    
    // 更新显示内容
    const contentElem = fileEditDiv.querySelector('.file-edit-content');
    if (contentElem) {
        const codeElement = contentElem.querySelector('code.language-cpp');
        if (codeElement) {
            const processedContent = processFileEditContentWithHighlights(contentToShow);
            codeElement.innerHTML = processedContent;
        }
        
        // 更新自适应高度
        updateFileEditAutoHeight(fileEditDiv, contentToShow);
        
        // 根据 FileEdit 状态决定滚动行为
        if (fileEditDiv.classList.contains('collapsed')) {
            setTimeout(() => {
                contentElem.scrollTop = contentElem.scrollHeight;
            }, 0);
        } else if (!fileEditDiv.classList.contains('collapsed') && isNearBottom()) {
            setTimeout(() => {
                scrollToBottom();
            }, 0);
        }
    }
}

/**
 * 更新 FileEdit 窗口的自适应高度
 * @param {HTMLElement} fileEditDiv - FileEdit 元素
 * @param {string} content - 内容文本
 */
function updateFileEditAutoHeight(fileEditDiv, content) {
    // 添加自适应高度类
    fileEditDiv.classList.add('auto-height');
}

/**
 * Helper function to hide all file edit option menus
 */
function hideAllFileEditOptionMenus() {
    document.querySelectorAll('.file-edit-options-menu.show').forEach(menu => {
        menu.classList.remove('show');
    });
}

/**
 * 开始 FileEdit 修改状态（显示旋转动画）
 * @param {string} fileEditId - FileEdit ID
 */
function startFileEditModification(fileEditId) {
    const modificationIcon = document.getElementById(fileEditId + '-modification-icon');
    if (modificationIcon) {
        modificationIcon.classList.add('active');
        console.log('Started modification animation for FileEdit:', fileEditId);
    } else {
        console.warn('Modification icon not found for FileEdit:', fileEditId);
    }
}

/**
 * 停止 FileEdit 修改状态（隐藏旋转动画）
 * @param {string} fileEditId - FileEdit ID
 */
function stopFileEditModification(fileEditId) {
    const modificationIcon = document.getElementById(fileEditId + '-modification-icon');
    if (modificationIcon) {
        modificationIcon.classList.remove('active');
        console.log('Stopped modification animation for FileEdit:', fileEditId);
    } else {
        console.warn('Modification icon not found for FileEdit:', fileEditId);
    }
}

// ====== FileEdit Progress Label 相关函数 ======

/**
 * 显示 FileEdit 进度标签
 * @param {string} messageId - 消息 ID
 * @param {string} fileName - 文件名
 * @param {string} fullPath - 完整文件路径
 */
function showFileEditProgressLabel(messageId, fileName, fullPath) {
    const messageElem = document.getElementById(messageId);
    if (!messageElem) {
        console.error('Message element not found for progress label:', messageId);
        return;
    }

    const messageContentElem = messageElem.querySelector('.message-content');
    if (!messageContentElem) {
        console.error('Message content element not found for progress label:', messageId);
        return;
    }

    // 检查是否已经存在进度标签
    const existingLabel = messageContentElem.querySelector('.file-edit-progress-label');
    if (existingLabel) {
        // 获取已存在标签的文件名
        const existingText = existingLabel.querySelector('.file-edit-progress-text');
        if (existingText) {
            const existingFileName = existingText.innerHTML; // 改用 innerHTML 以支持 HTML 标签
            
            // 构建新的显示文本
            let newDisplayText;
            if (fileName && fileName.trim()) {
                newDisplayText = 'Editing ' + fileName + ' ...';
            } else {
                newDisplayText = 'Editing file ...';
            }
            
            // 如果文件名没有变化，立即返回，什么也不做
            if (existingFileName === newDisplayText) {
                console.log('FileEdit progress label fileName unchanged, skipping update for message:', messageId);
                return;
            }
        }
        
        // 文件名有变化，移除旧标签
        existingLabel.remove();
    }

    // 创建进度标签元素
    const progressLabel = document.createElement('div');
    progressLabel.className = 'file-edit-progress-label';
    progressLabel.id = messageId + '-progress-label';
    
    // 设置 tooltip 显示完整路径（只有设置了路径名才显示）
    if (fullPath && fullPath.trim()) {
        progressLabel.title = fullPath;
        // 添加可点击样式和事件
        progressLabel.classList.add('file-edit-progress-label-clickable');
        progressLabel.style.cursor = 'pointer';
        progressLabel.onclick = (e) => {
            e.stopPropagation();
            // 发送消息给 C++ 端打开文件
            window.chrome.webview.postMessage({
                action: 'openFile',
                filePath: fullPath
            });
        };
    }

    // 创建旋转图标
    const progressIcon = document.createElement('div');
    progressIcon.className = 'file-edit-progress-icon';

    // 创建文本元素
    const progressText = document.createElement('div');
    progressText.className = 'file-edit-progress-text';
    
    // 设置显示文本（使用 innerHTML 以支持 HTML 标签）
    if (fileName && fileName.trim()) {
        progressText.innerHTML = 'Editing ' + fileName + ' ...';
    } else {
        progressText.innerHTML = 'Editing file ...';
    }

    // 组装元素
    progressLabel.appendChild(progressIcon);
    progressLabel.appendChild(progressText);

    // 添加到消息内容的最下方
    messageContentElem.appendChild(progressLabel);

    // 如果聊天窗口在底部附近，滚动到底部
    if (isNearBottom()) {
        scrollToBottom();
    }

    console.log('FileEdit progress label shown for message:', messageId, 'fileName:', fileName);
}

/**
 * 隐藏 FileEdit 进度标签
 * @param {string} messageId - 消息 ID
 */
function hideFileEditProgressLabel(messageId) {
    const progressLabel = document.getElementById(messageId + '-progress-label');
    if (progressLabel) {
        progressLabel.remove();
        console.log('FileEdit progress label hidden for message:', messageId);
    } else {
        console.warn('FileEdit progress label not found for message:', messageId);
    }
}