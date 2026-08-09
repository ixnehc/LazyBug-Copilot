// ===== Provider & API Tab - JavaScript =====

// Provider / API 树形展开状态
let expandedProviders = [];  // 当前展开的 provider names 数组
let expandedApis = [];       // 当前展开的 api names 数组
let pendingFocusApiName = null; // 新建API后待聚焦的名称
let pendingFocusProviderName = null; // 新建Provider后待聚焦的名称

// 辅助：发送消息到C++
function postMsg(obj) {
    if (window.chrome && window.chrome.webview)
        window.chrome.webview.postMessage(obj);
}

// 请求Provider数据
function requestProviderData() {
    if (window.chrome && window.chrome.webview) {
        window.chrome.webview.postMessage({
            action: 'requestProviderData'
        });
    }
}

// 请求Capability状态
function requestCapabilityStatus() {
    if (window.chrome && window.chrome.webview) {
        window.chrome.webview.postMessage({
            action: 'requestCapabilityStatus'
        });
    }
}
 
// 请求Cast Sheet数据
function requestCastSheetData() {
    if (window.chrome && window.chrome.webview) {
        window.chrome.webview.postMessage({
            action: 'requestCastSheetData'
        });
    }
}

// 设置Cast Sheet数据
function setCastSheetData(data) {
    const majorChatSelect = document.getElementById('cast-sheet-majorChat');
    const briefSelect = document.getElementById('cast-sheet-brief');
    const summarizeSelect = document.getElementById('cast-sheet-summarize');
    const inputHintSelect = document.getElementById('cast-sheet-inputHint');
    const embeddingSelect = document.getElementById('cast-sheet-embedding');
    const castSheetPanel = document.getElementById('cast-sheet-panel');
    
    if (!majorChatSelect || !briefSelect || !summarizeSelect || !inputHintSelect || !embeddingSelect || !castSheetPanel) return;
    
    // 分别获取不同的API列表
    const majorChatApis = data.majorChatApis || [];
    const briefApis = data.briefApis || [];
    const summarizeApis = data.summarizeApis || [];
    const inputHintApis = data.inputHintApis || [];
    const embeddingApis = data.embeddingApis || [];
    
    function rebuildSelect(select, newValue, apiList) {
        select.innerHTML = '';
        if (apiList.length === 0) {
            const opt = document.createElement('option');
            opt.textContent = 'No available APIs';
            opt.disabled = true;
            select.appendChild(opt);
        } else {
            apiList.forEach(api => {
                const opt = document.createElement('option');
                opt.value = api.name;
                opt.textContent = api.name;
                select.appendChild(opt);
            });
        }
        // 使用传入的新值设置选中项
        if (newValue && [...select.options].some(o => o.value === newValue)) {
            select.value = newValue;
        } else if (select.options.length > 0 && !select.options[0].disabled) {
            select.selectedIndex = 0;
        }
    }
    
    rebuildSelect(majorChatSelect, data.majorChatApi, majorChatApis);
    rebuildSelect(briefSelect, data.briefApi, briefApis);
    rebuildSelect(summarizeSelect, data.summarizeApi, summarizeApis);
    rebuildSelect(inputHintSelect, data.inputHintApi, inputHintApis);
    rebuildSelect(embeddingSelect, data.embeddingApi, embeddingApis);
    
    // 更新Evaluate按钮状态
    updateEvaluateBtnState();
    
    // 根据major chat是否有值设置颜色样式
    castSheetPanel.classList.remove('has-value', 'no-value');
    if (data.majorChatApi && data.majorChatApi.trim() !== '') {
        castSheetPanel.classList.add('has-value');
    } else {
        castSheetPanel.classList.add('no-value');
    }
}

