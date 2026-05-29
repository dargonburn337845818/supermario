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
    if(m_GameScreen)delete m_GameScreen;
}

void Game::Init(){
    initgraph(windowWidth,windowHeight); //创建窗口

    BeginBatchDraw(); // 开启双缓冲模式

    loadimage(&m_LevelBackground, _T("res\\level\\level_1.png")); //铺背景

    m_Camera.Init(windowWidth, windowHeight,
              m_LevelBackground.getwidth(),
              m_LevelBackground.getheight());

    int imgW = m_LevelBackground.getwidth();
    int imgH = m_LevelBackground.getheight();
    if (imgW == 0 || imgH == 0) {
        MessageBox(NULL, "底图加载失败，请检查路径或文件是否存在！", "错误", MB_OK);
        isRunning = false;
        return;
    }
    
    if (!m_LevelMgr.LoadLevel("res\\level\\level_1.json")) {
        // 如果加载失败，可以弹框提示或写日志
        MessageBox(NULL, "地图加载失败！", "错误", MB_OK);
        isRunning = false;
        return;
    }

    // 创建与背景图同尺寸的离屏画布
    m_GameScreen = new IMAGE(m_LevelBackground.getwidth(), m_LevelBackground.getheight());
    Player = new Mario(110.0f, 200.0f); 
    Player->LoadResources("res\\graphics\\mario_bros.png", "res\\graphics\\mario.json");
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
    Cleanup(); 
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

    SetWorkingImage(m_GameScreen); // 换绘图目标到内存画布
    //清除内存中的渲染
    cleardevice();
    // 绘制背景
    putimage(0, 0, &m_LevelBackground);
    // 绘制碰撞体 (坐标都是基于 223 高度的)
    for (const auto& wall : m_LevelMgr.GetSolidColliders()) {
        switch (wall.type) {
            case SolidType::GROUND: setfillcolor(RGB(0, 0, 0)); break;
            case SolidType::STEP:   setfillcolor(RGB(100, 100, 100)); break;
            case SolidType::PIPE:   setfillcolor(RGB(0, 255, 0)); break;
            case SolidType::BRICK:  setfillcolor(RGB(255, 0, 0)); break;
            case SolidType::BOX:    setfillcolor(RGB(255, 255, 0)); break;
            default: setfillcolor(RGB(255, 255, 255)); break;
        }
        fillrectangle(wall.bounds.x, wall.bounds.y, 
                      wall.bounds.x + wall.bounds.width, 
                      wall.bounds.y + wall.bounds.height);
    }   
    // 绘制玩家
    if(Player) Player->Render(m_GameScreen);
    // 摄像机跟随
    m_Camera.Update(Player->GetX(), Player->GetWidth());
    SetWorkingImage(); // 切换绘图目标回物理窗口

    cleardevice();

    HDC dstHdc = GetImageHDC(NULL);
    HDC srcHdc = GetImageHDC(m_GameScreen);
    SetStretchBltMode(dstHdc, COLORONCOLOR);
    StretchBlt(
        dstHdc,
        0, 0, windowWidth, windowHeight,                              // 目标：整个窗口
        srcHdc,
        (int)m_Camera.GetX(), 0,                                      // 源起点
        m_Camera.GetViewW(), m_Camera.GetViewH(),                     // 源区域
        SRCCOPY
    );
    // 投送画面
    FlushBatchDraw();
}
