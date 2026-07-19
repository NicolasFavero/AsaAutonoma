#include "PID.h"
#include "DataTypes.h"

// Instâncias globais ou membros da classe

// Função auxiliar privada para processar o PID e atualizar o Servo
void PID::computePID(PID_variables& pid, ServoParamters& servo, float currentAngle, float dt, float integralLimit, float maxMovement, int minLimit, int maxLimit) {
    // 1. Cálculo do erro
    pid.error = pid.target - currentAngle;

    // 2. Integral com Anti-Windup
    pid.integral += pid.error * dt;
    if(pid.integral > pid.integralLimit)  pid.integral = integralLimit;
    if(pid.integral < -pid.integralLimit) pid.integral = -integralLimit;

    // 3. Derivada
    pid.derivative = (pid.error - pid.lastError) / dt;

    // 4. Saída do PID
    pid.proporcional = pid.kp * pid.error;
    pid.output = pid.proporcional + (pid.ki * pid.integral) + (pid.kd * pid.derivative);
    pid.lastError = pid.error;

    // 5. Cálculo do ângulo alvo baseado na saída do PID
    //servo.targetAngle = servo.neutralAngle + pid.output;

    // 6. Limitador de Taxa (Slew Rate) baseado no safeAngle do ciclo anterior
    //servo.safeAngle = constrain(servo.targetAngle, servo.safeAngle - maxMovement, servo.safeAngle + maxMovement);

    // 7. Limite de hardware físico gravado diretamente no setAngle do servo
    servo.setAngle = constrain(pid.output , minLimit, maxLimit);
}

void PID::setAngles(ServoPositions& angle, AttitudeData& attitudeData, NavigationData& nav, bool gpsData) {

    // Definição dos alvos do PID
    pitch.target = -3.0f;
    roll.target = 0.0f;

    // Configuração dos ângulos neutros dos servos
    leftAileron.neutralAngle = 85.0f;
    rightAileron.neutralAngle = 103.0f;
    elevator.neutralAngle = 90.0f;

    // Cálculo do Delta Time (dt)
    uint32_t now = micros();
    float dt = (lastTime == 0) ? 0.005f : (now - lastTime) / 1000000.0f;
    lastTime = now;

    // Inicialização do safeAngle no primeiro ciclo para evitar saltos bruscos
    if (elevator.safeAngle == 0.0f) elevator.safeAngle = elevator.neutralAngle;
    if (leftAileron.safeAngle == 0.0f) leftAileron.safeAngle = leftAileron.neutralAngle;
    if (rightAileron.safeAngle == 0.0f) rightAileron.safeAngle = rightAileron.neutralAngle;

    // Configuração do Limitador de Taxa (Slew Rate)
    const float MAX_VELOCITY_DEG_PER_SEC = 120.0f; 
    float maxMovementThisCycle = MAX_VELOCITY_DEG_PER_SEC * dt;

    //================ EXECUÇÃO DO PITCH (Profundor) =================
    computePID(pitch, elevator, attitudeData.pitch, dt, pitch.integralLimit, maxMovementThisCycle, 20, 160);

    //================ EXECUÇÃO DO ROLL (Ailerons) =================

    computePID(roll, leftAileron,  attitudeData.roll, dt, roll.integralLimit, maxMovementThisCycle, 20, 165);
    computePID(roll, rightAileron, attitudeData.roll, dt, roll.integralLimit, maxMovementThisCycle, 20, 165);

    //================ REPASSA O RESULTADO PARA A STRUCT USADA PELOS SERVOS =================
    // Sem isso, "angle" nunca sai dos valores padrao (90/90/90) e os servos
    // ficam sempre no mesmo angulo, mesmo com o PID calculando corretamente.
    angle.elevator     = elevator.setAngle;
    angle.leftAileron  = leftAileron.setAngle;
    angle.rightAileron = rightAileron.setAngle;
}