// C++ 回调：从剪贴板读取文本后调用（绕过浏览器权限弹窗）
window.onClipboardData = function(text) {
    // 处理"新建 Provider"粘贴
    if (window._pendingPasteNew) {
        const pendingNew = window._pendingPasteNew;
        window._pendingPasteNew = null;
        const { providers } = pendingNew;
        let setting;
        try {
            setting = JSON.parse(text);
        } catch (_) {
            return;
        }
        if (!setting || typeof setting.name !== 'string' || !setting.name.trim()) {
            return;
        }
        // 生成不重名的 Provider 名称
        let newName = setting.name.trim();
        const existingNames = providers.map(p => p.name);
        if (existingNames.includes(newName)) {
            let suffix = 1;
            while (existingNames.includes(newName + suffix)) {
                suffix++;
            }
            newName = newName + suffix;
        }
        const endpoint = (typeof setting.endpoint === 'string') ? setting.endpoint.trim() : '';
        const format = (typeof setting.format === 'string') ? setting.format.trim() : '';
        const storeResponses = (typeof setting.storeResponses === 'boolean') ? setting.storeResponses : false;
        // 自动展开并标记待聚焦
        if (!expandedProviders.includes(newName)) {
            expandedProviders.push(newName);
        }
        pendingFocusProviderName = newName;
        postMsg({ action: 'addProviderFromClipboard', name: newName, endpoint: endpoint, format: format, storeResponses: storeResponses });

        // 粘贴成功反馈
        const addPasteBtn = document.querySelector('.add-paste-provider-btn');
        if (addPasteBtn) {
            addPasteBtn.classList.add('pasted');
            setTimeout(() => addPasteBtn.classList.remove('pasted'), 1500);
        }
        return;
    }

    // 处理"新建 API"粘贴
    if (window._pendingPasteNewApi) {
        const pendingNewApi = window._pendingPasteNewApi;
        window._pendingPasteNewApi = null;
        const { provider, providers } = pendingNewApi;
        let setting;
        try {
            setting = JSON.parse(text);
        } catch (_) {
            return;
        }
        if (!setting || typeof setting.name !== 'string' || !setting.name.trim()) {
            return;
        }
        // 生成不重名的 API 名称（在所有 API 中检查）
        let newName = setting.name.trim();
        const existingNames = [];
        providers.forEach(p => {
            if (p.apis) p.apis.forEach(a => existingNames.push(a.name));
        });
        if (existingNames.includes(newName)) {
            let suffix = 1;
            while (existingNames.includes(newName + suffix)) {
                suffix++;
            }
            newName = newName + suffix;
        }
        // 自动展开并标记待聚焦
        if (!expandedApis.includes(newName)) {
            expandedApis.push(newName);
        }
        pendingFocusApiName = newName;
        // 构建消息（只传非空字段）
        const msg = { action: 'addApiFromClipboard', providerName: provider.name, apiName: newName };
        ['model','rule','maxToken','contextCapacity','priceInputToken','priceOutputToken','priceCacheRead','priceCacheWrite','temperature','thinkingMode','cacheControl','role','enable','tools','openRouterOptions'].forEach(f => {
            if (setting[f] !== undefined && setting[f] !== null) msg[f] = setting[f];
        });
        postMsg(msg);
        // 反馈
        const pasteBtn = document.querySelector('.add-api-paste-btn');
        if (pasteBtn) {
            pasteBtn.classList.add('pasted');
            setTimeout(() => pasteBtn.classList.remove('pasted'), 1500);
        }
        return;
    }

    // 处理已有 API 的粘贴
    const pendingApi = window._pendingPasteApi;
    if (pendingApi) {
        window._pendingPasteApi = null;
        const { api, apiPasteBtn, provider, providers } = pendingApi;
        let setting;
        try {
            setting = JSON.parse(text);
        } catch (_) {
            return;
        }
        if (!setting || typeof setting.name !== 'string' || !setting.name.trim()) {
            return;
        }
        const newName = setting.name.trim();
        // 更新 name（重复则跳过，不弹框）
        let effectiveName = api.name;
        if (newName !== api.name) {
            const duplicate = providers.some(p => p.apis && p.apis.find(a => a.name === newName));
            if (!duplicate) {
                postMsg({ action: 'updateApiName', oldName: api.name, newName: newName });
                effectiveName = newName;
            }
        }
        // 更新其他字段（逐一比较，只发变化的）
        const stringFields = ['model', 'rule', 'thinkingMode', 'cacheControl', 'role'];
        stringFields.forEach(f => {
            const v = (typeof setting[f] === 'string') ? setting[f] : '';
            const cur = (api[f] != null) ? String(api[f]) : '';
            if (v !== cur) postMsg({ action: 'updateApiField', apiName: effectiveName, field: f, value: v });
        });
        const intFields = ['maxToken', 'contextCapacity'];
        intFields.forEach(f => {
            if (typeof setting[f] === 'number' && setting[f] !== (api[f] || 0))
                postMsg({ action: 'updateApiField', apiName: effectiveName, field: f, value: setting[f] });
        });
        const floatFields = ['priceInputToken', 'priceOutputToken', 'priceCacheRead', 'priceCacheWrite', 'temperature'];
        floatFields.forEach(f => {
            if (typeof setting[f] === 'number' && setting[f] !== (api[f] || 0))
                postMsg({ action: 'updateApiField', apiName: effectiveName, field: f, value: setting[f] });
        });
        if (typeof setting.enable === 'boolean' && setting.enable !== api.enable)
            postMsg({ action: 'updateApiField', apiName: effectiveName, field: 'enable', value: setting.enable });
        if (Array.isArray(setting.tools))
            postMsg({ action: 'updateApiField', apiName: effectiveName, field: 'tools', value: setting.tools });
        if (setting.openRouterOptions && typeof setting.openRouterOptions === 'object') {
            if (typeof setting.openRouterOptions.disableReasoning === 'boolean' &&
                setting.openRouterOptions.disableReasoning !== (api.openRouterOptions && api.openRouterOptions.disableReasoning))
                postMsg({ action: 'updateApiField', apiName: effectiveName, field: 'disableReasoning', value: setting.openRouterOptions.disableReasoning });
            if (Array.isArray(setting.openRouterOptions.only))
                postMsg({ action: 'updateApiField', apiName: effectiveName, field: 'openRouterOnly', value: setting.openRouterOptions.only });
        }

        apiPasteBtn.classList.add('pasted');
        setTimeout(() => apiPasteBtn.classList.remove('pasted'), 1500);
        return;
    }

    // 处理已有 Provider 的粘贴
    const pending = window._pendingPaste;
    if (!pending) return;
    window._pendingPaste = null;

    const { provider, pasteBtn, providers } = pending;
    let setting;
    try {
        setting = JSON.parse(text);
    } catch (_) {
        return;
    }
    if (!setting || typeof setting.name !== 'string' || !setting.name.trim()) {
        return;
    }
    const newName = setting.name.trim();
    const newEndpoint = (typeof setting.endpoint === 'string') ? setting.endpoint.trim() : '';
    const newFormat = (typeof setting.format === 'string') ? setting.format.trim() : '';
    const newStoreResponses = (typeof setting.storeResponses === 'boolean') ? setting.storeResponses : undefined;

    // 更新 name（重复则跳过，不弹框）
    let effectiveName = provider.name;
    if (newName !== provider.name) {
        const duplicate = providers.find(p => p.name === newName);
        if (!duplicate) {
            postMsg({ action: 'updateProviderName', oldName: provider.name, newName: newName });
            effectiveName = newName;
        }
    }
    // 更新 endpoint
    if (newEndpoint !== (provider.endpoint || '')) {
        postMsg({ action: 'updateProviderEndpoint', providerName: effectiveName, endpoint: newEndpoint });
    }
    // 更新 format
    if (newFormat !== (provider.format || '')) {
        postMsg({ action: 'updateProviderFormat', providerName: effectiveName, format: newFormat });
    }
    // 更新 storeResponses
    if (newStoreResponses !== undefined && newStoreResponses !== (provider.storeResponses || false)) {
        postMsg({ action: 'updateProviderStoreResponses', providerName: effectiveName, storeResponses: newStoreResponses });
    }

    pasteBtn.classList.add('pasted');
    setTimeout(() => pasteBtn.classList.remove('pasted'), 1500);
};

// 辅助：绑定内联编辑输入框（失焦/Enter提交，Escape还原）
function bindInlineInput(input, onCommit) {
    let lastValue = input.value;
    input.addEventListener('focus', function() { lastValue = this.value; this.select(); });
    input.addEventListener('blur', function() { onCommit(this.value, lastValue); });
    input.addEventListener('keydown', function(e) {
        if (e.key === 'Enter') { e.preventDefault(); this.blur(); }
        else if (e.key === 'Escape') { this.value = lastValue; this.blur(); }
    });
    input.addEventListener('click', e => e.stopPropagation());
    input.addEventListener('mousedown', e => e.stopPropagation());
}

// 辅助：绑定普通参数输入框（失焦提交数值/字符串）
function bindParamInput(input, apiName, field, isNumber) {
    let last = input.value;
    input.addEventListener('focus', function() { last = this.value; });
    input.addEventListener('blur', function() {
        let val = this.value.trim();
        if (val === last) return;
        postMsg({ action: 'updateApiField', apiName, field, value: isNumber ? (parseFloat(val) || 0) : val });
    });
    input.addEventListener('keydown', function(e) {
        if (e.key === 'Enter') { e.preventDefault(); this.blur(); }
        else if (e.key === 'Escape') { this.value = last; this.blur(); }
    });
}

// 辅助：绑定浮点数输入框（显示两位小数，无上下箭头）
// min/max 可选，超出范围自动 clamp
function bindFloatInput(input, apiName, field, min, max) {
    let lastValue = input.value;
    input.addEventListener('focus', function() { 
        lastValue = this.value; 
        this.select(); 
    });
    input.addEventListener('blur', function() {
        let val = this.value.trim();
        // 格式化为两位小数显示
        let numVal = parseFloat(val);
        if (isNaN(numVal)) numVal = 0;
        if (typeof min === 'number') numVal = Math.max(min, numVal);
        if (typeof max === 'number') numVal = Math.min(max, numVal);
        this.value = numVal.toFixed(2);
        if (val !== lastValue) {
            postMsg({ action: 'updateApiField', apiName, field, value: numVal });
        }
    });
    input.addEventListener('keydown', function(e) {
        if (e.key === 'Enter') { e.preventDefault(); this.blur(); }
        else if (e.key === 'Escape') { this.value = lastValue; this.blur(); }
    });
}

