#pragma once

class Game{

public:

    Game();
    ~Game();

    void Init(); //初始化游戏
    void Run(); //运行游戏主循环
    void Cleanup(); //清理

private:
    void ProcessInput(); //处理输入
    void Update(); //更新状态
    void Render(); //渲染画面

    bool isRunning; //游戏是否在运行
    int windowWidth; //窗口宽度
    int windowHeight; //窗口高度
    int FPS; //游戏帧率
};