/**
  ****************************(C) COPYRIGHT 2019 DJI****************************
  * @file       referee_control.c/h
  * @brief      chassis power control.���̹��ʿ���
  * @note       this is only controling 80 w power, mainly limit motor current set.
  *             if power limit is 40w, reduce the value JUDGE_TOTAL_CURRENT_LIMIT 
  *             and POWER_CURRENT_LIMIT, and chassis max speed (include max_vx_speed, min_vx_speed)
  *             ֻ����80w���ʣ���Ҫͨ�����Ƶ�������趨ֵ,������ƹ�����40w������
  *             JUDGE_TOTAL_CURRENT_LIMIT��POWER_CURRENT_LIMIT��ֵ�����е�������ٶ�
  *             (����max_vx_speed, min_vx_speed)
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     Nov-11-2019     RM              1. add chassis power control
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2019 DJI****************************
  */
#include "referee_control.h"
#include "referee.h"
#include "arm_math.h"
#include "detect_task.h"




#define POWER_LIMIT         80.0f
#define WARNING_POWER       40.0f   
#define WARNING_POWER_BUFF  50.0f   

#define NO_JUDGE_TOTAL_CURRENT_LIMIT    64000.0f    //16000 * 4, 
#define BUFFER_TOTAL_CURRENT_LIMIT      16000.0f
#define POWER_TOTAL_CURRENT_LIMIT       20000.0f



/*
17mm�������� 15 18 30 m/s
17mm�������� 50 100 150 280 400
17mm������ȴ 10 20 30 40 60 80
һ��17mm 10����

42mm�������� 10 16 m/s
42mm�������� 100 200 300 350 500
42mm������ȴ 20 40 60 80 100 120
һ��42mm 100����
*/


//ͨ����ȡ��������,ֱ���޸����ٺ���Ƶ�ȼ�
//���ٵȼ�  Ħ�����
fp32 shoot_fric_grade[4] = {0, 1000, 2000, 4000};

//��Ƶ�ȼ� �������
fp32 shoot_grigger_grade[6] = {0, 5.0f, 10.0f, 15.0f, 28.0f, 40.0f};



/**
  * @brief          ���ƹ��ʣ���Ҫ���Ƶ������
  * @param[in]      chassis_power_control: ��������
  * @retval         none
  */
void chassis_power_control(chassis_move_t *chassis_power_control)
{
    fp32 chassis_power = 0.0f;
    fp32 chassis_power_buffer = 0.0f;
    fp32 total_current_limit = 0.0f;
    fp32 total_current = 0.0f;
    uint8_t robot_id = get_robot_id();
    if(toe_is_error(REFEREE_TOE))
    {
        total_current_limit = NO_JUDGE_TOTAL_CURRENT_LIMIT;
    }
    else if(robot_id == RED_ENGINEER || robot_id == BLUE_ENGINEER || robot_id == 0)
    {
        total_current_limit = NO_JUDGE_TOTAL_CURRENT_LIMIT;
    }
    else
    {
        get_chassis_power_and_buffer(&chassis_power, &chassis_power_buffer);
        //���ʳ���80w �ͻ�������С��60j,��Ϊ��������С��60��ζ�Ź��ʳ���80w
        if(chassis_power_buffer < WARNING_POWER_BUFF)
        {
            fp32 power_scale;
            if(chassis_power_buffer > 5.0f)
            {
                //��СWARNING_POWER_BUFF
                power_scale = chassis_power_buffer / WARNING_POWER_BUFF;
            }
            else
            {
                //only left 10% of WARNING_POWER_BUFF
                power_scale = 5.0f / WARNING_POWER_BUFF;
            }
            //��С
            total_current_limit = BUFFER_TOTAL_CURRENT_LIMIT * power_scale;
        }
        else
        {
            //���ʴ���WARNING_POWER
            if(chassis_power > WARNING_POWER)
            {
                fp32 power_scale;
                //����С��80w
                if(chassis_power < POWER_LIMIT)
                {
                    //scale down
                    //��С
                    power_scale = (POWER_LIMIT - chassis_power) / (POWER_LIMIT - WARNING_POWER);
                    
                }
                //���ʴ���80w
                else
                {
                    power_scale = 0.0f;
                }
                
                total_current_limit = BUFFER_TOTAL_CURRENT_LIMIT + POWER_TOTAL_CURRENT_LIMIT * power_scale;
            }
            //����С��WARNING_POWER
            else
            {
                total_current_limit = BUFFER_TOTAL_CURRENT_LIMIT + POWER_TOTAL_CURRENT_LIMIT;
            }
        }
    }

    
    total_current = 0.0f;
    //����ԭ����������趨
    for(uint8_t i = 0; i < 4; i++)
    {
        total_current += fabs(chassis_power_control->motor_speed_pid[i].out);
    }
    

    if(total_current > total_current_limit)
    {
        fp32 current_scale = total_current_limit / total_current;
        chassis_power_control->motor_speed_pid[0].out*=current_scale;
        chassis_power_control->motor_speed_pid[1].out*=current_scale;
        chassis_power_control->motor_speed_pid[2].out*=current_scale;
        chassis_power_control->motor_speed_pid[3].out*=current_scale;
    }
}