// 构建 API 参数面板
function buildApiParams(api) {
    const panel = document.createElement('div');
    panel.className = 'api-params';

    // 辅助：创建一行参数
    function makeRow(label, el, fullWidth) {
        const row = document.createElement('div');
        row.className = 'param-row' + (fullWidth ? ' full-width' : '');
        const lbl = document.createElement('div');
        lbl.className = 'param-label';
        lbl.textContent = label;
        row.appendChild(lbl);
        row.appendChild(el);
        return row;
    }

    // model
    const modelInput = document.createElement('input');
    modelInput.className = 'param-input';
    modelInput.value = api.model || '';
    modelInput.placeholder = 'model id';
    bindParamInput(modelInput, api.name, 'model', false);
    panel.appendChild(makeRow('Model', modelInput));

    // maxToken
    const maxTokenInput = document.createElement('input');
    maxTokenInput.className = 'param-input';
    maxTokenInput.type = 'number';
    maxTokenInput.value = api.maxToken || 0;
    bindParamInput(maxTokenInput, api.name, 'maxToken', true);
    panel.appendChild(makeRow('Max Output Token', maxTokenInput));

    // priceInputToken（浮点数，显示两位小数）
    const priceInInput = document.createElement('input');
    priceInInput.className = 'param-input';
    priceInInput.type = 'text';
    priceInInput.value = (api.priceInputToken || 0).toFixed(2);
    bindFloatInput(priceInInput, api.name, 'priceInputToken');
    panel.appendChild(makeRow('Price/Input Token', priceInInput));

    // priceOutputToken（浮点数，显示两位小数）
    const priceOutInput = document.createElement('input');
    priceOutInput.className = 'param-input';
    priceOutInput.type = 'text';
    priceOutInput.value = (api.priceOutputToken || 0).toFixed(2);
    bindFloatInput(priceOutInput, api.name, 'priceOutputToken');
    panel.appendChild(makeRow('Price/Output Token', priceOutInput));

    // priceCacheRead（浮点数，显示两位小数）
    const priceCRInput = document.createElement('input');
    priceCRInput.className = 'param-input';
    priceCRInput.type = 'text';
    priceCRInput.value = (api.priceCacheRead || 0).toFixed(2);
    bindFloatInput(priceCRInput, api.name, 'priceCacheRead');
    panel.appendChild(makeRow('Price/Cache Read', priceCRInput));

    // priceCacheWrite（浮点数，显示两位小数）
    const priceCWInput = document.createElement('input');
    priceCWInput.className = 'param-input';
    priceCWInput.type = 'text';
    priceCWInput.value = (api.priceCacheWrite || 0).toFixed(2);
    bindFloatInput(priceCWInput, api.name, 'priceCacheWrite');
    panel.appendChild(makeRow('Price/Cache Write', priceCWInput));

    // temperature（浮点数，显示两位小数）
    const tempInput = document.createElement('input');
    tempInput.className = 'param-input';
    tempInput.type = 'text';
    tempInput.value = (api.temperature || 0).toFixed(2);
    bindFloatInput(tempInput, api.name, 'temperature', 0, 1);
    panel.appendChild(makeRow('Temperature', tempInput));

    // thinkingMode
    const thinkSel = document.createElement('select');
    thinkSel.className = 'param-select';
    ['Auto','Enable','Disable'].forEach(v => {
        const opt = document.createElement('option');
        opt.value = v; opt.textContent = v;
        if (api.thinkingMode === v) opt.selected = true;
        thinkSel.appendChild(opt);
    });
    thinkSel.addEventListener('change', function() {
        postMsg({ action: 'updateApiField', apiName: api.name, field: 'thinkingMode', value: this.value });
    });
    panel.appendChild(makeRow('Thinking Mode', thinkSel));

    // cacheControl
    const cacheSel = document.createElement('select');
    cacheSel.className = 'param-select';
    ['Auto','None','Anthropic'].forEach(v => {
        const opt = document.createElement('option');
        opt.value = v; opt.textContent = v;
        if (api.cacheControl === v) opt.selected = true;
        cacheSel.appendChild(opt);
    });
    cacheSel.addEventListener('change', function() {
        postMsg({ action: 'updateApiField', apiName: api.name, field: 'cacheControl', value: this.value });
    });
    panel.appendChild(makeRow('Cache Control', cacheSel));

    // role combo box
    const roleSel = document.createElement('select');
    roleSel.className = 'param-select';
    ['Agent','Auxiliary','Embedding'].forEach(v => {
        const opt = document.createElement('option');
        opt.value = v; opt.textContent = v;
        if (api.role === v) opt.selected = true;
        roleSel.appendChild(opt);
    });
    roleSel.addEventListener('change', function() {
        postMsg({ action: 'updateApiField', apiName: api.name, field: 'role', value: this.value });
    });
    panel.appendChild(makeRow('Role', roleSel));

    return panel;
}

