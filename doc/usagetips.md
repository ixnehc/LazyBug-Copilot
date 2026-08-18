# Usage Tips

Best practicing using LazyBug Copilot:

- Always break down tasks into small steps — LazyBug is not designed for large tasks.
- In general, always set Context Level to 1 (Context Usage will be kept under ~30k tokens), which works well in most cases.
- Install the necessary working environment software (Node.js, Python, GIT, Perforce, etc.) to support CLI tool work.
- After adding new files to a solution, remember to save the solution so that the new files are tracked correctly.
- Understand the capability limits of the model you are using; within the model's capability range, always start with the weakest model, and if the task result is unsatisfactory, undo and try a stronger model.
- The input hint feature requires a model with fast response time, good context understanding, and strong instruction-following capability — while also being affordable. We recommend **DeepSeek 4 Flash** as the go-to model for input hints, as it strikes the best balance between speed and quality for real-time completions.
- The LazyBug chat database is independent of the project and is stored centrally on the C drive. Please ensure sufficient free space on your C drive (for ultra-large projects, more than 10 GB may be required).