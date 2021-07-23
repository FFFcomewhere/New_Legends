## 官方提供的资料位于doc文件夹内，由于部分源码已经过修改，所以以此文档为准。

如果不加特殊说明,默认的坐标系为,人站在车后面,正前方为X正方向,正右方为Y轴正方向,正上方为Z轴正方向.

C板的陀螺仪的坐标设定：
站在车体后方,以C板平放，C板电源口在左侧,逆时针为正方向,
!!!注意,C板固定应该平放!!!!


这是哨兵下云台代码

###
这是整个哨兵的CAN
//底盘和发射机构电机接收ID   下云台
    CAN_CHASSIS_SHOOT_ALL_ID = 0x200,
    CAN_3508_M1_ID = 0x201,
    CAN_TRIGGER_MOTOR_ID = 0x202,
    CAN_LEFT_FRIC_MOTOR_ID = 0x203,
    CAN_RIGHT_FRIC_MOTOR_ID = 0x204,

//发射机构电机接收ID   上云台
    CAN_CHASSIS_SHOOT_ALL_ID = 0x200,
    CAN_TRIGGER_MOTOR_ID = 0x205,
    CAN_LEFT_FRIC_MOTOR_ID = 0x206,
    CAN_RIGHT_FRIC_MOTOR_ID = 0x207,

//云台电机接收ID
    CAN_GIMBAL_ALL_ID = 0x2FF,
    CAN_YAW_MOTOR_ID = 0x20A,
    CAN_PIT_MOTOR_ID = 0x20B,
    CAN_YAW_MOTOR_ID = 0x20A,
    CAN_PIT_MOTOR_ID = 0x20B,
    


## 操作手参数：

### 遥控器控制:

左侧按键：上 打开和关闭摩擦轮
         中 无状态
         下 从中拨到下,快速拨回中为单发
            从中拨到下,停留为连发
​		  


右侧按键：上为云台跟随底盘

​		  中为底盘跟随云台

​		  下为刹车，云台静止

拨杆：
左侧拨杆控制底盘前后左右，右侧拨杆控制云台pitch和yaw，在底盘跟随底盘模式下也控制底盘旋转。





### 键鼠控制：将遥控器左右按键拨至中间 !!! 如果车辆失控,将遥控器右按键拨至下
注意!!! 同时按下ctrl z x 一秒,单片机将重启  !!!

​若无为特殊说明,均为单击操作

底盘：
    W A S D 前后左右平移
    F 小陀螺 
    C 扭腰
    SHIFT 长按 超级电容加速
    X 45度角对敌

云台:
    鼠标左右移动 yaw轴左右运动
    鼠标前后移动 pitch轴上下运动
    Q 左90度转头 E 右 90度转头 V 180度转头
   


射击: 
    G 打开和关闭摩擦轮 以及激光   
    R 双击 打开和关闭弹仓 !注意,弹仓打开时,云台无法运动
    鼠标左键 单击单发 长按连发 
    鼠标右键 打开和关闭自瞄


### 操作手UI界面:

底盘模式: 
L45 左45度角对敌
R45 右45度角对敌
top 小陀螺状态
swing 扭腰状态

T 为打开 F 为关闭

云台模式:
normal 默认底盘跟随云台
auto    自瞄模式
hit_buff 打符

弹仓:
magazine T or F

摩擦轮:
fric T or F


​	

## 场地人员说明:

### 硬件连接说明：
从左向右数:
裁判串口线: C板UART1  G TX RX
裁判学生串口接口 RX TX G
视觉串口线: C板UART2  RX TXV G V

CAN1: L H
CAN2: V G H L

底盘电机：can2  ID 为1 2 3 4 右前，左前，左后，右后
///////
2     1
3     4   
///////
云台电机：can1 yaw 9 pitch 10 

摩擦轮电机：can1 left 5 right 6

拨盘电机：can1 7

弹仓舵机: 左边数第2个PWM
限位舵机: 左边数第3个PWM  


射弹 触发条件为  BUTTEN_TRIG_PIN 为低电平 对应C板最左侧的PWM口
        


关于电机正反装
#define YAW_TURN    0
#define PITCH_TURN  0

这两个宏定义与电机正反装相关,当yaw轴电机转子与云台相对运动时,YAW_TURN为0,否则为1;pitch轴电机转子与云台相对运动时,PITCH_TURN为0,否则为1,


### 校准操作说明：
开启校准模式：          左摇杆右下，右摇杆左下  且左右按键拨至下档

陀螺仪校准           左摇杆左下，右摇杆右下  且左右按键拨至下档

云台校准               左摇杆左上，右摇杆右上  且左右按键拨至下档

底盘校准             左摇杆右上，右摇杆左上   且左右按键拨至下档



### 代码适配说明:
以下以步兵的适配为例: 调试前应将底盘悬空, 云台调试和陀螺仪调试过程应将云台CAN输出为0
1.完成硬件的连接
2.底盘校准,调整底盘电机ID
3.陀螺仪校准,根据C板固定方式,读取INS_angle内的三轴角度,通过调整C板固定方式或者
修改代码内的INS_YAW_ADDRESS_OFFSET等宏定义以及云台绝对坐标更新函数,调整适配的欧拉角
4.云台调试,根据电机正反装调节宏定义,YAW_TURN,PITCH_TURN;根据反馈的电机编码值,调整YAW_OFFSET, PITCH_OFFSET为云台归中编码值,再依此调节限幅,
5.调试完以上一个