// 设置Provider数据（全量重建，保持展开状态）
function setProviderData(providers) {
    const providersContainer = document.getElementById('providers-list');
    if (!providersContainer) return;

    providersContainer.innerHTML = '';

    if (!providers || providers.length === 0) {
        const emptyDiv = document.createElement('div');
        emptyDiv.className = 'providers-empty';
        emptyDiv.innerHTML = `
            <div class="empty-state-icon">🔌</div>
            <div class="empty-state-text">No Providers Available</div>
            <div class="empty-state-subtext">No AI service providers found.</div>
        `;
        providersContainer.appendChild(emptyDiv);
        return;
    }

    providers.forEach((provider, index) => {
        const providerDiv = document.createElement('div');
        providerDiv.className = 'provider-item';

        const isAvailable = provider.isAvailable;
        const statusClass = isAvailable ? 'available' : 'unavailable';
        const isProviderExpanded = expandedProviders.includes(provider.name);

        // ── Provider 标题栏（折叠/展开均可点击）──
        const header = document.createElement('div');
        header.className = 'provider-header';
        const headerId = 'provider-header-' + index;
        header.id = headerId;
        header.style.cursor = 'pointer';
        header.title = isProviderExpanded ? 'Click to collapse' : 'Click to expand';
        header.addEventListener('click', () => {
            const rectBefore = header.getBoundingClientRect();
            const topBefore = rectBefore.top;

            if (isProviderExpanded) {
                expandedProviders = expandedProviders.filter(p => p !== provider.name);
            } else {
                expandedProviders.push(provider.name);
            }
            setProviderData(providers);

            const newHeader = document.getElementById(headerId);
            if (newHeader) {
                const rectAfter = newHeader.getBoundingClientRect();
                const diff = rectAfter.top - topBefore;
                if (diff !== 0) {
                    const scrollContainer = document.getElementById('providers-list');
                    if (scrollContainer) {
                        scrollContainer.scrollTop += diff;
                    }
                }
            }
        });

        // ── Drag & Drop 排序 ──
        header.draggable = true;

        header.addEventListener('dragstart', (e) => {
            e.dataTransfer.setData('text/plain', provider.name);
            e.dataTransfer.effectAllowed = 'move';
            providerDiv.classList.add('dragging');
        });

        header.addEventListener('dragend', () => {
            providerDiv.classList.remove('dragging');
            providersContainer.querySelectorAll('.provider-item').forEach(el => {
                el.classList.remove('drag-before', 'drag-after');
            });
        });

        providerDiv.addEventListener('dragover', (e) => {
            e.preventDefault();
            e.dataTransfer.dropEffect = 'move';
            const rect = providerDiv.getBoundingClientRect();
            const midY = rect.top + rect.height / 2;
            providerDiv.classList.remove('drag-before', 'drag-after');
            if (e.clientY < midY) {
                providerDiv.classList.add('drag-before');
            } else {
                providerDiv.classList.add('drag-after');
            }
        });

        providerDiv.addEventListener('dragleave', () => {
            providerDiv.classList.remove('drag-before', 'drag-after');
        });

        providerDiv.addEventListener('drop', (e) => {
            e.preventDefault();
            providerDiv.classList.remove('drag-before', 'drag-after');
            const draggedName = e.dataTransfer.getData('text/plain');
            if (!draggedName || draggedName === provider.name) return;

            const rect = providerDiv.getBoundingClientRect();
            const midY = rect.top + rect.height / 2;

            // 在前端本地重排 DOM
            const items = [...providersContainer.querySelectorAll('.provider-item')];
            const draggedItem = items.find(el => {
                const nameEl = el.querySelector('.provider-name');
                return nameEl && nameEl.textContent === draggedName;
            });
            if (!draggedItem || draggedItem === providerDiv) return;

            if (e.clientY < midY) {
                providersContainer.insertBefore(draggedItem, providerDiv);
            } else {
                if (providerDiv.nextSibling) {
                    providersContainer.insertBefore(draggedItem, providerDiv.nextSibling);
                } else {
                    providersContainer.appendChild(draggedItem);
                }
            }

            // 发送新顺序到 C++
            const orderedNames = [];
            providersContainer.querySelectorAll('.provider-item .provider-name').forEach(span => {
                orderedNames.push(span.textContent);
            });
            postMsg({ action: 'reorderProviders', orderedNames: orderedNames });
        });

        const nameSection = document.createElement('div');
        nameSection.className = 'provider-name-section';

        // 状态点
        const dot = document.createElement('div');
        dot.className = `provider-status-dot ${statusClass}`;
        dot.setAttribute('data-provider-type', provider.type);
        nameSection.appendChild(dot);

        // 名称（始终静态，点击标题栏折叠/展开）
        const nameSpan = document.createElement('span');
        nameSpan.className = 'provider-name';
        nameSpan.textContent = provider.name;
        nameSection.appendChild(nameSpan);
        header.appendChild(nameSection);

        // ── 右侧按钮组（copy / paste / delete）──
        const providerActionsContainer = document.createElement('div');
        providerActionsContainer.style.cssText = 'display:flex;align-items:center;gap:2px;flex-shrink:0;';

        // ── Copy 按钮 ──
        const copyBtn = document.createElement('div');
        copyBtn.className = 'copy-provider-btn';
        copyBtn.title = 'Copy settings as JSON';
        copyBtn.innerHTML = `
            <svg viewBox="0 0 24 24">
                <rect x="9" y="9" width="13" height="13" rx="2" ry="2"/>
                <path d="M5 15H4a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2h9a2 2 0 0 1 2 2v1"/>
            </svg>
        `;
        copyBtn.addEventListener('click', (e) => {
            e.stopPropagation();
            const setting = {
                name: provider.name,
                endpoint: provider.endpoint,
                format: provider.format,
                storeResponses: provider.storeResponses
            };
            const json = JSON.stringify(setting, null, 2);
            navigator.clipboard.writeText(json).then(() => {
                copyBtn.classList.add('copied');
                setTimeout(() => copyBtn.classList.remove('copied'), 1500);
            }).catch(err => {
                console.error('Copy failed:', err);
            });
        });
        providerActionsContainer.appendChild(copyBtn);

        // ── Paste 按钮 ──
        const pasteBtn = document.createElement('div');
        pasteBtn.className = 'paste-provider-btn';
        pasteBtn.title = 'Paste settings from clipboard';
        pasteBtn.innerHTML = `
            <svg viewBox="0 0 24 24">
                <path d="M9 5H7a2 2 0 0 0-2 2v12a2 2 0 0 0 2 2h10a2 2 0 0 0 2-2V7a2 2 0 0 0-2-2h-2"/>
                <rect x="9" y="3" width="6" height="4" rx="1" ry="1"/>
                <line x1="9" y1="12" x2="15" y2="12"/>
                <line x1="9" y1="16" x2="15" y2="16"/>
            </svg>
        `;
        pasteBtn.addEventListener('click', (e) => {
            e.stopPropagation();
            window._pendingPaste = { provider, pasteBtn, providers };
            postMsg({ action: 'readClipboard' });
        });
        providerActionsContainer.appendChild(pasteBtn);

        // ── 删除按钮（长按3秒）──
        const deleteBtn = document.createElement('div');
        deleteBtn.className = 'delete-provider-btn';
        deleteBtn.title = 'Hold for 3 seconds to delete';
        
        // 进度环SVG
        deleteBtn.innerHTML = `
            <svg class="progress-ring" viewBox="0 0 26 26">
                <circle class="progress-bg" cx="13" cy="13" r="11.5"/>
                <circle class="progress-fill" cx="13" cy="13" r="11.5"/>
            </svg>
        `;
        
        let longPressTimer = null;
        let isDeleting = false;
        
        const startLongPress = (e) => {
            if (e.button !== 0) return; // 只响应左键
            e.stopPropagation();
            isDeleting = false;
            deleteBtn.classList.add('deleting');
            
            longPressTimer = setTimeout(() => {
                isDeleting = true;
                deleteBtn.classList.remove('deleting');
                // 发送删除消息
                postMsg({ action: 'deleteProvider', name: provider.name });
            }, 3000);
        };
        
        const cancelLongPress = (e) => {
            if (longPressTimer) {
                clearTimeout(longPressTimer);
                longPressTimer = null;
            }
            if (!isDeleting) {
                deleteBtn.classList.remove('deleting');
            }
        };
        
        deleteBtn.addEventListener('mousedown', startLongPress);
        deleteBtn.addEventListener('mouseup', cancelLongPress);
        deleteBtn.addEventListener('mouseleave', cancelLongPress);
        deleteBtn.addEventListener('touchstart', (e) => {
            e.preventDefault();
            startLongPress(e.touches[0]);
        });
        deleteBtn.addEventListener('touchend', cancelLongPress);
        deleteBtn.addEventListener('touchcancel', cancelLongPress);
        deleteBtn.addEventListener('click', (e) => {
            e.stopPropagation();
        });
        
        providerActionsContainer.appendChild(deleteBtn);
        header.appendChild(providerActionsContainer);
        providerDiv.appendChild(header);

        // ── 展开状态：显示详细内容（暗色背景区域）──
        if (isProviderExpanded) {
            const expandedArea = document.createElement('div');
            expandedArea.className = 'provider-expanded-area';

            // Name行
            const nameRow = document.createElement('div');
            nameRow.className = 'provider-edit-row';
            const nameLabel = document.createElement('span');
            nameLabel.className = 'provider-edit-label';
            nameLabel.textContent = 'Name:';
            nameRow.appendChild(nameLabel);
            const nameInput = document.createElement('input');
            nameInput.className = 'provider-edit-input';
            nameInput.value = provider.name;
            bindInlineInput(nameInput, (newVal, oldVal) => {
                newVal = newVal.trim();
                if (newVal && newVal !== oldVal) {
                    // 检查是否与其他 provider 重名
                    const duplicateProvider = providers.find(p => p.name === newVal);
                    if (duplicateProvider) {
                        nameInput.value = oldVal;
                        postMsg({ action: 'showError', message: 'Provider name "' + newVal + '" already exists' });
                        return;
                    }
                    expandedProviders = expandedProviders.map(p => p === oldVal ? newVal : p);
                    postMsg({ action: 'updateProviderName', oldName: oldVal, newName: newVal });
                } else {
                    nameInput.value = oldVal;
                }
            });
            nameInput.addEventListener('click', e => e.stopPropagation());
            nameInput.addEventListener('mousedown', e => e.stopPropagation());
            nameRow.appendChild(nameInput);
            expandedArea.appendChild(nameRow);

            // Endpoint行
            const endpointRow = document.createElement('div');
            endpointRow.className = 'provider-edit-row';
            const endpointLabel = document.createElement('span');
            endpointLabel.className = 'provider-edit-label';
            endpointLabel.textContent = 'Endpoint:';
            endpointRow.appendChild(endpointLabel);
            const endpointInput = document.createElement('input');
            endpointInput.className = 'provider-edit-input';
            endpointInput.value = provider.endpoint || '';
            endpointInput.placeholder = 'https://api.example.com/v1';
            let lastEndpoint = endpointInput.value;
            endpointInput.addEventListener('focus', function() { lastEndpoint = this.value; this.select(); });
            endpointInput.addEventListener('blur', function() {
                const v = this.value.trim();
                postMsg({ action: 'updateProviderEndpoint', providerName: provider.name, endpoint: v });
            });
            endpointInput.addEventListener('keydown', function(e) {
                if (e.key === 'Enter') { e.preventDefault(); this.blur(); }
                else if (e.key === 'Escape') { this.value = lastEndpoint; this.blur(); }
            });
            endpointInput.addEventListener('click', e => e.stopPropagation());
            endpointInput.addEventListener('mousedown', e => e.stopPropagation());
            endpointRow.appendChild(endpointInput);
            expandedArea.appendChild(endpointRow);

            // Format行（下拉选择）
            const formatRow = document.createElement('div');
            formatRow.className = 'provider-edit-row';
            const formatLabel = document.createElement('span');
            formatLabel.className = 'provider-edit-label';
            formatLabel.textContent = 'Format:';
            formatRow.appendChild(formatLabel);
            const formatSelect = document.createElement('select');
            formatSelect.className = 'provider-edit-input';
            formatSelect.style.cursor = 'pointer';
            const formatOptions = ['OpenAI', 'Anthropic', 'Gemini', 'OpenRouter', 'Kimi', 'GLM', 'Minimax', 'DeepSeek', 'OpenAIResponses'];
            formatOptions.forEach(v => {
                const opt = document.createElement('option');
                opt.value = v;
                opt.textContent = v;
                if (provider.format === v) opt.selected = true;
                formatSelect.appendChild(opt);
            });
            formatSelect.addEventListener('change', function() {
                postMsg({ action: 'updateProviderFormat', providerName: provider.name, format: this.value });
                // 当 format 变化时，刷新以显示/隐藏 storeResponses 行
                setProviderData(providers);
            });
            formatSelect.addEventListener('click', e => e.stopPropagation());
            formatSelect.addEventListener('mousedown', e => e.stopPropagation());
            formatRow.appendChild(formatSelect);
            expandedArea.appendChild(formatRow);

            // Store Responses 开关行（仅 OpenAIResponses 格式显示）
            if (provider.format === 'OpenAIResponses') {
                const srRow = document.createElement('div');
                srRow.className = 'provider-edit-row';
                const srToggle = document.createElement('label');
                srToggle.style.cssText = 'display:flex;align-items:center;cursor:pointer;gap:6px;margin-left:71px;font-size:0.78em;color:#888;';
                const srCheckbox = document.createElement('input');
                srCheckbox.type = 'checkbox';
                srCheckbox.checked = provider.storeResponses === true;
                srCheckbox.style.cssText = 'width:13px;height:13px;cursor:pointer;flex-shrink:0;';
                srCheckbox.addEventListener('click', e => e.stopPropagation());
                srCheckbox.addEventListener('change', function() {
                    postMsg({ action: 'updateProviderStoreResponses', providerName: provider.name, storeResponses: this.checked });
                });
                srToggle.appendChild(srCheckbox);
                srToggle.appendChild(document.createTextNode('Store responses on server'));
                srToggle.addEventListener('click', e => e.stopPropagation());
                srToggle.addEventListener('mousedown', e => e.stopPropagation());
                srRow.appendChild(srToggle);
                expandedArea.appendChild(srRow);
            }

            // Api Key行
            const keyRow = document.createElement('div');
            keyRow.className = 'provider-edit-row';
            const keyLabel = document.createElement('span');
            keyLabel.className = 'provider-edit-label';
            keyLabel.textContent = 'Api Key:';
            keyRow.appendChild(keyLabel);
            const keyInput = document.createElement('input');
            keyInput.type = 'password';
            keyInput.className = 'provider-edit-input';
            keyInput.placeholder = 'Enter API key...';
            keyInput.value = provider.key || '';
            keyInput.setAttribute('data-provider-type', provider.type);
            let lastKey = keyInput.value;
            keyInput.addEventListener('focus', function() { lastKey = this.value; this.select(); });
            keyInput.addEventListener('blur', function() {
                postMsg({ action: 'updateProviderKey', providerType: provider.type, key: this.value });
                lastKey = this.value;
            });
            keyInput.addEventListener('keydown', function(e) {
                if (e.key === 'Enter') { e.preventDefault(); this.blur(); }
                else if (e.key === 'Escape') { this.value = lastKey; this.blur(); }
            });
            keyInput.addEventListener('dblclick', function(e) {
                e.stopPropagation();
                this.type = this.type === 'password' ? 'text' : 'password';
            });
            keyInput.addEventListener('click', e => e.stopPropagation());
            keyInput.addEventListener('mousedown', e => e.stopPropagation());
            keyRow.appendChild(keyInput);
            expandedArea.appendChild(keyRow);

            // API 列表（始终显示）
            const apisSection = document.createElement('div');
            apisSection.className = 'provider-apis-section';
            
            // 标题行（APIs + 添加按钮）
            const apisHeaderRow = document.createElement('div');
            apisHeaderRow.className = 'apis-header-row';
            
            const apisLabel = document.createElement('div');
            apisLabel.className = 'provider-apis-label';
            apisLabel.textContent = 'APIs';
            apisHeaderRow.appendChild(apisLabel);
            
            // 添加 API 按钮
            const addApiBtn = document.createElement('div');
            addApiBtn.className = 'add-api-btn';
            addApiBtn.title = 'Add new API';
            addApiBtn.addEventListener('click', (e) => {
                e.stopPropagation();
                // 生成不重名的 API 名称
                let newName = 'NewApi';
                let suffix = 1;
                const existingNames = [];
                providers.forEach(p => {
                    if (p.apis) p.apis.forEach(a => existingNames.push(a.name));
                });
                while (existingNames.includes(newName)) {
                    newName = 'NewApi' + suffix;
                    suffix++;
                }
                // 自动展开新API并标记待聚焦
                if (!expandedApis.includes(newName)) {
                    expandedApis.push(newName);
                }
                pendingFocusApiName = newName;
                postMsg({ action: 'addApi', providerName: provider.name, apiName: newName });
            });
            apisHeaderRow.appendChild(addApiBtn);

            // API Paste 按钮（剪贴板新建）
            const addApiPasteBtn = document.createElement('div');
            addApiPasteBtn.className = 'add-api-paste-btn';
            addApiPasteBtn.title = 'Paste new API from clipboard';
            addApiPasteBtn.innerHTML = `
                <svg viewBox="0 0 24 24">
                    <path d="M9 5H7a2 2 0 0 0-2 2v12a2 2 0 0 0 2 2h10a2 2 0 0 0 2-2V7a2 2 0 0 0-2-2h-2"/>
                    <rect x="9" y="3" width="6" height="4" rx="1" ry="1"/>
                    <line x1="9" y1="12" x2="15" y2="12"/>
                    <line x1="9" y1="16" x2="15" y2="16"/>
                </svg>
            `;
            addApiPasteBtn.addEventListener('click', (e) => {
                e.stopPropagation();
                window._pendingPasteNewApi = { provider, providers };
                postMsg({ action: 'readClipboard' });
            });
            apisHeaderRow.appendChild(addApiPasteBtn);
            
            apisSection.appendChild(apisHeaderRow);

            if (!provider.apis || provider.apis.length === 0) {
                // 没有 API 时显示 <none>
                const noneDiv = document.createElement('div');
                noneDiv.className = 'apis-none';
                noneDiv.textContent = '<none>';
                apisSection.appendChild(noneDiv);
            } else {
                // 有 API 时显示列表
                provider.apis.forEach((api, apiIndex) => {
                    const isApiExpanded = expandedApis.includes(api.name);

                    const apiItem = document.createElement('div');
                    apiItem.className = 'api-item';

                    // API 标题栏（单击展开/折叠）
                    const apiHeader = document.createElement('div');
                    apiHeader.className = 'api-header-bar';
                    const apiHeaderId = 'api-header-' + index + '-' + apiIndex;
                    apiHeader.id = apiHeaderId;
                    apiHeader.title = isApiExpanded ? 'Click to collapse' : 'Click to expand';
                    apiHeader.addEventListener('click', () => {
                        const rectBefore = apiHeader.getBoundingClientRect();
                        const topBefore = rectBefore.top;

                        if (isApiExpanded) {
                            expandedApis = expandedApis.filter(a => a !== api.name);
                        } else {
                            expandedApis.push(api.name);
                        }
                        setProviderData(providers);

                        const newApiHeader = document.getElementById(apiHeaderId);
                        if (newApiHeader) {
                            const rectAfter = newApiHeader.getBoundingClientRect();
                            const diff = rectAfter.top - topBefore;
                            if (diff !== 0) {
                                const scrollContainer = document.getElementById('providers-list');
                                if (scrollContainer) {
                                    scrollContainer.scrollTop += diff;
                                }
                            }
                        }
                    });

                    // API 名称容器（包含 checkbox 和名称文本）
                    const apiNameContainer = document.createElement('div');
                    apiNameContainer.className = 'api-name-container';

                    // API Enable Checkbox - 自定义黑白样式
                    const apiEnableCheckbox = document.createElement('input');
                    apiEnableCheckbox.type = 'checkbox';
                    apiEnableCheckbox.checked = api.enable !== false; // 默认为true
                    apiEnableCheckbox.className = 'api-enable-checkbox';
                    apiEnableCheckbox.title = 'Enable/Disable this API';
                    apiEnableCheckbox.addEventListener('click', (e) => {
                        e.stopPropagation(); // 阻止触发折叠/展开
                    });
                    apiEnableCheckbox.addEventListener('change', (e) => {
                        postMsg({ action: 'updateApiField', apiName: api.name, field: 'enable', value: e.target.checked });
                    });
                    apiNameContainer.appendChild(apiEnableCheckbox);

                    // API 名称文本
                    const apiNameSpan = document.createElement('span');
                    apiNameSpan.style.cssText = 'font-size:0.9em;font-weight:500;color:#ffffff;text-shadow:0 1px 2px rgba(0,0,0,0.5);background:transparent;padding:0;line-height:28px;';
                    apiNameSpan.textContent = api.name;
                    apiNameContainer.appendChild(apiNameSpan);

                    apiHeader.appendChild(apiNameContainer);

                    // ── 右侧按钮组（copy / paste / delete）──
                    const apiActionsContainer = document.createElement('div');
                    apiActionsContainer.style.cssText = 'display:flex;align-items:center;gap:2px;flex-shrink:0;';

                    // API Copy 按钮
                    const apiCopyBtn = document.createElement('div');
                    apiCopyBtn.className = 'copy-provider-btn';
                    apiCopyBtn.title = 'Copy API settings as JSON';
                    apiCopyBtn.innerHTML = `
                        <svg viewBox="0 0 24 24">
                            <rect x="9" y="9" width="13" height="13" rx="2" ry="2"/>
                            <path d="M5 15H4a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2h9a2 2 0 0 1 2 2v1"/>
                        </svg>
                    `;
                    apiCopyBtn.addEventListener('click', (e) => {
                        e.stopPropagation();
                        const setting = {
                            name: api.name,
                            model: api.model,
                            maxToken: api.maxToken,
                            contextCapacity: api.contextCapacity,
                            priceInputToken: api.priceInputToken,
                            priceOutputToken: api.priceOutputToken,
                            priceCacheRead: api.priceCacheRead,
                            priceCacheWrite: api.priceCacheWrite,
                            temperature: api.temperature,
                            thinkingMode: api.thinkingMode,
                            cacheControl: api.cacheControl,
                            role: api.role,
                            enable: api.enable,
                            openRouterOptions: api.openRouterOptions
                        };
                        navigator.clipboard.writeText(JSON.stringify(setting, null, 2)).then(() => {
                            apiCopyBtn.classList.add('copied');
                            setTimeout(() => apiCopyBtn.classList.remove('copied'), 1500);
                        }).catch(err => { console.error('Copy failed:', err); });
                    });
                    apiActionsContainer.appendChild(apiCopyBtn);

                    // API Paste 按钮
                    const apiPasteBtn = document.createElement('div');
                    apiPasteBtn.className = 'paste-provider-btn';
                    apiPasteBtn.title = 'Paste API settings from clipboard';
                    apiPasteBtn.innerHTML = `
                        <svg viewBox="0 0 24 24">
                            <path d="M9 5H7a2 2 0 0 0-2 2v12a2 2 0 0 0 2 2h10a2 2 0 0 0 2-2V7a2 2 0 0 0-2-2h-2"/>
                            <rect x="9" y="3" width="6" height="4" rx="1" ry="1"/>
                            <line x1="9" y1="12" x2="15" y2="12"/>
                            <line x1="9" y1="16" x2="15" y2="16"/>
                        </svg>
                    `;
                    apiPasteBtn.addEventListener('click', (e) => {
                        e.stopPropagation();
                        window._pendingPasteApi = { api, apiPasteBtn, provider, providers };
                        postMsg({ action: 'readClipboard' });
                    });
                    apiActionsContainer.appendChild(apiPasteBtn);

                    // API 删除按钮（长按3秒）
                    const deleteApiBtn = document.createElement('div');
                    deleteApiBtn.className = 'delete-provider-btn';
                    deleteApiBtn.title = 'Hold for 3 seconds to delete';
                    deleteApiBtn.innerHTML = `
                        <svg class="progress-ring" viewBox="0 0 26 26">
                            <circle class="progress-bg" cx="13" cy="13" r="11.5"/>
                            <circle class="progress-fill" cx="13" cy="13" r="11.5"/>
                        </svg>
                    `;
                    
                    let apiLongPressTimer = null;
                    let isApiDeleting = false;
                    
                    const startApiLongPress = (e) => {
                        if (e.button !== 0) return;
                        e.stopPropagation();
                        isApiDeleting = false;
                        deleteApiBtn.classList.add('deleting');
                        
                        apiLongPressTimer = setTimeout(() => {
                            isApiDeleting = true;
                            deleteApiBtn.classList.remove('deleting');
                            postMsg({ action: 'deleteApi', name: api.name });
                        }, 3000);
                    };
                    
                    const cancelApiLongPress = (e) => {
                        if (apiLongPressTimer) {
                            clearTimeout(apiLongPressTimer);
                            apiLongPressTimer = null;
                        }
                        if (!isApiDeleting) {
                            deleteApiBtn.classList.remove('deleting');
                        }
                    };
                    
                    deleteApiBtn.addEventListener('mousedown', startApiLongPress);
                    deleteApiBtn.addEventListener('mouseup', cancelApiLongPress);
                    deleteApiBtn.addEventListener('mouseleave', cancelApiLongPress);
                    deleteApiBtn.addEventListener('touchstart', (e) => {
                        e.preventDefault();
                        startApiLongPress(e.touches[0]);
                    });
                    deleteApiBtn.addEventListener('touchend', cancelApiLongPress);
                    deleteApiBtn.addEventListener('touchcancel', cancelApiLongPress);
                    deleteApiBtn.addEventListener('click', (e) => {
                        e.stopPropagation();
                    });
                    
                    apiActionsContainer.appendChild(deleteApiBtn);
                    apiHeader.appendChild(apiActionsContainer);

                    apiItem.appendChild(apiHeader);

                    // API 展开区域（暗色背景）
                    if (isApiExpanded) {
                        const apiExpandedArea = document.createElement('div');
                        apiExpandedArea.className = 'api-expanded-area';

                        // Name编辑行
                        const apiNameRow = document.createElement('div');
                        apiNameRow.className = 'provider-edit-row';
                        apiNameRow.style.height = '28px';
                        const apiNameLabel = document.createElement('span');
                        apiNameLabel.className = 'provider-edit-label';
                        apiNameLabel.style.minWidth = '50px';
                        apiNameLabel.textContent = 'Name:';
                        apiNameRow.appendChild(apiNameLabel);
                        const apiNameInput = document.createElement('input');
                        apiNameInput.className = 'provider-edit-input';
                        apiNameInput.style.fontSize = '0.82em';
                        apiNameInput.value = api.name;
                        bindInlineInput(apiNameInput, (newVal, oldVal) => {
                            newVal = newVal.trim();
                            if (newVal && newVal !== oldVal) {
                                // 检查是否与所有 provider 下的所有 api 重名
                                let duplicateApi = null;
                                for (const p of providers) {
                                    if (p.apis) {
                                        const found = p.apis.find(a => a.name === newVal);
                                        if (found) {
                                            duplicateApi = found;
                                            break;
                                        }
                                    }
                                }
                                if (duplicateApi) {
                                    apiNameInput.value = oldVal;
                                    postMsg({ action: 'showError', message: 'API name "' + newVal + '" already exists' });
                                    return;
                                }
                                expandedApis = expandedApis.map(a => a === oldVal ? newVal : a);
                                postMsg({ action: 'updateApiName', oldName: oldVal, newName: newVal });
                            } else {
                                apiNameInput.value = oldVal;
                            }
                        });
                        apiNameInput.addEventListener('click', e => e.stopPropagation());
                        apiNameInput.addEventListener('mousedown', e => e.stopPropagation());
                        apiNameRow.appendChild(apiNameInput);
                        apiExpandedArea.appendChild(apiNameRow);

                        // 其他参数面板
                        apiExpandedArea.appendChild(buildApiParams(api));

                        apiItem.appendChild(apiExpandedArea);
                    }

                    apisSection.appendChild(apiItem);
                });
            }

            expandedArea.appendChild(apisSection);

            providerDiv.appendChild(expandedArea);
        }

        providersContainer.appendChild(providerDiv);
    });

    // 添加新 Provider 按钮
    const addButton = document.createElement('div');
    addButton.className = 'add-provider-button';
    addButton.innerHTML = '+';
    addButton.title = 'Add new provider';
    addButton.addEventListener('click', () => {
        // 生成不重名的 Provider 名称
        let newName = 'NewProvider';
        let suffix = 1;
        const existingNames = providers.map(p => p.name);
        while (existingNames.includes(newName)) {
            newName = 'NewProvider' + suffix;
            suffix++;
        }
        // 自动展开新Provider并标记待聚焦
        if (!expandedProviders.includes(newName)) {
            expandedProviders.push(newName);
        }
        pendingFocusProviderName = newName;
        postMsg({ action: 'addProvider', name: newName });
    });

    // 新增按钮行（+ 和 paste 并排）
    const addRow = document.createElement('div');
    addRow.style.cssText = 'display:flex;justify-content:center;align-items:center;gap:4px;margin:12px auto;';
    addRow.appendChild(addButton);

    // 从剪贴板粘贴新建 Provider 按钮
    const addPasteBtn = document.createElement('div');
    addPasteBtn.className = 'add-paste-provider-btn';
    addPasteBtn.title = 'Paste new provider from clipboard';
    addPasteBtn.innerHTML = `
        <svg viewBox="0 0 24 24">
            <path d="M9 5H7a2 2 0 0 0-2 2v12a2 2 0 0 0 2 2h10a2 2 0 0 0 2-2V7a2 2 0 0 0-2-2h-2"/>
            <rect x="9" y="3" width="6" height="4" rx="1" ry="1"/>
            <line x1="9" y1="12" x2="15" y2="12"/>
            <line x1="9" y1="16" x2="15" y2="16"/>
        </svg>
    `;
    addPasteBtn.addEventListener('click', (e) => {
        e.stopPropagation();
        window._pendingPasteNew = { providers: providers };
        postMsg({ action: 'readClipboard' });
    });
    addRow.appendChild(addPasteBtn);
    providersContainer.appendChild(addRow);

    // 新建Provider后自动聚焦名称输入框
    if (pendingFocusProviderName) {
        const focusName = pendingFocusProviderName;
        pendingFocusProviderName = null;
        setTimeout(() => {
            const areas = document.querySelectorAll('.provider-expanded-area');
            for (const area of areas) {
                const nameInput = area.querySelector('.provider-edit-input');
                if (nameInput && nameInput.value === focusName) {
                    nameInput.focus();
                    nameInput.select();
                    nameInput.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
                    break;
                }
            }
        }, 200);
    }

    // 新建API后自动聚焦名称输入框
    if (pendingFocusApiName) {
        const focusName = pendingFocusApiName;
        pendingFocusApiName = null;
        setTimeout(() => {
            const inputs = document.querySelectorAll('.api-expanded-area .provider-edit-row .provider-edit-input');
            for (const input of inputs) {
                if (input.value === focusName) {
                    input.focus();
                    input.select();
                    input.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
                    break;
                }
            }
        }, 200);
    }
}

