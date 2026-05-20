#include "ChatTask_FastApply.h"
#include <iostream>

// 声明静态函数
extern void GenerateDiffString(const std::string& oldContent, const std::string &newContent, std::string &diffString);

// 测试_GenerateDiffString函数的示例
void TestDiffGeneration()
{
    CChatTask_FastApply fastApply("", "", L"");
    
    // 创建测试用的旧文件内容
    std::string oldContent = 
        "#include <iostream>\n"
        "#include <vector>\n"
        "#include <string>\n"
        "\n"
        "class TestClass {\n"
        "public:\n"
        "    TestClass() {}\n"
        "    \n"
        "    void oldFunction() {\n"
        "        std::cout << \"Old implementation\" << std::endl;\n"
        "    }\n"
        "    \n"
        "    void anotherFunction() {\n"
        "        int x = 10;\n"
        "        int y = 20;\n"
        "        std::cout << x + y << std::endl;\n"
        "    }\n"
        "    \n"
        "    void unchangedFunction() {\n"
        "        // This function remains the same\n"
        "        return;\n"
        "    }\n"
        "    \n"
        "    void yetAnotherFunction() {\n"
        "        // Another unchanged function\n"
        "        for (int i = 0; i < 10; i++) {\n"
        "            std::cout << i << std::endl;\n"
        "        }\n"
        "    }\n"
        "};\n";
    
    // 创建测试用的新文件内容  
    std::string newContent = 
        "#include <iostream>\n"
        "#include <vector>\n"
        "#include <string>\n"
        "#include <algorithm>\n"  // 新增的头文件
        "\n"
        "class TestClass {\n"
        "public:\n"
        "    TestClass() : m_value(0) {}\n"  // 修改了构造函数
        "    \n"
        "    void newFunction() {\n"  // 替换了oldFunction
        "        std::cout << \"New implementation\" << std::endl;\n"
        "        std::cout << \"With additional line\" << std::endl;\n"
        "    }\n"
        "    \n"
        "    void anotherFunction() {\n"
        "        int x = 15;\n"  // 修改了值
        "        int y = 25;\n"  // 修改了值
        "        std::cout << x + y << std::endl;\n"
        "    }\n"
        "    \n"
        "    void unchangedFunction() {\n"
        "        // This function remains the same\n"
        "        return;\n"
        "    }\n"
        "    \n"
        "    void yetAnotherFunction() {\n"
        "        // Another unchanged function\n"
        "        for (int i = 0; i < 10; i++) {\n"
        "            std::cout << i << std::endl;\n"
        "        }\n"
        "    }\n"
        "    \n"
        "private:\n"  // 新增的私有成员
        "    int m_value;\n"
        "};\n";
    
    std::string diffString;
    GenerateDiffString(oldContent, newContent, diffString);
    
    std::cout << "Generated Diff String:\n";
    std::cout << "======================\n";
    std::cout << diffString << std::endl;
    std::cout << "======================\n";
}

/*
示例输出说明：

生成的diffString将包含：
- [+] 前缀表示新增的行（绿色高亮）
- [-] 前缀表示删除的行（红色高亮）  
- 普通行表示未修改的行
- [...] 表示被省略的未修改行块

特点：
1. 只在有变化的行周围显示最多4行上下文
2. 连续的未修改行块用[...]代替
3. 修改的行会显示为删除旧行+添加新行
4. 生成的字符串可以直接用于FileEdit的行高亮显示

使用场景：
- 代码对比和差异显示
- 文件修改历史查看
- AI修改建议的可视化展示
*/ 