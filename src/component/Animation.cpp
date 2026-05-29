#include <windows.h>
#include <tchar.h>
#include "component\Animation.h"
#include <fstream>

//定义 AlphaBlend 函数指针类型
typedef BOOL(WINAPI* PFN_AlphaBlend)(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION);
static PFN_AlphaBlend pfnAlphaBlend = nullptr;
using json = nlohmann::json;

Animation::Animation() : currentSet(nullptr), currentFrameIndex(0) {}

Animation::~Animation() {
    ClearFrames();
}

void LoadAlphaBlendFunc() {
    if (pfnAlphaBlend == nullptr) {
        // 运行时直接加载系统自带的 msimg32.dll
        HMODULE hMod = LoadLibrary(_T("msimg32.dll"));
        if (hMod) {
            // 找到 AlphaBlend 函数的内存地址
            pfnAlphaBlend = (PFN_AlphaBlend)GetProcAddress(hMod, "AlphaBlend");
        }
    }
}

void Animation::ClearFrames() {
    for (auto& [setName, frames] : animSets) {
        for (auto& frame : frames) {
            if (frame.imgOriginal) delete frame.imgOriginal;
            if (frame.imgFlipped) delete frame.imgFlipped;
        }
    }
    animSets.clear();
}

// 专门用于绘制带 Alpha 通道的透明图片
void Animation::DrawAlpha(int x, int y, IMAGE* img) {
    if (!img) return;
    
    // 确保函数已加载
    LoadAlphaBlendFunc(); 
    
    // 如果成功找到了 AlphaBlend 函数
    if (pfnAlphaBlend) {
        HDC hdcDest = GetImageHDC(NULL); 
        HDC hdcSrc = GetImageHDC(img);   
        
        int w = img->getwidth();
        int h = img->getheight();
        
        BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
        
        // 通过函数指针调用真正的 AlphaBlend
        pfnAlphaBlend(hdcDest, x, y, w, h, hdcSrc, 0, 0, w, h, bf);
    } else {
        // 极端情况兜底：如果系统连这个 dll 都没有（几乎不可能），就用普通贴图
        putimage(x, y, img);
    }
}

void Animation::FlipImage(const IMAGE* src, IMAGE* dest) {
    int w = src->getwidth();
    int h = src->getheight();

    //获取源图和目标图的显存缓冲区指针
    //DWORD 在 EasyX 中代表一个像素的颜色值（32位，格式为 ARGB）
    DWORD* srcBuffer = GetImageBuffer(src);
    DWORD* destBuffer = GetImageBuffer(dest);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            // 水平翻转：目标图的像素取自源图对称位置
            if (!srcBuffer || !destBuffer) return;
            destBuffer[y * w + x] = srcBuffer[y * w + (w - 1 - x)];
        }
    }
}

void Animation::Load(const std::string& imagePath, const std::string& jsonPath) {
    ClearFrames(); // 防止重复加载导致内存泄漏

    //将整张精灵大图加载到内存 spriteSheet 中
    loadimage(&spriteSheet, imagePath.c_str());

    //打开并解析 JSON 文件。如果文件不存在直接返回。
    std::ifstream file(jsonPath);
    if (!file.is_open()) return;
    json data = json::parse(file);

    //检查 JSON 是否包含 image_frames 节点，然后遍历每个动作集
    if (data.contains("image_frames")) {
        for (auto& [setName, framesArray] : data["image_frames"].items()) {
            std::vector<Frame> frameList;
            for (auto& f : framesArray) {
                Frame frame;
                frame.x = f["x"]; frame.y = f["y"];
                frame.width = f["width"]; frame.height = f["height"];

                // 核心优化：加载时就把图切出来，并且生成翻转图
                // 1. 切出原图
                frame.imgOriginal = new IMAGE(frame.width, frame.height);
                SetWorkingImage(frame.imgOriginal); // 把小图设为绘图设备
                // 将大图的 (frame.x, frame.y) 开始的 (frame.width, frame.height) 区域，画到小图的 (0,0) 处
                putimage(0, 0, frame.width, frame.height, &spriteSheet, frame.x, frame.y); 
                SetWorkingImage(NULL);               // 恢复屏幕绘图设备
                // 2. 创建同等大小的翻转图并生成
                frame.imgFlipped = new IMAGE(frame.width, frame.height);
                FlipImage(frame.imgOriginal, frame.imgFlipped);

                frameList.push_back(frame);
            }
            animSets[setName] = frameList;
        }
    }

    //默认设置第一个动作集
    if (!animSets.empty()) SetAnimSet(animSets.begin()->first);
}

//切换动作集,如果相同则跳过（避免重置动画）,不同则更新指针和索引。
void Animation::SetAnimSet(const std::string& setName) {
    if (currentSetName == setName) return;
    auto it = animSets.find(setName);
    if (it != animSets.end()) {
        currentSetName = setName;
        currentSet = &(it->second);
        currentFrameIndex = 0;
    }
}

//安全地设置帧索引
void Animation::SetFrameIndex(int index) {
    if (!currentSet) return;
    if (index >= 0 && index < currentSet->size()) currentFrameIndex = index;
}

//根据方向贴图
void Animation::Draw(int x, int y, bool flip) {
    if (!currentSet || currentSet->empty()) return;
    Frame& f = (*currentSet)[currentFrameIndex];

    //根据方向直接贴预切好的小图
    //putimage 默认是不透明的
    if (flip) {
        DrawAlpha(x, y, f.imgFlipped);
    } else {
        DrawAlpha(x, y, f.imgOriginal);
    }
}

//获取当前帧的宽高
int Animation::GetCurrentFrameWidth() const {
    if (!currentSet || currentSet->empty()) return 0;
    return (*currentSet)[currentFrameIndex].width;
}

int Animation::GetCurrentFrameHeight() const {
    if (!currentSet || currentSet->empty()) return 0;
    return (*currentSet)[currentFrameIndex].height;
}