// 创建 Providers Tab 内容（从 renderTabContents 提取）
function createProvidersTabContent(contentDiv) {
    // Providers tab的特殊内容 - 创建容器结构
    const providersContainer = document.createElement('div');
    providersContainer.className = 'providers-container';
    
    // 创建Cast Sheet面板
    const castSheetPanel = document.createElement('div');
    castSheetPanel.className = 'cast-sheet-panel';
    castSheetPanel.id = 'cast-sheet-panel';
    castSheetPanel.innerHTML = `
        <div class="cast-sheet-title">Cast Sheet</div>
        <div class="cast-sheet-row">
            <span class="cast-sheet-label">Major Chat</span>
            <select class="cast-sheet-select" id="cast-sheet-majorChat" data-api-type="majorChat">
                <option>Loading...</option>
            </select>
        </div>
        <div class="cast-sheet-row">
            <span class="cast-sheet-label">Title Brief</span>
            <select class="cast-sheet-select" id="cast-sheet-brief" data-api-type="brief">
                <option>Loading...</option>
            </select>
        </div>
        <div class="cast-sheet-row">
            <span class="cast-sheet-label">Compress Summarize</span>
            <select class="cast-sheet-select" id="cast-sheet-summarize" data-api-type="summarize">
                <option>Loading...</option>
            </select>
            <button class="cast-sheet-btn" id="evaluate-summarize-btn" title="Evaluate the compression quality of the selected API using the latest conversations in current chat">
                <span class="cast-sheet-btn-text">Evaluate</span>
                <div class="cast-sheet-btn-spinner"></div>
            </button>
        </div>
        <div class="cast-sheet-row">
            <span class="cast-sheet-label">Input Hint</span>
            <select class="cast-sheet-select" id="cast-sheet-inputHint" data-api-type="inputHint">
                <option>Loading...</option>
            </select>
        </div>
        <div class="cast-sheet-row">
            <span class="cast-sheet-label">Embedding</span>
            <select class="cast-sheet-select" id="cast-sheet-embedding" data-api-type="embedding">
                <option>Loading...</option>
            </select>
        </div>
    `;
    providersContainer.appendChild(castSheetPanel);
    
    // 绑定Cast Sheet select的change事件
    castSheetPanel.querySelectorAll('.cast-sheet-select').forEach(select => {
        select.addEventListener('change', function() {
            const apiType = this.getAttribute('data-api-type');
            const apiName = this.value;
            postMsg({ action: 'updateCastSheetApi', apiType: apiType, apiName: apiName });
            
            // 如果是summarize select变化，更新Evaluate按钮状态
            if (apiType === 'summarize') {
                updateEvaluateBtnState();
            }
        });
    });
    
    // 绑定Evaluate按钮的click事件
    const evaluateBtn = castSheetPanel.querySelector('#evaluate-summarize-btn');
    if (evaluateBtn) {
        evaluateBtn.addEventListener('click', function() {
            const select = document.getElementById('cast-sheet-summarize');
            const apiName = select ? select.value : '';
            postMsg({ action: 'evaluateCompressSummarize', apiName: apiName });
        });
    }
    
    // 创建provider列表区域
    const providersListArea = document.createElement('div');
    providersListArea.className = 'providers-list-area';
    providersListArea.id = 'providers-list';
    providersContainer.appendChild(providersListArea);
    
    contentDiv.appendChild(providersContainer);
    
    // 请求Provider数据和Cast Sheet数据
    requestProviderData();
    requestCastSheetData();
}

