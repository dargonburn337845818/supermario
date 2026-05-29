#include "core\Game.h"
#include <graphics.h>
#include <ctime>
#include <windows.h>

Game::Game()
    :isRunning(false),
    windowWidth(800),
    windowHeight(600),
    FPS(60)
{
    // 初始化列表负责给成员变量赋初值
}

Game::~Game(){
    // 析构函数，如果 Init 里 new 了东西，这里要 delete
    if(Player)delete Player;
}

void Game::Init(){
    initgraph(windowWidth,windowHeight); //创建窗口
    setbkcolor(RGB(92, 148, 252)); // 设置背景色为经典马里奥蓝天
    BeginBatchDraw(); // 开启双缓冲模式
    if (!m_LevelMgr.LoadLevel("res/level/level_1.json")) {
        // 如果加载失败，可以弹框提示或写日志
        MessageBox(NULL, "地图加载失败！", "错误", MB_OK);
        isRunning = false;
        return;
    }

    Player = new Mario(110.0f, 522.0f); 
    Player->LoadResources("res/graphics/mario_bros.png", "res/graphics/mario.json");
    isRunning = true; //游戏开始运行
}

void Game::Run(){
    const int frameDelay = 1000/FPS; //计算理论延迟(ms)

    clock_t lastFrameTime = clock(); //记录上一帧的时间戳

    while(isRunning){
         clock_t curTime = clock(); //当前时间戳
         clock_t realdelay = curTime - lastFrameTime; //帧之间的实际耗时
         lastFrameTime = curTime;

         ProcessInput();
         Update();
         Render();

         clock_t frameTime = clock()-curTime;
         if(frameTime < frameDelay){
            Sleep(frameDelay-frameTime);
         }
    }
}

void Game::Cleanup(){
    EndBatchDraw(); //关闭双缓冲
    closegraph(); //关闭窗口
}

void Game::ProcessInput(){
    //esc退出
    if(GetAsyncKeyState(VK_ESCAPE) & 0x8000){
        isRunning = false;
    }
}

void Game::Update(){
    //计算逻辑
    if(Player) Player->Update(m_LevelMgr);
}

void Game::Render(){
    //清除内存中的渲染
    cleardevice();
    //绘制
    setfillcolor(GREEN);
    for (const auto& wall : m_LevelMgr.GetColliders()) {
        fillrectangle(
            static_cast<int>(wall.box.x), 
            static_cast<int>(wall.box.y), 
            static_cast<int>(wall.box.x + wall.box.width), 
            static_cast<int>(wall.box.y + wall.box.height)
        );
    }
    if(Player) Player->Render();
    //投送画面
    FlushBatchDraw();
}