/**
  * @brief          ����17mm����������ٺ���Ƶ����Ҫ���Ƶ������
  * @param[in]      shoot_heat0_speed_and_cooling_control: ���ͻ�������
  * @retval         none
  */
 void shoot_heat0_speed_and_cooling_control(shoot_control_t *shoot_heat0_speed_and_cooling_control)
{
    //17mmǹ����������, 17mmǹ��ʵʱ����
    uint16_t heat0_cooling_limit;
    uint16_t heat0;
    //17mmǹ��ǹ����������,17mmʵʱ����
    uint16_t heat0_speed_limit; 
    uint16_t bullet_speed;
    //���̵ȼ� Ħ���ֵȼ�
    uint8_t grigger_speed_grade;
    uint8_t fric_speed_grade;



    if(toe_is_error(REFEREE_TOE))
    {
        grigger_speed_grade = 0;
        fric_speed_grade = 0;
    }
    else
    {
        get_shooter_heat0_cooling_limit_and_heat0(&heat0_cooling_limit,&heat0);   //��ȡ17mmǹ����������, 17mmǹ��ʵʱ����
        get_shooter_heat0_speed_limit_and_heat0(&heat0_speed_limit, &bullet_speed); // ��ȡ17mmǹ��ǹ����������,17mmʵʱ����

        //�������������������޸ĵȼ�
        //����
        if(heat0_cooling_limit == 50)
            grigger_speed_grade = 1;
        else if(heat0_cooling_limit == 100)
            grigger_speed_grade = 2;
        else if(heat0_cooling_limit == 150)
            grigger_speed_grade = 3;
        else if(heat0_cooling_limit == 280)
            grigger_speed_grade = 4;
        else if(heat0_cooling_limit == 400)
            grigger_speed_grade = 5;

        //����
        if(heat0_speed_limit == 15)
            fric_speed_grade = 1;
        else if(heat0_speed_limit == 18)
            fric_speed_grade = 2;
        else if(heat0_speed_limit == 30)
            fric_speed_grade = 3;


        //���������ٺ���Ƶ�ȼ�����ʱ������ʱע��
        //���ݵ�ǰ�����������޸ĵȼ�,ȷ���������޿�Ѫ,
        //���� ��ʣ����������30,ǿ�ƽ��Ͳ���ת�� ����0,ǿ���ƶ�
        if(heat0_cooling_limit - heat0 <= 30)
            grigger_speed_grade -- ;
        else if(heat0_speed_limit <= 0)
            grigger_speed_grade = 0;
        
        //���� ������,ǿ�ƽ���Ħ����ת��
        if(bullet_speed < heat0_speed_limit)
            fric_speed_grade -- ;

    }


    //��Ħ���ֵ���������ֵ
    shoot_heat0_speed_and_cooling_control->fric_motor[LEFT].speed_set = shoot_fric_grade[fric_speed_grade];
    shoot_heat0_speed_and_cooling_control->fric_motor[RIGHT].speed_set = shoot_fric_grade[fric_speed_grade];
    //�Բ��̵���������ֵ
    shoot_heat0_speed_and_cooling_control->speed_set = shoot_grigger_grade[grigger_speed_grade];


}