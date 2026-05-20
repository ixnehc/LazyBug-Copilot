---
name: WriteSkill
description: Activate this skill when the user needs to write a skill
---

## Overview
This skill is used to write a skill.md document, including a YAML front matter header and the specific document content.
The YAML front matter header includes:
name: [The name of the skill]
description: [A brief description of the skill and when this skill document should be loaded]

It is mandatory that the user specifies a file path for the skill.md. If the user does not provide a file path, use AskQuestion tool to request a path from the user, and prompt the user that the path can be obtained by creating a new skill in the skill control panel.

Before starting to write, first read the skill.md. If the original file has existing content, modifications should be made on top of the existing content.

Never modify the name of the skill if it's already existing
You can use various tools (FindSymbol, Grep, ReadFile, etc.) to obtain the required information.
Always use the EditFile tool to modify this skill.md file.

Follow the user's instruction to fill the skill.md content