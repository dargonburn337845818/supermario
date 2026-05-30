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
    if(m_ScaledBg) delete m_ScaledBg;
}

void Game::Init(){
    initgraph(windowWidth,windowHeight); //创建窗口

    BeginBatchDraw(); // 开启双缓冲模式

    loadimage(&m_LevelBackground, _T("res\\level\\level_1.png")); //铺背景
    loadimage(&m_ImgBrick, _T("res\\graphics\\brick\\2.png")); 
    loadimage(&m_ImgBox, _T("res\\graphics\\box\\1.png"));
    loadimage(&m_ImgEmptyBox, _T("res\\graphics\\box\\4.png"));
    loadimage(&m_ImgCoin, _T("res\\graphics\\coin\\1.png"));

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
    int worldW = 9086;
    int worldH = 600;
    m_Camera.Init(windowWidth, windowHeight, worldW, worldH);

    m_ScaledBg = new IMAGE(worldW, worldH);
    HDC dstHdc = GetImageHDC(m_ScaledBg);
    HDC srcHdc = GetImageHDC(&m_LevelBackground);
    SetStretchBltMode(dstHdc, COLORONCOLOR);
    StretchBlt(
        dstHdc, 0, 0, worldW, worldH,          // 目标：世界尺寸
        srcHdc, 0, 0, imgW, imgH,              // 源：原始图片
        SRCCOPY
    );

    // 创建与背景图同尺寸的离屏画布
     m_GameScreen = new IMAGE(worldW, worldH);
    Player = new Mario(110.0f, 400.0f); 
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
    m_Camera.Update(Player->GetX(), Player->GetWidth());
    m_LevelMgr.UpdateEffects();
}

void Game::Render(){

    SetWorkingImage(m_GameScreen); // 换绘图目标到内存画布
    //清除内存中的渲染
    cleardevice();
    // 绘制背景
    putimage(0, 0, m_ScaledBg);
    // 绘制碰撞体 (坐标都是基于 223 高度的)
    for (const auto& wall : m_LevelMgr.GetSolidColliders()) {
        switch (wall.type) {
            case SolidType::BRICK: {
                HDC dstHdc = GetImageHDC(m_GameScreen); // 目标：内存画布
                HDC srcHdc = GetImageHDC(&m_ImgBrick);   // 源：砖块贴图
                SetStretchBltMode(dstHdc, COLORONCOLOR);
                StretchBlt(
                    dstHdc, 
                    wall.bounds.x, wall.bounds.y + wall.bounceOffset, wall.bounds.width, wall.bounds.height, // 拉伸到碰撞框大小
                    srcHdc, 
                    0, 0, m_ImgBrick.getwidth(), m_ImgBrick.getheight(),                 // 整张原图
                    SRCCOPY
                );
                continue;
            }
            case SolidType::BOX: {
                HDC dstHdc = GetImageHDC(m_GameScreen); // 目标：内存画布
                IMAGE* imgToDraw = wall.isHit ? &m_ImgEmptyBox : &m_ImgBox;
                HDC srcHdc = GetImageHDC(imgToDraw);     // 源：问号箱贴图
                SetStretchBltMode(dstHdc, COLORONCOLOR);
                StretchBlt(
                    dstHdc, 
                    wall.bounds.x, wall.bounds.y + wall.bounceOffset, wall.bounds.width, wall.bounds.height, // 拉伸到碰撞框大小
                    srcHdc, 
                    0, 0, m_ImgBox.getwidth(), m_ImgBox.getheight(),                     // 整张原图
                    SRCCOPY
                );
                continue;
            }
        }
    }   
    // 绘制玩家
    if(Player) Player->Render(m_GameScreen);
    for (const auto& coin : m_LevelMgr.GetCoinEffects()) {
        if (coin.life < 10 && (coin.life % 2 == 0)) continue; // 闪烁逻辑保留
        
        Animation::DrawAlpha(m_GameScreen, (int)coin.x, (int)coin.y, 32, 32, &m_ImgCoin);
    }
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
