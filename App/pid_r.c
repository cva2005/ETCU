#include <string.h>
#include <stdbool.h>
#include "timers.h"
#include "pid_r.h"

void pid_r_init (pid_r_instance *S) {
	memset(S->i, 0, (ST_SIZE + 3) * sizeof(float32_t));
}

/*
%===========================================================
K=0.005;
Ti=100;
Td=alfa*Ti;
%,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
%   ANR
tmx=300*n;	% длительность моделирования автоколебаний[сек]
b1=0.38;    % косвенные условия оптимальности
b2=1.0;
b3=3.7;
Tfz=Ti*b1;	% пост. времени фазосдвигающего фильтра
d=An/5;     % выход релейного элемента
iter=0;
t_sum=1;	%суммарное время самонастройки
dy=d/100;	%зона вычисления экстремумов
%,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
while(1)
    Xk=[0 0 0 0 0 0 0]; % входное воздействие
    Amax=-dy;
    Amin=d;
    iter=iter+1;
    j=0; %пропуск начальных периодов
    for i=1:1:tmx
        if i>2
            e(i)=d-Y(i-1); % релейный элемент
            if e(i)>=0
                e(i)=d;
            else
                e(i)=-d;
            end
            %D(i)=(Td/6)*(e(i)+2*e(i-1)-6*e(i-2)+2*e(i-3)+e(i-4));
            D(i)=Td*(e(i)-2*e(i-1)+e(i-2));
            P(i)=e(i)-e(i-1);
            I(i)=(1/Ti)*e(i-1);
            u(i)=u(i-1)+K*(P(i)+I(i)+D(i));
        else    % начальные условия
            I(i)=0;
            D(i)=0;
            Y(i)=0;
            ufz(i)=0;
            e(i)=0;
            u(i)=0;
        end
        uu(i)=u(i);
        if u(i)>1
            uu(i)=1;
        elseif u(i)<0
            uu(i)=0;
        end
        if i>tzn+1
            ufz(i)=(1-1/(1+Tfz))*ufz(i-1)+(1/(1+Tfz))*uu(i); %фазосдвигающий фильтр
            Xk(1)=ufz(i); % входное воздействие
        else
            ufz(i)=0;
        end
        [Xk]=ModelORn_0(Xk, An, tau, dt, n);
        YY=Xk(n+1)+fone*randn(size(i)); %имерительный тракт + шум
        if i>1
            Y(i)=(1-1/Tf)*Y(i-1)+(1/Tf)*YY; %входной фильтр
        else
            Y(i)=0;
        end
        if Y(i)-dy>Amax
            Amax=Y(i);
            T_0=i;
        end
        if Y(i)+dy<Amax
            if Y(i)+dy<Amin
                Amin=Y(i);
                T_1=i;
            end
            if (Y(i)-dy>Amin)&(Y(i)<d)
                j=j+1;
                if j==1
                    break
                else
                    Amin=d;
                    Amax=-dy;
                end
            end
        end
    end;
    t_sum=t_sum+i;
    A=(Amax-Amin)/2;
    T=(T_1-T_0)*2;
    b2_k=A/d
    b3_k=T/Ti
    if (abs(b2_k-b2)<0.5)&(abs(b3_k-b3)<0.5) %проверка выполнения косв. усл. оптимальности
        break
    else
        Ti=Ti*b3_k/b3 % вычисление параметров ѕ»ƒ для очередной итерации
        K=K*b2/b2_k
        Tfz=Ti*b1;
        Td=alfa*Ti
    end
end
%===========================================================
*/
#define K_GAIN_INIT			0.001f
#define TI_INIT_VAL			150.0f
#define TD_ALPHA_MUL		0.250f
#define IF_TAU				10.0f // пост. времени входного фильтра
#define DF_TAU				30.0f // пост. времени дифф. звена
#define B1_CONST			0.380f // косвенные условия оптимальности
#define B2_CONST			1.000f
#define B3_CONST			3.700f
#define STEP_TIME			100    // врем€ шага опроса (дискретизации), мс
#define FULL_TIME			300000 // суммарное врем€ самонастройки, мс
/*
b1=0.38;    % косвенные условия оптимальности
b2=1.0;
b3=3.7;
Tfz=Ti*b1;	% пост. времени фазосдвигающего фильтра
d=An/5;     % выход релейного элемента
iter=0;
t_sum=1;	%суммарное время самонастройки
dy=d/100;	%зона вычисления экстремумов
*/