// 开始验证Provider
function startValidatingProvider(providerType) {
    const statusDot = document.querySelector(`.provider-status-dot[data-provider-type="${providerType}"]`);
    if (statusDot) {
        statusDot.className = 'provider-status-dot validating';
    }
}

// 结束验证Provider
function endValidatingProvider(providerType, available, errorMessage) {
    const statusDot = document.querySelector(`.provider-status-dot[data-provider-type="${providerType}"]`);
    if (statusDot) {
        statusDot.className = `provider-status-dot ${available ? 'available' : 'unavailable'}`;
    }
    
    // 失败时显示 toast 提示
    if (!available && errorMessage) {
        showValidationErrorToast(providerType, errorMessage);
    }
}

let _validationToastTimer = null;

function showValidationErrorToast(providerType, errorMessage) {
    // 移除已有的 toast
    const existing = document.querySelector('.validation-error-toast');
    if (existing) {
        existing.remove();
        if (_validationToastTimer) {
            clearTimeout(_validationToastTimer);
            _validationToastTimer = null;
        }
    }
    
    const toast = document.createElement('div');
    toast.className = 'validation-error-toast';
    toast.innerHTML = 
        '<span class="validation-error-toast-icon">⚠</span>' +
        '<span class="validation-error-toast-text">' +
            '<b>' + escapeHtml(providerType) + ':</b> ' + escapeHtml(errorMessage) +
        '</span>' +
        '<span class="validation-error-toast-close">✕</span>';
    
    // 点击关闭
    toast.querySelector('.validation-error-toast-close').addEventListener('click', function() {
        hideToast(toast);
    });
    
    // 鼠标悬停暂停自动关闭
    toast.addEventListener('mouseenter', function() {
        if (_validationToastTimer) {
            clearTimeout(_validationToastTimer);
            _validationToastTimer = null;
        }
    });
    toast.addEventListener('mouseleave', function() {
        _validationToastTimer = setTimeout(function() { hideToast(toast); }, 3000);
    });
    
    document.body.appendChild(toast);
    
    // 触发入场动画
    requestAnimationFrame(function() {
        toast.classList.add('show');
    });
    
    // 自动关闭
    _validationToastTimer = setTimeout(function() {
        hideToast(toast);
    }, 5000);
}

