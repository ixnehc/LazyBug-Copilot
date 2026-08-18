# 使用技巧

使用 LazyBug Copilot 的最佳实践：

- 永远拆分任务为小步骤，LazyBug 不是为大型任务设计的。
- 一般情况下，总是设置 Context Level 为 1（Context Usage 会尽量被限制在 <30 kToken），绝大部分情况下可以工作的很好。
- 安装必须的工作环境软件（Node.js, Python, GIT, Perforce 等），以支持 CLI tool 的工作。
- 在 Solution 里新增文件后，记得保存 solution 以确保新文件被正确跟踪。
- 了解你使用的模型的能力极限，在模型的能力范围内，总是从最弱的模型开始尝试，如果完成任务不满意，则 undo 并尝试更强的模型。
- Input Hint 功能需要一个具备快速响应、良好的上下文理解能力和较强指令遵循能力的模型，同时还要足够便宜。我们推荐使用 **DeepSeek 4 Flash** 作为 Input Hint 的首选模型——它在速度和质量之间达到了最佳平衡，非常适合实时补全场景。
- LazyBug 聊天数据库独立于项目，集中存储在 C 盘。请确保 C 盘有足够的可用空间（对于超大型项目，可能需要 10 GB 以上）。