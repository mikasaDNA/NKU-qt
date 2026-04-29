#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include "gameobject.h"
#include "player.h"
#include "enemy.h"
#include "experiencegem.h"
#include "weapon.h"
#include <QWidget>//窗口
#include <QTimer>//定时器
#include <QElapsedTimer>//高精度定时器
#include <QSet>//集合
#include <vector>//向量
#include <memory>//智能指针
#include <QPixmap>//图片
#include <QMediaPlayer>
#include <QAudioOutput>

//游戏状态枚举
enum class GameState
{
    MainMenu,   //主菜单/开始界面
    Playing,    //游戏中
    LevelUp,    //升级界面
    Paused,     //暂停
    Settings,   //设置
    GameOver    //游戏结束
};

//主游戏窗口
class GameWidget : public QWidget
{
    Q_OBJECT//使该类可使用信号与槽机制
    
public:
    explicit GameWidget(QWidget* parent = nullptr);//父窗口默认为空
    ~GameWidget() override = default;
    
    // 判断游戏状态
    GameState getState() const { return m_state; }
    bool isPaused() const { return m_paused; }
    bool isLevelUp() const { return m_levelUp; }
    bool isGameOver() const { return m_gameOver; }
    int getScore() const { return m_score; }
    float getGameTime() const { return m_gameTime; }

protected:
    // 重写Qwidget中继承来的函数
    void paintEvent(QPaintEvent* event) override;//绘制
    void keyPressEvent(QKeyEvent* event) override;//键盘响应
    void keyReleaseEvent(QKeyEvent* event) override;//键盘响应
    void mousePressEvent(QMouseEvent* event) override;//鼠标响应
    
private slots:
    void gameLoop();//私有槽函数，被定时器触发，作为游戏的主循环入口
    void updateMenuBackground();// 更新开始界面背景轮播
    
private:
    // 游戏初始化
    void initializeGame();//初始化游戏
    void initializeMenu();//初始化开始界面
    
    // 游戏更新
    void updateGame(float deltaTime);//更新游戏
    void handleInput(float deltaTime);//处理输入
    void checkCollisions();//检查碰撞
    void cleanupDeadEntities();//清理死掉的实体
    void spawnGems();//生成宝石
    
    // 边界检测，防止玩家出界
    void clampPlayerPosition();
    
    // 渲染
    void renderGame(QPainter& painter);
    void renderMainMenu(QPainter& painter);
    void renderBackground(QPainter& painter);
    void renderHUD(QPainter& painter);
    void renderLevelUp(QPainter& painter);
    void renderSettings(QPainter& painter);
    void renderGameOver(QPainter& painter);
    
    // 圆与圆的碰撞检测
    bool checkCircleCollision(const Vec2& pos1, float r1,const Vec2& pos2, float r2);

    //应用升级
    void applyUpgrade(const UpgradeOption& option);
    
private:
    // 游戏循环
    QTimer* m_gameTimer;//定时器，用于固定频率触发gameloop
    //流逝计时器，用于精确计算两帧之间的时间差（即deltatime）
    QElapsedTimer m_elapsedTimer;
    
    // 声明记录游戏状态的变量
    GameState m_state = GameState::MainMenu;//初始化为开始界面
    bool m_paused = false;
    bool m_gameOver = false;
    bool m_levelUp=false;
    float m_gameTime = 0.0f;
    int m_score = 0;

    // 输入
    QSet<int> m_keysPressed;

    std::unique_ptr<Player> m_player;
    std::vector<std::unique_ptr<Enemy>> m_enemies;
    std::vector<std::unique_ptr<GameObject>> m_projectiles;
    std::vector<std::unique_ptr<ExperienceGem>> m_gems;
    
    // 敌人生成器
    std::unique_ptr<EnemySpawner> m_spawner;
    
    // 待生成的经验宝石
    std::vector<std::pair<Vec2, int>> m_pendingGems;

    int m_randomNum;

    QMediaPlayer* m_currentBgm;
    QAudioOutput* m_bgmAudioOutPut;
    
    // ----------升级系统----------
    std::vector<UpgradeOption> m_currentUpgradeOptions;//现有升级选项
    std::vector<QRect> m_upgradeButtomRects;//升级按钮区域

    // ----------地图系统----------
    QPixmap m_currentBackground;//当前使用的背景图片
    float m_tileSize = 83.3;//单个瓦片大小（像素）
    int m_mapGridWidth = 16;//地图网格宽度（瓦片数）
    int m_mapGridHeight = 9;//地图网格高度（瓦片数）
    int m_mapWidth = 1333;
    int m_mapHeight = 750;

    // ----------开始界面----------
    std::vector<QPixmap> m_menuBackgrounds;//开始界面背景图片列表
    int m_currentMenuBgIndex = 0;//当前显示的背景索引
    QTimer* m_menuBgTimer;//背景轮播定时器
    QMediaPlayer* m_menuBgm;//开始界面bgm
    QAudioOutput* m_menuAudioOutput;
    QRect m_startButtonRect;//开始游戏按钮区域
    QRect m_settingsButtonRect;//设置按钮区域

    // ----------设置界面----------
    std::vector<std::unique_ptr<Weapon>> m_weaponCatalog;//武器图鉴
    QRect m_settingsBackButtonRect;//返回按钮区域
};

#endif