static pid_r_instance *pS;
static float32_t d; // d=An/5 выход релейного элемента
static float32_t Ufz; // выход фазосдвигающего фильтра
static float32_t Inf; // выход входного фильтра
static tune_st State = TUNE_COMPLETE;
static stime_t Stime;
static stime_t Ftime;
static pf_ctl pContrl;
static float32_t *pInp;
static float32_t Amax;
static float32_t Amin;
static uint32_t T_1, T_0, i;

void pid_tune_new (pid_r_instance *s, float32_t *pi, pf_ctl contrl) {
	pS = s;
	pInp = pi;
	Inf = *pi;
	pContrl = contrl;
	d = s->d;
	Ufz = 0;
	pid_r_init(s);
	s->Kp = K_GAIN_INIT;
	s->Ti = TI_INIT_VAL;
	s->Td = TI_INIT_VAL * TD_ALPHA_MUL;
	s->Tf = DF_TAU;
	State = TUNE_PROCEED;
	Stime = timers_get_finish_time(STEP_TIME);
	Ftime = timers_get_finish_time(FULL_TIME);
	Amin = d;
	Amax = -d / 100;
	i = 0;
}

tune_st pid_tune_step (void) {
	if (State != TUNE_PROCEED) goto ret_state;
	if (timers_get_time_left(Stime) == 0) {
		State = TUNE_STOP_ERR;
		goto ret_state;
	}
	if (timers_get_time_left(Stime) == 0) {
		Stime = timers_get_finish_time(STEP_TIME);
		i++; // счетчик временных интервалов
		float32_t Tfz = pS->Ti * B1_CONST;
		float32_t inp = d - pS->u; // релейный элемент
        if (inp >= 0) inp = d;
        else inp = -d;
		float32_t u = pid_r(pS, inp);
        if (u > 1) u = 1;
        else if (u < 0) u = 0;
        Ufz = (1 - 1 / (1 + Tfz)) * Ufz // фазосдвигающий фильтр
        		+ (1 / (1 + Tfz)) * u;
        pContrl(Ufz); // управл€ющее воздействие
        inp = (1 - 1 / IF_TAU) * Inf // входной фильтр
        		+ (1 / IF_TAU) * *pInp;
        Inf = inp;
        float32_t dy = d / 100;	// зона вычисления экстремумов
        if (inp - dy > Amax) {
            Amax = inp;
            T_0 = i;
        }
        if (inp + dy < Amax) {
            if (inp + dy < Amin) {
                Amin = inp;
                T_1 = i;
            }
            if ((inp-dy > Amin) & (inp < d)) {
                t_sum=t_sum+i;
                float32_t A = (Amax - Amin) / 2;
                uint32_t T = (T_1 - T_0) * 2; //
                b2_k=A/d;
                b3_k=T/Ti;
                if ((fabs(b2_k-b2)<0.5)&(fabs(b3_k-b3)<0.5)) { //проверка выполнения косв. усл. оптимальности
                	State = TUNE_COMPLETE;
                } else {
                    Ti=Ti*b3_k/b3 // вычисление параметров ѕ»ƒ для очередной итерации
                    K=K*b2/b2_k
                    Tfz=Ti*b1;
                    Td=alfa*Ti
                }
            }
        }
	}
	ret_state:
	return State;
}