function hideToast(toast) {
    toast.classList.remove('show');
    setTimeout(function() {
        if (toast.parentNode) {
            toast.remove();
        }
    }, 300); // 等待 fade-out 动画结束
}

function escapeHtml(str) {
    return str.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
}

// 开始评估压缩
function startEvaluateSummarize() {
    const btn = document.getElementById('evaluate-summarize-btn');
    if (btn) {
        btn.classList.add('loading');
        btn.title = 'Compressing and summarizing...';
    }
}

// 结束评估压缩
function endEvaluateSummarize() {
    const btn = document.getElementById('evaluate-summarize-btn');
    if (btn) {
        btn.classList.remove('loading');
        updateEvaluateBtnState();
    }
}

// 更新Evaluate按钮状态（根据选中的API）
function updateEvaluateBtnState() {
    const btn = document.getElementById('evaluate-summarize-btn');
    const select = document.getElementById('cast-sheet-summarize');
    if (!btn || !select) return;
    
    const apiName = select.value;
    if (apiName === '<auto>' || apiName === '<disable>') {
        btn.classList.add('disabled');
        btn.title = 'Select an API to evaluate';
    } else {
        btn.classList.remove('disabled');
        btn.title = 'Evaluate the compression quality of the selected API using the latest conversations in current chat';
    }
}
